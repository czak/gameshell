#include <assert.h>
#include <stdio.h>
#include <GLES2/gl2.h>

#include "window.h"

static const char *vertex_shader_source =
"attribute vec2 a_Position;\n"
"attribute vec2 a_TexCoord;\n"
"uniform vec2 u_Offset;\n"
"varying vec2 v_TexCoord;\n"
"void main()\n"
"{\n"
"  float xs = 0.001041666666666666666666667; // 2 / 1920\n"
"  float ys = 0.001851851851851851851851852; // 2 / 1080\n"
"  gl_Position = vec4(xs * (a_Position.x + u_Offset.x) - 1.0, 1.0 - ys * (a_Position.y + u_Offset.y), 0.0, 1.0);\n"
"  v_TexCoord = a_TexCoord / 256.0;\n"
"}\n";

static const char *fragment_shader_source =
"precision mediump float;\n"
"uniform sampler2D u_Sampler;\n"
"varying vec2 v_TexCoord;\n"
"void main()\n"
"{\n"
"  gl_FragColor = texture2D(u_Sampler, v_TexCoord);\n"
"}\n";

static const GLushort vertices[] = {
	0, 0, 0, 0,
	0, 128, 0, 256,
	128, 0, 256, 0,

	128, 0, 256, 0,
	0, 128, 0, 256,
	128, 128, 256, 256,
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

	uint8_t data[256*256];

	for (int y = 0; y < 256; y++) {
		for (int x = 0; x < 256; x++) {
			data[256 * y + x] = x ^ y;
		}
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 256, 256, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, data);
}

static void on_draw()
{
	glClearColor(0.2f, 0.0f, 0.0f, 0.3f);
	glClear(GL_COLOR_BUFFER_BIT);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glUseProgram(program);
	glBindTexture(GL_TEXTURE_2D, texture);

	glUniform2f(0, 10.0f, 10.0f);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glUniform2f(0, 200.0f, 10.0f);
	glDrawArrays(GL_TRIANGLES, 0, 6);
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

	glVertexAttribPointer(0, 2, GL_UNSIGNED_SHORT, GL_FALSE, 4 * sizeof(GLushort), &vertices[0]);
	glVertexAttribPointer(1, 2, GL_UNSIGNED_SHORT, GL_FALSE, 4 * sizeof(GLushort), &vertices[2]);

	while (running && window_dispatch() != -1) {
	}
}
