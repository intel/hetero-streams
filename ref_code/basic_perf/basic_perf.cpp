/*
 * Copyright 2014-2016 Intel Corporation.
 *
 * This file is subject to the Intel Sample Source Code License. A copy
 * of the Intel Sample Source Code License is included.
 */

//********************************************************************************
// Author:   CJ Newburn
// Purpose:  Very basic directed hStreams performance tests
// Created:  8/12/14
// API level:
//  app_api, convenience functions in hStreams_app_api.h,
//  rather than the core APIs in hStreams_source.h
// Functionality exercised
//   version 1.0
//     init
//     create_bufs
//     xfer_memory
//     thread_sync
//     dgemm
//     finit
//   future versions
//     memset, but called from the remote side
//     invoke
//     event_wait
//     stream_sync
//     event_wait_in_stream
// What to show, for the most basic performance
//   Repeated transfers to   remote domain, in 1 and MAX_CONCURRENCY streams
//   Repeated transfers from remote domain, in 1 and MAX_CONCURRENCY streams
//   Repeated transfer to, compute at, and transfer back from remote domain,
//     in 1 and MAX_CONCURRENCY streams
// Structure of driver app
//   For each of two concurrencies, 1 and MAX_CONCURRENCY (e.g. 4)
//    Walk through the sequence of events of interest
//    Time the interesting parts and report it
//   Illustrate other functionality
//
//********************************************************************************

// includes used for this driver
#include <stdio.h>
#ifndef _WIN32
#include <unistd.h>
#endif
#include <stdlib.h>
#include <string.h>

#include <mkl.h>
#include <math.h>

// This is the only include needed for hStreams
#include <hStreams_app_api.h>
// This is convenient functionality for timing, that's available with hStreams
#include "dtime.h"

// Driver parameters
#define SIZE 4096                               // size of each 2D matrix dimension
#define DATA_TYPE double                        // double, float
#define HSTR_APP_XGEMM hStreams_app_dgemm       // *gemm type
#define MAX_CONCURRENCY 4                       // number of streams per domain (card)
#define ITERATIONS 100                          // timing iterations

int main()
{
    uint32_t places_per_domain = MAX_CONCURRENCY;
    uint32_t logical_streams_per_place = 1; // keep it simple
    uint32_t stream;
    double timeBegin, timeEnd;
    int iters;
    // DATA_TYPE is set above
    // MAX_CONCURRENCY enables working with many of these at a time
    DATA_TYPE **A[MAX_CONCURRENCY],
              **B[MAX_CONCURRENCY],
              **C[MAX_CONCURRENCY];
    int conc;

    dtimeInit();
    // This is non-performant, but easier to read
    for (conc = 0; conc < MAX_CONCURRENCY; conc++) {
        A[conc] = (DATA_TYPE **)malloc(sizeof(DATA_TYPE) * SIZE * SIZE);
        B[conc] = (DATA_TYPE **)malloc(sizeof(DATA_TYPE) * SIZE * SIZE);
        C[conc] = (DATA_TYPE **)malloc(sizeof(DATA_TYPE) * SIZE * SIZE);
    }

    // Iterate through number of places_per_domain: 1, MAX_CONCURRENCY
    for (places_per_domain = 1; places_per_domain <= MAX_CONCURRENCY;
            places_per_domain *= MAX_CONCURRENCY) {

        //     init
        printf(">>init\n");
        CHECK_HSTR_RESULT(
            hStreams_app_init(
                places_per_domain,
                logical_streams_per_place));

        //     create_bufs
        printf(">>create bufs\n");

        // Walk through streams
        for (stream = 0; stream < places_per_domain; stream++) {

            CHECK_HSTR_RESULT(
                hStreams_app_create_buf(A[stream], sizeof(DATA_TYPE)*SIZE * SIZE));
            CHECK_HSTR_RESULT(
                hStreams_app_create_buf(B[stream], sizeof(DATA_TYPE)*SIZE * SIZE));
            CHECK_HSTR_RESULT(
                hStreams_app_create_buf(C[stream], sizeof(DATA_TYPE)*SIZE * SIZE));

        }


        //------     Begin xfer_memory TO domain     -------------------
        printf(">>xfer mem to remote domain\n");

        // Begin timing
        timeBegin = dtimeGet();

        // Walk through timing iterations
        for (iters = 0; iters < ITERATIONS; iters++) {

            // Walk through streams in a single domain
            for (stream = 0; stream < places_per_domain; stream++) {

                // Transfer to remote domain
                CHECK_HSTR_RESULT(
                    hStreams_app_xfer_memory(stream,           // Logical stream
                                             A[stream], A[stream], // src & dest addr, as though in source domain
                                             sizeof(DATA_TYPE)*SIZE * SIZE,
                                             HSTR_SRC_TO_SINK, // XferDirection
                                             NULL));           // completion event; wait for thread vs. indiv xfer
                CHECK_HSTR_RESULT(
                    hStreams_app_xfer_memory(stream,           // Logical stream
                                             B[stream], B[stream], // src & dest addr, as though in source domain
                                             sizeof(DATA_TYPE)*SIZE * SIZE,
                                             HSTR_SRC_TO_SINK, // XferDirection
                                             NULL));           // completion event; wait for thread vs. indiv xfer
            }
        }

        //     thread_sync just once, vs. every iteration
        CHECK_HSTR_RESULT(
            hStreams_app_thread_sync());

        // End timing
        timeEnd = dtimeGet();

        printf("Transfer A and B to remote domain: %.3f ms for %d iterations, %d streams\n",
               1.0e3 * (timeEnd - timeBegin), ITERATIONS, stream);
        if (timeEnd != timeBegin) {
            printf("Host-card data rate (%d streams, %d-square) = %.3f GB/s\n",
                   stream, SIZE,
                   1.0e-9 * sizeof(DATA_TYPE)*SIZE * SIZE * ITERATIONS * 2 *
                   places_per_domain / (timeEnd - timeBegin)); // *2 is for A+B
        }
        //------     End xfer_memory TO domain     -------------------

        //------     Begin xfer_memory FROM domain     -------------------
        printf(">>xfer mem from remote domain\n");

        // Begin timing
        timeBegin = dtimeGet();

        // Walk through timing iterations
        for (iters = 0; iters < ITERATIONS; iters++) {

            // Walk through streams in a single domain
            for (stream = 0; stream < places_per_domain; stream++) {

                // Transfer to remote domain
                CHECK_HSTR_RESULT(
                    hStreams_app_xfer_memory(stream,           // Logical stream
                                             A[stream], A[stream], // src & dest addr, as though in source domain
                                             sizeof(DATA_TYPE)*SIZE * SIZE,
                                             HSTR_SINK_TO_SRC, // XferDirection
                                             NULL));           // completion event; wait for thread vs. indiv xfer
                CHECK_HSTR_RESULT(
                    hStreams_app_xfer_memory(stream,           // Logical stream
                                             B[stream], B[stream], // src & dest addr, as though in source domain
                                             sizeof(DATA_TYPE)*SIZE * SIZE,
                                             HSTR_SINK_TO_SRC, // XferDirection
                                             NULL));           // completion event; wait for thread vs. indiv xfer
            }
        }

        //     thread_sync just once, vs. every iteration
        CHECK_HSTR_RESULT(
            hStreams_app_thread_sync());

        // End timing
        timeEnd = dtimeGet();
        printf("Transfer C from remote domain: %.3f ms for %d iterations, %d streams\n",
               1.0e3 * (timeEnd - timeBegin), ITERATIONS, stream);
        if (timeEnd != timeBegin) {
            printf("Card-host data rate (%d streams, %d-square) = %.3f GB/s\n",
                   stream, SIZE,
                   1.0e-9 * sizeof(DATA_TYPE)*SIZE * SIZE * ITERATIONS * 2 *
                   places_per_domain / (timeEnd - timeBegin)); // *2 is for A+B
        }
        //------     End xfer_memory FROM domain     -------------------

        //------     Begin xGEMMs in remote domain     -------------------
        printf(">>xgemm - sample fixed functionality\n");
        double alpha = 1.0;
        double beta  = 1.0;

        // Begin timing
        timeBegin = dtimeGet();

        // Walk through timing iterations
        for (iters = 0; iters < ITERATIONS; iters++) {

            // Walk through streams in a single domain
            for (stream = 0; stream < places_per_domain; stream++) {

                // Nothing special about use of macros for DATA_TYPE and HSTR_APP_XGEMM
                //  These are just used here so it's easier to change types and function names
                CHECK_HSTR_RESULT(
                    HSTR_APP_XGEMM(stream,
                                   CblasColMajor, CblasNoTrans, CblasNoTrans,
                                   SIZE, SIZE, SIZE,  // M, N, K - square in this case
                                   alpha,
                                   (DATA_TYPE *)A[stream], SIZE,
                                   (DATA_TYPE *)B[stream], SIZE, beta,
                                   (DATA_TYPE *)C[stream], SIZE,
                                   NULL));  // completion event; wait for thread vs. indiv computes

            } // for all streams
        } // for all iterations

        //     thread_sync just once, vs. every iteration
        CHECK_HSTR_RESULT(
            hStreams_app_thread_sync());

        // End timing
        timeEnd = dtimeGet();
        //get Gflops
        printf("Remote domain xgemm time= %.3f ms for %d iterations, using %d streams\n",
               1.0e3 * (timeEnd - timeBegin), ITERATIONS, stream);
        if (timeEnd != timeBegin) {
            double flopsPerMatrixMul = 2.0 * (double)SIZE * (double)SIZE * (double)SIZE;
            double gigaFlops = (flopsPerMatrixMul * 1.0e-9f) * stream * ITERATIONS / (timeEnd - timeBegin);
            printf("Remote domain dgem data rate (%d streams, %d-square) = %.3f GFl/s\n",
                   stream, SIZE, gigaFlops);
        }
        //------     End xGEMMs in remote domain     -------------------

        //     fini
        printf(">>fini\n");
        CHECK_HSTR_RESULT(
            hStreams_app_fini()); // Finalize before re-init with diff partition


    } // iterate through places_per_domain, printing performance for each

    exit(0);

}

