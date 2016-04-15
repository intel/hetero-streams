/*
 *    Copyright 2014-2016 Intel Corporation.
 *
 *    This file is subject to the Intel Sample Source Code License. A copy
 *    of the Intel Sample Source Code License is included.
 *
 */

//!!a hStreams header files
// For sink side
#include <hStreams_sink.h>

// Standard C header files
#include <stdio.h>

/*
 * This file is an example of a device (sink) side code.
 *  It receives the parameter passed to it from the source side, via an
 *   hStreams thunk on the sink side.
 * It converts the parameter back to the type that a value was intended to
 *  have and then print the value on the console.
 **/

//!!e Sink-side user function
// This MACRO is required to make the function a findable symbol at the source
//  it maps to extern "C" __declspec(dllexport) on Windows, others on Linux
// The function is a normal C function that does whatever you wish,
//  but since the arguments will always be uint64_t, conversion is needed
HSTREAMS_EXPORT
void hello_hStreams_world_sink_1(uint64_t a_double_value_converted_to_uint)
{
    // Need to convert the uint64_t back to the original type.
    // We show a couple of alternatives
    double value;
    value = *(double *)&a_double_value_converted_to_uint;

    printf("Hello hStreams world from sink side!! Received: %f from host!\n",
           value);
}
