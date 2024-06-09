#include <assert.h>
#include <stddef.h>
#include <GLES2/gl2.h>

#define SHADER_ID_IMAGE 0
#define SHADER_ID_TEXT 1
#define SHADER_ID_SOLID 2

#include "font.h"
#include "image.h"
#include "vertex_shader.h"
#include "fragment_shader.h"

extern struct font font;

static GLuint program;
static GLuint texture_font;

static struct vertex {
	GLshort x, y;
	GLushort s, t;
} vertices[4];

static struct {
	GLint shader_id;
	GLint offset;
	GLint viewport;
	GLint color;
} uniforms;

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

void gfx_init()
{
	// Build shader program
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

	// Load font texture
	texture_font = texture_init(GL_ALPHA, 512, 512, font.texture);

	// Prepare to draw quads with texture coords
	glVertexAttribPointer(0, 2, GL_SHORT, GL_FALSE, sizeof(struct vertex), (void *) vertices);
	glVertexAttribPointer(1, 2, GL_UNSIGNED_SHORT, GL_FALSE, sizeof(struct vertex), (void *) vertices + 2 * sizeof(GLushort));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	// Alpha blending
	glEnable(GL_BLEND);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
}

void gfx_clear(float red, float green, float blue, float alpha)
{
	glClearColor(red, green, blue, alpha);
	glClear(GL_COLOR_BUFFER_BIT);
}

void gfx_resize(int width, int height)
{
	glViewport(0, 0, width, height);
	glUniform2f(uniforms.viewport, 2.0f / width, 2.0f / height);
}

unsigned int gfx_image_load(const char *filename)
{
	int w, h;
	unsigned char *pixels = image_load(filename, &w, &h);
	int texture = texture_init(GL_RGB, w, h, pixels);
	image_free(pixels);
	return texture;
}

void gfx_draw_text(const char *msg, int px, int py, float r, float g, float b)
{
	glBindTexture(GL_TEXTURE_2D, texture_font);
	glUniform1i(uniforms.shader_id, SHADER_ID_TEXT);
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

void gfx_draw_image(unsigned int texture, int px, int py)
{
	glBindTexture(GL_TEXTURE_2D, texture);
	glUniform1i(uniforms.shader_id, SHADER_ID_IMAGE);

	vertices[0] = (struct vertex){ 0, 0, 0, 0 };
	vertices[1] = (struct vertex){ 0, 450, 0, 512 };
	vertices[2] = (struct vertex){ 300, 0, 512, 0 };
	vertices[3] = (struct vertex){ 300, 450, 512, 512 };

	glUniform2f(uniforms.offset, px, py);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void gfx_draw_rect(int px, int py, float r, float g, float b)
{
	glUniform1i(uniforms.shader_id, SHADER_ID_SOLID);
	glUniform3f(uniforms.color, r, g, b);

	vertices[0] = (struct vertex){ -10, -10 };
	vertices[1] = (struct vertex){ -10, 460 };
	vertices[2] = (struct vertex){ 310, -10 };
	vertices[3] = (struct vertex){ 310, 460 };

	glUniform2f(uniforms.offset, px, py);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
