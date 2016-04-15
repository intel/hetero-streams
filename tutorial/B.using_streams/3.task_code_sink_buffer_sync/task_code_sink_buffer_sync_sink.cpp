/*
 *    Copyright 2014-2016 Intel Corporation.
 *
 *    This file is subject to the Intel Sample Source Code License. A copy
 *    of the Intel Sample Source Code License is included.
 *
 */

// Basic hStreams sink side library
#include <hStreams_sink.h>

// C standard input/output library
#include <stdio.h>
#include <omp.h>
#include <string.h>
#include <math.h>
/*
 * This file is an example of a device (sink) side code.
 * It receives the value passed to it from the value, converts it back to
 * the type that a value was intended to, perform the action in the task
 * and return the value (if any). This code also demonstrate how to use
 * other parallel constructs like openMP or cilkplus inside a task code
 * to provide additional thread level parallelism.
 **/

// A sink-side version of a task that copy from a source vector X to a destination vector Y.
HSTREAMS_EXPORT
void copy_from_X_to_Y_sink_1(uint64_t a, uint64_t A, uint64_t B)
{
    long int N = (long int) a;

    double *AA = (double *)A;
    double *BB = (double *)B;
    #pragma omp parallel for
    for (int i = 0; i < N; ++i) {
        BB[i] = AA[i];
    }
}

// A sink-side version of a task that computes the content of an array based on some other arrays. It also returns a value at the end of the computation. The return value is the 20th parameter and its size is the 21st one in hstreams.
HSTREAMS_EXPORT
void compute_sink_1(uint64_t N, uint64_t AA, uint64_t BB, uint64_t CC, uint64_t DD,
                    uint64_t EE,  uint64_t, uint64_t,             /*  6-8 */
                    uint64_t, uint64_t, uint64_t, uint64_t,       /*  9-12 */
                    uint64_t, uint64_t, uint64_t, uint64_t,       /* 13-16 */
                    uint64_t, uint64_t, uint64_t,                 /* 17-19 */
                    void *ret_val,                                /* return value */
                    uint16_t ret_val_size                         /* size of return value */
                   )
{
    double sum = 0.0;
    double *A = (double *) AA;
    double *B = (double *) BB;
    double *C = (double *) CC;
    double *D = (double *) DD;
    double *E = (double *) EE;
    long int n = (long int) N;
    #pragma omp parallel for reduction(+:sum)
    for (long int i = 0; i < n; ++i) {
        if (B[i] != 0 && D[i] != 0) {
            E[i] = (A[i] * C[i]) / (B[i] * D[i]);
        }
        sum += E[i];
    }
    memcpy(ret_val, (void *)&sum, ret_val_size);
}

