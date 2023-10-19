#include "IP.h"
using namespace IP;

// CopyRowToCircBuffer() methods
void floydCircBuffer(int row, short* buf[], ChannelPtr<uchar> start, int w);
void jarvisCircBuffer(int row, short* buf[], ChannelPtr<uchar> start, int w);

void floydMethod(short* in1, short* in2, int e);
void jarvisMethod(short* in1, short* in2, short* in3, int e);

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_errDiffusion:
//
// Apply error diffusion algorithm to image I1.
//
// This procedure produces a black-and-white dithered version of I1.
// Each pixel is visited and if it + any error that has been diffused to it
// is greater than the threshold, the output pixel is white, otherwise it is black.
// The difference between this new value of the pixel from what it used to be
// (somewhere in between black and white) is diffused to the surrounding pixel
// intensities using different weighting systems.
//
// Use Floyd-Steinberg     weights if method=0.
// Use Jarvis-Judice-Ninke weights if method=1.
//
// Use raster scan (left-to-right) if serpentine=0.
// Use serpentine order (alternating left-to-right and right-to-left) if serpentine=1.
// Serpentine scan prevents errors from always being diffused in the same direction.
//
// A circular buffer is used to pad the edges of the image.
// Since a pixel + its error can exceed the 255 limit of uchar, shorts are used.
//
// Apply gamma correction to I1 prior to error diffusion.
// Output is saved in I2.
//
void
HW_errDiffusion(ImagePtr I1, int method, bool serpentine, double gamma, ImagePtr I2)
{
	// apply gamma correction to I1
	ImagePtr I3;
	IP_gammaCorrect(I1, gamma, I3);

	// copy image header (width, height) of input image I3 to output image I2
	IP_copyImageHeader(I3, I2);
	int thr = MXGRAY / 2; // init threshold value

	// init vars for width, height, and total number of pixels
	int w = I3->width();
	int h = I3->height();
	int total = w * h;

	// declarations for image channel pointers and datatype
	ChannelPtr<uchar> in, out;
	int type;

	// Floyd Method
	if (method == 0) {
		short* in1, * in2, * buf[2] = {};
		buf[0] = new short[w + 2];
		buf[1] = new short[w + 2];

		for (int i = 0; i < w + 2; i++) {
			buf[0][i] = 0;
			buf[1][i] = 0;
		}

		// Raster scan
		if (!serpentine) {
			for (int ch = 0; IP_getChannel(I3, ch, in, type); ch++) {
				IP_getChannel(I2, ch, out, type);

				floydCircBuffer(0, buf, in, w); // copy row 0 to circular buffer
				for (int y = 0; y < h; y++) { // visit all input rows
					floydCircBuffer(y + 1, buf, in, w); // copy next row to circ buffer

					in1 = buf[y % 2] + 1; // circ buffer ptr for current row
					in2 = buf[(y + 1) % 2] + 1; // circ buffer ptr for 1 row below

					for (int x = 0; x < w; x++) {
						*out = (*in1 < thr) ? 0 : 255; // threshold

						int e = *in1 - *out; // eval error

						floydMethod(in1, in2, e);

						in1++; // advance circ buffer ptrs
						in2++; // advance circ buffer ptrs
						out++; // advance output ptr
					}
				}
			}
		}
		// Serpentine scan
		else {
			for (int ch = 0; IP_getChannel(I3, ch, in, type); ch++) {
				IP_getChannel(I2, ch, out, type);

				floydCircBuffer(0, buf, in, w); // copy row 0 to circular buffer
				for (int y = 0; y < h; y++) { // visit all input rows
					floydCircBuffer(y + 1, buf, in, w); // copy next row to circ buffer

					// left to right scan
					if (y % 2 == 0) {
						in1 = buf[y % 2] + 1; // circ buffer ptr
						in2 = buf[(y + 1) % 2] + 1; // circ buffer ptr

						for (int x = 0; x < w; x++) {
							*out = (*in1 < thr) ? 0 : 255; // threshold

							int e = *in1 - *out; // eval error

							floydMethod(in1, in2, e);

							in1++; // advance circ buffer ptrs
							in2++; // advance circ buffer ptrs
							out++; // advance output ptr
						}
					}
					// right to left scan
					else {
						in1 = buf[y % 2] + w; // circ buffer ptr at the end
						in2 = buf[(y + 1) % 2] + w; // circ buffer ptr at the end
						out += w - 1; // make output pointer go to end of row

						for (int x = 0; x < w; x++) {
							*out = (*in1 < thr) ? 0 : 255; // threshold

							int e = *in1 - *out; // eval error

							floydMethod(in1, in2, e);

							in1--; // advance circ buffer ptrs
							in2--; // advance circ buffer ptrs
							out--; // advance output ptr
						}
						out += w + 1; // point to next row
					}

				}
			}
		}
	}
	// Jarvis Method
	else {
		short* in1, * in2, * in3, * buf[3] = {};
		buf[0] = new short[w + 4];
		buf[1] = new short[w + 4];
		buf[2] = new short[w + 4];

		for (int i = 0; i < w + 2; i++) {
			buf[0][i] = 0;
			buf[1][i] = 0;
			buf[2][i] = 0;
		}

		// Raster scan
		if (!serpentine) {
			for (int ch = 0; IP_getChannel(I3, ch, in, type); ch++) {
				IP_getChannel(I2, ch, out, type);

				jarvisCircBuffer(0, buf, in, w); // copy row 0 to circular buffer
				jarvisCircBuffer(1, buf, in, w); // copy row 1 to circular buffer
				for (int y = 0; y < h; y++) { // visit all input rows
					jarvisCircBuffer(y + 2, buf, in, w); // copy next row to circ buffer

					in1 = buf[y % 3] + 2; // circ buffer ptr
					in2 = buf[(y + 1) % 3] + 2; // circ buffer ptr
					in3 = buf[(y + 2) % 3] + 2;

					for (int x = 0; x < w; x++) {
						*out = (*in1 < thr) ? 0 : 255; // threshold

						int e = *in1 - *out; // eval error

						jarvisMethod(in1, in2, in3, e);

						in1++; // advance circ buffer ptrs
						in2++; // advance circ buffer ptrs
						in3++; // advance circ buffer ptrs
						out++; // advance output ptr
					}
				}
			}
		}
		// Serpentine scan
		else {
			for (int ch = 0; IP_getChannel(I3, ch, in, type); ch++) {
				IP_getChannel(I2, ch, out, type);

				jarvisCircBuffer(0, buf, in, w); // copy row 0 to circular buffer
				jarvisCircBuffer(1, buf, in, w); // copy row 1 to circular buffer
				for (int y = 0; y < h; y++) { // visit all input rows
					jarvisCircBuffer(y + 2, buf, in, w); // copy next row to circ buffer

					in1 = buf[y % 3] + 2; // circ buffer ptr
					in2 = buf[(y + 1) % 3] + 2; // circ buffer ptr
					in3 = buf[(y + 2) % 3] + 2;

					// left to right scan
					if (y % 2 == 0) {
						for (int x = 0; x < w; x++) {
							*out = (*in1 < thr) ? 0 : 255; // threshold

							int e = *in1 - *out; // eval error

							jarvisMethod(in1, in2, in3, e);

							in1++; // advance circ buffer ptrs
							in2++; // advance circ buffer ptrs
							in3++; // advance circ buffer ptrs
							out++; // advance output 
						}
					}
					// right to left scan
					else {
						in1 = buf[y % 3] + w + 1; // circ buffer ptr
						in2 = buf[(y + 1) % 3] + w + 1; // circ buffer ptr
						in3 = buf[(y + 2) % 3] + w + 1;
						out += w - 1; // make output pointer go to end of row

						for (int x = 0; x < w; x++) {
							*out = (*in1 < thr) ? 0 : 255; // threshold

							int e = *in1 - *out; // eval error

							jarvisMethod(in1, in2, in3, e);

							in1--; // advance circ buffer ptrs
							in2--; // advance circ buffer ptrs
							in3--; // advance circ buffer ptrs
							out--; // advance output ptr
						}
						out += w + 1; // point to next row
					}

				}
			}
		}
	}
}

// 2 row circular buffer
void floydCircBuffer(int row, short* buf[], ChannelPtr<uchar> in, int w) {
	short* buffer = buf[row % 2];
	in = (in + w * row);
	for (int i = 1; i < w + 1; i++)
		buffer[i] = *in++;
}

// 3 row circular buffer
void jarvisCircBuffer(int row, short* buf[], ChannelPtr<uchar> in, int w) {
	short* buffer = buf[row % 3];
	in = (in + w * row);
	for (int i = 2; i < w + 2; i++)
		buffer[i] = *in++;
}

void floydMethod(short* in1, short* in2, int e) {
	in1[1] += (e * 7 / 16.); // add error to E
	in2[-1] += (e * 3 / 16.); // add error to SW
	in2[0] += (e * 5 / 16.); // add error to S
	in2[1] += (e * 1 / 16.); // add error to SE
}

void jarvisMethod(short* in1, short* in2, short* in3, int e) {
	in1[1] += (e * 7 / 48.); // add error to E
	in1[1] += (e * 5 / 48.); // add error to E2

	in2[-2] += (e * 3 / 48.); // add error to SW
	in2[-1] += (e * 5 / 48.); // add error to SW
	in2[0] += (e * 7 / 48.); // add error to S
	in2[1] += (e * 5 / 48.); // add error to SE
	in2[2] += (e * 3 / 48.); // add error to SE

	in3[-2] += (e * 1 / 48.); // add error to SSW
	in3[-1] += (e * 3 / 48.); // add error to SSW
	in3[0] += (e * 5 / 48.); // add error to SS
	in3[1] += (e * 3 / 48.); // add error to SSE
	in3[2] += (e * 1 / 48.); // add error to SSE
}
