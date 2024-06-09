#include <stdio.h>

#include "window.h"
#include "gfx.h"
#include "entries.h"

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
			fprintf(stderr, "key: %d\n", key);
	}
}

int main(int argc, char *argv[])
{
	entries_load();

	window_init(on_draw, gfx_resize, on_key);
	gfx_init();

	for (int i = 0; i < entries_count; i++) {
		entries[i]->image = gfx_image_load(entries[i]->filename);
	}

	while (running && window_dispatch() != -1) {
	}
}
