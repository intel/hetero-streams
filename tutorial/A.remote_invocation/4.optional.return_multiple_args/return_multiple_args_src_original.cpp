/*
 *   Copyright 2014-2016 Intel Corporation.
 *
 *   This file is subject to the Intel Sample Source Code License. A copy
 *   of the Intel Sample Source Code License is included.
 *
 */


/*
 * The goal of this exercise is to practice
 *  a) using a non-standard sink-side name and location
 *  b) returning multiple arguments from the sink side
*/

// hStreams header files
// For source side
#include <hStreams_app_api.h>  // hStreams APIs

// Standard C header files
#include <stdio.h>

int main(int argc, char *argv[])
{
    // Variable to hold the average, min and max of the scalars.
    double ret_val[3];
    // Tracks whether the computation was done on the sink side.
    HSTR_EVENT out_Event;

    //--------------------------------------------------------------
    //!!a Point to a non-standard sink file name
    // Library to be loaded for sink-side:
    // a4_sink_1.so for x100 architecture (for ISA type HSTR_ISA_KNC)
    // a4_sink_2.so for x200 architecture (for ISA type HSTR_ISA_KNL)

    //!!%% EXERCISE: fill in ISA type
    HSTR_ISA_TYPE isaTypeKNC = % %;
    HSTR_ISA_TYPE isaTypeKNL = % %;

    //!!%% EXERCISE: fill in number of libraries you're adding
    uint32_t numLibNames = % %; // one library to load on each architecture.

    //!!%% EXERCISE: fill in strings
    char *libNamesKNC[] = { % % };
    char *libNamesKNL[] = { % % };

    CHECK_HSTR_RESULT(hStreams_SetLibrariesToLoad(HSTR_ISA_KNC, numLibNames, libNamesKNC, NULL));
    CHECK_HSTR_RESULT(hStreams_SetLibrariesToLoad(HSTR_ISA_KNL, numLibNames, libNamesKNL, NULL));
    //--------------------------------------------------------------

    //--------------------------------------------------------------
    // Set up initialization parameters
    // Number of streams to create in each domain: 1
    int streams_per_domain = 1;
    // no over-subscription, i.e., one logical stream per physical stream.
    int oversubscription = 1;
    CHECK_HSTR_RESULT(hStreams_app_init(streams_per_domain, oversubscription));
    //--------------------------------------------------------------

    //--------------------------------------------------------------
    // Set up 2 scalar arguments, no heap arguments
    const int num_scalar_values_to_send = 2;
    const int num_heap_values_to_send   = 0;

    // 1st scalar value to pass to sink side.
    int a = 1000;
    // 2nd scalar value to pass to sink side.
    int b = 500;
    //!! Optional: Add additional variables as a practice.

    // You can pass these values from command line.
    if (argc > num_scalar_values_to_send) {
        a = atoi(argv[1]);
        b = atoi(argv[2]);
    }

    // Arguments to pass to sink side. You need to pass two arguments.
    uint64_t args[num_scalar_values_to_send + num_heap_values_to_send];

    // Pack the first argument.
    args[0] = (uint64_t)(a);
    //Pack the second argument.
    args[1] = (uint64_t)(b);
    // Pack other arguments you want to send.

    //--------------------------------------------------------------
    // Invoke the "average_min_max_sink_1" app on the sink side on stream 0.
    CHECK_HSTR_RESULT(hStreams_app_invoke(
                          0,                             // stream_id = 0
                          "average_min_max_sink_1",      // sink-side function name
                          num_scalar_values_to_send,     // Change the number of scalar args
                          num_heap_values_to_send,       // number of heap arguments
                          args,                          // arguments array
                          &out_Event,                    // completion event
                          //  used to check result availability
                          &ret_val,                      //!!b Point to the blob to return
                          //!!b %% EXERCISE: fill in the number of doubles
                          sizeof(double)* % %));          //!!b Give the blob size to return
    //--------------------------------------------------------------

    //--------------------------------------------------------------
    // Wait for the result to be back.
    CHECK_HSTR_RESULT(hStreams_app_event_wait(1, &out_Event));
    //--------------------------------------------------------------

    // Check the value on the host side.
    printf("Avg, min, max received on host, from the sink-side: %f, %d, %d\n",
           ret_val[0], (int) ret_val[1], (int) ret_val[2]);

    //--------------------------------------------------------------
    // Cleanup before exiting.
    CHECK_HSTR_RESULT(hStreams_app_fini());
    //--------------------------------------------------------------

}
