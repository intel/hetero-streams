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

int tile_size = 1; //  Default tile_size is 1.

/* returns in the unit of ms */
double GetTime()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1e3 + tv.tv_usec / 1e3;
}

/*
 This code is a tiled version of the code shown in folder
 0.compute_math_not_tiled_host. Here, we have tiled loop j and k and kept loop i
 as is. You might be tempted to tile loop i as well. But it turns out that tiling
 loop i, in addition to loop j and k reduces parallelism and as a result slows
 down the program. We assume that the Y and Z dimensions are of the same size.
 Therefore, we use the same tile size for both.
 */

//A compute intensive O(dimX * dimY * dimZ * length * length * length) kernel
//of random computation.
__attribute__((noinline))
void compute(int dimX, int dimY, int dimZ, int length, REAL_POINTER out1,
             REAL_POINTER out2)
{

    // Shows an efficient way of tiling the original code.
    // Define number of tiles based on tile_size.
    int num_tile = dimY / tile_size;

    #pragma omp parallel for collapse(3)
    for (int i = 0; i < dimX; i++) {
        //!!a Tile loop j.
        for (int j = 0; j < num_tile; j++) {
            //!!a Tile loop k.
            //%% EXERCISE: Correctly put limits for loop k as shown in loop j.
            for (int k = 0; k < % %; k++) {
                int startj = j * tile_size;
                int endj = startj + tile_size;
                int startk = k * tile_size;
                int endk = startk + tile_size;
                //!!b Indexing inside a particular j tile.
                for (int jj = startj; jj < endj; jj++) {
                    //!!b Indexing inside a particular k tile.
                    //%% EXERCISE: Correctly put limits for loop kk as shown in loop jj.
                    for (int kk = % %; kk < % %; kk++) {

                        //!!b Notice how the index offset was found in the code shown in folder 0.
                        //%% EXERCISE: Correctly fill in % % based following example in folder 0.
                        int idx = (i * dimY * dimZ + % % * dimZ +  % %) * length;

                        REAL_POINTER pout1 = out1 + idx;
                        REAL_POINTER pout2 = out2 + idx;
                        for (int n = 0; n < length; n++) {
                            REAL val1 = log2f(i + n + 2.0);
                            for (int m = 0; m < length; m++) {
                                /* NOTE: For MIC architecture, Intel compiler
                                 * understands the importance of aligned memory
                                 * access. Therefore, in this case, Intel
                                 * compiler will perform loop peeling which
                                 * splits the whole loop into three sections.
                                 * The middle part will have aligned memory
                                 * access, while the first part (PEEL) and the
                                 * last part (REMAINDER) may have unaligned
                                 * memory accesses. Adding "#pragma vector
                                 * aligned" makes the code generation for the
                                 * PEEL part unnecessary.
                                 */
                                REAL val2 = log2f(j + m + 2.0);
#pragma vector aligned
                                for (int l = 0; l < length - 16; l++) {
                                    REAL val3 = log2f(k + l + 2.0);
                                    REAL inc = sqrtf(val1 + val2) - sqrtf(
                                                   val2 + val3);
                                    pout1[l] = pout1[l + 16] + inc;
                                    pout2[l] = pout2[l] + inc;
                                }
                            }
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
        }
        // Also take tile_size from command line*/
        else if (!strcmp(one, "-t") && argc > argi + 1) {
            tile_size = atoi(argv[argi + 1]);

            if (dimY % tile_size != 0) {
                printf("The given tile_size does not divide dimY evenly!"
                       "\n");
                return -1;
            }
            argi += 2;
        }

        else {
            errFlag = 1;
            break;
        }
    }
    if (errFlag) {
        printf("Usage: %s [-d [the size of each dimension]] "
               "[-n [the number of iterations in the kernel]] "
               "[-t tile_size]\n", argv[0]);
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

    printf("DimX=%d, DimY=%d, DimZ=%d, #Loop_Iterations=%d, tile_size=%d\n",
           dimX, dimY, dimZ, numIters, tile_size);

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
