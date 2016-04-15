/*
 *   Copyright 2014-2016 Intel Corporation.
 *
 *   This file is subject to the Intel Sample Source Code License. A copy
 *   of the Intel Sample Source Code License is included.
 *
 */


/*
 * The goal of this exercise is to practice sending arguments to the sink side.
 * a. You will send two or three integer arguments to the sink side,
 * b. the sink-side will return their average back to host,
 * c. the host side waits for the value to come using an event wait, and
 * then prints the received value.
 */

// hStreams header files
// For source side
#include <hStreams_app_api.h>  // hStreams APIs

// Standard C header files
#include <stdio.h>

int main(int argc, char *argv[])
{
    //%% EXERCISE: Take note of this return argument name
    double avg = -0.1, diff;  // Variable to hold the average of the scalars.
    HSTR_EVENT out_Event; // Tracks completion of sink-side computation

    //--------------------------------------------------------------
    // Set up initialization parameters
    // Number of streams to create in each domain: 1
    int streams_per_domain = 1;
    // no over-subscription, i.e., one logical stream per physical stream.
    int oversubscription = 1;
    CHECK_HSTR_RESULT(hStreams_app_init(streams_per_domain, oversubscription));
    //--------------------------------------------------------------

    //--------------------------------------------------------------
    //!!a Set up 2 scalar arguments instead of one, no heap arguments
    const int num_scalar_values_to_send = ; //%% EXERCISE set to 2
    const int num_heap_values_to_send   = 0;

    // 1st scalar value to pass to sink side.
    int a = 1000;
    // 2nd scalar value to pass to sink side.
    int b = 500;
    //%% Optional: Add additional variables as a practice.

    // You can pass these values from command line.
    if (argc > 1) {
        a = atoi(argv[1]);
    }
    if (argc > 2) {
        b = atoi(argv[2]);
        //%% Optional: add more, if you like
    }

    // Arguments to pass to sink side. You need to pass two arguments.
    uint64_t args[num_scalar_values_to_send + num_heap_values_to_send];

    // Pack the first argument.
    args[0] = (uint64_t)(a);
    //Pack the second argument.
    args[1] = (uint64_t)(b);
    //%% Optional: Pack other arguments you want to send.
    //--------------------------------------------------------------

    //--------------------------------------------------------------
    // Invoke the "average_sink_1" app on the sink side on stream 0.
    // NOTE: sizes should match to the actual allocated size, otherwise you will get segfaults
    CHECK_HSTR_RESULT(hStreams_app_invoke(
                          0,                             // stream_id = 0
                          "average_sink_1", // sink-side function name
                          num_scalar_values_to_send,     //!!a Changed above, as a variable
                          num_heap_values_to_send,       // number of heap arguments
                          args,                          // arguments array
                          &out_Event,                    // completion event
                          //  used to check result availability
                          //%% EXERCISE: replace <return variable> with the name of the return arg
                          &<return variable>,                          //!!b Point to the blob to return
                          sizeof(double)));              //!!b Give the blob size to return
    //--------------------------------------------------------------

    //--------------------------------------------------------------
    //!!d Wait for the result to be back.
    //%% EXERCISE: Use hStreams_app_event_wait in stream 1, with &out_Event
    CHECK_HSTR_RESULT( % %);
    //--------------------------------------------------------------

    // Check the value on the host side.
    printf("Average received on host-side, from the sink-side: %f\n", avg);
    diff = avg - (a + b) / 2;
    if (diff < 0) {
        diff = -diff;
    }
    if (diff > 0.001) {
        printf("Error.  You need to sync on completion of remote function.\n");
    } else {
        printf("Passed.\n");
    }

    //--------------------------------------------------------------
    // Cleanup before exiting.
    CHECK_HSTR_RESULT(hStreams_app_fini());
    //--------------------------------------------------------------
}
