#include <stdio.h>

#include "window.h"
#include "gfx.h"

static struct {
	char *name;
	char *filename;
	int image;
} entries[] = {
	{ "Ghostrunner", "/home/czak/projects/gameshell/boxart/ghostrunner.png" },
	{ "GRID", "/home/czak/projects/gameshell/boxart/grid.png" },
	{ "Virtua Tennis 3", "/home/czak/projects/gameshell/boxart/virtuatennis3.png" },
	{ "Witcher 3", "/home/czak/projects/gameshell/boxart/witcher3.png" },
	{ "Wolfenstein: The New Colossus", "/home/czak/projects/gameshell/boxart/wolfenstein.png" },
};

static int running = 1;

static void on_draw()
{
	gfx_clear(0.0f, 0.0f, 0.0f, 0.5f);

	int px = 50;
	for (int i = 0; i < sizeof(entries) / sizeof(entries[0]); i++) {
		gfx_draw_image(entries[i].image, px, 50);
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

		default:
			fprintf(stderr, "key: %d\n", key);
	}
}

int main(int argc, char *argv[])
{
	window_init(on_draw, gfx_resize, on_key);
	gfx_init();

	for (int i = 0; i < sizeof(entries) / sizeof(entries[0]); i++) {
		entries[i].image = gfx_image_load(entries[i].filename);
	}

	while (running && window_dispatch() != -1) {
	}
}
