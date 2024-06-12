#include <poll.h>

#include "window.h"
#include "gfx.h"
#include "entries.h"
#include "log.h"
#include "gamepad.h"

static int running = 1;
static int active = 0;

static void on_draw()
{
	gfx_clear(0.0f, 0.0f, 0.0f, 0.5f);

	int px = 50;
	for (int i = 0; i < entries_count; i++) {
		if (i == active) {
			gfx_draw_rect(px, 50, 1.0f, 0.75f, 0.3f);
		}

		gfx_draw_image(entries[i]->image, px, 50);
		px += 350;
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

		case BTN_DPAD_LEFT:
			if (active > 0) active--;
			window_redraw();
			break;

		case BTN_DPAD_RIGHT:
			if (active < entries_count - 1) active++;
			window_redraw();
			break;
	}

	window_redraw();
}

static void on_key(int key)
{
	switch (key) {
		case 1: // Escape
			running = 0;
			break;

		case 15: // Tab
			window_toggle();
			break;

		case 57: // Space
			window_redraw();
			break;

		case 105: // Left
			if (active > 0) active--;
			window_redraw();
			break;

		case 106: // Right
			if (active < entries_count - 1) active++;
			window_redraw();
			break;

		default:
			LOG("key: %d", key);
	}
}

int main(int argc, char *argv[])
{
	entries_load();

	window_init(on_draw, gfx_resize, on_key);
	gamepad_init(GAMEPAD_GRABBED, on_button);
	gfx_init();

	for (int i = 0; i < entries_count; i++) {
		entries[i]->image = gfx_image_load(entries[i]->filename);
	}

	enum {
		WINDOW,
		GAMEPAD_INOTIFY,
		GAMEPAD_EVENT,
	};

	while (running) {
		window_flush();

		struct pollfd pollfds[] = {
			[WINDOW] = { .fd = window_get_fd(), .events = POLLIN },
			[GAMEPAD_INOTIFY] = { .fd = gamepad_get_inotify(), .events = POLLIN },
			[GAMEPAD_EVENT] = { .fd = gamepad_get_fd(), .events = POLLIN },
		};

		poll(pollfds, sizeof(pollfds) / sizeof(pollfds[0]), -1);

		if (pollfds[WINDOW].revents) {
			window_dispatch();
		}

		if (pollfds[GAMEPAD_INOTIFY].revents || pollfds[GAMEPAD_EVENT].revents) {
			gamepad_dispatch();
		}
	}
}
