#include "IP.h"
using namespace IP;

void histoMatchApprox(ImagePtr, ImagePtr, ImagePtr);

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_histoMatch:
//
// Apply histogram matching to I1. Output is in I2.
//
void
HW_histoMatch(ImagePtr I1, ImagePtr targetHisto, bool approxAlg, ImagePtr I2)
{
	if(approxAlg) {
		histoMatchApprox(I1, targetHisto, I2);
		return;
	}

	// copy image header (width, height) of input image I1 to output image I2
	IP_copyImageHeader(I1, I2);
	int i, p, R = 0;
	int left[MXGRAY], right[MXGRAY], limit[MXGRAY], index[MXGRAY];
	int total, Hsum = 0, Havg = 0, h1[MXGRAY];
	ChannelPtr<uchar> in, out;
	ChannelPtr<int> h2;
	double scale;
	int type;

	// init vars for width, height, and total number of pixels
	int w = I1->width();
	int h = I1->height();
	total = w * h;

	// Clear histogram
	for (i = 0; i < MXGRAY; ++i)
		h1[i] = 0;

	// eval histogram
	IP_getChannel(I1, 0, in, type);
	for (i = 0; i < total; i++)
		h1[in[i]]++;

	// target histogram
	IP_getChannel(targetHisto, 0, h2, type);

	// normalize h2 to conform with dimensions of I1
	for (i = Havg; i < MXGRAY; i++)
		Havg += h2[i];
	scale = (double)total / Havg;
	if (scale != 1) {
		for (i = 0; i < MXGRAY; i++)
			h2[i] *= scale;
	}

	// Evaluate remapping of all input gray levels
	for (i = 0; i < MXGRAY; i++) {
		left[i] = R; // left end of interval
		limit[i] = h2[R] - Hsum; // keeps track of the limits for each grey level
		Hsum += h1[i]; // cumulative value for interval
		while (Hsum > h2[R] && R < MXGRAY - 1) {
			Hsum -= h2[R]; // adjust Hsum as interval widens
			R++;
		}
		index[i] = left[i]; // stores starting position for each gray level
		right[i] = R; // init right end of interval
	}

	// clear h1 and reuse it
	for (i = 0; i < MXGRAY; i++)
		h1[i] = 0;

	// visit all input pixels
	for (int ch = 0; IP_getChannel(I1, ch, in, type); ch++) {    // get input  pointer for channel ch
		IP_getChannel(I2, ch, out, type);        // get output pointer for channel ch
		for (i = 0; i < total; i++) {
			p = left[*in];
			if (h1[p] < h2[p]) {
				// makes sure that we dont exceed the left and right limits
				if (left[*in] != right[*in] && left[*in] == index[*in]) {
					if (limit[*in] > 0) {
						*out++ = p;
						limit[*in]--;
					}
					else *out++ = p = left[*in] = MIN(p + 1, right[*in]);
				}
				else *out++ = p;
			}
			else *out++ = p = left[*in] = MIN(p + 1, right[*in]);
			h1[p]++;
			in++;
		}
	}
}

void
histoMatchApprox(ImagePtr I1, ImagePtr targetHisto, ImagePtr I2)
{
	// copy image header (width, height) of input image I1 to output image I2
	IP_copyImageHeader(I1, I2);
	int i, p, R = 0;
	int left[MXGRAY], right[MXGRAY];
	int total, Hsum = 0, Havg = 0, h1[MXGRAY];
	ChannelPtr<uchar> in, out;
	ChannelPtr<int> h2;
	double scale;
	int type;

	// init vars for width, height, and total number of pixels
	int w = I1->width();
	int h = I1->height();
	total = w * h;

	// clear histogram
	for (i = 0; i < MXGRAY; i++)
		h1[i] = 0;

	// eval histogram
	IP_getChannel(I1, 0, in, type);
	for (i = 0; i < total; i++)
		h1[in[i]]++;

	// target histogram
	IP_getChannel(targetHisto, 0, h2, type);

	// normalize h2 to conform with dimensions of I1
	for (i = Havg = 0; i < MXGRAY; i++) Havg += h2[i];
	scale = (double)total / Havg;
	if (scale != 1)
		for (i = 0; i < MXGRAY; i++)
			h2[i] *= scale;

	// Evaluate remapping of all input gray levels
	for (i = 0; i < MXGRAY; i++) {
		left[i] = R;
		Hsum += h1[i];
		while (Hsum > h2[R] && R < MXGRAY - 1) {
			Hsum -= h2[R];
			R++;
		}
		right[i] = R;
	}

	// clear h1 and reuse it
	for (i = 0; i < MXGRAY; i++)
		h1[i] = 0;

	// visit all input pixels
	for (int ch = 0; IP_getChannel(I1, ch, in, type); ch++) {
		IP_getChannel(I2, ch, out, type);

		for (i = 0; i < total; i++) {
			p = left[in[i]];
			if (h1[p] < h2[p])
				out[i] = p;
			else out[i] = p = left[in[i]] = MIN(p + 1, right[in[i]]);
			h1[p]++;
		}
	}
}