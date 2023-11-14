#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define PI 3.1415927
#define MAG(i)		(sqrt(Fr[i]*Fr[i] + Fi[i]*Fi[i]))
#define PHASE(i)	(atan2(Fi[i], Fr[i]))

// Function to compute the 1D discrete Fourier transform (DFT) or inverse DFT
void compute_dft(int N, int dir, double* fr, double* fi, double* Fr, double* Fi) {
    int u, x;
    double c, s, real, imag;

    for (u = 0; u < N; u++) {
        real = imag = 0;

        // Compute the sum over all input pixels for each frequency u
        for (x = 0; x < N; x++) {
            c = cos(2.0 * PI * u * x / N);
            // Adjust the sign of sin term for forward DFT and inverse DFT
            s = (dir == 0) ? -sin(2.0 * PI * u * x / N) : sin(2.0 * PI * u * x / N);
            real += (fr[x] * c - fi[x] * s);
            imag += (fr[x] * s + fi[x] * c);
        }

        // Store the computed frequency components in Fr and Fi arrays
        Fr[u] = real / N;
        Fi[u] = imag / N;
    }
}

int main(int argc, char* argv[]) {
    // Check if the correct number of command-line arguments is provided
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <input_file> <direction> <output_file>\n", argv[0]);
        fprintf(stderr, "Direction: 0 - Forward DFT, 1 - Inverse DFT\n");
        return 1;
    }

    // Extract command-line arguments
    char* input_file = argv[1];
    int dir = atoi(argv[2]);
    char* output_file = argv[3];

    // Open the input file for reading
    FILE* file = fopen(input_file, "r");
    if (file == NULL) {
        perror("Error opening input file");
        return 1;
    }

    // Read the width and height (2 and N) from the file header
    int N;
    fscanf(file, "%*d %d", &N);

    // Allocate memory for arrays to store input and output data
    double* fr = (double*)malloc(N * sizeof(double));
    double* fi = (double*)malloc(N * sizeof(double));
    double* Fr = (double*)malloc(N * sizeof(double));
    double* Fi = (double*)malloc(N * sizeof(double));

    // Read complex numbers (real and imaginary parts) from the input file
    for (int x = 0; x < N; x++) {
        fscanf(file, "%lf %lf", &fr[x], &fi[x]);
    }

    // Close the input file
    fclose(file);

    // Compute the 1D DFT or inverse DFT based on the specified direction
    compute_dft(N, dir, fr, fi, Fr, Fi);

    // Open the output file for writing
    file = fopen(output_file, "w");
    if (file == NULL) {
        perror("Error opening output file");
        return 1;
    }

    // Write the width and height to the output file header
    fprintf(file, "2 %d\n", N);

    // Write the computed frequency components to the output file
    for (int u = 0; u < N; u++) {
        fprintf(file, "%f %f\n", Fr[u], Fi[u]);
    }

    // Close the output file
    fclose(file);

    // Free allocated memory
    free(fr);
    free(fi);
    free(Fr);
    free(Fi);

    return 0;
}
