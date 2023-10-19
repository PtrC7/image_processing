#include "IP.h"
using namespace IP;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_convolve:
//
// Convolve I1 with filter kernel in Ikernel.
// Output is in I2.
//
void
HW_convolve(ImagePtr I1, ImagePtr Ikernel, ImagePtr I2)
{

	// copy image header (width, height) of input image I1 to output image I2
	IP_copyImageHeader(I1, I2);

	// init vars for width, height, and total number of pixels
	int w = I1->width();
	int h = I2->height();
	int total = w * h;

	// declarations for image channel pointers and datatype
	ChannelPtr<uchar> in, out, start;
	ChannelPtr<float> kernel;
	int type;

    // declare buffer
    int kw = Ikernel->width();
    int kh = Ikernel->height();
    int pw = kw / 2;
    int ph = kh / 2;
    int maxH = ph + h + ph;
    int maxW = pw + w + pw;

    unsigned char** buff = new unsigned char* [maxH];
    for (int i = 0; i < maxH; i++) {
        buff[i] = new unsigned char[maxW];
    }


    // initialize buffer
    for (int ch = 0; IP_getChannel(I1, ch, in, type); ch++) {    // get input  pointer for channel ch
        IP_getChannel(I2, ch, out, type);        // get output pointer for channel ch
        start = in; // making start point at the beginning of image (0,0) to use as reference
        for (int i = 0; i < maxH; i++) {
            for (int j = 0; j < maxW; j++) {
                if (i < ph || i >= ph + h || j < pw || j >= pw + w) {
                    buff[i][j] = 0;
                }
                else {
                    buff[i][j] = *start++;
                }
            }
        }

        ChannelPtr<float> kp;
        for (int i = ph; i < h + ph; i++) {
            for (int j = pw; j < w + pw; j++) {
                double sum = 0;

                IP_getChannel(Ikernel, 0, kp, type);
                //All the rows of the kernel
                for (int kr = 0; kr < kh; kr++) {
                    //Each column of the kernel
                    for (int kc = 0; kc < kw; kc++) {
                        sum += buff[i - ph + kr][j - pw + kc] * (*kp++);
                    }
                }
                *out++ = CLIP(sum, 0, 255);
            }
        }
    }

}
