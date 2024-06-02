#include <stdio.h>
#include <GLES2/gl2.h>

#include "window.h"

const int width = 256;
const int height = 256;

int main(int argc, char *argv[])
{
	window_init(width, height);

	fprintf(stderr, "%s\n", glGetString(GL_VERSION));

	while (window_dispatch() != -1) {
		glClearColor(0.3f, 0.0f, 0.0f, 0.3f);
		glClear(GL_COLOR_BUFFER_BIT);

		window_swap();
	}
}
