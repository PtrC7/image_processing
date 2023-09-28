#include "IP.h"
using namespace IP;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_quantize:
//
// Quantize I1 to specified number of levels. Apply dither if flag is set.
// Output is in I2.
//
void
HW_quantize(ImagePtr I1, int levels, bool dither, ImagePtr I2)
{
    // copy image header (width, height) of input image I1 to output image I2
    IP_copyImageHeader(I1, I2);

    // init vars for width, height, and total number of pixels
    int w = I1->width();
    int h = I1->height();
    int total = w * h;

    // init lookup table
    int i, lut[MXGRAY];
    double scale = MXGRAY / levels;
    int bias = 128 / levels;
    for (i = 0; i < MXGRAY; i++)
        lut[i] = scale * (int) (i / scale);

    // declarations for image channel pointers and datatype
    ChannelPtr<uchar> p1, p2;
    int type;

    for (int ch = 0; IP_getChannel(I1, ch, p1, type); ch++) { // gets input pointer for channel ch
        IP_getChannel(I2, ch, p2, type);
        for (i = 0; i < total; i++) {
            if (dither) {
                double randn = (double)rand() / RAND_MAX; // normalized random number [0,1]
                int jitter = bias * (1 - (2 * randn)); // scaled randomized number by bias [-bias, bias]
                int out = jitter + *p1++; // added jitter to output
                // checks for clipping
                if (out > 255)
                    out = 255;
                if (out < 0)
                    out = 0;
                *p2++ = lut[out];
            }
            else
                *p2++ = lut[*p1++];
        }
    }
}
