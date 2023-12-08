#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define PI		3.1415926535897931160E0
#define SGN(A)		((A) > 0 ? 1 : ((A) < 0 ? -1 : 0 ))
#define FLOOR(A)	((int) (A))
#define CEILING(A)	((A)==FLOOR(A) ? FLOOR(A) : SGN(A)+FLOOR(A))
#define CLAMP(A,L,H)	((A)<=(L) ? (L) : (A)<=(H) ? (A) : (H))

// Function prototypes
double boxFilter(double, double);
double triFilter(double);
double cubicConv(double, double);
double sinc(double);
double lanczos(double, double);
double hann(double, double);
double hamming(double, double);

void resample1D(float* IN, float* OUT, int INlen, int OUTlen, int filtertype, double param);

int main(int argc, char* argv[]) {
	if (argc == 7) { // Check if the correct number of command-line arguments is provided
		FILE* input, * output; // File pointers
		char* in = argv[1];    // Input file name
		char* out = argv[2];   // Output file name
		int INlen = atoi(argv[3]);    // Input size
		int OUTlen = atoi(argv[4]);   // Output size
		int filtertype = atoi(argv[5]);  // Filter type
		double param = atoi(argv[6]);    // Filter parameter

		input = fopen(in, "r"); // Open input file
		output = fopen(out, "w"); // Open output file

		float* IN = malloc(sizeof(float) * INlen);
		float* OUT = malloc(sizeof(float) * OUTlen);

		// Read input data from file to array
		for (int i = 0; i < INlen; i++) {
			fscanf(input, "%f", &IN[i]);
		}

		// Call resample function
		resample1D(IN, OUT, INlen, OUTlen, filtertype, param);

		// Write output data to file
		for (int i = 0; i < OUTlen; i++) {
			fprintf(output, "%f\n", OUT[i]);
		}

		// Close files
		fclose(input);
		fclose(output);

		// Free allocated memory
		free(IN);
		free(OUT);

	}
	else { // Display an error message if the number of arguments is incorrect
		printf("Usage: ./HW_resize1D [Input].txt [Output].txt [Input Size] [Output Size] [Filter Type] [Param]\n");
		printf("Filter Type:\n");
		printf("0 = Box Filter\n");
		printf("1 = Triangle Filter\n");
		printf("2 = Cubic Convolution Filter\n");
		printf("3 = Lanczos Filter\n");
		printf("4 = Hann Filter\n");
		printf("5 = Hamming Filter\n");
	}

	return 0;
}

// Function to perform 1D resampling
void resample1D(float* IN, float* OUT, int INlen, int OUTlen, int filtertype, double param) {

	int i;
	int left, right;	// Kernel extent in input
	int pixel;		// Input pixel value
	double u, x;		// Input (u), output (x)
	double scale;		// Resampling scale factor
	double (*filter)();	// Pointer to the filter function
	double fwidth;		// Filter width (support)
	double fscale;		// Filter amplitude scale
	double weight;		// Kernel weight
	double acc;		// Convolution accumulator

	scale = (double)OUTlen / INlen;
	int offset = 1;

	switch (filtertype) {
	case 0: filter = boxFilter;	// Box filter (nearest neighbor)
		fwidth = .5;
		break;
	case 1:	filter = triFilter;	// Triangle filter (linear interpolation)
		fwidth = 1;
		break;
	case 2:	filter = cubicConv;	// Cubic convolution filter
		fwidth = 2;
		break;
	case 3:	filter = lanczos;	// Lanczos windowed sinc function
		fwidth = param;
		break;
	case 4:	filter = hann;	// Hann windowed sinc function
		fwidth = param;	// 8-point kernel
		break;
	case 5: filter = hamming;
		fwidth = param;
	}

	if (scale < 1.0) {		// Minification: h(x) -> h(x*scale)*scale
		fwidth = fwidth / scale;		// Broaden the filter
		fscale = scale;	// Lower amplitude

		// Round off fwidth to int to avoid intensity modulation
		if (filtertype == 0) {
			fwidth = CEILING(fwidth);
			fscale = 1.0 / (2 * fwidth);
		}
	}
	else	fscale = 1.0;

	// Project each output pixel to input, center kernel, and convolve
	for (x = 0; x < OUTlen; x++) {
		// Map output x to input u: inverse mapping
		u = x / scale;

		// Left and right extent of the kernel centered at u
		if (u - fwidth < 0)  left = FLOOR(u - fwidth);
		else	left = CEILING(u - fwidth);
		right = FLOOR(u + fwidth);

		// Reset accumulator for collecting convolution products
		acc = 0;

		// Weigh input pixels around u with kernel
		for (i = left; i <= right; i++) {
			pixel = IN[CLAMP(i, 0, INlen - 1) * offset];
			weight = (*filter)((u - i) * fscale, param);
			acc += (pixel * weight);
		}

		// Assign weighted accumulator to OUT
		OUT[(int)x * offset] = acc * fscale;
	}
}

// Filter functions

double boxFilter(double t, double p) {
	if ((t > -0.5) && (t <= 0.5))
		return (1.0);
	return (0.0);
}

double triFilter(double t) {
	if (t < 0)
		t = -t;
	if (t < 1.0)
		return (1.0 - t);
	return (0.0);
}

double cubicConv(double t, double p) {
	double A, t2, t3;

	if (t < 0)
		t = -t;
	t2 = t * t;
	t3 = t2 * t;

	A = p;	// User-specified free parameter
	if (t < 1.0)
		return ((A + 2) * t3 - (A + 3) * t2 + 1);
	if (t < 2.0)
		return (A * (t3 - 5 * t2 + 8 * t - 4));
	return (0.0);
}

double sinc(double t) {
	t *= PI;
	if (t != 0)
		return (sin(t) / t);
	return (1.0);
}

double lanczos(double t, double p) {
	int N = (int)p;
	if (t < 0) t = -t;
	if (t < N) return (sinc(t) * sinc(t / N));
	return (0.0);
}

double hann(double t, double p) {
	int N = (int)p;	// Fixed filter width

	if (t < 0) t = -t;
	if (t < N) return (sinc(t) * (0.5 + 0.5 * cos(PI * t / N)));
	return (0.0);
}

double hamming(double t, double p) {
	int N = (int)p;	// Fixed filter width

	if (t < 0) t = -t;
	if (t < N) return (sinc(t) * (0.54 + 0.46 * cos(PI * t / N)));
	return (0.0);
}
