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

struct action {
	char *name;
	void (*callback)(void *data);
};

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
		struct menu_item menu_item = menu_get_menu_item(menu, i);
		struct color c = colors[menu == active_menu][i == menu->selected_item];

		gfx_draw_text(menu_item.name, px, py, 64.0f, c);
		if (menu_item.subtitle) {
			gfx_draw_text(menu_item.subtitle, px, py + 15, 20.0f, c);
		}

		py += 75;
	}
}

static void on_draw()
{
	gfx_clear(0.0f, 0.0f, 0.0f, 0.5f);

	draw_menu(&commands_menu, 50, 100);

	if (active_menu == &actions_menu) {
		draw_menu(&actions_menu, 500, 100);
	}

	gfx_draw_text("Gamepad:", 50, 1000, 24.0f, (struct color){0.7f, 0.7f, 0.7f, 1.0f});
	gfx_draw_text(gamepad_get_name(), 170, 1000, 24.0f, (struct color){1.0f, 1.0f, 1.0f, 1.0f});
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
	LOG("DEBUG: Visible? %d", window_visible());

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
			menu_trigger(active_menu);
			window_redraw();
			break;

		case BTN_EAST:
			active_menu = &commands_menu;
			window_redraw();
			break;

		case BTN_DPAD_UP:
			menu_select_prev(active_menu);
			window_redraw();
			break;

		case BTN_DPAD_DOWN:
			menu_select_next(active_menu);
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
			menu_trigger(active_menu);
			window_redraw();
			break;

		case 103: // Up
			menu_select_prev(active_menu);
			window_redraw();
			break;

		case 108: // Down
			menu_select_next(active_menu);
			window_redraw();
			break;

		default:
			LOG("key: %d", key);
	}
}

static void on_child(uint32_t pid, int32_t code)
{
	struct command *command = command_find(pid);

	if (command == NULL) return;

	switch (code) {
	case CLD_EXITED:
	case CLD_KILLED:
	case CLD_DUMPED:
		LOG("Child %d exited (%d)", pid, code);
		waitpid(pid, NULL, 0);
		command->pid = 0;
		command->stopped = 0;

		if (command == active_command) {
			active_menu = &commands_menu;
			active_command = NULL;
		}

		break;

	case CLD_STOPPED:
		LOG("Child %d stopped", pid);
		command->stopped = 1;
		break;

	case CLD_CONTINUED:
		LOG("Child %d continued", pid);
		command->stopped = 0;
		break;
	}

	window_redraw();
}

static struct menu_item actions_menu_item(void *item)
{
	struct action *action = item;

	return (struct menu_item){
		.name = action->name,
		.callback = action->callback,
		.item = item,
	};
}

static struct menu_item commands_menu_item(void *item)
{
	struct command *command = item;

	char *subtitle = NULL;
	if (command->pid) {
		subtitle = command->stopped ? "stopped": "running";
	}

	return (struct menu_item){
		.name = command->name,
		.subtitle = subtitle,
		.callback = on_command,
		.item = item,
	};
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
	commands_menu.resolver = commands_menu_item;
	for (int i = 0; i < commands_count; i++) {
		menu_append(&commands_menu, &commands[i]);
	}

	// Build actions menu
	actions_menu.resolver = actions_menu_item;
	menu_append(&actions_menu, &(struct action){"Terminate", on_terminate});
	menu_append(&actions_menu, &(struct action){"Stop", on_stop});
	menu_append(&actions_menu, &(struct action){"Continue", on_continue});

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
