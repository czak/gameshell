#include <assert.h>
#include <stddef.h>
#include <GLES2/gl2.h>

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

	uniforms.offset = glGetUniformLocation(program, "u_Offset");
	uniforms.viewport = glGetUniformLocation(program, "u_Viewport");
	uniforms.color = glGetUniformLocation(program, "u_Color");

	glUseProgram(program);

	// Load font texture
	texture_font = texture_init(GL_RGB, 512, 512, font.texture);

	// Prepare to draw quads with texture coords
	glVertexAttribPointer(0, 2, GL_SHORT, GL_FALSE, sizeof(struct vertex), (void *) vertices);
	glVertexAttribPointer(1, 2, GL_UNSIGNED_SHORT, GL_FALSE, sizeof(struct vertex), (void *) vertices + 2 * sizeof(GLshort));
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

void gfx_draw_text(const char *msg, int px, int py, float r, float g, float b)
{
	glBindTexture(GL_TEXTURE_2D, texture_font);
	glUniform3f(uniforms.color, r, g, b);

	while (*msg != '\0') {
		int index = *msg - ' ';

		struct glyph *g = &font.glyphs[index];

		vertices[0] = (struct vertex){
			g->xoffset,            g->yoffset,
			g->x,                  g->y
		};
		vertices[1] = (struct vertex){
			g->xoffset,            g->yoffset + g->height,
			g->x,                  g->y + g->height
		};
		vertices[2] = (struct vertex){
			g->xoffset + g->width, g->yoffset,
			g->x + g->width,       g->y
		};
		vertices[3] = (struct vertex){
			g->xoffset + g->width, g->yoffset + g->height,
			g->x + g->width,       g->y + g->height
		};

		glUniform2f(uniforms.offset, px, py);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		px += g->xadvance;

		msg++;
	}
}
