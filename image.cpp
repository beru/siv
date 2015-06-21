
#include "image.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Image::Image()
	:
	data(nullptr),
	width(0),
	height(0),
	ncomponents(0)
{
	
}

Image::~Image()
{
	if (data != nullptr) {
		stbi_image_free(data);
	}
}

void Image::load(FILE* f)
{
	data = stbi_load_from_file(f, &width, &height, &ncomponents, 0);
}

