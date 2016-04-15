/*
 *    Copyright 2014-2016 Intel Corporation.
 *
 *    This file is subject to the Intel Sample Source Code License. A copy
 *    of the Intel Sample Source Code License is included.
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <math.h>
#include <omp.h>

typedef float REAL;
// restrict means the pointer does not overlap with others.
typedef REAL *restrict REAL_POINTER;
#define NUM_TESTS_ITERS 4 // Number of times you want to run the program. Running multiple
// times help to settle the running time and almost always
// the first run takes more time than others.

/* returns in the unit of ms */
double GetTime()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1e3 + tv.tv_usec / 1e3;
}

//!!a A compute intensive O(dimX * dimY * dimZ * length * length * length)
// kernel of random computation.
__attribute__((noinline))
void compute(int dimX, int dimY, int dimZ, int length, REAL_POINTER out1,
             REAL_POINTER out2)
{
    #pragma omp parallel for collapse(3)
    for (int i = 0; i < dimX; i++) {
        for (int j = 0; j < dimY; j++) {
            for (int k = 0; k < dimZ; k++) {
                // Notice how the index offset is found based on i, j and k.
                int idx = (i * dimY * dimZ + j * dimZ + k) * length;
                REAL_POINTER pout1 = out1 + idx;
                REAL_POINTER pout2 = out2 + idx;
                for (int n = 0; n < length; n++) {
                    REAL val1 = log2f(i + n + 2.0);
                    for (int m = 0; m < length; m++) {
                        /* NOTE: For MIC architecture, Intel compiler
                         * understands the importance of aligned memory access.
                         * Therefore, in this case, Intel compiler will perform
                         * loop peeling which splits the whole loop into three
                         * sections. The middle part will have aligned memory
                         * access, while the first part (PEEL) and the last
                         * part (REMAINDER) may have unaligned memory accesses.
                         * Adding "#pragma vector aligned" makes the code
                         * generation for the PEEL part unnecessary.
                         */
                        REAL val2 = log2f(j + m + 2.0);
#pragma vector aligned
                        for (int l = 0; l < length - 16; l++) {
                            REAL val3 = log2f(k + l + 2.0);
                            REAL inc = sqrtf(val1 + val2) - sqrtf(val2 + val3);
                            pout1[l] = pout1[l + 16] + inc;
                            pout2[l] = pout2[l] + inc;
                        }
                    }
                }
            }
        }
    }
}

int main(int argc, char **argv)
{
    int dimX = 4;
    int dimY = 64;
    int dimZ = 64;
    int numIters = 64;

    // Process args from command line*/
    int argi = 1;
    int errFlag = 0;
    while (argi < argc) {
        char *one = argv[argi];
        if (!strcmp(one, "-d") && argc > argi + 3) {
            dimX = atoi(argv[argi + 1]);
            dimY = atoi(argv[argi + 2]);
            dimZ = atoi(argv[argi + 3]);
            argi += 4;
        } else if (!strcmp(one, "-n") && argc > argi + 1) {
            numIters = atoi(argv[argi + 1]);
            /* Make numIters a multiple of 16 */
            numIters = numIters & (~0xf);
            argi += 2;
        } else {
            errFlag = 1;
            break;
        }
    }
    if (errFlag) {
        printf("Usage: %s [-d [the size of each dimension]] "
               "[-n [the number of iterations in the kernel]]\n", argv[0]);
        return -1;
    }

    if (dimY != dimZ) {
        printf("DimY is not the same as DimZ, "
               "changing dimZ to make them alike!\n");
        dimZ = dimY;
    }

    if (dimX * dimY * dimZ * numIters % 64 != 0) {
        printf("The production of DimX, DimY, DimZ and "
               "#iters must be multiples of 64!\n");
        return -1;
    }

    printf("DimX=%d, DimY=%d, DimZ=%d, #Loop_Iterations=%d\n", dimX, dimY,
           dimZ, numIters);

    REAL *out = NULL;

    // 64 bytes aligned allocation.
    posix_memalign((void **) &out, 64,
                   dimX * dimY * dimZ * numIters * sizeof(REAL) * 2);

    if (out == NULL) {
        printf("Memory allocation failed!\n");
        return -1;
    }

    double iterTimes[NUM_TESTS_ITERS];
    double mintime = 1e6;

    // Notice how the out1 and out2 pointers are calculated.
    REAL *out1 = out;
    REAL *out2 = out + dimX * dimY * dimZ * numIters;

    for (int i = 0; i < NUM_TESTS_ITERS; i++) {
        iterTimes[i] = GetTime();

        memset(out, 0.5, dimX * dimY * dimZ * numIters * sizeof(REAL) * 2);
        //!!a Invoked on host
        compute(dimX, dimY, dimZ, numIters, out1, out2);

        iterTimes[i] = GetTime() - iterTimes[i];
        double result = out1[numIters / 2] + out2[numIters / 2];
        printf("Test %d takes %.3lf ms with result %.3lf\n", i, iterTimes[i],
               result);
        if (iterTimes[i] < mintime) {
            mintime = iterTimes[i];
        }
    }
    printf("Test's min time is %.3lf ms\n", mintime);

    free(out);
    return 0;
}
