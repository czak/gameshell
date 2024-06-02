#include <assert.h>
#include <stdio.h>
#include <GLES2/gl2.h>

#include "window.h"

static const char *vertex_shader_source =
"attribute vec2 vPosition;\n"
"void main()\n"
"{\n"
"  float xs = 0.001041666666666666666666667; // 2 / 1920\n"
"  float ys = 0.001851851851851851851851852; // 2 / 1080\n"
"  gl_Position = vec4(xs * vPosition.x - 1.0, 1.0 - ys * vPosition.y, 0.0, 1.0);\n"
"}\n";

static const char *fragment_shader_source =
"precision mediump float;\n"
"uniform vec4 uColor;\n"
"void main()\n"
"{\n"
"  gl_FragColor = uColor;\n"
"}\n";

static GLuint program;
static GLushort vertices[] = {
	10, 10,
	10, 180,
	120, 10,
};

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

static void on_draw()
{
	static int frame = 0;

	glClearColor(0.2f, 0.0f, 0.0f, 0.3f);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(program);
	glEnableVertexAttribArray(0);

	float r = (float) ((frame * 47) % 256) / 256.0f;
	float g = (float) ((frame * 63) % 256) / 256.0f;
	float b = (float) ((frame * 99) % 256) / 256.0f;

	glUniform4f(0, r, g, b, 0.5f);

	glDrawArrays(GL_TRIANGLES, 0, 3);

	frame++;
}

static void on_key(int key)
{
	if (key == 1) {
		running = 0;
	} else if (key == 57) {
		window_redraw();
	}
}

int main(int argc, char *argv[])
{
	window_init(on_draw, on_key);

	program_init();

	glVertexAttribPointer(0, 2, GL_UNSIGNED_SHORT, GL_FALSE, 0, vertices);

	while (running && window_dispatch() != -1) {
	}
}
