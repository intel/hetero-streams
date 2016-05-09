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

// Helper library
#include "type_converter.h"

/*
 *This file is an example of a device (sink) side code. It receives the value passed to it from the
 *value, converts it back to the type that a value was intended to and then print the value on
 *the console.
 **/
HSTREAMS_EXPORT
void hello_hStreams_world_sink_1(uint64_t a_double_value_converted_to_uint,
                                 uint64_t an_array_to_sink_side)
{
    // Need to convert the uint64_t back to the original type.
    doubleToUint64_t converter;
    converter.Set_uint64_t(a_double_value_converted_to_uint);
    double *an_array_in_sink = (double *) an_array_to_sink_side;

    printf("Hello hStreams world from sink side!!"
           "Received: %f and an array with %f as the first element from host!\n",
           converter.Get(), an_array_in_sink[0]);
}
