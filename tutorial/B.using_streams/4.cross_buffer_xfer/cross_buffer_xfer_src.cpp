/*
 *   Copyright 2014-2016 Intel Corporation.
 *
 *   This file is subject to the Intel Sample Source Code License. A copy
 *   of the Intel Sample Source Code License is included.
 *
 */

/*
 * The goal of this test is to:
 *     a. Practice transferring from one buffer to another
 *     b. Eliminate a redundant copy operation
 *     c. Take a different approach to synchronization
 *
 **/

// hStreams header files needed now
// For source side
#include <hStreams_app_api.h> //user level apis

#include <stdio.h>
#include <omp.h>
#include <math.h>

#include "dtime.h" // A user-defined timing library

// A task for random initialization.
void rand_initialize(double *X, long int n)
{
    #pragma omp parallel for
    for (int i = 0; i < n; ++i) {
        X[i] = rand() % n + 0.1;
    }
}

void copy_from_X_to_Y(double *X, double *Y, long int n)
{
    #pragma omp parallel for
    for (int i = 0; i < n; ++i) {
        Y[i] = X[i];
    }
}

double compute(double *A, double *B, double *C, double *D, double *E, long int n)
{
    double sum = 0.0;
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < n; ++i) {
        E[i] = (A[i] * C[i]) / (B[i] * D[i]);
        sum += E[i];
    }
    return sum;
}

int main(int argc, char *argv[])
{
    //--------------------------------------------------------------
    // Set up initialization parameters
    // Number of streams to create in each domain: 2, instead of 1
    int streams_per_domain = 2;
    // no over-subscription, i.e., one logical stream per physical stream.
    int oversubscription = 1;
    CHECK_HSTR_RESULT(hStreams_app_init(streams_per_domain, oversubscription));
    //--------------------------------------------------------------

    //--------------------------------------------------------------
    // Size and allocate vectors
    // Size of the vectors.
    long int N = 50000000;
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
    rand_initialize(C, N);
    //--------------------------------------------------------------

    //--------------------------------------------------------------
    // Wrap all user-allocated memory in buffers
    // MUST DO: wrap memory in buffers, so heap addresses can get converted
    //  to their sink-side values, and dependences can be maintained
    CHECK_HSTR_RESULT(hStreams_app_create_buf(
                          (void *)A, N * sizeof(double)));
    CHECK_HSTR_RESULT(hStreams_app_create_buf(
                          (void *)B, N * sizeof(double)));
    CHECK_HSTR_RESULT(hStreams_app_create_buf(
                          (void *)C, N * sizeof(double)));
    CHECK_HSTR_RESULT(hStreams_app_create_buf(
                          (void *)D, N * sizeof(double)));
    CHECK_HSTR_RESULT(hStreams_app_create_buf(
                          (void *)E, N * sizeof(double)));
    //--------------------------------------------------------------

    //--------------------------------------------------------------
    // Prepare to invoke copy_from_X_to_Y, first using
    //   A and B as parameters on the sink-side
    // Arguments to pass to sink side. You need to pass 3
    // arguments, one scalar and two heap.
    uint64_t args0[3];

    // Send scalar arguments first.
    args0[0] = (uint64_t)(N);

    // Send heap arguments next. All heap arguments must be allocated
    //  on the sink side before you transfer them or use remote invocation.
    args0[1] = (uint64_t)(A);
    args0[2] = (uint64_t)(B);
    //--------------------------------------------------------------

    //--------------------------------------------------------------
    // Prepare to invoke copy_from_X_to_Y, next using
    //   C and D as parameters on the sink-side
    // Arguments to pass to sink side. You need to pass 3
    // arguments, one scalar and two heap.
    uint64_t args1[3];

    // Send scalar arguments first.
    args1[0] = (uint64_t)(N);

    // Send heap arguments next. All heap arguments must be allocated
    //  on the sink side before you transfer them or use remote invocation.
    args1[1] = (uint64_t)(C);
    args1[2] = (uint64_t)(D);
    //--------------------------------------------------------------

    //--------------------------------------------------------------
    // Prepare to invoke compute E task using
    //   A, B, C and D as parameters on the sink-side
    // Arguments to pass to sink side. You need to pass 6
    // arguments, 1 scalar and 5 heap.
    double sum = 0.0;

    uint64_t args2[6];
    args2[0] = (uint64_t) N;
    args2[1] = (uint64_t) A;
    args2[2] = (uint64_t) B;
    args2[3] = (uint64_t) C;
    args2[4] = (uint64_t) D;
    args2[5] = (uint64_t) E;
    //--------------------------------------------------------------

    //--------------------------------------------------------------
    // Begin timed section
    double tstart = dtimeGet();

    //--------------------------------------------------------------
    // Stream 0:  Move A, Copy from A to B
    // Stream 1:  Move C to D (no copy), wait for move to B, E=f(A,B,C,D)
    //%% Optional: you could also make changes like
    // Stream 0:  Move A to B (no copy)

    // Completion events to track completion of tasks.
    HSTR_EVENT completionA, // no need to carry dep within stream 0
               completionB, // carries dependence from stream 0 to stream 1
               completionC, // Move C from host to MIC is eliminated
               completionD, // no need to carry dep within stream 1
               completionE; // signals completion of all work

    // Transfer data from A at host to A on sink, similarly for C.
    // The transfer happens in two different streams
    CHECK_HSTR_RESULT(hStreams_app_xfer_memory(
                          0,                             // stream_id = 0
                          //%% Optional EXERCISE: copy from A directly to B, and skip the other
                          // copy_from_X_to_Y_sink_1 for remote A to remote B
                          A,                             // source proxy address to write
                          A,                             // source proxy address to read
                          N * sizeof(double),            // number of bytes to send
                          HSTR_SRC_TO_SINK,              // transfer direction
                          &completionA));                // completion event

    // Transfer data from C at host to C on sink.
    // The transfer happens in two different streams
    CHECK_HSTR_RESULT(hStreams_app_xfer_memory(
                          1,                             // stream_id = 1
                          C,                             // source proxy address to write
                          C,                             // source proxy address to read
                          N * sizeof(double),            // number of bytes to send
                          HSTR_SRC_TO_SINK,              // transfer direction
                          &completionC));                // completion event

    //  not needed: copy in same stream 0
    //!!a Transfer data from C at host directly to D on sink - one
    //  buffer to another
    CHECK_HSTR_RESULT(hStreams_app_xfer_memory(
                          1,                             // stream_id = 1
                          //%% EXERCISE: copy from C directly to D
                          % %,                            // source proxy address to write
                          % %,                            // source proxy address to read
                          N * sizeof(double),            // number of bytes to send
                          HSTR_SRC_TO_SINK,              // transfer direction
                          &completionD));                // completion event
    //  not needed: copy in same stream 1

    // Invoke "copy_from_X_to_Y_sink_1" on stream 0, with args0, completionB
    // practice enqueuing a task
    //%% Optional EXERCISE: Can also get rid of the app_invoke if move A to remote B
    CHECK_HSTR_RESULT(hStreams_app_invoke(
                          0,                             // stream_id = 0
                          "copy_from_X_to_Y_sink_1",     // sink-side function name
                          1,                             // number of scalar arguments
                          2,                             // number of heap arguments
                          args0,                         // arguments array
                          &completionB,                  // completion event
                          //  used for cross-stream sync
                          NULL,                          // ptr to return value
                          0));                           // return value size
    // Invoke "copy_from_X_to_Y_sink_1" on stream 1, with args1, completionD
    //!!b Since we transfer directly from C on host to D on sink, sink-side
    //  copy is redundant
    //%% EXERCISE: Get rid of the app_invoke
#if 1
    CHECK_HSTR_RESULT(hStreams_app_invoke(
                          1,                             // stream_id = 1
                          "copy_from_X_to_Y_sink_1",     // sink-side function name
                          1,                             // number of scalar arguments
                          2,                             // number of heap arguments
                          args1,                         // arguments array
                          &completionD,                  // completion event
                          //  not needed: compute in same stream
                          NULL,                          // ptr to return value
                          0));                           // return value size
#endif
    // Make stream 1 wait on completion event B that happened on stream 0,
    //  before invoking compute E.
    CHECK_HSTR_RESULT(hStreams_app_event_wait_in_stream(
                          1,                             // stream_id = 1
                          1,                             // number events to wait for
                          &completionB,                  // array of events to wait for
                          1,                             // number dependences induced
                          (void **)&B,                   // array of deps induced
                          NULL));                        // completion of the wait
    //  not needed

    // Compute E on the sink side. Sink will return sum as a return argument.
    // Like the app_event_wait_in_stream, this is also in stream 1,
    //  and since a dependence was induced on the variable B, and args2
    //  includes B, hStreams is smart enough to figure out that the
    //  dependence must be enforced
    CHECK_HSTR_RESULT(hStreams_app_invoke(
                          1,                             // stream_id = 1
                          "compute_sink_1",              // function to invoke
                          1,                             // number scalar args
                          5,                             // number heap args
                          args2,                         // array of args
                          &completionE,                  // completion event
                          &sum,                          // return variable
                          sizeof(double)));              // return var size

    //!!c Wait for specific completion event so you get correct data for sum
    // There were 3 choices; move from using app_event_wait on completionE
    //  in stream 1 to making the whole source thread wait on completion of all
    // NOTE: Another interesting exercise would be to do a timing comparison among these
    // three different kinds of synchronizations.
#if 0
    CHECK_HSTR_RESULT(hStreams_app_event_wait(
                          1,                             // number events to wait for
                          &completionE));                // array of events to wait for
#else
#if 1
    // An alternative is the heavy hammer of waiting for all in stream 1
    CHECK_HSTR_RESULT(hStreams_app_stream_sync(1));
#else
    //%% EXERCISE: switch from sync on stream to sync whole thread
    // An alternative is the heavier hammer of waiting for all in source thread
    CHECK_HSTR_RESULT(hStreams_app_thread_sync());
#endif
#endif

    //--------------------------------------------------------------

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
    //--------------------------------------------------------------

    //--------------------------------------------------------------
    // Cleanup before exiting.
    CHECK_HSTR_RESULT(hStreams_app_fini());
    delete []A;
    delete []B;
    delete []C;
    delete []D;
    delete []E;

    //--------------------------------------------------------------
}
