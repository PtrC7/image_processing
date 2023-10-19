#include <algorithm>
#include <vector>
#include "IP.h"
using namespace IP;
using std::vector;

void copyRowtoCircBuffer(ChannelPtr<uchar> p1, short* buffer, int w, int sz);
int getMedian(vector<int> v);

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

	short** buf = new short* [sz]; // array of sz pointers
	for (int i = 0; i < sz; i++) {
		buf[i] = new short[sz + w + 1];
	}

	// declarations for image channel pointers and datatype
	ChannelPtr<uchar> in, out;
	int type;

	for (int ch = 0; IP_getChannel(I1, ch, in, type); ch++) {
		IP_getChannel(I2, ch, out, type);

		// copy row to circular buffer
		for (int i = 0; i < sz / 2; i++)
			copyRowtoCircBuffer(in, buf[i], w, sz);

		// copy rest to circular buffer
		for (int i = sz / 2; i < sz; i++) {
			copyRowtoCircBuffer(in, buf[i], w, sz);
			in += w;
		}

		vector<int> filtered; // vector used to store values of neighbors to find median
		for (int y = 0; y < h; y++) { // rows
			for (int i = 0; i < sz; i++) { // store values in vector from x
				for (int j = 0; j < sz; j++) { // store values in vector from y
					filtered.push_back(buf[j][i]);
				}
			}

			// go through all the pixels in each row
			for (int x = 0; x < w; x++) {
				*out++ = getMedian(filtered); // stores median in output

				if (x < w - 1) {
					// add new column values
					for (int i = 0; i < sz; i++)
						filtered.push_back(buf[i][x + sz]);
					filtered.erase(filtered.begin(), filtered.begin() + sz); // delete old column values
				}
			}
			filtered.clear(); // clear the vector

			// padding the last rows
			copyRowtoCircBuffer(in, buf[(y + sz - 1) % sz], w, sz);

			if (in < in + total - w) in += w;

		}
	}
	// delete buffer
	for (int i = 0; i < sz; i++)
		delete[]buf[i];
}


void copyRowtoCircBuffer(ChannelPtr<uchar> in, short* buf, int w, int sz) {
	for (int i = 0; i < sz / 2; i++) buf[i] = *in;
	for (int i = sz / 2; i < sz / 2 + w - 1; i++) buf[i] = *in++;
	for (int i = sz / 2 + w - 1; i < sz + w - 1; i++) buf[i] = *in;
}

int getMedian(vector<int> v) {
	sort(v.begin(), v.end());
	return v[v.size() / 2];
}