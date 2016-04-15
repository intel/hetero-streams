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

// hStreams header files not needed yet

#include <stdio.h>
#include <omp.h>
#include <math.h>

#include "dtime.h" // A user-defined timing library

// A task for random initialization.
void rand_initialize(double *X, long int n)
{
    #pragma omp parallel for
    for (long int i = 0; i < n; ++i) {
        X[i] = rand() % n + 0.1;
    }
}

void copy_from_X_to_Y(double *X, double *Y, long int n)
{
    #pragma omp parallel for
    for (long int i = 0; i < n; ++i) {
        Y[i] = X[i];
    }
}

double compute(double *A, double *B, double *C, double *D, double *E, long int n)
{
    double sum = 0.0;
    #pragma omp parallel for reduction(+:sum)
    for (long int i = 0; i < n; ++i) {
        if (B[i] != 0 && D[i] != 0) {
            E[i] = (A[i] * C[i]) / (B[i] * D[i]);
        }
        sum += E[i];
    }
    return sum;
}

int main(int argc, char *argv[])
{
    //--------------------------------------------------------------
    // Size and allocate vectors
    // Size of the vectors.
    long int N = 50000000;
    // For correctness checking
    double diff;
    // Take the vector size from command line.
    if (argc > 1) {
        N = atol(argv[1]);
        if (N <= 0) {
            N = 50000000;
        }
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
    //initialize A, and C with random values.
    rand_initialize(A, N);
    //%% EXERCISE: Do the same initialization for C
    rand_initialize(C, N);
    //--------------------------------------------------------------

    //--------------------------------------------------------------
    // Begin timed section
    double tstart = dtimeGet();

    //!!a Invoke re-factored functions
    //!!b Variables become arguments
    // Copy content of A into B.
    copy_from_X_to_Y(A, B, N);

    // Copy content of C into D.
    //%% EXERCISE: Do the same copy from C to D
    copy_from_X_to_Y(C, D, N);

    // Compute content of E based on A and D.
    //%% EXERCISE: call compute with A, B, C, D, E, N
    double sum = compute(A, B, C, D, E, N);

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
    // Report times
    printf("Time= %.3f seconds\n", (tend - tstart));

    delete []A;
    delete []B;
    delete []C;
    delete []D;
    delete []E;

}

