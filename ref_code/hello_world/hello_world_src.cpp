/*
 *   Copyright 2014-2016 Intel Corporation.
 *
 *   This file is subject to the Intel Sample Source Code License. A copy
 *   of the Intel Sample Source Code License is included.
 *
 */


#ifndef _WIN32
#include <unistd.h>
#endif

#include <hStreams_app_api.h> //user level APIs
#include "type_converter.h"   //helper type conversion APIs
#include <stdio.h>

/*
 * The goal of the test is to show how to write a simple hStream hello world program in hStreams.
 * This file demonstrates the host(source) side code. It first initializes the hStreams library,
 * providing the name for the sink/device side library to be called.
 * It passes a double scaler and a pointer variable to the sink side by calling a sink side
 * library function. It shows how to wrap a source proxy address (pointer) to a buffer, how
 * to allocate a buffer on the sink side, how to transfer data in a buffer at sink-side
 * and how to wait on a completion event
 * The sink side library will print the value of the received value.
 * */
int main()
{
    // A scalar value to pass to sink side.
    double a_double_value_to_pass = 0.99;

    // All arguments that are passed to the sink-side are unit64_t type. Therefore, we need to
    //convert the standard type to hStreams argument type before passing to
    doubleToUint64_t converter;
    converter.Set(a_double_value_to_pass);

    const int size = 4;
    double array_to_pass_to_sink[size] = {0.988, 0.988, 0.988, 0.991}; // This could also be a
    //dynamically allocated array as well

    int streams_per_domain = 1;
    int oversubscription = 1; //no over-subscription, i.e., one logical stream per physical stream.

    // Initialize hStreams with 1 StreamsPerDomain and 1 logical stream per physical stream.
    CHECK_HSTR_RESULT(hStreams_app_init(streams_per_domain, oversubscription));


    // To pass a pointer, you need to first allocate it using either use
    // HSTR_RESULT hStreams_app_create_buf (void *in_BufAddr, const uint64_t in_NumBytes)
    // or hStreams_Alloc1DEx(...)

    CHECK_HSTR_RESULT(hStreams_app_create_buf(array_to_pass_to_sink, size * sizeof(double)));

    // NOTE: This only allocates the array on the sink-side, but DOES NOT transfer the data to
    // the sink-side. Therefore, the sink-side array will have
    // garbage or 0 values. To get source side array data on sink-side, you need to call either
    // hStreams_app_xfer_memory or hStreams_EnqueueData1D.

    // Transfer data from array_to_pass_to_sink at host to array_to_pass_to_sink on sink.
    // CHECK_HSTR_RESULT(hStreams_app_xfer_memory (HSTR_LOG_STR in_LogStreamID, void *in_pWriteAddr,
    // void *in_pReadAddr, uint64_t in_NumBytes, HSTR_XFER_DIRECTION in_-XferDirection,
    // HSTR_EVENT *out_pEvent));

    CHECK_HSTR_RESULT(
        hStreams_app_xfer_memory(0,                      // stream id
                                 array_to_pass_to_sink,  // destination address
                                 array_to_pass_to_sink,  // source address
                                 size * sizeof(double),  // transfer size in bytes
                                 HSTR_SRC_TO_SINK,       // xfer direction
                                 NULL)                   // event pointer.
    );
    // You could also use: CHECK_HSTR_RESULT(hStreams_EnqueueData1D(0, array_to_pass_to_sink,
    // array_to_pass_to_sink, size * sizeof(double), HSTR_SRC_TO_SINK, NULL));

    // Arguments to pass to sink side. At this point passing one scaler value and one pointer.
    // Hence, args size is 2.
    // Increment size if you want to pass more parameters to the sink side.
    // Always, the scaler arguments as packed first before the pointer/heap arguments.

    uint64_t args[2]; // two arguments.
    args[0] = (uint64_t)(converter.Get_uint64_t());
    args[1] = (uint64_t)(array_to_pass_to_sink);


    // Completion events to track completion of tasks.
    HSTR_EVENT completion;

    // Invoke the "hello_hStreams_world_sink_1" app on the sinkside. This call is a blocking call.
    // Means: this call will not return until the function finishes executing. To make it blacking
    // we have passes a NULL pointer in the event. Passing a non-null pointer in the event makes
    // the call non-blocking.
    CHECK_HSTR_RESULT(
        hStreams_app_invoke(0,                               // stream_id = 0
                            "hello_hStreams_world_sink_1",   // Sink-side function to call
                            1,                               // #scaler argument = 1
                            1,                               // #heap argument = 1
                            args,                            // parameters argument holder,
                            &completion,                     // non-null event pointer.
                            // Need to sync later for correctness.
                            NULL,                            // return_value_pointer
                            0));                             // size_of_return_values


    // Wait for the completion event. i.e., wait for the app to return.
    CHECK_HSTR_RESULT(hStreams_EventWait(1,   &completion, true, -1, NULL, NULL));

    // Cleanup before exiting.
    CHECK_HSTR_RESULT(hStreams_app_fini());
    return 0;
}
