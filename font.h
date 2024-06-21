struct font {
	struct glyph {
		short x, y, width, height;
		short xoffset, yoffset;
		short xadvance;
	} glyphs[95]; // ASCII 32-126
	unsigned char texture[512*512*3];
};
