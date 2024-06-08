#include <assert.h>
#include <stdio.h>
#include <GLES2/gl2.h>

#include "window.h"
#include "font.h"

#include "vertex_shader.h"
#include "fragment_shader.h"

extern struct font font;

static struct vertex {
	GLushort x, y;
	GLushort s, t;
} vertices[4];

static GLuint program;
static GLuint texture;

static struct {
	GLint offset;
	GLint viewport;
	GLint color;
} uniforms;

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

	uniforms.offset = glGetUniformLocation(program, "u_Offset");
	uniforms.viewport = glGetUniformLocation(program, "u_Viewport");
	uniforms.color = glGetUniformLocation(program, "u_Color");
}

static void texture_init()
{
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 512, 512, 0, GL_ALPHA, GL_UNSIGNED_BYTE, font.texture);
}

static void text_write(const char *msg, int px, int py)
{
	while (*msg != '\0') {
		int index = *msg - ' ';

		struct glyph *g = &font.glyphs[index];

		vertices[0].x = g->pl;	vertices[0].y = g->pt;
		vertices[0].s = g->tl;	vertices[0].t = g->tt;

		vertices[1].x = g->pl;	vertices[1].y = g->pb;
		vertices[1].s = g->tl;	vertices[1].t = g->tb;

		vertices[2].x = g->pr;	vertices[2].y = g->pt;
		vertices[2].s = g->tr;	vertices[2].t = g->tt;

		vertices[3].x = g->pr;	vertices[3].y = g->pb;
		vertices[3].s = g->tr;	vertices[3].t = g->tb;

		glUniform2f(uniforms.offset, px, py);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		px += g->xadvance;

		msg++;
	}
}

static void on_draw()
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.6f);
	glClear(GL_COLOR_BUFFER_BIT);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glUseProgram(program);
	glBindTexture(GL_TEXTURE_2D, texture);

	glUniform3f(uniforms.color, 1.0f, 0.75f, 0.3f);
	text_write("Hello, world!", 10, 10);
}

static void on_resize(int width, int height)
{
	glViewport(0, 0, width, height);
	glUniform2f(uniforms.viewport, 2.0f / width, 2.0f / height);
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
	window_init(on_draw, on_resize, on_key);

	program_init();
	texture_init();

	glVertexAttribPointer(0, 2, GL_UNSIGNED_SHORT, GL_FALSE, sizeof(struct vertex), (void *) vertices);
	glVertexAttribPointer(1, 2, GL_UNSIGNED_SHORT, GL_FALSE, sizeof(struct vertex), (void *) vertices + 2 * sizeof(GLushort));

	glEnable(GL_BLEND);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	while (running && window_dispatch() != -1) {
	}
}
