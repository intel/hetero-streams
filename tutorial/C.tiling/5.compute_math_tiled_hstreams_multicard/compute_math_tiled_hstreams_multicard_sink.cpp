/*
 *    Copyright 2014-2016 Intel Corporation.
 *
 *    This file is subject to the Intel Sample Source Code License. A copy
 *    of the Intel Sample Source Code License is included.
 *
 */

// Basic hStreams sink side library
#include <hStreams_sink.h>

#include <stdio.h>
#include <math.h>
#include <omp.h>

typedef float REAL;
typedef REAL *restrict REAL_POINTER;

HSTREAMS_EXPORT
void compute(uint64_t ii, uint64_t dimX_, uint64_t dimY_, uint64_t dimZ_,
             uint64_t length_, uint64_t tile_size_, uint64_t out1_, uint64_t out2_)
{
    int iii = (int) ii;
    int dimX = (int) dimX_;
    int dimY = (int) dimY_;
    int dimZ = (int) dimZ_;
    int length = (int) length_;
    int tile_size = (int) tile_size_;
    REAL_POINTER out1 = (REAL_POINTER) out1_;
    REAL_POINTER out2 = (REAL_POINTER) out2_;

    // Shows an efficient way of tiling the original code.
    // Define number of tiles based on tile_size.
    int num_tile = dimY / tile_size;
    #pragma omp parallel for collapse(3)

    for (int i = iii; i <= iii; i++) {
        //!!d Tiling the sink-side code following similar technique used in #1,
        //!!d with address calculation adjusted based on the given intermediate
        //!!d address.
        // Tile j loop.
        for (int j = 0; j < num_tile; j++) {
            // Tile k loop.
            for (int k = 0; k < num_tile; k++) {
                int startj = j * tile_size;
                int endj = startj + tile_size;
                int startk = k * tile_size;
                int endk = startk + tile_size;

                // Indexing inside a particular j tile.
                for (int jj = startj; jj < endj; jj++) {
                    // Indexing inside a particular k tile.
                    for (int kk = startk; kk < endk; kk++) {
                        int idx = (jj * dimZ + kk) * length;
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

