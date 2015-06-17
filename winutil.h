#pragma once

#include <windows.h>

HBITMAP CreateDIB(int width, int height, int bitsPerPixel, BITMAPINFO& bmi, void*& pBits);
