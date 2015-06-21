#pragma once

#include <stdio.h>
#include "noncopyable.h"

struct Image : private noncopyable
{
	Image();
	virtual ~Image();
	
	void load(FILE* f);
	
	unsigned char* data;
	int width;
	int height;
	int ncomponents;
};

