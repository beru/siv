#include "winutil.h"

#include <assert.h>
#include <windows.h>

HBITMAP CreateDIB(int width, int height, int bitsPerPixel, BITMAPINFO& bmi, void*& pBits)
{
	assert(bitsPerPixel % 8 == 0);
	int bytesPerPixel = bitsPerPixel / 8;
	assert(bytesPerPixel <= 4);
	if (width % 4) {
		width += 4 - (width % 4);
	}
	assert(width % 4 == 0);
	BITMAPINFOHEADER& header = bmi.bmiHeader;
	header.biSize = sizeof(BITMAPINFOHEADER);
	header.biWidth = width;
	header.biHeight = height;
	header.biPlanes = 1;
	header.biBitCount = bitsPerPixel;
	header.biCompression = BI_RGB;
	header.biSizeImage = width * abs(height) * bytesPerPixel;
	header.biXPelsPerMeter = 0;
	header.biYPelsPerMeter = 0;
	header.biClrUsed = 0;
	header.biClrImportant = 0;
	
	return ::CreateDIBSection(
		(HDC)0,
		&bmi,
		DIB_RGB_COLORS,
		&pBits,
		NULL,
		0
	);
}


