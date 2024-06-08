#define STBI_ONLY_PNG
#define STB_IMAGE_IMPLEMENTATION
#include "vendor/stb_image.h"

unsigned char *image_load(const char *filename, int *w, int *h)
{
	int n;
	return stbi_load(filename, w, h, &n, 3);
}

void image_free(unsigned char *data)
{
	stbi_image_free(data);
}
