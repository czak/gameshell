#include <assert.h>
#include <stdio.h>
#include <GLES2/gl2.h>

#include "window.h"
#include "font.h"

#include "vertex_shader.h"
#include "fragment_shader.h"

#define SHADER_ID_IMAGE 0
#define SHADER_ID_SOLID 1

extern struct font font;
extern unsigned char ___boxart_ghostrunner_rgb[];

static struct vertex {
	GLshort x, y;
	GLushort s, t;
} vertices[4];

static GLuint program;
static GLuint texture_font;
static GLuint texture_boxart;

static struct {
	GLint shader_id;
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

	uniforms.shader_id = glGetUniformLocation(program, "u_ShaderId");
	uniforms.offset = glGetUniformLocation(program, "u_Offset");
	uniforms.viewport = glGetUniformLocation(program, "u_Viewport");
	uniforms.color = glGetUniformLocation(program, "u_Color");

	glUseProgram(program);
}

static GLuint texture_init(GLint format, GLsizei width, GLsizei height, const void *pixels)
{
	GLuint texture;

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, pixels);

	return texture;
}

static void text_write(const char *msg, int px, int py, float r, float g, float b)
{
	glBindTexture(GL_TEXTURE_2D, texture_font);
	glUniform1i(uniforms.shader_id, SHADER_ID_SOLID);
	glUniform3f(uniforms.color, r, g, b);

	while (*msg != '\0') {
		int index = *msg - ' ';

		struct glyph *g = &font.glyphs[index];

		vertices[0] = (struct vertex){ g->pl, g->pt, g->tl, g->tt };
		vertices[1] = (struct vertex){ g->pl, g->pb, g->tl, g->tb };
		vertices[2] = (struct vertex){ g->pr, g->pt, g->tr, g->tt };
		vertices[3] = (struct vertex){ g->pr, g->pb, g->tr, g->tb };

		glUniform2f(uniforms.offset, px, py);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		px += g->xadvance;

		msg++;
	}
}

static void image_draw(int px, int py)
{
	glBindTexture(GL_TEXTURE_2D, texture_boxart);
	glUniform1i(uniforms.shader_id, SHADER_ID_IMAGE);

	vertices[0] = (struct vertex){ 0, 0, 0, 0 };
	vertices[1] = (struct vertex){ 0, 450, 0, 512 };
	vertices[2] = (struct vertex){ 300, 0, 512, 0 };
	vertices[3] = (struct vertex){ 300, 450, 512, 512 };

	glUniform2f(uniforms.offset, px, py);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

static void on_draw()
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
	glClear(GL_COLOR_BUFFER_BIT);

	text_write("Hello, world!", 10, 10, 1.0f, 0.75f, 0.3f);
	text_write("How are jou?", 10, 100, 1.0f, 1.0f, 1.0f);

	image_draw(600, 10);
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

	texture_font = texture_init(GL_ALPHA, 512, 512, font.texture);
	texture_boxart = texture_init(GL_RGB, 600, 900, ___boxart_ghostrunner_rgb);

	program_init();

	glVertexAttribPointer(0, 2, GL_SHORT, GL_FALSE, sizeof(struct vertex), (void *) vertices);
	glVertexAttribPointer(1, 2, GL_UNSIGNED_SHORT, GL_FALSE, sizeof(struct vertex), (void *) vertices + 2 * sizeof(GLushort));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glEnable(GL_BLEND);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	while (running && window_dispatch() != -1) {
	}
}
