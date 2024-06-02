#include <assert.h>
#include <stdio.h>
#include <GLES2/gl2.h>

#include "window.h"

static const char *vertex_shader_source =
"attribute vec2 vPosition;\n"
"void main()\n"
"{\n"
"  gl_Position = vec4(2.0f * vPosition.x / 1920.0f - 1.0f, 1.0f - 2.0f * vPosition.y / 1080.0f, 0, 1);\n"
"}\n";

static const char *fragment_shader_source =
"void main()\n"
"{\n"
"  gl_FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
"}\n";

static GLuint create_shader_program()
{
	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
	glCompileShader(vertex_shader);

	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
	glCompileShader(fragment_shader);

	GLuint program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	glLinkProgram(program);

	int success;
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	assert(success);

	return program;
}

int main(int argc, char *argv[])
{
	window_init();

	GLuint program = create_shader_program();

	GLushort vertices[] = {
		10, 10,
		10, 540,
		1800, 10,
	};

	glVertexAttribPointer(0, 2, GL_UNSIGNED_SHORT, GL_FALSE, 0, vertices);

	while (window_dispatch() != -1) {
		glClearColor(0.3f, 0.0f, 0.0f, 0.3f);
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(program);
		glEnableVertexAttribArray(0);

		glDrawArrays(GL_TRIANGLES, 0, 3);

		window_swap();
	}
}
