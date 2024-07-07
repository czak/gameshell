#include <assert.h>
#include <stddef.h>
#include <GLES2/gl2.h>

#include "gfx.h"
#include "font.h"
#include "image.h"
#include "vertex_shader.h"
#include "fragment_shader.h"

extern struct font font;

static GLuint program;
static GLuint texture_font;

static struct vertex {
	GLfloat x, y;
	GLfloat s, t;
} vertices[4];

static struct {
	GLint position;
	GLint offset;
	GLint scale;
	GLint viewport;
	GLint viewport_ratio;
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

	uniforms.position = glGetUniformLocation(program, "u_Position");
	uniforms.offset = glGetUniformLocation(program, "u_Offset");
	uniforms.scale = glGetUniformLocation(program, "u_Scale");
	uniforms.viewport = glGetUniformLocation(program, "u_Viewport");
	uniforms.viewport_ratio = glGetUniformLocation(program, "u_ViewportRatio");

	uniforms.color = glGetUniformLocation(program, "u_Color");

	glUseProgram(program);

	// Load font texture
	texture_font = texture_init(GL_RGB, 512, 512, font.atlas);

	// Prepare to draw quads with texture coords
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(struct vertex), (void *) vertices);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(struct vertex), (void *) vertices + 2 * sizeof(GLfloat));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	// Alpha blending
	glEnable(GL_BLEND);
	glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
}

void gfx_clear(float red, float green, float blue, float alpha)
{
	glClearColor(red, green, blue, alpha);
	glClear(GL_COLOR_BUFFER_BIT);
}

void gfx_resize(int device_width, int device_height, int viewport_width, int viewport_height)
{
	glViewport(0, 0, device_width, device_height);
	glUniform2f(uniforms.viewport, viewport_width, viewport_height);
	glUniform1f(uniforms.viewport_ratio, (float) device_width / viewport_width);
}

void gfx_draw_text(const char *msg, int x, int y, float scale, struct color c)
{
	glBindTexture(GL_TEXTURE_2D, texture_font);
	glUniform4f(uniforms.color, c.r, c.g, c.b, c.a);
	glUniform2f(uniforms.offset, x, y);
	glUniform1f(uniforms.scale, scale);

	float px = 0, py = 0;
	while (*msg != '\0') {
		int index = *msg - ' ';

		struct glyph *g = &font.glyphs[index];

		vertices[0] = (struct vertex){ g->pl, g->pt, g->tl, g->tt };
		vertices[1] = (struct vertex){ g->pl, g->pb, g->tl, g->tb };
		vertices[2] = (struct vertex){ g->pr, g->pt, g->tr, g->tt };
		vertices[3] = (struct vertex){ g->pr, g->pb, g->tr, g->tb };

		glUniform2f(uniforms.position, px, py);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		px += g->advance;

		msg++;
	}
}
