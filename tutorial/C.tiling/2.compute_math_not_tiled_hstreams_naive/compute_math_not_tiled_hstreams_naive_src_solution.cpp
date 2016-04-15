/*
 *    Copyright 2014-2016 Intel Corporation.
 *
 *    This file is subject to the Intel Sample Source Code License. A copy
 *    of the Intel Sample Source Code License is included.
 *
 */

#include <hStreams_app_api.h> //user level APIs
/*
 * This program is a naive hstreams implementation of the application shown in
 * example: 0.compute_math_not_tiled_host. It uses only one stream, and
 * put the entire computation as a single compute function on that streams after
 * creating a buffer and initializing the data properly on MIC. At the end, it
 * transfer the results back from MIC to source/host and verify the correctness.
 */

//!!a Set number of streams you want to create and over-subscription level.
int streams_per_domain = 1;
int oversubscription = 1; //no over-subscription, i.e., one logical stream
//per physical stream.


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <math.h>
#include <omp.h>

typedef float REAL;
// restrict means the pointer does not overlap with others.
typedef REAL *restrict REAL_POINTER;
#define NUM_TESTS_ITERS 4 // Number of times you want to run the program. Running multiple
// times help to settle the running time and almost always
// the first run takes more time than others.

/* returns in the unit of ms */
double GetTime()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1e3 + tv.tv_usec / 1e3;
}

int main(int argc, char **argv)
{
    int dimX = 4;
    int dimY = 64;
    int dimZ = 64;
    int numIters = 64;

    //  Process args from command line*/
    int argi = 1;
    int errFlag = 0;
    while (argi < argc) {
        char *one = argv[argi];
        if (!strcmp(one, "-d") && argc > argi + 3) {
            dimX = atoi(argv[argi + 1]);
            dimY = atoi(argv[argi + 2]);
            dimZ = atoi(argv[argi + 3]);
            argi += 4;
        } else if (!strcmp(one, "-n") && argc > argi + 1) {
            numIters = atoi(argv[argi + 1]);
            /* Make numIters a multiple of 16 */
            numIters = numIters & (~0xf);
            argi += 2;
        } else {
            errFlag = 1;
            break;
        }
    }
    if (errFlag) {
        printf("Usage: %s [-d [the size of each dimension]] "
               "[-n [the number of iterations in the kernel]]\n", argv[0]);
        return -1;
    }

    if (dimY != dimZ) {
        printf("DimY is not the same as DimZ, "
               "changing dimZ to make them alike!\n");
        dimZ = dimY;
    }

    if (dimX * dimY * dimZ * numIters % 64 != 0) {
        printf("The production of DimX, DimY, DimZ and "
               "#iters must be multiples of 64!\n");
        return -1;
    }

    printf("DimX=%d, DimY=%d, DimZ=%d, #Loop_Iterations=%d\n", dimX, dimY,
           dimZ, numIters);

    REAL *out = NULL;

    // 64 bytes aligned allocation.
    posix_memalign((void **) &out, 64,
                   dimX * dimY * dimZ * numIters * sizeof(REAL) * 2);

    if (out == NULL) {
        printf("Memory allocation failed!\n");
        return -1;
    }

    double iterTimes[NUM_TESTS_ITERS];
    double mintime = 1e6;

    // Notice how the out1 and out2 pointers are calculated.
    REAL *out1 = out;
    REAL *out2 = out + dimX * dimY * dimZ * numIters;

    //--------------------------------------------------------------


    //!!a Initialize hStreams with the given StreamsPerDomain and
    // over-subscription.
    //%% EXERCISE: Call hStreams_app_init and put arguments in the initializer.
    CHECK_HSTR_RESULT(hStreams_app_init(streams_per_domain, oversubscription));

    //--------------------------------------------------------------

    //--------------------------------------------------------------
    // Saving buffer length in a temporary variable to save computation.
    long long buffer_len = dimX * dimY * dimZ * numIters
                           * sizeof(REAL) * 2;

    // Set up buffers.
    // Wrap out with a buffer.
    //%% EXERCISE: Call hStreams_app_create_buf to create buffer of size buffer_len.
    hStreams_app_create_buf((void *) out, buffer_len);
    //--------------------------------------------------------------

    for (int i = 0; i < NUM_TESTS_ITERS; i++) {
        iterTimes[i] = GetTime();

        //--------------------------------------------------------------
        // initialize data in out buffer in sink side, all with 0.5.
        // use hStreams_app_memset
        hStreams_app_memset(0, // stream id
                            out, // source proxy address to write
                            0.5, // value
                            buffer_len, // number of bytes to send
                            NULL); // completion event


        //--------------------------------------------------------------


        //--------------------------------------------------------------
        // Call device side API.
        // Prepare to perform computation on the sink-side.
        uint64_t args[6];

        // Pack scalar arguments first, then heap args.
        args[0] = (uint64_t)(dimX);
        args[1] = (uint64_t)(dimY);
        args[2] = (uint64_t)(dimZ);
        args[3] = (uint64_t)(numIters);
        args[4] = (uint64_t)(out1);
        args[5] = (uint64_t)(out2);
        //--------------------------------------------------------------
        //!!b Invoke remote function
        //%% EXERCISE: correctly set the missing % % parameters.
        hStreams_app_invoke(0, // stream id
                            "compute", // remote function name
                            4, // scalar arg
                            2, // heap args
                            args, // array of args
                            NULL, // event pointer
                            NULL, // return variable
                            0); // return variable size


        //-----------------------------------------------------------------

        //--------------------------------------------------------------
        //!!c Collect result. Transfer data in out buffer from sink to src.
        // See prior tutorials and ref doc.
        //%% EXERCISE: Call hStreams_app_xfer_memory to transfer data in
        // out buffer from sink to src.
        hStreams_app_xfer_memory(0, // stream id
                                 out, // source proxy address to write
                                 out, // source proxy address to read
                                 buffer_len, // number of bytes to send
                                 HSTR_SINK_TO_SRC, // transfer direction
                                 NULL); // completion event

        //--------------------------------------------------------------

        // Synchronize.
        CHECK_HSTR_RESULT(hStreams_app_thread_sync());

        //--------------------------------------------------------------

        iterTimes[i] = GetTime() - iterTimes[i];

        double result = out1[numIters / 2] + out2[numIters / 2];
        printf("Test %d takes %.3lf ms with result %.3lf\n", i, iterTimes[i],
               result);
        if (iterTimes[i] < mintime) {
            mintime = iterTimes[i];
        }
    }
    printf("Test's min time is %.3lf ms\n", mintime);

    //--------------------------------------------------------------
    //!!a Cleanup before exiting.
    //%% EXERCISE: Call hStreams_app_fini(). to finalize.
    CHECK_HSTR_RESULT(hStreams_app_fini());
    //--------------------------------------------------------------
    free(out);
    return 0;
}

