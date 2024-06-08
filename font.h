struct font {
	struct glyph {
		short pl, pt, pr, pb;
		short tl, tt, tr, tb;
		short xadvance;
	} glyphs[95]; // ASCII 32-126
	unsigned char texture[512*512];
};
