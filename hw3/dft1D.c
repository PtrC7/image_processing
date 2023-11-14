#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define PI 3.1415927
#define MAG(i)		(sqrt(Fr[i]*Fr[i] + Fi[i]*Fi[i]))
#define PHASE(i)	(atan2(Fi[i], Fr[i]))

void compute_dft(int N, int dir, double* fr, double* fi, double* Fr, double* Fi) {
    int u, x;
    double c, s, real, imag;

    for (u = 0; u < N; u++) {
        real = imag = 0;
        for (x = 0; x < N; x++) {
            c = cos(2.0 * PI * u * x / N);
            s = (dir == 0) ? -sin(2.0 * PI * u * x / N) : sin(2.0 * PI * u * x / N);
            real += (fr[x] * c - fi[x] * s);
            imag += (fr[x] * s + fi[x] * c);
        }
        Fr[u] = real / N;
        Fi[u] = imag / N;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <input_file> <direction> <output_file>\n", argv[0]);
        fprintf(stderr, "Direction: 0 - Forward DFT, 1 - Inverse DFT\n");
        return 1;
    }

    char* input_file = argv[1];
    int dir = atoi(argv[2]);
    char* output_file = argv[3];

    FILE* file = fopen(input_file, "r");
    if (file == NULL) {
        perror("Error opening input file");
        return 1;
    }

    int N;
    fscanf(file, "%*d %d", &N); // Read N from the file
    double* fr = (double*)malloc(N * sizeof(double));
    double* fi = (double*)malloc(N * sizeof(double));
    double* Fr = (double*)malloc(N * sizeof(double));
    double* Fi = (double*)malloc(N * sizeof(double));

    for (int x = 0; x < N; x++) {
        fscanf(file, "%lf %lf", &fr[x], &fi[x]);
    }

    fclose(file);

    compute_dft(N, dir, fr, fi, Fr, Fi);

    file = fopen(output_file, "w");
    if (file == NULL) {
        perror("Error opening output file");
        return 1;
    }

    fprintf(file, "2 %d\n", N);
    for (int u = 0; u < N; u++) {
        fprintf(file, "%f %f\n", Fr[u], Fi[u]);
    }

    fclose(file);

    free(fr);
    free(fi);
    free(Fr);
    free(Fi);

    return 0;
}
