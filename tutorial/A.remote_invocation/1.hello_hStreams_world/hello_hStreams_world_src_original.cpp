/*
 *   Copyright 2014-2016 Intel Corporation.
 *
 *   This file is subject to the Intel Sample Source Code License. A copy
 *   of the Intel Sample Source Code License is included.
 *
 */


/*
 * The goal of this exercise is to show how to write a simple hello world
 *  program in hStreams.
 * This file demonstrates the host(source)-side code.
 *  a) It illustrates necessary header files
 *  b) It first initializes the hStreams library, and finalizes at the end
 *  c) It type converts and packs arguments
 *  d) It passes a double variable in a call to a sink-side library function.
 * */

//!!a hStreams header files
// For source side
//%% EXERCISE Add hStreams app api include file
#include <hStreams_%%.h>  // hStreams APIs

// Optional header, for convenience only
#include "type_converter.h"   // helper-type conversion APIs

// Standard C header files
#include <stdio.h>

int main(int argc, char *argv[])
{

    //--------------------------------------------------------------
    //!!b Set up initialization parameters
    // Number of streams to create in each domain: 1
    //%% EXERCISE: create one stream per domain
    int streams_per_domain = % %;
    // no over-subscription, i.e., one logical stream per physical stream.
    //  don't worry about what over-subscription means, but it enables the
    //  use of an arbitrarily large # of logical streams
    int oversubscription = 1;
    CHECK_HSTR_RESULT(hStreams_app_init(streams_per_domain, oversubscription));
    //--------------------------------------------------------------

    //--------------------------------------------------------------
    //!!c Set up parameter to pass to the sink side.
    // A scalar, with a default value
    double a_double_value_to_pass = 0.99;
    // You can override the default with a value from a command line.
    if (argc > 1) {
        a_double_value_to_pass = atof(argv[1]);
    }

    // All arguments that are passed to the sink-side are uint64_t type.
    // Therefore, we need to convert the standard type to this hStreams
    //  argument type before passing to sink. Conversion for double is shown.
    //  For integer types with positive values, the process is much simpler.
    // We intend to make this easier in the future
    // We show three alternatives - a C++ converter, a macro, and manual
    uint64_t arg0;
    doubleToUint64_t converter;
    converter.Set(a_double_value_to_pass);
    //!!c Type conversion
    arg0 = (uint64_t)(converter.Get_uint64_t());  // C++ convenience
    //%% EXERCISE replace <%%variable_name> with a_double_value_to_pass
    arg0 = *(uint64_t *)& < % % variable_name >; // A manual non-standard alternative
    // might have non-deterministic
    // behavior in some compiler.

    // Arguments to pass to sink side. At this point passing only one
    //  scalar value. Hence, args size is 1. Increment size if you want
    // to pass more parameters to the sink side.
    uint64_t args[1]; // one argument.
    args[0] = arg0;   // set elements of the arg array
    //--------------------------------------------------------------

    //--------------------------------------------------------------
    //!!d Invoke the "hello_hStreams_world_sink_1" app on the sink side.
    //CHECK_HSTR_RESULT is defined in include/hStreams_source.h, which is a convenience
    //Macro and checks the return value returned by an hstreams app call.
    CHECK_HSTR_RESULT(hStreams_app_invoke(
                          0,                             // stream_id = 0
                          //%% EXERCISE: replace <name> with hello_hStreams_world_sink_1
                          "<%%name>", // sink-side function name
                          1,                             // number of scalar arguments
                          0,                             // number of heap arguments
                          args,                          // arguments array
                          NULL,                          // completion event
                          NULL,                          // ptr to return value
                          0));                           // return value size
    //--------------------------------------------------------------

    //--------------------------------------------------------------
    //!!b Clean up before exiting.
    //%% EXERCISE: use hStreams_app_fini
    CHECK_HSTR_RESULT(hStreams_app_ % % ());
    //--------------------------------------------------------------

}
