#include "IP.h"
using namespace IP;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_blur:
//
// Blur image I1 with a box filter (unweighted averaging).
// The filter has width filterW and height filterH.
// We force the kernel dimensions to be odd.
// Output is in I2.
//

void blur(ChannelPtr<uchar>, int, int, int, ChannelPtr<uchar>);

void
HW_blur(ImagePtr I1, int filterW, int filterH, ImagePtr I2)
{
	// copy image header (width, height) of input image I1 to output image I2
	IP_copyImageHeader(I1, I2);

	ImagePtr I3; // intermediate buffer image
	IP_copyImageHeader(I1, I3);

	// init vars for width, height, and total number of pixels
	int w = I1->width();
	int h = I1->height();
	int total = w * h;

	// declarations for image channel pointers and datatype
	ChannelPtr<uchar> in, p2, p3, out;
	int type;

	// visit all image channels and evaluate output image
	for (int ch = 0; IP_getChannel(I1, ch, in, type); ch++) {	// get input  pointer for channel ch
		IP_getChannel(I2, ch, p2, type);
		IP_getChannel(I3, ch, p3, type);

		if (filterW == 1) {
			for (out = in + total; in < out;) {
				*p3++ = *in++;
			}
		}
		if (filterW > 1) {
			// blur all rows one at a time
			for (int y = 0; y < h; y++) {
				blur(in, w, 1, filterW, p3);
				in += w;
				p3 += w; 
			}
		}

		p3 = p3 - total; // set p3 to 0

		if (filterH == 1) {
			for (out = p3 + total; p3 < out;) {
				*p2++ = *p3++;
			}
		}
		if (filterH > 1) {
			// blur all columns one at a time
			for (int x = 0; x < w; x++) {
				blur(p3, h, w, filterH, p2);
				p3 += 1;
				p2 += 1;
			}
		}
	}
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// blur:
// 
// Blur row/column with 1D box filter with width fw
//

void blur(ChannelPtr<uchar> src, int width, int step, int fw, ChannelPtr<uchar> dst) {
	if (fw % 2 == 0) fw++; // force filter width to be odd
	int padding = fw / 2;  // init necessary padding on the borders 
	int bufferSize = width + fw - 1; // size of buffer including padding
	short* buffer = new short[bufferSize]; // init buffer to store padded row/column

	// copy row/column to buffer
	for (int i = 0; i < padding; i++) buffer[i] = *src; // fill left padded area with value from first pixel
	
	int ptr = 0;
	
	// copy pixels from src to buffer
	for (int i = padding; i < width + padding; i++) {
		buffer[i] = src[ptr];
		ptr += step;
	}
	
	
	for (int i = width + padding; i < bufferSize; i++) buffer[i] = src[ptr - step]; // fill right padded area with value from last pixel

	unsigned short sum = 0;
	
	for (int i = 0; i < fw; i++) sum += buffer[i]; // sum of pixels in window 
	
	// slide window across buffer, write average to dst 
	for (int i = 0; i < width; i++) {
		dst[i * step] = sum / fw; // average of pixels in window
		sum += (buffer[i + fw] - buffer[i]); // subtract left pixel and add right pixel to calulate new sum
	}

	delete []buffer;
}


