#pragma once

#include <stdio.h>

struct Image
{
	Image();
	virtual ~Image();
	
	void load(FILE* f);
	
	unsigned char* data;
	int width;
	int height;
	int ncomponents;
};

