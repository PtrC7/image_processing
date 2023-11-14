#include "IP.h"
#include <stdio.h>
#include <algorithm>
using namespace IP;
using namespace std;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_spectrum:
//
// Convolve magnitude and phase spectrum from image I1.
// Output is in Imag and Iphase.
//

typedef struct {
    int len;	// length of complex number list
    float* real;	// pointer to real number list
    float* imag;	// pointer to imaginary number list
}
complexS,* complexP;

extern void HW_fft2MagPhase(ImagePtr Ifft, ImagePtr Imag, ImagePtr Iphase);

void padImage(ImagePtr I1, ImagePtr I1Padded);
void fft1D(complexP q1, int dir, complexP q2);
void fft1DRow(ImagePtr I1, ImagePtr Image1);
void fft1DCol(ImagePtr I1, ImagePtr Image1, ImagePtr Image2);
float getMin(ImagePtr I1, int total);
float getMax(ImagePtr I1, int total);

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_spectrum:
//
// Compute magnitude and phase spectrum from input image I1.
// Save results in Imag and Iphase.
//
void
HW_spectrum(ImagePtr I1, ImagePtr Imag, ImagePtr Iphase)
{
	int w = I1->width();
	int h = I1->height();
    int total = w * h;
	// compute FFT of the input image
	
// PUT YOUR CODE HERE...

    ImagePtr I2;
    // Check for padding
    if (ceil(log2(w)) != floor(log2(w)) || ceil(log2(h)) != floor(log2(h))) { 
        padImage(I1, I2);
        // updates width and height with new padded image
        w = I2->width();
        h = I2->height();
        total = w * h;
    }
    else I2 = I1;

    ImagePtr Image1, Image2;
    Image1->allocImage(w, h, FFT_TYPE);
    Image2->allocImage(w, h, FFT_TYPE);
    fft1DRow(I2, Image1);
    fft1DCol(I2, Image1, Image2);

	// compute magnitute and phase spectrums from FFT image
	ImagePtr Im = NEWIMAGE;
	ImagePtr Ip = NEWIMAGE;
// PUT YOUR CODE HERE...
    HW_fft2MagPhase(Image2, Im, Ip);

	// find min and max of magnitude and phase spectrums

// PUT YOUR CODE HERE...
    float minMag, maxMag, minPhase, maxPhase;

    minMag = getMin(Im, total);
    maxMag = getMax(Im, total);
    minPhase = getMin(Ip, total);
    maxPhase = getMax(Ip, total);

	// allocate uchar image for displaying magnitude and phase
	Imag  ->allocImage(w, h, BW_TYPE);
	Iphase->allocImage(w, h, BW_TYPE);

	// set imagetypes for single 8-bit channel
	Imag  ->setImageType(BW_IMAGE);
	Iphase->setImageType(BW_IMAGE);

	// get channel pointers for input magnitude and phase spectrums
	ChannelPtr<float>   magF = Im[0];
	ChannelPtr<float> phaseF = Ip[0];

	// get channel pointers for output magnitude and phase spectrums
	ChannelPtr<uchar> mag    = Imag  [0];
	ChannelPtr<uchar> phase  = Iphase[0];

	// scale magnitude and phase to fit between [0, 255]

// PUT YOUR CODE HERE...
    IP_copyImageHeader(I2, Imag);
    IP_copyImageHeader(I2, Iphase);
    maxMag /= 255;
    int type;

    for (int ch = 0; IP_getChannel(Imag, ch, mag, type); ch++) {
        for (int i = 0; i < total; i++) {
            *mag++ = CLIP((magF[i] - minMag) / (maxMag - minMag) * MaxGray, 0, MaxGray);
        }
    }

    for (int ch = 0; IP_getChannel(Iphase, ch, phase, type); ch++) {
        for (int i = 0; i < total; i++) {
            *phase++ = CLIP((phaseF[i] - minPhase) / (maxPhase - minPhase) * MaxGray, 0, MaxGray);
        }
    }
}

void padImage(ImagePtr I1, ImagePtr paddedImage) {

    int w = I1->width();
    int h = I1->height();

    // gets the new hieght and width
    int paddedH = pow(2, floor(log2(h)) + 1);
    int paddedW = pow(2, floor(log2(w)) + 1);
    paddedImage->allocImage(paddedW, paddedH, BW_TYPE);
    ChannelPtr<uchar> in, out;

    int type;
    
    for (int ch = 0; IP_getChannel(I1, ch, in, type); ch++) {
        IP_getChannel(paddedImage, ch, out, type);
        for (int i = 0; i < paddedH; i++) {
            for (int j = 0; j < paddedW; j++) {
                // makes sure the original image is same
                if (i < h && j < w) {
                    *out++ = *in++; 
                }
                // padded part is 0
                else { 
                    *out++ = 0; 
                }
            }
        }
    }
}

void fft1D(complexP q1, int dir, complexP q2)
{
    int i, N, N2;
    float* r1, * i1, * r2, * i2, * ra, * ia, * rb, * ib;
    float FCTR, fctr, a, b, c, s;
    complexP qa, qb;

    N = q1->len;
    r1 = q1->real;
    i1 = q1->imag;
    r2 = q2->real;
    i2 = q2->imag;

    if (N == 2) {
        a = r1[0] + r1[1];	// F(0)=f(0)+f(1);F(1)=f(0)-f(1)
        b = i1[0] + i1[1];	// a,b needed when r1=r2
        r2[1] = r1[0] - r1[1];
        i2[1] = i1[0] - i1[1];
        r2[0] = a;
        i2[0] = b;

    }
    else {
        N2 = N / 2;
        complexS c1;
        qa = &c1;
        qa->len = N2;
        qa->real = new float[N2];
        qa->imag = new float[N2];

        complexS c2;
        qb = &c2;
        qb->len = N2;
        qb->real = new float[N2];
        qb->imag = new float[N2];

        ra = qa -> real;
        ia = qa -> imag;
        rb = qb -> real;
        ib = qb -> imag;

        // split list into 2 halves; even and odd
        for (i = 0; i < N2; i++) {
            ra[i] = *r1++;
            ia[i] = *i1++;
            rb[i] = *r1++;
            ib[i] = *i1++;
        }

        // compute fft on both lists
        fft1D(qa, dir, qa);
        fft1D(qb, dir, qb);

        // build up coefficients
        if (!dir)	// forward
            FCTR = -2 * PI / N;
        else	FCTR = 2 * PI / N;

        for (fctr = i = 0; i < N2; i++, fctr += FCTR) {
            c = cos(fctr);
            s = sin(fctr);
            a = c * rb[i] - s * ib[i];
            r2[i] = ra[i] + a;
            r2[i + N2] = ra[i] - a;

            a = s * rb[i] + c * ib[i];
            i2[i] = ia[i] + a;
            i2[i + N2] = ia[i] - a;
        }

        // Free Memory
        delete[] qa->real;
        delete[] qa->imag;
        delete[] qb->real;
        delete[] qb->imag;
    }

    if (!dir) {	// inverse : divide by logN
        for (i = 0; i < N; i++) {
            q2 -> real[i] = q2 -> real[i] / 2;
            q2 -> imag[i] = q2 -> imag[i] / 2;
        }
    }
}

void fft1DRow(ImagePtr I1, ImagePtr Image1) {
    int w = I1->width();
    int h = I1->height();
    ChannelPtr<float> real, imag;
    ChannelPtr<uchar> p1;
    int type;

    real = Image1[0];
    imag = Image1[1];

    complexP q1, q2;
    complexS c1, c2;
    q1 = &c1;
    q2 = &c2;
    q1->len = w;
    q1->real = new float[w];
    q1->imag = new float[w];
    q2->len = w;
    q2->real = new float[w];
    q2->imag = new float[w];

    for (int ch = 0; IP_getChannel(I1, ch, p1, type); ch++) {
        // fft by rows
        for (int i = 0; i < h; i++) {
            // initialize the real values for rows and 0 for columns since no imaginary numbers with original
            for (int j = 0; j < q1->len; j++) {
                q1->real[j] = *p1++;
                q1->imag[j] = 0;
            }
            fft1D(q1, 0, q2);
            
            // get the real and imaginary outputs after fft
            for (int k = 0; k < q2->len; k++) {
                *real++ = q2->real[k];
                *imag++ = q2->imag[k];
            }
        }
        // free memory
        delete[] q1->real;
        delete[] q1->imag;
        delete[] q2->real;
        delete[] q2->imag;
    }
}

void fft1DCol(ImagePtr I1, ImagePtr Image1, ImagePtr Image2) {
    int w = I1->width();
    int h = I1->height();
    ChannelPtr<float> real, imag, real2, imag2;
    ChannelPtr<uchar> p1;
    int type;

    // Init real and imaginary numbers
    real = Image1[0];
    imag = Image1[1];
    real2 = Image2[0];
    imag2 = Image2[1];
   
    complexP q1, q2;
    complexS c1, c2;
    q1 = &c1;
    q2 = &c2;
    q1->len = w;
    q1->real = new float[w];
    q1->imag = new float[w];
    q2->len = w;
    q2->real = new float[w];
    q2->imag = new float[w];

    // fft by columns
    for (int j = 0; j < w; j++) {
        // create temp storage for real and imaginary
        ChannelPtr<float> tmpR = real;
        ChannelPtr<float> tmpI = imag;

        // stores real and imaginary values into q1
        for (int i = 0; i < h; i++) {
            q1->real[i] = *tmpR;
            q1->imag[i] = *tmpI;
            // goes to next row after each column
            if (i < h - 1) {
                tmpR += w;
                tmpI += w;
            }
        }

        // fft
        fft1D(q1, 0, q2);

        // sets the output to image2 as image1 is still being changed
        for (int k = 0; k < h; k++) {
            *real2++ = q2->real[k];
            *imag2++ = q2->imag[k];
        }
        real++;
        imag++;
    }
    // free memory
    delete[] q1->real;
    delete[] q1->imag;
    delete[] q2->real;
    delete[] q2->imag;
}

float getMin(ImagePtr I1, int total) {
    ChannelPtr<float> arr = I1[0];
    float currMin = arr[0];
    for (int i = 1; i < total; i++) {
        currMin = min(currMin, arr[i]);
    }
    return currMin;
}

float getMax(ImagePtr I1, int total) {
    ChannelPtr<float> arr = I1[0];
    float currMax = arr[0];
    for (int i = 1; i < total; i++) {
        currMax = max(currMax, arr[i]);
    }
    return currMax;
}