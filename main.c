#include <poll.h>
#include <signal.h>
#include <sys/wait.h>

#include "window.h"
#include "gfx.h"
#include "commands.h"
#include "log.h"
#include "gamepad.h"
#include "menu.h"
#include "signals.h"

static int running = 1;

static const int virtual_width = 1920;
static const int virtual_height = 1080;

static struct menu commands_menu = {};
static struct menu actions_menu = {};

static struct menu *active_menu = &commands_menu;
static struct command *active_command = NULL;

static void on_command(void *data)
{
	active_menu = &actions_menu;
	active_command = data;

	if (active_command->pid == 0) {
		command_exec(active_command);
	}

	window_redraw();
}

static void on_terminate()
{
	kill(active_command->pid, SIGCONT);
	kill(active_command->pid, SIGTERM);
}

static void on_stop()
{
	kill(-active_command->pid, SIGSTOP);
}

static void on_continue()
{
	kill(-active_command->pid, SIGCONT);
}

static void on_shell_action(void *data)
{
	if (fork() == 0) {
		execl("/bin/sh", "/bin/sh", "-c", data, NULL);
	}
}

static void draw_menu(struct menu *menu, int px, int py)
{
	static struct color colors[2][2] = {
		// Menu inactive
		{
			{1.0f, 1.0f, 1.0f, 0.25f}, // not selected
			{0.4f, 1.0f, 0.5f, 1.0f},  // selected
		},

		// Menu active
		{
			{1.0f, 1.0f, 1.0f, 1.0f},  // not selected
			{1.0f, 0.75f, 0.3f, 1.0f}, // selected
		},
	};

	for (int i = 0; i < menu->items_count; i++) {
		struct color c = colors[menu == active_menu][i == menu->selected_item];

		gfx_draw_text(menu->items[i].name, px + 4, py + 4, 4.0f, (struct color){0.0f, 0.0f, 0.0f, 0.6f});
		gfx_draw_text(menu->items[i].name, px, py, 4.0f, c);

		py += 75;
	}
}

static void on_draw()
{
	gfx_clear(0.0f, 0.0f, 0.0f, 0.5f);

	draw_menu(&commands_menu, 50, 50);

	if (active_menu == &actions_menu) {
		draw_menu(&actions_menu, 500, 50);
	}

	gfx_draw_text("Gamepad:", 50, 1000, 2.0f, (struct color){0.7f, 0.7f, 0.7f, 1.0f});
	gfx_draw_text(gamepad_get_name(), 190, 1000, 2.0f, (struct color){1.0f, 1.0f, 1.0f, 1.0f});
}

static void on_resize(int width, int height)
{
	gfx_resize(width, height, virtual_width, virtual_height);
}

static void on_gamepad()
{
	window_redraw();
}

static void on_button(int button)
{
	LOG("Button: %d", button);

	// Only MODE is handled regardless if visible/invisible
	if (button == BTN_MODE) {
		window_toggle();
		if (window_visible())
			gamepad_grab();
		else
			gamepad_ungrab();
		return;
	}

	// All other inputs need window to be visible
	if (!window_visible()) return;

	switch (button) {
		case BTN_SELECT:
			running = 0;
			break;

		case BTN_SOUTH:
			menu_trigger_item(active_menu);
			window_redraw();
			break;

		case BTN_EAST:
			active_menu = &commands_menu;
			window_redraw();
			break;

		case BTN_DPAD_UP:
			menu_select_prev_item(active_menu);
			window_redraw();
			break;

		case BTN_DPAD_DOWN:
			menu_select_next_item(active_menu);
			window_redraw();
			break;
	}
}

static void on_key(int key)
{
	switch (key) {
		case 1: // Escape
			running = 0;
			break;

		case 14: // Backspace
			active_menu = &commands_menu;
			window_redraw();
			break;

		case 28: // Enter
			menu_trigger_item(active_menu);
			window_redraw();
			break;

		case 103: // Up
			menu_select_prev_item(active_menu);
			window_redraw();
			break;

		case 108: // Down
			menu_select_next_item(active_menu);
			window_redraw();
			break;

		default:
			LOG("key: %d", key);
	}
}

static void on_child(uint32_t child_pid, int32_t code)
{
	struct command *child_command = command_find(child_pid);

	if (child_command == NULL) return;

	switch (code) {
	case CLD_EXITED:
	case CLD_KILLED:
	case CLD_DUMPED:
		LOG("Child %d exited (%d)", child_pid, code);
		waitpid(child_pid, NULL, 0);
		child_command->pid = 0;

		if (child_command == active_command) {
			active_menu = &commands_menu;
			active_command = NULL;
		}

		break;

	case CLD_STOPPED:
		LOG("Child %d stopped", child_pid);
		break;

	case CLD_CONTINUED:
		LOG("Child %d continued", child_pid);
		break;
	}

	window_redraw();
}

int main(int argc, char *argv[])
{
	commands_load();

	int use_keyboard = argc > 1;

	window_init(on_draw, on_resize, use_keyboard ? on_key : NULL);
	gamepad_init(on_gamepad, on_button);
	signals_init(on_child);

	gfx_init();

	// Build commands menu from commands
	for (int i = 0; i < commands_count; i++) {
		menu_append(&commands_menu, commands[i].name, on_command, &commands[i]);
	}

	// Build actions menu
	menu_append(&actions_menu, "Terminate", on_terminate, NULL);
	menu_append(&actions_menu, "Stop", on_stop, NULL);
	menu_append(&actions_menu, "Continue", on_continue, NULL);
	menu_append(&actions_menu, "Fullscreen", on_shell_action, "swaymsg fullscreen toggle");
	menu_append(&actions_menu, "Hide cursor", on_shell_action, "swaymsg seat seat0 cursor set 3840 2160");

	enum {
		WINDOW,
		GAMEPAD_INOTIFY,
		GAMEPAD_EVENT,
		SIGNALS,
	};

	while (running) {
		window_flush();

		struct pollfd pollfds[] = {
			[WINDOW] = { .fd = window_get_fd(), .events = POLLIN },
			[GAMEPAD_INOTIFY] = { .fd = gamepad_get_inotify(), .events = POLLIN },
			[GAMEPAD_EVENT] = { .fd = gamepad_get_fd(), .events = POLLIN },
			[SIGNALS] = { .fd = signals_get_fd(), .events = POLLIN },
		};

		poll(pollfds, sizeof(pollfds) / sizeof(pollfds[0]), -1);

		if (pollfds[WINDOW].revents) {
			window_dispatch();
		}

		if (pollfds[GAMEPAD_INOTIFY].revents || pollfds[GAMEPAD_EVENT].revents) {
			gamepad_dispatch();
		}

		if (pollfds[SIGNALS].revents) {
			signals_dispatch();
		}
	}
}
