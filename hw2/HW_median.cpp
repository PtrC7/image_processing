#include <algorithm>
#include <vector>
#include "IP.h"
using namespace IP;
using std::vector;
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_median:
//
// Apply median filter of size sz x sz to I1.
// Clamp sz to 9.
// Output is in I2.
//
void
HW_median(ImagePtr I1, int sz, ImagePtr I2)
{
	// copy image header (width, height) of input image I1 to output image I2
	IP_copyImageHeader(I1, I2);

	// init vars for width, height, and total number of pixels
	int w = I1->width();
	int h = I1->height();
	int total = w * h;

	if (sz % 2 == 0) sz++;

	// declarations for image channel pointers and datatype
	ChannelPtr<uchar> in, out;
	int type;

}
