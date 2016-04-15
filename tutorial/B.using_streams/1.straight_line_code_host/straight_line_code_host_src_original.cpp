/*
 *   Copyright 2014-2016 Intel Corporation.
 *
 *   This file is subject to the Intel Sample Source Code License. A copy
 *   of the Intel Sample Source Code License is included.
 *
 */

/*
 * The goal of this exercise is to show performance difference of a task-based
 *  vs. a straight-line code. Task based factorization of a program is
 *  something that we need to do for hStreams. However, there is nothing
 *  specific to hstreams at this point. We will evolve this code to
 *  add hStreams
 */

// hStreams header files not needed yet, but will in a later exercise

#include <stdio.h>
#include <omp.h>
#include <math.h>

//!!b include header file for timing routine
#include "dtime.h" // A user-defined timing library

int main(int argc, char *argv[])
{
    //--------------------------------------------------------------
    // Size and allocate vectors
    // Size of the vectors.
    long int N = 50000000;
    double diff;
    // Take the vector size from command line.
    if (argc > 1) {
        N = atol(argv[1]);
    }

    // Some 1D vectors.
    double *A, *B, *C, *D, *E;

    A = new double[N];
    B = new double[N];
    C = new double[N];
    D = new double[N];
    E = new double[N];
    //--------------------------------------------------------------

    //--------------------------------------------------------------
    // initialize A, and C with random values.
    // set seed for rand()
    srand(02272015);

    // NOTE: If you do not use pragma omp parallel in the for loops, the program may
    // take quite a long time to get executed, specially for large N. You can try it
    // out yourself.
    #pragma omp parallel for
    // rand() may not be thread safe, but let's not worry about that
    for (long int i = 0; i < N; ++i) {
        A[i] = rand() % N + 0.1;
        C[i] = rand() % N + 0.1;
    }
    //--------------------------------------------------------------

    //--------------------------------------------------------------
    // Begin timed section
    //!!b dtimeGet returns a double, in seconds
    double tstart = dtimeGet();

    // Copy content of A into B.
    #pragma omp parallel for
    for (long int i = 0; i < N; ++i) {
        B[i] = A[i];
    }

    // Copy content of C into D.
    #pragma omp parallel for
    for (long int i = 0; i < N; ++i) {
        D[i] = C[i];
    }

    //!!a All of this is straight-line code, no tasks
    // Compute content of E based on A, B, C and D.
    double sum = 0.0;
    #pragma omp parallel for reduction(+:sum)
    for (long int i = 0; i < N; ++i) {
        E[i] = (A[i] * C[i]) / (B[i] * D[i]);
        sum += E[i];
    }
    //!!b dtimeGet returns a double, in seconds
    double tend = dtimeGet();
    // End timed section
    //--------------------------------------------------------------

    printf("Sum: %f\n", sum);
    diff = sum - N;

    if (fabs(diff) > 0.001) {
        printf("Error in checksum.\n");
    } else {
        printf("Passed.\n");
    }
    //!!b Report times
    printf("Time= %.3f seconds\n", (tend - tstart));

    delete []A;
    delete []B;
    delete []C;
    delete []D;
    delete []E;

}

