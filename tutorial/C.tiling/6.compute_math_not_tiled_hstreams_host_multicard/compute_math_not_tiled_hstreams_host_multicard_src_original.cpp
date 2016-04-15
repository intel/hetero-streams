/*
 *    Copyright 2014-2016 Intel Corporation.
 *
 *    This file is subject to the Intel Sample Source Code License. A copy
 *    of the Intel Sample Source Code License is included.
 *
 */

#include <hStreams_app_api.h> //user level APIs
//!!a Include header file to use core hStreams APIs
#include <hStreams_source.h>  //core level APIs
/*
 * This program uses both host and available MIC cards as sink side for the
 * streams. So, it uses both the host and MIC cards for computations.
 * It shows how to selectively not use some cores of host and use others for
 * streams.
 * Instead of sending/initializing the entire buffer data at the beginning and
 * collecting the entire result at the end, this version initializes and collects
 * only the portion of the data that is going to be used by a particular
 * iteration of i loop. This gives the opportunity of pipelining of not only
 * the computations (as enabled by the version in folder 3)
 * but also the data transfers.
 */

typedef float REAL;
// restrict means the pointer does not overlap with others.
typedef REAL *restrict REAL_POINTER;
#define NUM_TESTS_ITERS 4 // Number of times you want to run the program. Running multiple
// times help to settle the running time and almost always
// the first run takes more time than others.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <math.h>
#include <omp.h>

// Won't ever create more than these many threads.
#define HSTR_MAX_THREADS 240

// A user-defined custom hstreams initialization function that takes a) total number
// of physical domains, id of those physical domain (phys_domains_of_interest),
// number of logical domains, number of physical streams per domains
// (places_per_log_domain), logical streams per physical streams, and  a cpu
//  mask proving which cores will be used by hstreams computation.
//  Code taken from @Author: Gaurav at Intel used in ref_code/matmult_host_multicard
HSTR_RESULT hStreams_custom_init_selected_domains(uint32_t num_phys_domains,
        HSTR_PHYS_DOM *phys_domains_of_interest, uint32_t num_log_domains,
        uint32_t *places_per_log_domain, uint32_t log_streams_per_place,
        HSTR_CPU_MASK src_cpu_mask)
{

    //!! b First does a default initialization.
    //%% EXERCISE: call hStreams_Init() with empty args.
    % %

    // Find total number of places.
    uint32_t num_places = 0;
    for (uint32_t i = 0; i < num_log_domains; ++i) {
        num_places = num_places + places_per_log_domain[i];
    }

    // Number of logical domains per physical domain if evenly divides
    uint32_t ldom_low = num_log_domains / num_phys_domains;
    // Number of logical domains per physical domain, after distributing the remainders, if any
    uint32_t ldom_high = (num_phys_domains % num_log_domains) ? ldom_low + 1
                         : ldom_low;
    // id for those physical domains which will have extra logical domains.
    uint32_t last_high_pdom_idx = num_log_domains % num_phys_domains;

    // Total number of places.
    uint32_t global_place_idx = 0;

    // Total number of logical domains.
    uint32_t global_ldom_idx = 0;

    // Check that all the requested physical domains exist before creating anything
    for (uint32_t pdom_idx = 0; pdom_idx < num_phys_domains; ++pdom_idx) {
        HSTR_PHYS_DOM curr_pdom = phys_domains_of_interest[pdom_idx];
        uint32_t num_threads, max_freq;
        uint64_t mem_types, mem_avail[HSTR_MEM_TYPE_SIZE];
        HSTR_CPU_MASK max_mask, avoid_mask;
        HSTR_ISA_TYPE isa;

        // Get detailed info about a physical domain.
        // See /usr/share/doc/hStreams/hStreams_Reference.pdf for details for details
        CHECK_HSTR_RESULT(
            hStreams_GetPhysDomainDetails(curr_pdom, &num_threads, &isa,
                                          &max_freq, max_mask, avoid_mask, &mem_types, mem_avail));
    }

    // Same loop, this time creating the domains/streams/whatnot
    for (uint32_t pdom_idx = 0; pdom_idx < num_phys_domains; ++pdom_idx) {
        HSTR_PHYS_DOM curr_pdom = *(phys_domains_of_interest + pdom_idx);
        uint32_t num_threads, max_freq;
        uint64_t mem_types, mem_avail[HSTR_MEM_TYPE_SIZE];
        HSTR_CPU_MASK max_mask, avoid_mask;
        HSTR_ISA_TYPE isa;

        //!!c Get detailed info about a physical domain.
        //%% EXERCISE: Get detailed info about a physical domain as top.
        % %

        // Compute # logical domains to create in this physical domain
        uint32_t ldom_cnt = (pdom_idx >= last_high_pdom_idx) ? ldom_low
                            : ldom_high;

        // Get the CPU mask that we'll use for this particular logical domain
        // Last logical domain will have more threads (if needed)
        HSTR_CPU_MASK use_mask;

        //gbansal

        // Physical domain 0 is considered as source.
        // Use the passed mask to avoid the CPUs while creating streams.
        if (pdom_idx == 0) {
            HSTR_CPU_MASK_OR(avoid_mask, avoid_mask, src_cpu_mask);
        }

        // Use all other CPUs expect the one that are reserved by source.
        // XOR flips bit if set.
        // For domains other than the source, all bits will be set!
        HSTR_CPU_MASK_XOR(use_mask, max_mask, avoid_mask);

        // Counts number of set bits in a mask.
        int num_cores = HSTR_CPU_MASK_COUNT(use_mask);

        // Number of cores used by all streams expect the last!
        uint32_t ldom_width_low = num_cores / ldom_cnt;

        // Number of cores used by the last stream which gets all the extra core,
        // remaining after even distribution.
        uint32_t ldom_width_high = ldom_width_low + num_cores % ldom_cnt;

        // Bit operation: we're operating on for the logical domain mask


        int ldom_bit_idx = 0;
        // Here the last domain gets all the extra cores.
        int special_logical_domain = ldom_cnt - 1;

        for (uint32_t ldom_idx = 0; ldom_idx < ldom_cnt; ++ldom_idx, ++global_ldom_idx) {

            // How many cores this logical stream is going to use.
            uint32_t ldom_width =
                (ldom_idx == special_logical_domain) ? ldom_width_high
                : ldom_width_low;

            // Number of physical streams in this logical domain.
            uint32_t ldom_places_cnt = places_per_log_domain[global_ldom_idx];

            //!!d Create logical domain mask, based on set bits on
            // use_mask, so over subscription is 1.
            HSTR_CPU_MASK ldom_mask;
            HSTR_CPU_MASK_ZERO(ldom_mask);

            for (uint32_t bits_set = 0; bits_set < ldom_width; ++bits_set) {
                // Get to the next bit which is set
                while (!HSTR_CPU_MASK_ISSET(ldom_bit_idx, use_mask)) {
                    ++ldom_bit_idx;
                }
                HSTR_CPU_MASK_SET(ldom_bit_idx, ldom_mask);
                ++ldom_bit_idx;
            }
            // Now create the logical domains, in the physical domain.
            HSTR_LOG_DOM ldom_id;
            HSTR_OVERLAP_TYPE ldom_overlap;
            //!!d Add a logical domain with the current domain id,
            // and the domain mask.
            //%% EXERCISE: Call hStreams_AddLogDomain, see /usr/share/doc/hStreams/hStreams_Reference.pdf for details
            % %

            // Recall: ldom_width = #cores used by this logical domain, and
            // ldom_places_cnt = #physical streams per logical domains.
            uint32_t place_width_low = ldom_width / ldom_places_cnt;
            uint32_t place_width_high =
                (ldom_width % ldom_places_cnt) ? place_width_low + 1
                : place_width_low;
            uint32_t last_high_place_idx = ldom_width % ldom_places_cnt;

            // Bit operation: we're operating on for the place mask
            int place_bit_idx = 0;

            HSTR_CPU_MASK place_mask;
            for (uint32_t place_idx = 0; place_idx < ldom_places_cnt; ++place_idx, ++global_place_idx) {
                uint32_t place_width =
                    (place_idx >= last_high_place_idx) ? place_width_low
                    : place_width_high;

                HSTR_CPU_MASK_ZERO(place_mask);
                for (uint32_t bits_set = 0; bits_set < place_width; ++bits_set) {
                    // Get to the next bit which is set
                    while (!HSTR_CPU_MASK_ISSET(place_bit_idx, ldom_mask)) {
                        ++place_bit_idx;
                    }
                    HSTR_CPU_MASK_SET(place_bit_idx, place_mask);
                    //gbansal - avoid host thread
                    ++place_bit_idx;
                }

                // Now, add streams for this place.
                for (uint32_t lstr_idx = 0; lstr_idx < log_streams_per_place; ++lstr_idx) {
                    HSTR_LOG_STR lstr_id = global_place_idx + lstr_idx
                                           * num_places;
                    //!!e Creates a new stream using a given stream id, the logical
                    // domain id where it should be created and a CPU mask.
                    //%% EXERCISE: Call hStreams_StreamCreate, see /usr/share/doc/hStreams/hStreams_Reference.pdf for details
                    % %
                }
            }
        }
    }
    return HSTR_RESULT_SUCCESS;
}

static HSTR_OPTIONS hstreams_options;

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

    // Process args from command line*/
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

    if (dimX * dimY * dimZ * numIters % 64 != 0) {
        printf("The production of DimX, DimY, DimZ and #iters must "
               "be multiples of 64!\n");
        return -1;
    }
    if (dimY != dimZ) {
        printf("dimY needs to be the same as dimZ.\n");
        return -1;
    }
    printf("DimX=%d, DimY=%d, DimZ=%d, #Loop_Iterations=%d\n", dimX, dimY,
           dimZ, numIters);

    REAL *out = NULL;

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

    // Get the current hstreams options setup and modify it as needed.
    hStreams_GetCurrentOptions(&hstreams_options, sizeof(hstreams_options));
    // Set a limit to #physical domains.
    hstreams_options.phys_domains_limit = 256; // Limit to 256 domains
    //!!f Setting thread affinity for load-balancing purpose.
    //%% EXERCISE: Tell why compact was used instead of scatter?
    // COMPACT: Assign hardware threads to first core,
    // before moving to the 2nd core. SCATTER distributes hardware threads
    // in a round-robin fashion.
    hstreams_options.kmp_affinity = HSTR_KMP_AFFINITY_COMPACT;

    int use_host = 1;
    const int MAX_SUPPORTED_NUM_MICS = 1;
    int num_streams = 1;
    int nstreams_host, nstreams_mic;
    nstreams_host = num_streams;
    nstreams_mic = 2 * num_streams; // it creates 2x number of streams on mic than on host

    uint32_t cards_found, NumActive;
    bool homog;

    // Query how many active cards hstreams has discovered and how are they?
    CHECK_HSTR_RESULT(hStreams_Init());
    CHECK_HSTR_RESULT(
        hStreams_GetNumPhysDomains(&cards_found, &NumActive, &homog));

    if (cards_found < MAX_SUPPORTED_NUM_MICS) {
        printf(
            "Number of cards requested, %d, exceeds those in the system, %d.  "
            "Terminating.\n", MAX_SUPPORTED_NUM_MICS, cards_found);
        exit(-1);
    }
    if (MAX_SUPPORTED_NUM_MICS > 1) {
        printf("Number of cards requested, %d, exceeds the limit (1) of what "
               "this code was tested for.  Terminating.\n", MAX_SUPPORTED_NUM_MICS);
        exit(-1);
    }

    //!!g Enable use of host
    //%% EXERCISE: Set the hstreams with the newly set up options
    // by calling hStreams_SetOptions. See hstreams_references.pdf for help.
    % %

    // number of domains for host is 1 and for mic, it #MAX_SUPPORTED_NUM_MICS
    int num_doms = use_host + MAX_SUPPORTED_NUM_MICS;
    // use one stream for host and 2 streams for each MIC.
    int total_streams = use_host + nstreams_mic * MAX_SUPPORTED_NUM_MICS;

    // Create physical domains of the size num_doms and assign IDs to it,
    // you want to assign between -1 to num_dom -1 or -2 based on whether
    // host is used as sink or not.
    HSTR_PHYS_DOM *physDomID = new HSTR_PHYS_DOM[num_doms];
    // Id start from -1, if host is used, otherwise,
    // it starts from 0, Id for host is -1, a reserved value.
    // If you have 2 MICS, i.e., MAX_SUPPORTED_NUM_MICS=2, you need to set value of
    // physDomID[2] = 1
    physDomID[0] = -1;
    physDomID[1] = 0;

    // Indicate how many physical streams you want to create in
    // each domain.
    uint32_t *places = new uint32_t[num_doms];

    // Host always comes first, if used.
    // If you have 2 MICS, i.e., MAX_SUPPORTED_NUM_MICS=2, you need to set value of
    // places[2]= nstreams_mic;
    places[0] = nstreams_host;
    places[1] = nstreams_mic;

    // Create CPU mask, if you want to use only some specified CPUs
    // for a stream.
    HSTR_CPU_MASK src_hstr_cpu_mask;
    HSTR_CPU_MASK_ZERO(src_hstr_cpu_mask);

    // Reserve CPU 0 and 1 for overhead work!
    // Create a mask accordingly, by setting those bits.
    HSTR_CPU_MASK_SET(0, src_hstr_cpu_mask);
    HSTR_CPU_MASK_SET(1, src_hstr_cpu_mask);

    // Initialize hstreams by calling a custom initialization routine.
    CHECK_HSTR_RESULT(
        hStreams_custom_init_selected_domains(num_doms, physDomID,
                num_doms, places, 1, src_hstr_cpu_mask));

    // Done with initialization.

    //--------------------------------------------------------------

    //--------------------------------------------------------------

    // Creating two dimX buffer addresses pointing inside out buffer.
    // out1_addr[i] points to the position of out1 buffer where
    // iteration i of the outer loop of the compute kernel writes.
    // out2_addr[i] points to the position of out2 buffer where
    // iteration i of the outer loop of the compute kernel writes.
    REAL *out1_addr[dimX];
    REAL *out2_addr[dimX];

    // Separation between out1 and out2.
    long int out_length = dimX * dimY * dimZ * numIters;

    // Wrap out with a buffer of size
    hStreams_app_create_buf((void *) out,
                            dimX * dimY * dimZ * numIters * sizeof(REAL) * 2);

    long int buff_length = dimY * dimZ * numIters * sizeof(REAL);
    for (int i = 0; i < dimX; i++) {
        out1_addr[i] = out + i * dimY * dimZ * numIters;
        out2_addr[i] = out1_addr[i] + out_length;
        //    hStreams_app_create_buf((void *)out1_addr[i], buff_length);
        //    hStreams_app_create_buf((void *)out2_addr[i], buff_length);
    }
    HSTR_EVENT eout1[numIters][dimX], eout2[numIters][dimX],
               eout3[numIters][dimX], eout4[numIters][dimX], eout5[numIters][dimX];

    for (int i = 0; i < NUM_TESTS_ITERS; i++) {
        iterTimes[i] = GetTime();

        // Outer loop.

        for (int ii = 0; ii < dimX; ii++) {

            int stream = ii % total_streams;

            //--------------------------------------------------------------
            // Initialize data at the sink-side

            // Initializes intermediate data at sink, only the portion
            // being worked on
            hStreams_app_memset(stream, out1_addr[ii], // source proxy address to write
                                0.5, buff_length, // number of bytes to send
                                &eout4[i][ii]); // completion event

            hStreams_app_memset(stream, out2_addr[ii], // source proxy address to write
                                0.5, buff_length, // number of bytes to send
                                &eout5[i][ii]); // completion event

            //--------------------------------------------------------------
            // Call device side API.
            // Prepare to perform computation on the sink-side.
            //!!h Call the compute function at the sink-side similarly
            // irrespective of whether the sink side is a host
            //%% EXERCISE: Prepare and call compute at the sink-side.
            % %

            //-----------------------------------------------------------------
            // Collect result.
            hStreams_app_xfer_memory(stream, out1_addr[ii], // source proxy address to write
                                     out1_addr[ii], // source proxy address to read
                                     buff_length, // number of bytes to send
                                     HSTR_SINK_TO_SRC, // transfer direction
                                     &eout2[i][ii]); // completion event

            //--------------------------------------------------------------

            hStreams_app_xfer_memory(stream, out2_addr[ii], // source proxy address to write
                                     out2_addr[ii], // source proxy address to read
                                     buff_length, // number of bytes to send
                                     HSTR_SINK_TO_SRC, // transfer direction
                                     &eout3[i][ii]); // completion event
        }
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
    delete [] physDomID;
    delete [] places;
    return 0;
}

