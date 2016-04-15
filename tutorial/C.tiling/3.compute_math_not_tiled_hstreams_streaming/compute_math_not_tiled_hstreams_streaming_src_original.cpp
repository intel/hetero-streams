/*
 *    Copyright 2014-2016 Intel Corporation.
 *
 *    This file is subject to the Intel Sample Source Code License. A copy
 *    of the Intel Sample Source Code License is included.
 *
 */


#include <hStreams_app_api.h> //user level APIs
/*
 * This program is slight improved hstreams implementation of the application
 * shown in example in folder 2. It uses only 4 streams, and splits the original
 * computation loops. It keeps the parallel i loop on host and the rest as a MIC
 * side computation. The body of the original i loop is enqueued on the streams
 * in a round robin fashion, after creating a buffer and initializing the data
 * properly on MIC as before. At the end, it transfers the results back from MIC
 * to source/host and verify the correctness.
 */

// Set number of streams you want to create and over-subscription level.
//!!a Using multiple streams this time.
int streams_per_domain = 4;
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

    // Initialize hStreams with the given StreamsPerDomain and
    // over-subscription level.
    CHECK_HSTR_RESULT(hStreams_app_init(streams_per_domain, oversubscription));

    //--------------------------------------------------------------

    //--------------------------------------------------------------
    // Saving buffer length in a temporary variable to save computation.
    long long buffer_len = dimX * dimY * dimZ * numIters
                           * sizeof(REAL) * 2;

    // Set up buffers.
    // Wrap out with a buffer.
    hStreams_app_create_buf((void *) out, buffer_len);

    //--------------------------------------------------------------

    for (int i = 0; i < NUM_TESTS_ITERS; i++) {
        iterTimes[i] = GetTime();

        //--------------------------------------------------------------
        // Initialize data in out buffer in sink side, all with 0.5.
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
        // Outer loop.
        //!!a Splits 3 nested outer loops onto two parts to use multiple
        //  streams. It keeps the outer most parallel loop on host and
        //  converts the body of the loop as a compute function.
        //%% EXERCISE: Figure out how the sink-side compute function
        //  would look without looking into the code first.
        //  Try to write your own compute function.
        for (int ii = 0; ii < dimX; ii++) {
            uint64_t args[7];

            // Pack scalar arguments first, then heap args.
            args[0] = (uint64_t)(ii);
            args[1] = (uint64_t)(dimX);
            args[2] = (uint64_t)(dimY);
            args[3] = (uint64_t)(dimZ);
            args[4] = (uint64_t)(numIters);
            args[5] = (uint64_t)(out1);
            args[6] = (uint64_t)(out2);
            //--------------------------------------------------------------

            //%% EXERCISE: Call the sink-side app/compute function
            // using hStreams_app_invoke
            //!!b Use ii % streams_per_domain as the stream id,
            // to use streams in a round-robin fashion.
            % %

        }
        //-----------------------------------------------------------------
        //%% EXERCISE: Consider why this synchronization is needed here.
        CHECK_HSTR_RESULT(hStreams_app_thread_sync());
        //--------------------------------------------------------------
        // Collect result. Transfer data in out buffer from sink to src.
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
    // Cleanup before exiting.
    CHECK_HSTR_RESULT(hStreams_app_fini());
    //--------------------------------------------------------------
    free(out);
    return 0;
}
