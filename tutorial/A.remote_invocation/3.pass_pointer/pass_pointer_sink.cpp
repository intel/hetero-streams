/*
 *    Copyright 2014-2016 Intel Corporation.
 *
 *    This file is subject to the Intel Sample Source Code License. A copy
 *    of the Intel Sample Source Code License is included.
 *
 */

// hStreams header files
// For sink side
#include <hStreams_sink.h>

// C standard input/output library
#include <stdio.h>

/*
 * This file is an example of a device (sink) side code.
 *  It receives the parameter passed to it from the source side, via an
 *   hStreams thunk on the sink side.
 * It converts the parameter back to the type that a value was intended to
 *  have and then print the value on the console.
 **/
HSTREAMS_EXPORT
void a3_sink_1(uint64_t a, uint64_t b, uint64_t A, uint64_t B)
{
    int n1 = (int) a;
    int n2 = (int) b;

    double *AA = (double *)A;
    int *BB = (int *)B;

    printf("On the side: received  n1=%d, n2=%d\n", n1, n2);
    printf("Sink-side addresses:    A=0x%016lx, B=0x%016lx\n", AA, BB);
}
