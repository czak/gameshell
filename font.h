struct font {
	struct glyph {
		float pl, pb, pr, pt;
		float tl, tb, tr, tt;
		float advance;
	} glyphs[95]; // ASCII 32-126
	unsigned char texture[512*512*3];
};
