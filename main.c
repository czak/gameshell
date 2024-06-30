#include <assert.h>
#include <poll.h>
#include <signal.h>

#include "window.h"
#include "gfx.h"
#include "commands.h"
#include "log.h"
#include "gamepad.h"
#include "menu.h"
#include "signals.h"

static int running = 1;

const struct color selected_color = {0.4f, 1.0f, 0.5f, 1.0f};
const struct color hover_color = {1.0f, 0.75f, 0.3f, 1.0f};
const struct color default_color = {1.0f, 1.0f, 1.0f, 1.0f};
const struct color dim_color = {1.0f, 1.0f, 1.0f, 0.25f};

struct menu commands_menu = {};
struct menu actions_menu = {};

struct menu *active_menu = &commands_menu;
struct command *active_command = NULL;

static void on_command(void *data)
{
	active_command = data;
	command_exec(active_command);

	menu_select(&commands_menu);
	active_menu = &actions_menu;

	window_redraw();
}

static void on_terminate()
{
	kill(active_command->pid, SIGCONT);
	kill(active_command->pid, SIGTERM);
}

static void on_stop()
{
	kill(active_command->pid, SIGSTOP);
}

static void on_continue()
{
	kill(active_command->pid, SIGCONT);
}

static void draw_menu(struct menu *menu, int px, int py)
{
	struct color c;
	
	for (int i = 0; i < menu->items_count; i++) {
		if (i == menu->selected) {
			c = selected_color;
		}
		else if (i == menu->hover) {
			c = hover_color;
		}
		else {
			if (menu->selected >= 0) {
				c = dim_color;
			} else {
				c = default_color;
			}
		}

		gfx_draw_text(menu->items[i].name, px, py, 64.0f, c);

		py += 75;
	}
}

static void on_draw()
{
	gfx_clear(0.0f, 0.0f, 0.0f, 0.5f);

	draw_menu(&commands_menu, 50, 100);

	if (commands_menu.selected >= 0) {
		draw_menu(&actions_menu, 500, 100);
	}
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

		case BTN_DPAD_UP:
			menu_hover_prev_item(active_menu);
			window_redraw();
			break;

		case BTN_DPAD_DOWN:
			menu_hover_next_item(active_menu);
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

		case 28: // Enter
			menu_trigger_item(active_menu);
			window_redraw();
			break;

		case 103: // Up
			menu_hover_prev_item(active_menu);
			window_redraw();
			break;

		case 108: // Down
			menu_hover_next_item(active_menu);
			window_redraw();
			break;

		default:
			LOG("key: %d", key);
	}
}

static void on_child(uint32_t child_pid, int32_t code)
{
	assert(child_pid == active_command->pid);

	switch (code) {
	case CLD_EXITED:
	case CLD_KILLED:
	case CLD_DUMPED:
		LOG("Child %d exited (%d)", child_pid, code);
		menu_deselect(&commands_menu);
		active_menu = &commands_menu;
		active_command = NULL;
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

	window_init(on_draw, gfx_resize, on_key);
	gamepad_init(GAMEPAD_GRABBED, on_button);
	signals_init(on_child);

	gfx_init();

	// Build commands menu from commands
	commands_menu.hover = 0;
	commands_menu.selected = -1;
	for (int i = 0; i < commands_count; i++) {
		commands_menu.items[i].name = commands[i]->name;
		commands_menu.items[i].action = on_command;
		commands_menu.items[i].data = commands[i];
	}
	commands_menu.items_count = commands_count;

	// Build actions menu
	actions_menu.hover = 0;
	actions_menu.selected = -1;
	actions_menu.items[0].name = "Terminate";
	actions_menu.items[0].action = on_terminate;
	actions_menu.items[1].name = "Stop";
	actions_menu.items[1].action = on_stop;
	actions_menu.items[2].name = "Continue";
	actions_menu.items[2].action = on_continue;
	actions_menu.items_count = 3;

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
