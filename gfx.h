struct color {
	float r, g, b, a;
};

void gfx_init();
void gfx_clear(float red, float green, float blue, float alpha);
void gfx_resize(int device_width, int device_height, int viewport_width, int viewport_height);
void gfx_draw_text(const char *msg, int x, int y, float scale, struct color c);
void gfx_draw_rect(int x, int y, int width, int height, struct color c);
