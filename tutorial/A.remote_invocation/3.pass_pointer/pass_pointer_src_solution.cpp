/*
 *   Copyright 2014-2016 Intel Corporation.
 *
 *   This file is subject to the Intel Sample Source Code License. A copy
 *   of the Intel Sample Source Code License is included.
 *
 */


/*
 * The goal of this exercise is to show
 *  a) that the user allocates their own memory
 *  b) how user-allocated memory is wrapped in buffers
 *  c) how data is transferred from host to MIC
 *  d) how heap arguments are passed, in addition to scalar args
 **/

// hStreams header files
// For source side
#include <hStreams_app_api.h>  // hStreams APIs

// Standard C header files
#include <stdio.h>

int main(int argc, char *argv[])
{

    //--------------------------------------------------------------
    // Set up initialization parameters
    // Number of streams to create in each domain: 1
    int streams_per_domain = 1;
    // no over-subscription, i.e., one logical stream per physical stream.
    int oversubscription = 1;
    CHECK_HSTR_RESULT(hStreams_app_init(streams_per_domain, oversubscription));
    //--------------------------------------------------------------

    //--------------------------------------------------------------
    // Create arrays of a given size
    // Size of the arrays/buffers A and B.
    // You need to pass the size in addition to the pointers.
    const int n1 = 1000;
    const int n2 = 500;
    //!!a User allocation of arrays/buffers on the host side.
    double *A; // First pointer/heap address to pass to the sink-side.
    int    *B; // Second pointer/heap address to pass to the sink-side.
    A = new double[n1];
    B = new int[n2];
    printf("Source proxy addresses: A=0x%016lx, B=0x%016lx\n", A, B);
    //--------------------------------------------------------------

    //--------------------------------------------------------------
    //!!b MUST DO: wrap memory in buffers, so heap addresses can get converted
    //  to their sink-side values, and dependences can be maintained
    CHECK_HSTR_RESULT(hStreams_app_create_buf(
                          (void *)A, n1 * sizeof(double)));
    //%% EXERCISE: Do the same for B, with n2 and sizeof(int)
    CHECK_HSTR_RESULT(hStreams_app_create_buf(
                          (void *)B, n2 * sizeof(int)));
    //--------------------------------------------------------------

    //--------------------------------------------------------------
    //!!c Transfer the data from the source to the sink
    CHECK_HSTR_RESULT(hStreams_app_xfer_memory(
                          0,                             // stream_id = 0
                          A,                             // source proxy address to write
                          A,                             // source proxy address to read
                          n1 * sizeof(double),           // number of bytes to send
                          HSTR_SRC_TO_SINK,              // transfer direction
                          NULL));                        // completion event
    //  NULL means ignore
    //%% EXERCISE Likewise, transfer B, with n2 * sizeof(int)
    CHECK_HSTR_RESULT(hStreams_app_xfer_memory(
                          0,                             // stream_id = 0
                          B,                             // source proxy address to write
                          B,                             // source proxy address to read
                          n2 * sizeof(int),              // number of bytes to send
                          HSTR_SRC_TO_SINK,              // transfer direction
                          NULL));                        // completion event
    //  NULL means ignore
    //--------------------------------------------------------------

    //--------------------------------------------------------------
    //!!d Set up arguments, including heap arguments
    // Arguments to pass to sink side. You need to pass four arguments
    //  (2 scalar and 2 heap).
    //%% Optional: Modify this to send more
    const int num_scalar_values_to_send = 2;
    const int num_heap_args_to_send     = 2;

    uint64_t args[num_scalar_values_to_send + num_heap_args_to_send];

    // Send scalar arguments first.
    args[0] = (uint64_t)(n1);
    //%% EXERCISE: set the other scalar argument.
    args[1] = (uint64_t)(n2);

    // Send heap arguments next. All heap arguments must be wrapped in
    //  buffers, which allocates on the sink side, before you invoke remotely.
    args[2] = (uint64_t)(A);
    //%% EXERCISE: Set the other heap argument for B.
    args[3] = (uint64_t)(B);
    //%% Optional: Modify this to send more
    //--------------------------------------------------------------

    //--------------------------------------------------------------
    //%% EXERCISE: Call app_invoke to execute (a3_sink_1).
    //%%  Make it have no return value, completion event does not matter
    CHECK_HSTR_RESULT(hStreams_app_invoke(
                          0,                             // stream_id = 0
                          "a3_sink_1",                   // sink-side function name
                          num_scalar_values_to_send,     // number of scalar args
                          num_heap_args_to_send,         // number of heap arguments
                          args,                          // arguments array
                          NULL,                          // completion event
                          NULL,                          // ptr to return value
                          0));                           // return value size
    //--------------------------------------------------------------

    //--------------------------------------------------------------
    // Cleanup before exiting.
    CHECK_HSTR_RESULT(hStreams_app_fini());
    //--------------------------------------------------------------
    delete A, B;

}

