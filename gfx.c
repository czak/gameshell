#include <assert.h>
#include <stddef.h>
#include <GLES2/gl2.h>

#include "gfx.h"
#include "font.h"
#include "image.h"

#include "text_vert_shader.h"
#include "text_frag_shader.h"
#include "rect_vert_shader.h"
#include "rect_frag_shader.h"

extern struct font font;
static GLuint font_texture;

struct program {
	GLuint id;
	struct {
		GLint offset;
		GLint scale;
		GLint viewport;
		GLint color;
		GLint screenpxrange;
	} uniforms;
};

static struct program text_program;
static struct program rect_program;

static struct vertex {
	GLfloat x, y;
	GLfloat s, t;
} vertices[4];

static struct {
	int device_width, device_height;
	int viewport_width, viewport_height;
} dimensions;

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

static void program_init(struct program *program, const char *vs_source, const char *fs_source)
{
	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vs_source, NULL);
	glCompileShader(vs);

	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &fs_source, NULL);
	glCompileShader(fs);

	GLuint id = glCreateProgram();
	glAttachShader(id, vs);
	glAttachShader(id, fs);
	glLinkProgram(id);

	int success;
	glGetProgramiv(id, GL_LINK_STATUS, &success);
	assert(success);

	glDeleteShader(vs);
	glDeleteShader(fs);

	program->id = id;
	program->uniforms.offset = glGetUniformLocation(id, "u_Offset");
	program->uniforms.scale = glGetUniformLocation(id, "u_Scale");
	program->uniforms.viewport = glGetUniformLocation(id, "u_Viewport");
	program->uniforms.screenpxrange = glGetUniformLocation(id, "u_ScreenPxRange");
	program->uniforms.color = glGetUniformLocation(id, "u_Color");
}

void gfx_init()
{
	program_init(&text_program, text_vert_shader_source, text_frag_shader_source);
	program_init(&rect_program, rect_vert_shader_source, rect_frag_shader_source);

	// Load font texture
	font_texture = texture_init(GL_RGB, 512, 512, font.atlas);

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

	dimensions.device_width = device_width;
	dimensions.device_height = device_height;
	dimensions.viewport_width = viewport_width;
	dimensions.viewport_height = viewport_height;
}

void gfx_draw_text(const char *msg, int x, int y, float scale, struct color c)
{
	glUseProgram(text_program.id);

	glBindTexture(GL_TEXTURE_2D, font_texture);
	glUniform2f(text_program.uniforms.viewport, dimensions.viewport_width, dimensions.viewport_height);
	glUniform2f(text_program.uniforms.offset, x, y);
	glUniform1f(text_program.uniforms.scale, scale);
	glUniform4f(text_program.uniforms.color, c.r, c.g, c.b, c.a);

	// see https://github.com/Chlumsky/msdfgen/blob/master/README.md#using-a-multi-channel-distance-field
	float viewport_scale = (float) dimensions.device_width / dimensions.viewport_width;
	float screenpxrange = scale * viewport_scale * font.pxrange / font.size;
	glUniform1f(text_program.uniforms.screenpxrange, screenpxrange >= 1 ? screenpxrange : 1);

	float px = 0;
	while (*msg != '\0') {
		int index = *msg - ' ';

		struct glyph *g = &font.glyphs[index];

		vertices[0] = (struct vertex){ px + g->pl, g->pt, g->tl, g->tt };
		vertices[1] = (struct vertex){ px + g->pl, g->pb, g->tl, g->tb };
		vertices[2] = (struct vertex){ px + g->pr, g->pt, g->tr, g->tt };
		vertices[3] = (struct vertex){ px + g->pr, g->pb, g->tr, g->tb };

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		px += g->advance;

		msg++;
	}
}

void gfx_draw_rect(int x, int y, int width, int height, struct color c)
{
	glUseProgram(rect_program.id);

	glUniform2f(rect_program.uniforms.viewport, dimensions.viewport_width, dimensions.viewport_height);
	glUniform4f(rect_program.uniforms.color, c.r, c.g, c.b, c.a);

	vertices[0] = (struct vertex){x, y};
	vertices[1] = (struct vertex){x, y + height};
	vertices[2] = (struct vertex){x + width, y};
	vertices[3] = (struct vertex){x + width, y + height};

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
