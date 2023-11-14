#include "IP.h"
using namespace IP;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_swapPhase:
//
// Swap the phase channels of I1 and I2.
// Output is in II1 and II2.
//

typedef struct {
	int len;	// length of complex number list
	float* real;	// pointer to real number list
	float* imag;	// pointer to imaginary number list
}
complexS, * complexP;

extern void HW_fft2MagPhase(ImagePtr Ifft, ImagePtr Imag, ImagePtr Iphase);
extern void HW_MagPhase2fft(ImagePtr Imag, ImagePtr Iphase, ImagePtr Ifft);

extern void padImage(ImagePtr I1, ImagePtr I1Padded);
extern void fft1D(complexP q1, int dir, complexP q2);
extern void fft1DRow(ImagePtr I1, ImagePtr Image1, int dir);
extern void fft1DCol(ImagePtr I1, ImagePtr Image1, ImagePtr Image2, int dir);

void Ifft1DRow(ImagePtr I1, ImagePtr Image1, ImagePtr Image2, int dir);
void Ifft1DCol(ImagePtr I1, ImagePtr Image1, ImagePtr Image2, int dir);

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// HW_swapPhase:
//
// Swap phase of I1 with I2.
// (I1_mag, I2_phase) -> II1
// (I1_phase, I2_mag) -> II2
//
void
HW_swapPhase(ImagePtr I1, ImagePtr I2, ImagePtr II1, ImagePtr II2)
{
	ImagePtr Ifft1, Ifft2, IinvFFT1, IinvFFT2;
	ImagePtr Imag1, Iphase1, Imag2, Iphase2;

	// compute FFT of I1 and I2
	
// PUT YOUR CODE HERE...
	// Height and width of I1 and I2
	int w1 = I1->width();
	int h1 = I1->height();
	int w2 = I2->width();
	int h2 = I2->height();
	int total1 = w1 * h1;
	int total2 = w2 * h2;

	// makes sure I1 and I2 have identical dimensions
	if (w1 != w2 || h1 != h2) {
		return;
	}

	ImagePtr I3, I4;
	// Check for padding
	if (ceil(log2(w1)) != floor(log2(w1)) || ceil(log2(h1)) != floor(log2(h1))) {
		padImage(I1, I3);
		// updates width and height with new padded image
		w1 = I3->width();
		h1 = I3->height();
		total1 = w1 * h1;
	}

	if (ceil(log2(w2)) != floor(log2(w2)) || ceil(log2(h2)) != floor(log2(h2))) {
		padImage(I2, I4);
		// updates width and height with new padded image
		w2 = I4->width();
		h2 = I4->height();
		total2 = w2 * h2;
	}

	IP_copyImageHeader(I3, II1);
	IP_copyImageHeader(I4, II2);

	ImagePtr tempI1, tempI2;

	Ifft1->allocImage(w1, h1, FFT_TYPE);
	tempI1->allocImage(w1, h1, FFT_TYPE);
	Ifft2->allocImage(w2, h2, FFT_TYPE);
	tempI2->allocImage(w2, h2, FFT_TYPE);

	// fft of I1
	fft1DRow(I3, tempI1, 0);
	fft1DCol(I3, tempI1, Ifft1, 0);
	// fft of I2
	fft1DRow(I4, tempI2, 0);
	fft1DCol(I4, tempI2, Ifft2, 0);


	// compute magnitude and phase from real and imaginary FFT channels
	
// PUT YOUR CODE HERE...
	Imag1 = NEWIMAGE;
	Iphase1 = NEWIMAGE;
	Imag2 = NEWIMAGE;
	Iphase2 = NEWIMAGE;

	// get magnitude and phase for I1 and I2
	HW_fft2MagPhase(Ifft1, Imag1, Iphase1);
	HW_fft2MagPhase(Ifft2, Imag2, Iphase2);

	// swap phases and convert back to FFT images

// PUT YOUR CODE HERE...
	HW_MagPhase2fft(Imag1, Iphase2, Ifft1);
	HW_MagPhase2fft(Imag2, Iphase1, Ifft2);

	// compute inverse FFT
	
	
// PUT YOUR CODE HERE...
	ImagePtr tempIfft1, tempIfft2;

	Ifft1DRow(I3, Ifft1, tempIfft1, 1);
	Ifft1DCol(I3, Ifft1, II1, 1);

	Ifft1DRow(I4, Ifft2, tempIfft2, 1);
	Ifft1DCol(I4, Ifft2, II2, 1);

	// extract magnitude from resulting images
	
// PUT YOUR CODE HERE...
	ImagePtr mag1, phase1, mag2, phase2;

	HW_fft2MagPhase(II1, mag1, phase1);
	HW_fft2MagPhase(II2, mag2, phase2);

	// allocate uchar image and cast float channel to uchar for mag1

// PUT YOUR CODE HERE...
	ChannelPtr<uchar> mag1Channel = mag1[0];
	ChannelPtr<float> mag1Float = Imag1[0];
	for (int i = 0; i < w1 * h1; i++) {
		*mag1Channel++ = (uchar)(*mag1Float++);
	}
	// allocate uchar image and cast float channel to uchar for mag2

// PUT YOUR CODE HERE...
	ChannelPtr<uchar> mag2Channel = mag2[0];
	ChannelPtr<float> mag2Float = Imag2[0];
	for (int i = 0; i < w2 * h2; i++) {
		*mag2Channel++ = (uchar)(*mag2Float++);
	}
}

void Ifft1DRow(ImagePtr I1, ImagePtr Image1, ImagePtr Image2, int dir) {
	int w = I1->width();
	int h = I1->height();
	ChannelPtr<float> real, imag, real2, imag2;

	Image2->allocImage(w, h, FFT_TYPE);

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

	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; ++j) {
			q1->real[j] = *real++;
			q1->imag[j] = *imag++;
		}

		// compute inverse fft (dir = 1)
		fft1D(q1, dir, q2);

		// get the real and imaginary outputs after fft
		for (int k = 0; k < q2->len; k++) {
			*real2++ = q2->real[k];
			*imag2++ = q2->imag[k];
		}
	}
}

void Ifft1DCol(ImagePtr I1, ImagePtr Image1, ImagePtr Image2, int dir) {
	int w = I1->width();
	int h = I1->height();
	ChannelPtr<float> real, imag;
	ChannelPtr<uchar> p1;
	int type;

	Image2->allocImage(w, h, FFT_TYPE);

	real = Image1[0];
	imag = Image1[1];

	for (int ch = 0; IP_getChannel(Image2, ch, p1, type); ++ch)
	{
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
			fft1D(q1, dir, q2);

			for (int k = 0; k < h; k++) {
				*p1++ = CLIP(q2->real[k], 0, MaxGray);
			}
			real++;
			imag++;
		}
	}
}
