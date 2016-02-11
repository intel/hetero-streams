/*
 * Copyright 2014-2016 Intel Corporation.
 *
 * This file is subject to the Intel Sample Source Code License. A copy
 * of the Intel Sample Source Code License is included.
 */

//********************************************************************************
// Derived from basic_perf.cpp
// For charting IO performance under various conditions.
// As a test of simple data transfer bandwidth under various conditions, this test
// creates a single buffer per (possibly only) stream, queues a number of transfers,
// then waits for all to complete. No test of data validity is made.
// The usual cache-warmup measures are taken.
// At the conclusion, the total bandwidth and runtime parameters are output in a
// CSV format friendly to excel import for charting. If errors are encountered,
// the token "FAILED" is emitted, and the test exits returning nonzero.
//
//
// API level:
//  app_api, convenience functions in hStreams_app_api.h,
//  rather than the core APIs in hStreams_source.h
// Functionality exercised
//     init
//     create_bufs
//     xfer_memory
//     thread_sync
//     fini
//
//      USAGE: io_perf [-b buffer-size] [-s concurrent-streams] [-i iterations] [-f] [-v]
//
//********************************************************************************

//
// Headers
//
#include <stdio.h>
#ifndef _WIN32
#include <unistd.h>
#endif
#include <stdlib.h>
#include <string.h>

#include <mkl.h>
#include <math.h>

#include <hStreams_app_api.h>
#include "dtime.h"  // elapsed time measurement.

//
// Default parameters
//
#define BUFSIZE 1024*1024                       // Size of each buffer
#define NSTREAMS 1                              // Number of streams per domain (card)
#define ITERATIONS 1000                         // Timing iterations
#define MAXSTREAMS  256                         // Because windows c++ sucks

//
// Fwd decls.
//
static void getparams(int argc, char **argv);
static void usage(const char *why);

//
// Cmdline params.
//
char *myname = "noname";
uint64_t bufsize = BUFSIZE;
int nstreams = NSTREAMS;
int iterations = ITERATIONS;
HSTR_XFER_DIRECTION xfer_direction = HSTR_SRC_TO_SINK;
bool verbose = false;

int main(int argc, char **argv)
{
    int stream;
    double timeBegin, timeEnd, elapsed, rate;
    int iters;
    int conc;
    HSTR_RESULT hstream_result;


    //
    // Parse args.
    //
    getparams(argc, argv);

#ifdef _WIN32
    unsigned char **A[MAXSTREAMS];
#else
    unsigned char **A[nstreams];
#endif

    dtimeInit();

    //
    // Alloc buffers
    //
    for (conc = 0; conc < nstreams; conc++) {
        A[conc] = (unsigned char **)malloc(bufsize);
        memset(A[conc], 0x5A, bufsize); // Factor out caching.
    }
    //
    //     init hstreams
    //
    if (verbose) {
        printf("init\n");
    }
    hstream_result = hStreams_app_init(nstreams, 1);
    if (hstream_result != HSTR_RESULT_SUCCESS) {
        printf("FAILED\n");
        exit(2);
    }


    //
    // Create bufs for each stream
    //
    if (verbose) {
        printf("create bufs\n");
    }
    for (stream = 0; stream < nstreams; stream++) {
        hstream_result = hStreams_app_create_buf(A[stream], bufsize);
        if (hstream_result != HSTR_RESULT_SUCCESS) {
            printf("FAILED\n");
            exit(3);
        }
    }

    //
    // Factor out caching.
    //
    if (verbose) {
        printf("caching xfer\n");
    }
    for (stream = 0; stream < nstreams; stream++) {
        hstream_result = hStreams_app_xfer_memory(
                             stream, A[stream], A[stream], bufsize, xfer_direction, NULL);
        if (hstream_result != HSTR_RESULT_SUCCESS) {
            printf("FAILED\n");
            exit(4);
        }
    }

    hstream_result = hStreams_app_thread_sync();
    if (hstream_result != HSTR_RESULT_SUCCESS) {
        printf("FAILED\n");
        exit(5);
    }

    //
    // Begin xfer_memory
    //
    if (verbose) {
        printf("xfer mem %s remote domain\n", xfer_direction ? "to" : "from");
    }

    timeBegin = dtimeGet();

    //
    // Transfer repetitions.
    //
    for (iters = 0; iters < iterations; iters++) {

        //
        // Iterate over "concurrent" streams.
        // Note that with only one DMA channel available, transfers between host and card are
        // actually serialized.
        // We wait for all per-stream and iteration transfers after all have been submitted.
        //
        for (stream = 0; stream < nstreams; stream++) {
            hstream_result = hStreams_app_xfer_memory(
                                 stream, A[stream], A[stream], bufsize, xfer_direction, NULL);
            if (hstream_result != HSTR_RESULT_SUCCESS) {
                printf("FAILED\n");
                exit(6);
            }
        }
    }

    //
    // Wait for all transfers once after all have been queued.
    //
    hstream_result = hStreams_app_thread_sync();
    if (hstream_result != HSTR_RESULT_SUCCESS) {
        printf("FAILED\n");
        exit(7);
    }

    //
    // End timing
    //
    timeEnd = dtimeGet();

    //
    // Figure and output results,
    // suitable for csv import into excel charts.
    //
    elapsed = timeEnd - timeBegin;
    if (elapsed > 0.0) {
        rate = 1.0e-9 * bufsize * iterations * nstreams / elapsed;
    } else {
        rate = 0.0;
    }

    //
    // NSTREAMS BUFSIZE DIRECTION ITERS SECS GB/S
    //
    printf("%d,%ld,%s-remote,%d,%.3f,%.3f\n",
           nstreams, bufsize, xfer_direction ? "to" : "from", iterations, elapsed, rate);

    //
    //     Finalize
    //
    if (verbose) {
        printf("fini\n");
    }
    hstream_result = hStreams_app_fini();
    if (hstream_result != HSTR_RESULT_SUCCESS) {
        printf("FAILED\n");
        exit(8);
    }

    //
    // Normal completion.
    //
    exit(0);

}

//
// Process command line options.
// Called from main.
// Shoulda used getopt().
//
static void
getparams(int argc, char **argv)
{
    int arg;
    char *argp;

    myname = argv[0];

    //
    // Scan the arglist.
    //
    for (arg = 1; arg < argc; ++arg) {
        argp = argv[arg];

        if (argp[0] != '-') {
            usage("missing \'-\'");
        }

        switch (argp[1])  {


        //
        // -b <buffer size>
        //
        case 'b':
            if (argp[2]) {
                usage(argp);
            }
            argp = argv[++arg];
            if ((arg == argc) || (argp[0] == '-')) {
                usage("missing integer parameter to -b");
            }
            bufsize = atol(argp);
            break;

        //
        // -s <concurrent streams>
        //
        case 's':
            if (argp[2]) {
                usage(argp);
            }
            argp = argv[++arg];
            if ((arg == argc) || (argp[0] == '-')) {
                usage("missing integer parameter to -c");
            }
            nstreams = atoi(argp);
            break;

        //
        // -i <iterations>
        //
        case 'i':
            if (argp[2]) {
                usage(argp);
            }
            argp = argv[++arg];
            if ((arg == argc) || (argp[0] == '-')) {
                usage("missing integer parameter to -i");
            }
            iterations = atoi(argp);
            break;

        //
        // -v
        //
        case 'v':
            if (argp[2]) {
                usage(argp);
            }
            verbose = true;
            break;

        //
        // -f
        //
        case 'f':
            if (argp[2]) {
                usage(argp);
            }
            xfer_direction = HSTR_SINK_TO_SRC;
            break;

        default:
            fprintf(stderr, "unknown option \'%s\'", argp);
            usage("Unknown option");
            break;
        }
    }

    if (verbose) printf("\n\tNSTREAMS:\t%d\n\tITERATIONS:\t%d\n\tBUFSIZE:\t%ld\n\tDIR:\t\t%s remote\n",
                            nstreams, iterations, bufsize, xfer_direction ? "to" : "from");


}

//
// Print error hint and explain usage, then exit.
//
static void
usage(const char *why)
{
    fprintf(stderr, "Command line error: %s\n\nUSAGE: %s [-b buffer-size] [-s concurrent streams] [-i iterations] [-f(rom remote)] [-v(erbose)]\n\n",
            why, myname);
    exit(1);
}

