#include "IP.h"
using namespace IP;
void HW_blur(ImagePtr, int, int, ImagePtr);

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_sharpen:
//
// Sharpen image I1. Output is in I2.
//
void
HW_sharpen(ImagePtr I1, int size, double factor, ImagePtr I2)
{

	// copy image header (width, height) of input image I1 to output image I2
	IP_copyImageHeader(I1, I2);

	// init vars for width, height, and total number of pixels
	int w = I1->width();
	int h = I2->height();
	int total = w * h;

	// declarations for image channel pointers and datatype
	ChannelPtr<uchar> p1, p2, out;
	int type;

	HW_blur(I1, size, size, I2); // get blurred image by invoking blur function 

	for (int ch = 0; IP_getChannel(I1, ch, p1, type); ch++) {
		IP_getChannel(I2, ch, p2, type);
		for (out = p1 + total; p1 < out;) {
			*p2 = MAX(MIN(*p1 + (*p1 - *p2) * factor, 255), 0); // difference between I1 and blurred image I2 is multiplied with factor and added to I1 and then clipped to range [0, 255]
			*p1++;
			*p2++;
		}
	}

}
