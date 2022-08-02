#include "dgemm.h"
#include <cstdio>
#include <cstdlib>
#include <immintrin.h>

#pragma GCC optimize ("-Ofast")

float solution[8];
const int aligned = MATRIX_SIZE - (MATRIX_SIZE%8);

void dgemm(float alpha, const float *a, const float *b, float *c) {
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            __m256 partial_sol = _mm256_set1_ps ((float)0);
            for (int k = 0; k < aligned; k+=8) {
                __m256 a_i = _mm256_loadu_ps(a + i * MATRIX_SIZE + k);
                __m256 b_i = _mm256_loadu_ps(b + j * MATRIX_SIZE + k);
                partial_sol = _mm256_add_ps(partial_sol, _mm256_mul_ps(a_i, b_i));
            }
            _mm256_storeu_ps(solution, partial_sol);
            for (int n = 0; n < 8; n++) {
                c[i * MATRIX_SIZE + j] += alpha * solution[n];
            }
            for (int m = aligned; m < MATRIX_SIZE; m++) {
                c[i * MATRIX_SIZE + j] += alpha * a[i * MATRIX_SIZE + m] * b[j * MATRIX_SIZE + m];
            }
        }
    }
}

int main(int, char **) {
    float alpha, beta;

    // mem allocations
    int mem_size = MATRIX_SIZE * MATRIX_SIZE * sizeof(float);
    auto a = (float *) malloc(mem_size);
    auto b = (float *) malloc(mem_size);
    auto c = (float *) malloc(mem_size);

    // check if allocated
    if (nullptr == a || nullptr == b || nullptr == c) {
        printf("Memory allocation failed\n");
        if (nullptr != a) free(a);
        if (nullptr != b) free(b);
        if (nullptr != c) free(c);
        return 0;
    }

    generateProblemFromInput(alpha, a, b, beta, c);

    std::cerr << "Launching dgemm step." << std::endl;
    // matrix-multiplication
    dgemm(alpha, a, b, c);

    outputSolution(c);

    free(a);
    free(b);
    free(c);
    return 0;
}