#include <stdio.h>

#include "window.h"
#include "gfx.h"

static unsigned int boxarts[2];

static int running = 1;

static void on_draw()
{
	gfx_clear(0.0f, 0.0f, 0.0f, 0.5f);

	gfx_draw_text("Hello, world!", 10, 10, 1.0f, 0.75f, 0.3f);
	gfx_draw_text("How are jou?", 10, 100, 1.0f, 1.0f, 1.0f);

	gfx_draw_image(boxarts[0], 600, 10);
	gfx_draw_image(boxarts[1], 950, 10);
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

	boxarts[0] = gfx_image_load("boxart/witcher3.png");
	boxarts[1] = gfx_image_load("boxart/ghostrunner.png");

	while (running && window_dispatch() != -1) {
	}
}
