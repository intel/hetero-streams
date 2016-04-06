/*
 * Copyright 2014-2016 Intel Corporation.
 *
 * This file is subject to the Intel Sample Source Code License. A copy
 * of the Intel Sample Source Code License is included.
 */

//******************************************************************************************
// Author:   Gaurav Bansal
// Purpose:  Matrix multiply using hStreams
// Created:  5/2/14
//********************************************************************************************

#include <stdio.h>
#ifndef _WIN32
#include <unistd.h>
#endif
#include <stdlib.h>
#include <string.h>

#include "dtime.h"
#include "host_cpu_mask.h"
#include <math.h>

//********************************************************************************************
#include <mkl.h>
#include <hStreams_source.h>
#include <hStreams_app_api.h>
#include "hStreams_custom_init.h"
#include "hStreams_custom.h"

#define SWITCH_CHAR  '-'
typedef FP_DATA_TYPE MATRIX_TYPE;

#define HSTR_MAX_THREADS 500

static bool loc_verbose = false;

//********************************************************************************************
//
//Modify this if need to reserve some other cpu masks
//Always reserve a full core, i.e. if HT is enabled, reserve all threads on the core.
//Reserving 2 to 3 threads give best performance
#ifdef HOST_HT_ON
//replace 28 with the cpu_mask corresponding to the hyper-thread on core 0
const int num_resv_cpus = 2;
const int resv_cpus[] = {0, 28};
#else
const int num_resv_cpus = 2;
const int resv_cpus[] = {0, 1};
#endif

int resv_cpu_master;
// load balance, start with invalid value and turn on by default if sensible
int LB = -1;

//#define HOST_HT_ON
typedef struct _mtxSize { // Optional Command-line multiplier for matrix sizes
    unsigned int numColsA, numRowsA, numColsB, numRowsB, numColsC, numRowsC;
}   sMtxSize;

static int pass        = 0;  // number of passes in former_main

// Calculate mean gigaFlops
double mean_and_stddev(double *gflops, int size, double &mean, double &stddev, double &max, double &min)
{
    mean = stddev = 0.0;
    if (size < 3) {
        return (0.0);
    }
    min = 1.0e38;
    max = -min;
    for (int i = 1; i < size; ++i) {
        mean = mean + gflops[i];
        if (gflops[i] > max) {
            max = gflops[i];
        }
        if (gflops[i] < min) {
            min = gflops[i];
        }
    }
    mean = mean / (size - 1);

    for (int i = 1; i < size; ++i) {
        stddev = stddev + pow(gflops[i] - mean, 2);
    }

    stddev = stddev / (size - 2);
    stddev = sqrt(stddev);
    return (stddev);
}

// Initializes a matrix with random MATRIX_TYPE entries.
void randomInit(MATRIX_TYPE *data, int size)
{
    for (int i = 0; i < size; ++i) {
        data[i] = rand() / (MATRIX_TYPE)RAND_MAX;
    }
}

void initializeMatrices(sMtxSize &allSizes, int m, int k, int n)
{
    allSizes.numRowsA =  m;
    allSizes.numColsA =  k;
    allSizes.numRowsB =  k;
    allSizes.numColsB =  n;
    allSizes.numRowsC =  m;
    allSizes.numColsC =  n;
}

//
// Do Block Matrix Multiply
//
int matrixMultiply(sMtxSize &allSizes,
                   int mblocks, int kblocks, int nblocks,
                   int nIter, int num_doms, int num_mics, int use_host,
                   int max_log_str, int host_ht_offset)
{
    int totalblocks_A = mblocks * kblocks;
    int totalblocks_B = kblocks * nblocks;
    int totalblocks_C = mblocks * nblocks;

    unsigned int size_A = allSizes.numColsA * allSizes.numRowsA;
    unsigned int size_B = allSizes.numColsB * allSizes.numRowsB;
    unsigned int size_C = allSizes.numColsC * allSizes.numRowsC;

    // allocate host memory for matrices A and B and C
    MATRIX_TYPE *h_A = new MATRIX_TYPE[size_A];
    MATRIX_TYPE *h_B = new MATRIX_TYPE[size_B];
    MATRIX_TYPE *h_C = new MATRIX_TYPE[size_C];

    // host memory for matrix blocks for A and B and C
    MATRIX_TYPE **h_Ablock = new MATRIX_TYPE*[totalblocks_A];
    MATRIX_TYPE **h_Bblock = new MATRIX_TYPE*[totalblocks_B];
    MATRIX_TYPE **h_Cblock = new MATRIX_TYPE*[totalblocks_C];

    unsigned int mem_size_Ablock = sizeof(MATRIX_TYPE) * size_A / totalblocks_A;
    unsigned int mem_size_Bblock = sizeof(MATRIX_TYPE) * size_B / totalblocks_B;
    unsigned int mem_size_Cblock = sizeof(MATRIX_TYPE) * size_C / totalblocks_C;

#define HSTR_BUFFER_PROPS_VALUES {        \
        HSTR_MEM_TYPE_NORMAL,             \
        HSTR_MEM_ALLOC_PREFERRED,         \
        HSTR_BUF_PROP_ALIASED}
    HSTR_BUFFER_PROPS buffer_props = HSTR_BUFFER_PROPS_VALUES;
    // malloc & initialize blocks of A/B/C on host and device
    for (int i = 0; i < totalblocks_A; ++i) {
        h_Ablock[i] = new MATRIX_TYPE[size_A / totalblocks_A];

        CHECK_HSTR_RESULT(hStreams_Alloc1DEx(
                              (void *)h_Ablock[i],
                              mem_size_Ablock,
                              &buffer_props,
                              -1,
                              NULL));
        if (loc_verbose) {
            printf("Block A[%d]: ", i);
        }

        // set seed for rand()
        srand(2006 + i);

        // initialize host memory
        randomInit(h_Ablock[i], size_A / totalblocks_A);
    }

    for (int i = 0; i < totalblocks_B; ++i) {
        h_Bblock[i] = new MATRIX_TYPE[size_B / totalblocks_B];
        //CHECK_HSTR_RESULT(hStreams_app_create_buf((void *) h_Bblock[i], mem_size_Bblock);
        CHECK_HSTR_RESULT(hStreams_Alloc1DEx(
                              (void *)h_Bblock[i],
                              mem_size_Bblock,
                              &buffer_props,
                              -1,
                              NULL));
        if (loc_verbose) {
            printf("Block B[%d]: ", i);
        }

        // set seed for rand()
        srand(201 + i);

        // initialize host memory
        randomInit(h_Bblock[i], size_B / totalblocks_B);
    }

    // Create buffers on the card
    for (int i = 0; i < totalblocks_C; ++i) {
        h_Cblock[i] = new MATRIX_TYPE[size_C / totalblocks_C];
        CHECK_HSTR_RESULT(hStreams_Alloc1DEx(
                              (void *)h_Cblock[i],
                              mem_size_Cblock,
                              &buffer_props,
                              -1,
                              NULL));
        if (loc_verbose) {
            printf("Block C[%d]: ", i);
        }
    }

    // Assemble matrices A and B for later host computation check
    int Bindex;
    for (int i = 0; i < kblocks; i++) {
        for (unsigned int k = 0; k < allSizes.numRowsB / kblocks; k++) {
            for (int j = i * nblocks; j < (i + 1)*nblocks; j++) {
                for (unsigned int l = 0; l < allSizes.numColsB / nblocks; l++) {
                    Bindex = allSizes.numColsB * (i * allSizes.numRowsB / kblocks + k) +
                             ((j - i * nblocks) * allSizes.numColsB / nblocks + l);
                    h_B[Bindex] = h_Bblock[j][l + k * allSizes.numColsB / nblocks];
                }
            }
        }
    }
    int Aindex;
    for (int i = 0; i < mblocks; i++) {
        for (unsigned int k = 0; k < allSizes.numRowsA / mblocks; k++) {
            for (int j = i; j <= i + (mblocks) * (kblocks - 1); j = j + mblocks) {
                for (unsigned int l = 0; l < allSizes.numColsA / kblocks; l++) {
                    Aindex = allSizes.numColsA * (i * allSizes.numRowsA / mblocks + k) +
                             (((j - i) / mblocks) * allSizes.numColsA / kblocks + l);
                    h_A[Aindex] = h_Ablock[j][l + k * allSizes.numColsA / kblocks];
                }
            }
        }
    }

    double *gigaFlops = new double[nIter];
    const MATRIX_TYPE alpha = 1.0;
    const MATRIX_TYPE beta  = 1.0;

    if (LB) {
        num_doms += num_mics;
    }

    HSTR_EVENT *eventAcpy = new HSTR_EVENT[totalblocks_A * num_doms];
    HSTR_EVENT *eventBcpy = new HSTR_EVENT[totalblocks_B];
    HSTR_EVENT *eventCcpy = new HSTR_EVENT[totalblocks_C];
    HSTR_EVENT *eventCset = new HSTR_EVENT[totalblocks_C];


    for (int iter = 0; iter < nIter; iter++) {
        double CdataRate = 0.0;
        double BdataRate = 0.0;
        double AdataRate = 0.0;

        mkl_mic_disable();

        int qindex;
        int cols_per_card = (int)(nblocks / num_doms);
        double iterStart = dtimeGet();
        int card_id; //for load balancing
        int is_mic;

        // Memset h_Cblock to zero initially
        for (int i = 0; i < totalblocks_C; ++i) {
            if ((i % nblocks) < (nblocks / num_doms)) {
                for (int k = 0; k < num_doms; ++k) {
                    if (LB) {
                        if (k >= num_doms - num_mics) {
                            card_id = k - num_mics;
                        } else {
                            card_id = k;
                        }
                    } else {
                        card_id = k;
                    }
                    if ((use_host == 1) && (num_mics >= 1)) {
                        if (card_id == 0) {
                            is_mic = 0;    //this is host
                        } else {
                            is_mic = 1;
                        }
                    } else {
                        is_mic = 0;
                    }


                    int c_index = i + k * ((int)nblocks / num_doms);
                    int row_id = (int)(c_index / nblocks);
                    int mod_ind = i + row_id * cols_per_card;
                    qindex = (int)mod_ind % max_log_str + (card_id) * max_log_str + (is_mic) * host_ht_offset; //the last one should only be for card_id != 0

                    CHECK_HSTR_RESULT(hStreams_custom_memset((int)(qindex),
                                      h_Cblock[c_index], 0, mem_size_Cblock,
                                      &eventCset[c_index]));
                }
            }
        }

        int cblock_index = 0;
        int countA = -1, countB = -1;
        div_t divresult1, divresult2;


        while (countA < totalblocks_A - 1 || countB < totalblocks_B - 1) {
            if ((countA <= countB || countB == totalblocks_B - 1) && (countA < totalblocks_A - 1)) {
                countA++;
                for (int i = 0; i < num_doms; ++i) {
                    if (LB) {
                        if (i >= num_doms - num_mics) {
                            card_id = i - num_mics;
                        } else {
                            card_id = i;
                        }
                    } else {
                        card_id = i;
                    }
                    if ((use_host == 1) && (num_mics >= 1)) {
                        if (card_id == 0) {
                            is_mic = 0;    //this is host
                        } else {
                            is_mic = 1;
                        }
                    } else {
                        is_mic = 0;
                    }

                    qindex = (int)(countA % max_log_str + (card_id) * max_log_str + (is_mic) * host_ht_offset);
                    double tbegin = dtimeGet();
                    // if (i != 0) //no transfer for host
                    {
                        CHECK_HSTR_RESULT(hStreams_app_xfer_memory(
                                              (int)(qindex),
                                              h_Ablock[countA], h_Ablock[countA], mem_size_Ablock,
                                              HSTR_SRC_TO_SINK,
                                              &eventAcpy[countA + i * totalblocks_A]));
                    }
                    double tend = dtimeGet();
                    if (loc_verbose) {
                        printf("Memcpy A Mic time= %.3f milliseconds\n", 1.0e3 * (tend - tbegin));
                    }
                    if (tend != tbegin) {
                        AdataRate = 1.0e-9 * mem_size_Ablock / (tend - tbegin);
                        if (loc_verbose) {
                            printf("A Data Rate= %.3f GB/s\n", AdataRate);
                        }
                    }
                }
            } else if ((countB < countA || countA == totalblocks_A - 1) && (countB < totalblocks_B - 1)) {
                countB++;
                for (int i = 0; i < num_doms; ++i) {
                    if (LB) {
                        if (i >= num_doms - num_mics) {
                            card_id = i - num_mics;
                        } else {
                            card_id = i;
                        }
                    } else {
                        card_id = i;
                    }
                    if ((use_host == 1) && (num_mics >= 1)) {
                        if (card_id == 0) {
                            is_mic = 0;    //this is host
                        } else {
                            is_mic = 1;
                        }
                    } else {
                        is_mic = 0;
                    }

                    if ((countB % nblocks) < (nblocks / num_doms)) {
                        int b_index = countB + i * ((int)nblocks / num_doms);

                        int row_id = (int)(b_index / nblocks);
                        int mod_ind = countB + row_id * cols_per_card;
                        qindex = (int)mod_ind % max_log_str + (card_id) * max_log_str + (is_mic) * host_ht_offset ;

                        double tbegin = dtimeGet();
                        // if (i != 0)
                        {
                            CHECK_HSTR_RESULT(hStreams_app_xfer_memory(
                                                  //(int)(countB%max_log_str + i*max_log_str),
                                                  (int)(qindex),
                                                  h_Bblock[b_index], h_Bblock[b_index], mem_size_Bblock,
                                                  HSTR_SRC_TO_SINK,
                                                  &eventBcpy[b_index]));
                        }
                        double tend = dtimeGet();
                        if (loc_verbose) {
                            printf("Memcpy B Mic time= %.3f milliseconds\n", 1.0e3 * (tend - tbegin));
                        }

                        if (tend != tbegin) {
                            BdataRate = 1.0e-9 * mem_size_Bblock / (tend - tbegin);
                            if (loc_verbose) {
                                printf("B Data Rate= %.3f GB/s\n", BdataRate);
                            }
                        }
                    }
                }
            }

            // start multiplications
            if ((countA + countB) >= 0) {
                if (countA > countB) {
                    //received a new A block
                    for (int j = 0; j <= countB; j++) {
                        if ((j % nblocks) < (nblocks / num_doms)) {
                            for (int k = 0; k < num_doms; ++k) {
                                if (LB) {
                                    if (k >= num_doms - num_mics) {
                                        card_id = k - num_mics;
                                    } else {
                                        card_id = k;
                                    }
                                } else {
                                    card_id = k;
                                }
                                if ((use_host == 1) && (num_mics >= 1)) {
                                    if (card_id == 0) {
                                        is_mic = 0;    //this is host
                                    } else {
                                        is_mic = 1;
                                    }
                                } else {
                                    is_mic = 0;
                                }


                                int b_index = j + k * ((int)nblocks / num_doms);

                                divresult1 = div(countA, mblocks);
                                divresult2 = div(b_index, nblocks);
                                if (divresult1.quot == divresult2.quot) {
                                    //validity
                                    cblock_index = divresult1.rem * nblocks + divresult2.rem;

                                    int row_id = (int)(cblock_index / nblocks);
                                    int mod_ind = (cblock_index - k * cols_per_card) + row_id * cols_per_card;
                                    //qindex = (int)cblock_index % max_log_str + k*max_log_str;
                                    qindex = (int)mod_ind % max_log_str + (card_id) * max_log_str + (is_mic) * host_ht_offset;

                                    if (loc_verbose) {
                                        printf("Multiplying A[%d]xB[%d] = C[%d].. Using logical stream %d...",
                                               countA, b_index, cblock_index, qindex);
                                    }

                                    CHECK_HSTR_RESULT(hStreams_app_event_wait_in_stream(qindex, 1, &eventBcpy[b_index], 0, NULL, NULL));
                                    CHECK_HSTR_RESULT(hStreams_app_event_wait_in_stream(qindex, 1, &eventAcpy[countA + k * totalblocks_A], 0, NULL, NULL));
                                    CHECK_HSTR_RESULT(hStreams_app_event_wait_in_stream(qindex, 1, &eventCset[cblock_index], 0, NULL, NULL));


                                    double tbegin = dtimeGet();
                                    //Async compute on the device
                                    CHECK_HSTR_RESULT(hStreams_custom_dgemm(
                                                          qindex,
                                                          CblasColMajor, CblasNoTrans, CblasNoTrans,
                                                          allSizes.numColsB / nblocks, allSizes.numRowsA / mblocks,
                                                          allSizes.numRowsB / kblocks,
                                                          alpha, h_Bblock[b_index], allSizes.numColsB / nblocks,
                                                          h_Ablock[countA], allSizes.numColsA / kblocks,
                                                          beta, h_Cblock[cblock_index], allSizes.numColsC / nblocks,
                                                          &eventCcpy[cblock_index]));
                                    double tend = dtimeGet();

                                    if (loc_verbose) {
                                        printf("GEMM Time= %.3f microseconds\n", 1.0e6 * (tend - tbegin));
                                    }
                                }
                            }
                        }
                    }
                } else { //received a new B block
                    for (int i = 0; i <= countA; i++) {
                        if ((countB % nblocks) < (nblocks / num_doms)) {
                            for (int k = 0; k < num_doms; ++k) {
                                if (LB) {
                                    if (k >= num_doms - num_mics) {
                                        card_id = k - num_mics;
                                    } else {
                                        card_id = k;
                                    }
                                } else {
                                    card_id = k;
                                }
                                if ((use_host == 1) && (num_mics >= 1)) {
                                    if (card_id == 0) {
                                        is_mic = 0;    //this is host
                                    } else {
                                        is_mic = 1;
                                    }
                                } else {
                                    is_mic = 0;
                                }

                                int b_index = countB + k * ((int)nblocks / num_doms);

                                divresult1 = div(i, mblocks);
                                divresult2 = div(b_index, nblocks);

                                if (divresult1.quot == divresult2.quot) {
                                    //validity
                                    cblock_index = divresult1.rem * nblocks + divresult2.rem;

                                    int row_id = (int)(cblock_index / nblocks);
                                    int mod_ind = (cblock_index - k * cols_per_card) + row_id * cols_per_card;
                                    //qindex = (int)cblock_index % max_log_str + k*max_log_str;
                                    qindex = (int)mod_ind % max_log_str + (card_id) * max_log_str + (is_mic) * host_ht_offset;

                                    if (loc_verbose) {
                                        printf("Multiplying A[%d]xB[%d] = C[%d].. Using logical stream %d...",
                                               i, b_index, cblock_index, qindex);
                                    }

                                    CHECK_HSTR_RESULT(hStreams_app_event_wait_in_stream(qindex, 1, &eventBcpy[b_index], 0, NULL, NULL));
                                    CHECK_HSTR_RESULT(hStreams_app_event_wait_in_stream(qindex, 1, &eventAcpy[i], 0, NULL, NULL));
                                    CHECK_HSTR_RESULT(hStreams_app_event_wait_in_stream(qindex, 1, &eventCset[cblock_index], 0, NULL, NULL));

                                    double tbegin = dtimeGet();
                                    //Async compute
                                    CHECK_HSTR_RESULT(hStreams_custom_dgemm(
                                                          qindex,
                                                          CblasColMajor, CblasNoTrans, CblasNoTrans,
                                                          allSizes.numColsB / nblocks, allSizes.numRowsA / mblocks,
                                                          allSizes.numRowsB / kblocks,
                                                          alpha, h_Bblock[b_index], allSizes.numColsB / nblocks,
                                                          h_Ablock[i], allSizes.numColsA / kblocks,
                                                          beta, h_Cblock[cblock_index], allSizes.numColsC / nblocks,
                                                          &eventCcpy[cblock_index]));
                                    double tend = dtimeGet();

                                    if (loc_verbose) {
                                        printf("GEMM Time= %.3f microseconds\n", 1.0e6 * (tend - tbegin));
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        // copy result from device to host
        for (int i = 0; i < totalblocks_C; ++i) {
            if ((i % nblocks) < (nblocks / num_doms)) {
                for (int k = 0; k < num_doms; ++k) {
                    if (LB) {
                        if (k >= num_doms - num_mics) {
                            card_id = k - num_mics;
                        } else {
                            card_id = k;
                        }
                    } else {
                        card_id = k;
                    }
                    if ((use_host == 1) && (num_mics >= 1)) {
                        if (card_id == 0) {
                            is_mic = 0;    //this is host
                        } else {
                            is_mic = 1;
                        }
                    } else {
                        is_mic = 0;
                    }

                    int c_index = i + k * ((int)nblocks / num_doms);
                    int row_id = (int)(c_index / nblocks);
                    int mod_ind = i + row_id * cols_per_card;
                    qindex = (int)mod_ind % max_log_str + (card_id) * max_log_str + (is_mic) * host_ht_offset;

                    if (loc_verbose) {
                        printf("transferring matrix h_Cblock[%d] from device to host..asynchronously..using logical stream %d.",
                               c_index, qindex);
                    }

                    //Synchronize before transferring the result back
                    CHECK_HSTR_RESULT(hStreams_app_stream_sync(i % max_log_str + (card_id)*max_log_str + (is_mic)*host_ht_offset));

                    double tbegin = dtimeGet();
                    //if (k != 0)
                    {
                        CHECK_HSTR_RESULT(hStreams_app_xfer_memory(
                                              // (int)(i%max_log_str + k*max_log_str),
                                              (int)(qindex),
                                              h_Cblock[c_index], h_Cblock[c_index], mem_size_Cblock,
                                              HSTR_SINK_TO_SRC, &eventCcpy[c_index]));
                    }
                    double tend = dtimeGet();
                    if (loc_verbose) {
                        printf("Memcpy C Time= %7.3f milliseconds --> ", 1.0e3 * (tend - tbegin));
                    }
                    if (tend != tbegin) {
                        CdataRate = 1.0e-9 * mem_size_Cblock / (tend - tbegin);
                        if (loc_verbose) {
                            printf("C Data Rate= %7.3f GB/s\n", CdataRate);
                        }
                    }
                }
            }
        }

        //Synchronize after transferring the result back
        CHECK_HSTR_RESULT(hStreams_app_thread_sync());

        double iterStop = dtimeGet();

        //get Gflops
        double flopsPerMatrixMul = 2.0 * (double)allSizes.numColsA *
                                   (double)allSizes.numRowsA *
                                   (double)allSizes.numColsB;
        if (iterStop != iterStart) {
            gigaFlops[iter] = (flopsPerMatrixMul * 1.0e-9f) / (iterStop - iterStart);
        } else {
            gigaFlops[iter] = 0.0;
        }

        printf("Perf: %.3f Gflops/sec, Time= %.3f msec\r\n",
               gigaFlops[iter], 1000.0 * (iterStop - iterStart));

    }

    double mean_gflops, stddev_gflops, max_gflops, min_gflops;
    mean_and_stddev(gigaFlops, nIter, mean_gflops, stddev_gflops, max_gflops, min_gflops);

    printf("Size=, %d, %d, %d, %d, Pass=, %d, Iters=, %d, Max=, %.2f, GF/s, Avg_DGEMM=, %.2f, GFlop/s, StdDev=, %.2f, GFlop/s, %.2f, percent (Ignoring first iteration)\n",
           allSizes.numRowsA, allSizes.numColsA, allSizes.numRowsB, allSizes.numColsB,
           pass, nIter, max_gflops, mean_gflops, stddev_gflops, 100.0 * stddev_gflops / mean_gflops);

    int Cindex;
    for (int i = 0; i < mblocks; i++) {
        for (unsigned int k = 0; k < allSizes.numRowsC / mblocks; k++) {
            for (int j = i * nblocks; j < (i + 1)*nblocks; j++) {
                for (unsigned int l = 0; l < allSizes.numColsC / nblocks; l++) {
                    Cindex = allSizes.numColsC * (i * allSizes.numRowsC / mblocks + k) +
                             ((j - i * nblocks) * allSizes.numColsC / nblocks + l);
                    h_C[Cindex] = h_Cblock[j][l + k * allSizes.numColsC / nblocks];
                }
            }
        }
    }

    printf("Computing result using host CPU...");

    MATRIX_TYPE *reference = new MATRIX_TYPE[size_C];
    bool resACBLAS = true;

    // faster MKL host compute reference solution
#if 1
    memset(reference, 0, size_C * sizeof(MATRIX_TYPE));
    double refFlops = (2.0 * allSizes.numRowsC * allSizes.numColsC * allSizes.numColsA);
    double trefStart = dtimeGet();
    cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans,
                allSizes.numRowsA, allSizes.numColsB, allSizes.numColsA,
                1.0, h_A, allSizes.numColsA, h_B, allSizes.numColsB,
                0.0, reference, allSizes.numColsC);
    double trefStop = dtimeGet();
    resACBLAS = true;
    uint32_t num_mismatch = 0;
    for (unsigned int i = 0; i < size_C; ++i) {
        FP_DATA_TYPE diff = fabs(reference[i] - h_C[i]);
        if (reference[i] != 0) {
            diff /= reference[i];
        }
        if (diff > 1.0e-7) {
            num_mismatch++;
            if (resACBLAS) {
                printf("MKL dgemm: Detected problem at i=%d: ref %g actual %g\n",
                       i, reference[i], h_C[i]);
            }
            resACBLAS = false;
        }
    }

    if (resACBLAS) {
        printf("[If no MKLdgemm failures, then] Block Multiplication was successful.\r\n");
    } else {
        printf("ERROR: Mismatch between MKL's result and hStreams' result.\n"
               "ERROR: Number of mismatching elements: %u, number of matching elements: %u\n",
               num_mismatch, size_C - num_mismatch);
    }

    double mklHostRefFlops = (refFlops * 1.0e-9) / (trefStop - trefStart);
    printf("MKL Host DGEMM Perf, %.3f, GFlops/sec, Time= %.3f msec\r\n",
           mklHostRefFlops, 1000.0 * (trefStop - trefStart));
#endif
    //
    // Free Memory
    //
    delete [] reference;
    delete [] h_A;
    delete [] h_B;
    delete [] h_C;
    delete [] gigaFlops;
    delete [] eventAcpy;
    delete [] eventBcpy;
    delete [] eventCcpy;
    delete [] eventCset;

    for (int i = 0; i < totalblocks_A; ++i) {
        delete [] h_Ablock[i];
    }
    for (int i = 0; i < totalblocks_B; ++i) {
        delete [] h_Bblock[i];
    }
    for (int i = 0; i < totalblocks_C; ++i) {
        delete [] h_Cblock[i];
    }

    // true resACBLAS indicates all OK
    if (resACBLAS) {
        return 0;
    }
    return 1;
}

static HSTR_OPTIONS hstreams_options;

static int formerMain(int argc, char **argv)
{
    hStreams_GetCurrentOptions(&hstreams_options, sizeof(hstreams_options));
    hstreams_options.phys_domains_limit = 256;  // Limit to 256 domains
    hstreams_options.kmp_affinity = HSTR_KMP_AFFINITY_COMPACT;

    //Library to be loaded for sink-side code
    hstreams_options.libNameCntHost = 0; // Use the implicit name approach
    hstreams_options.libNamesHost = NULL;
    int nIter = 5, blocksize = 128;
    int mat_size_m = 512, mat_size_k = 512, mat_size_n = 5120;
    sMtxSize allSizes;

    ++pass;
    printf("*** C(mxn) = A(mxk) x B(kxn)\n");

    uint32_t use_host = 1, num_mics = 1;
    //int nstreams_host = 3, nstreams_mic = 3;
    int nstreams_host, nstreams_mic;
    int num_streams = 3;

    uint32_t CardsFound, NumActive;
    bool homog;

    while (argc) {
        if (*argv[0] == SWITCH_CHAR) {
            switch (*(argv[0] + 1)) {
            case 'b':
                blocksize = (int)atol(argv[0] + 2);
                break;

            case 'm':
                mat_size_m = (int)atol(argv[0] + 2);
                break;

            case 'k':
                mat_size_k = (int)atol(argv[0] + 2);
                break;

            case 'n':
                mat_size_n = (int)atol(argv[0] + 2);
                break;

            case 'i':
                nIter = (int)atol(argv[0] + 2);
                if (nIter < 3) {
                    nIter = 3;
                }
                break;

            case 's':
                num_streams = (int)atol(argv[0] + 2);
                break;

            case 'h':
                use_host = (int)atol(argv[0] + 2);
                break;

            case 'c':
                num_mics = (int)atol(argv[0] + 2);
                CHECK_HSTR_RESULT(hStreams_Init());
                CHECK_HSTR_RESULT(hStreams_GetNumPhysDomains(
                                      &CardsFound, &NumActive, &homog));
                if (CardsFound < num_mics) {
                    printf(
                        "Number of cards requested, %d, exceeds those in the system, %d.  Terminating.\n",
                        num_mics, CardsFound);
                    exit(-1);
                }
                if (num_mics > 2) {
                    printf(
                        "Number of cards requested, %d, exceeds the limit (2) of what this code was tested for.  Terminating.\n",
                        num_mics);
                    exit(-1);
                }

                break;

            case 'l':
                LB = (int)atol(argv[0] + 2);
                break;

            case 'v':
                loc_verbose = true;
                break;

            default:
                break;
            }
        }
        argc -= 1;
        argv++;
    }

    nstreams_host = num_streams;
    nstreams_mic = num_streams;

    // Manage LB default, if not set explicitly
    if (LB == -1) {
        if (num_mics > 0 && use_host > 0) {
            LB = 1;
        } else {
            LB = 0;
        }
    }
    if (LB) {
        printf("Load balancing across host and MIC cards is on.\n");
    }
    if (use_host == 0 && num_mics == 0) {
        printf("Cannot run if not using either host or MIC cards\n");
        exit(-1);
    }

    if (use_host == 1) {
        printf("Using the host CPU for compute.. and\n");
    }
    printf("Using %d MIC cards for compute..\n", num_mics);

    CHECK_HSTR_RESULT(hStreams_SetOptions(&hstreams_options));
    int mblocks, nblocks, kblocks;
    mblocks = (int)mat_size_m / blocksize;
    kblocks = (int)mat_size_k / blocksize;
    nblocks = (int)mat_size_n / blocksize;

    printf("blocksize = %d x %d, m = %d, k = %d, n = %d\n",
           blocksize, blocksize, mat_size_m, mat_size_k, mat_size_n);

    printf("mblocks = %d, kblocks = %d, nblocks = %d\n",
           mblocks, kblocks, nblocks);

    printf("Matrix in blocks A(%d,%d), B(%d,%d), C(%d,%d)\n",
           mblocks, kblocks, kblocks, nblocks, mblocks, nblocks);

    if ((mat_size_m != mat_size_k) || (mat_size_k != mat_size_n)) {
        printf("This code only works for square matrices for now..\n");
        exit(-1);
    }
    //the second arg is logStrPerParition! not perCard
    int num_doms = use_host + num_mics;
    int max_log_str = 0;

    if (use_host == 1 && num_mics >= 1) {
        if (LB) {
            nstreams_mic = use_host + num_mics + num_mics;

#ifdef HOST_HT_ON
            nstreams_host = 2 * nstreams_mic;
#else
            nstreams_host = nstreams_mic;
#endif

        } else {
            nstreams_mic = use_host + num_mics;

#ifdef HOST_HT_ON
            nstreams_host = 2 * nstreams_mic;
#else
            nstreams_host = nstreams_mic;
#endif

        }

        max_log_str = nstreams_mic;
    } else if (num_mics == 0) {
        max_log_str = nstreams_host;
#ifdef HOST_HT_ON
        nstreams_host = 2 * nstreams_host;
#endif
    } else if (use_host == 0) {
        max_log_str = nstreams_mic;
    }

    int host_ht_offset = 0;

#ifdef HOST_HT_ON
    host_ht_offset = nstreams_host - max_log_str;
#endif

    if (use_host) {
        printf("number of streams used on host = %d\n", nstreams_host);
    }
    if (num_mics > 0) {
        printf("number of streams used on each MIC = %d\n", nstreams_mic);
    }

    if (loc_verbose) {
        printf("if HT is enabled on host, only top half streams will be used\n");
        printf("if number of streams on host do not evenly divide with number of cores, performance can suffer\n");
        printf("number of streams used on mic = %d\n", nstreams_mic);
    }

    if (!(use_host == 1 && num_mics >= 1)) {
        if (LB) {
            printf("LB can only be activated if BOTH host CPU and MIC cards are used\n");
            exit(-1);
        }
    }

    //check if num_tiles are as per requirement
    if (LB) {
        if (use_host == 1 && num_mics == 1) {
            if (nblocks != 3)  {
                printf("number of tiles are not per requirement\n");
                printf("if LB is activated, and using CPU + 1 MIC, then nblocks must be 3\n");
                exit(-1);
            }
        } else if (use_host == 1 && num_mics == 2) {
            if (nblocks != 5)  {
                printf("number of tiles are not per requirement\n");
                printf("if LB is activated, and using CPU + 2 MICs, then nblocks must be 5\n");
                exit(-1);
            }
        }
    } else {
        if (use_host == 1 && num_mics >= 1) {
            if (nblocks != num_doms) {
                printf("number of tiles are not per requirement\n");
                printf("if LB is NOT activated, and using CPU and MIC cards, then nblocks must be equal to number of MIC cards+1\n");
                exit(-1);
            }
        }
    }

    if (use_host == 1) {
        resv_cpu_master = 1;
    } else {
        resv_cpu_master = 0;
    }

    HSTR_PHYS_DOM *physDomID = new HSTR_PHYS_DOM[num_doms];
    HSTR_LOG_DOM *logDomID = new HSTR_LOG_DOM[num_doms];

    HSTR_CPU_MASK out_CPUmask, src_hstr_cpu_mask;
    HSTR_PHYS_DOM *out_pPhysDomainID = new HSTR_PHYS_DOM;
    //HSTR_OVERLAP_TYPE *out_pOverlap = new HSTR_OVERLAP_TYPE;

    uint32_t *places = new uint32_t[num_doms];

    for (int i = 0; i < num_doms; ++i) {
        if (i == 0) {
            if (use_host == 1) {
                places[i] = nstreams_host;
                physDomID[i] = -1;
            } else {
                places[i] = nstreams_mic;
                physDomID[i] = i;
            }
        } else {
            places[i] = nstreams_mic;
            if (use_host == 1) {
                physDomID[i] = i - 1;
            } else {
                physDomID[i] = i;
            }
        }
    }

    int iret;
    if (resv_cpu_master) {
        HostCPUMask host_cpu_mask;
        host_cpu_mask.cpu_zero();

        for (int i = 0; i < num_resv_cpus; ++i) {
            host_cpu_mask.cpu_set(resv_cpus[i]);
        }

        HSTR_CPU_MASK_ZERO(src_hstr_cpu_mask);
        int ret = setCurrentProcessAffinityMask(host_cpu_mask);
        if (ret != 0){
            printf("setCurrentProcessAffinityMask failed, counted Gflops values may be inaccurate.");
        }

        int first, last, num_set;
        last = 0;
        first = HSTR_MAX_THREADS;
        num_set = 0;
        for (int i = 0; i < HSTR_MAX_THREADS; i++) {
            if (host_cpu_mask.cpu_isset(i)) {
                if (i < first) {
                    first = i;
                }
                last = i;
                num_set++;
                HSTR_CPU_MASK_SET(i, src_hstr_cpu_mask);
            }
        }
        if (loc_verbose) {
            printf("Reserving the following cpu_set for master on CPU\n");
            ShowLimitCPUmask(src_hstr_cpu_mask);
        }
    }

    if (resv_cpu_master) {
        iret = hStreams_custom_init_selected_domains(
                   num_doms,
                   physDomID,
                   num_doms,
                   places,
                   1,
                   src_hstr_cpu_mask);
    } else {
        iret = hStreams_app_init_selected_domains(
                   num_doms,
                   physDomID,
                   num_doms,
                   places,
                   1);
    }

    mkl_mic_disable();
    //10 max streams for printout
    HSTR_LOG_STR *out_pLogStreamIDs = new HSTR_LOG_STR[10];

    if (loc_verbose) {
        for (int idom = 0; idom < num_doms; ++idom) {
            CHECK_HSTR_RESULT(hStreams_GetLogDomainIDList(physDomID[idom], 1, &logDomID[idom]));
            CHECK_HSTR_RESULT(hStreams_GetLogDomainDetails(logDomID[idom], out_pPhysDomainID, out_CPUmask));
            //ShowLimitCPUmask(out_CPUmask);
            CHECK_HSTR_RESULT(hStreams_GetLogStreamIDList(logDomID[idom], places[idom], out_pLogStreamIDs));
            for (uint32_t i = 0; i < places[idom]; ++i) {
                CHECK_HSTR_RESULT(hStreams_GetLogStreamDetails(out_pLogStreamIDs[i], logDomID[idom], out_CPUmask));
                printf("streamId = %d\n", (int)out_pLogStreamIDs[i]);
                ShowLimitCPUmask(out_CPUmask);
            }
        }
    }

    if (iret != 0) {
        printf("hstreams_app_init failed!\r\n");
        exit(-1);
    }

    initializeMatrices(allSizes, mat_size_m, mat_size_k, mat_size_n);

    int matmult_code = matrixMultiply(allSizes, mblocks, kblocks, nblocks, nIter, num_doms, num_mics, use_host, max_log_str, host_ht_offset);

    CHECK_HSTR_RESULT(hStreams_app_thread_sync());

    CHECK_HSTR_RESULT(hStreams_app_fini());

    return matmult_code;

}

int main(int argc, char **argv)
{
    dtimeInit();
    return formerMain(argc, argv);
}
