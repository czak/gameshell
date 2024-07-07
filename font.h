struct font {
	float size;
	int pxrange;
	int pxpadding;
	int outerpxpadding;
	int atlas_width;
	int atlas_height;
	struct glyph {
		float pl, pb, pr, pt;
		float tl, tb, tr, tt;
		float advance;
	} glyphs[95]; // ASCII 32-126
	unsigned char atlas[512*512*3];
};
