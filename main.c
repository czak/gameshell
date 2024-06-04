#include <assert.h>
#include <stdio.h>
#include <GLES2/gl2.h>

#include "window.h"
#include "font.h"

#include "vertex_shader.h"
#include "fragment_shader.h"

struct vertex {
	GLushort x, y;
	GLushort s, t;
};

static struct vertex vertices[] = {
	{ 0, 0, 0, 9 },
	{ 0, 36, 0, 18 },
	{ 20, 0, 5, 9 },
	{ 20, 36, 5, 18 },
};

static GLuint program;
static GLuint texture;

static int running = 1;

static void program_init()
{
	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
	glCompileShader(vertex_shader);

	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
	glCompileShader(fragment_shader);

	program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	glLinkProgram(program);

	int success;
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	assert(success);
}

static void texture_init()
{
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 80, 54, 0, GL_ALPHA, GL_UNSIGNED_BYTE, font);
}

static void text_write(const char *msg, int px, int py)
{
	while (*msg != '\0') {
		int index = *msg - ' ';

		int s = (index % 16) * 5;
		int t = (index / 16) * 9;

		vertices[0].s = s;     vertices[0].t = t;
		vertices[1].s = s;     vertices[1].t = t + 9;
		vertices[2].s = s + 5; vertices[2].t = t;
		vertices[3].s = s + 5; vertices[3].t = t + 9;

		glUniform2f(0, px, py);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		px += 24;

		msg++;
	}
}

static void on_draw()
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.2f);
	glClear(GL_COLOR_BUFFER_BIT);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glUseProgram(program);
	glBindTexture(GL_TEXTURE_2D, texture);

	text_write("Hello, world!", 10, 10);
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
	window_init(on_draw, on_key);

	program_init();
	texture_init();

	glVertexAttribPointer(0, 2, GL_UNSIGNED_SHORT, GL_FALSE, sizeof(struct vertex), (void *) vertices);
	glVertexAttribPointer(1, 2, GL_UNSIGNED_SHORT, GL_FALSE, sizeof(struct vertex), (void *) vertices + 2 * sizeof(GLushort));

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	while (running && window_dispatch() != -1) {
	}
}
