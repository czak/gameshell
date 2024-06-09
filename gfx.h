void gfx_init();
void gfx_clear(float red, float green, float blue, float alpha);
void gfx_resize(int width, int height);
unsigned int gfx_image_load(const char *filename);
void gfx_draw_text(const char *msg, int px, int py, float r, float g, float b);
void gfx_draw_image(unsigned int texture, int px, int py);
void gfx_draw_rect(int px, int py, float r, float g, float b);
