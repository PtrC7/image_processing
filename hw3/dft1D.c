#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define PI 3.1415927
#define MAG(i)    (sqrt(Fr[i]*Fr[i] + Fi[i]*Fi[i]))
#define PHASE(i)  (atan2(Fi[i], Fr[i]))

// Function declarations
void forwardDFT(FILE* input, FILE* output, int N);
void inverseDFT(FILE* input, FILE* output, int N);

int main(int argc, char* argv[]) {
    if (argc == 4) {
        FILE* input, * output;
        char* in = argv[1];
        int dir = atoi(argv[2]);
        char* out = argv[3];
        input = fopen(in, "r");
        output = fopen(out, "w");

        int width, height;
        fscanf(input, "%d\t%d", &width, &height);
        fprintf(output, "%d\t%d\n", width, height);

        if (height > 0) {
            if (dir == 0) {
                // Forward DFT operation
                forwardDFT(input, output, height);
            }
            else if (dir == 1) {
                // Inverse DFT operation
                inverseDFT(input, output, height);
            }
            else {
                printf("Invalid Dir\n");
                printf("Dir = 0, Forward DFT\n");
                printf("Dir = 1, Inverse DFT\n");
            }
        }
        else {
            printf("Invalid height.\n");
        }

        fclose(input);
        fclose(output);
    }
    else {
        printf("Invalid Arguments.\n");
        printf("Format: ./dtft1D Input Dir Output\n");
    }
    return 0;
}

// Function for forward DFT
void forwardDFT(FILE* input, FILE* output, int N) {
    int u, x;
    double c, s, real, imag;

    float* Fr = malloc(sizeof(float) * N);
    float* Fi = malloc(sizeof(float) * N);
    float* fr = malloc(sizeof(float) * N);
    float* fi = malloc(sizeof(float) * N);

    // Read input values
    for (x = 0; x < N; x++) {
        fscanf(input, "%f\t%f", &fr[x], &fi[x]);
    }

    // Compute forward DFT
    for (u = 0; u < N; u++) {
        real = imag = 0;

        for (x = 0; x < N; x++) {
            c = cos(2. * PI * u * x / N);
            s = -sin(2. * PI * u * x / N);
            real += (fr[x] * c - fi[x] * s);
            imag += (fr[x] * s + fi[x] * c);
        }

        Fr[u] = real / N;
        Fi[u] = imag / N;
        fprintf(output, "%f\t%f\n", Fr[u], Fi[u]);
    }

    // Free allocated memory
    free(Fr);
    free(Fi);
    free(fr);
    free(fi);
}

// Function for inverse DFT
void inverseDFT(FILE* input, FILE* output, int N) {
    int u, x;
    double c, s, real, imag;

    float* Fr = malloc(sizeof(float) * N);
    float* Fi = malloc(sizeof(float) * N);
    float* fr = malloc(sizeof(float) * N);
    float* fi = malloc(sizeof(float) * N);

    // Read input values
    for (x = 0; x < N; x++) {
        fscanf(input, "%f\t%f", &Fr[x], &Fi[x]);
    }

    // Compute inverse DFT
    for (x = 0; x < N; x++) {
        real = imag = 0;

        for (u = 0; u < N; u++) {
            c = cos(2. * PI * u * x / N);
            s = sin(2. * PI * u * x / N);
            real += (Fr[u] * c - Fi[u] * s);
            imag += (Fr[u] * s + Fi[u] * c);
        }

        fr[x] = real;
        fi[x] = imag;
        fprintf(output, "%f\t%f\n", fr[x], fi[x]);
    }

    // Free allocated memory
    free(Fr);
    free(Fi);
    free(fr);
    free(fi);
}
