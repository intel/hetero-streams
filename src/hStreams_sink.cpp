/*
 * Hetero Streams Library - A streaming library for heterogeneous platforms
 * Copyright (c) 2014 - 2016, Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU Lesser General Public License,
 * version 2.1, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 * more details.
 */

#include <stdio.h>
#ifndef _WIN32
#include <unistd.h>
#endif

#ifdef _BullseyeCoverage
// need this for setenv()
#include <stdlib.h>
#endif

#ifndef _WIN32
#include <dlfcn.h>
#endif

#include <string.h>
#include <omp.h>
#include "hStreams_common.h"
#include "hStreams_internal.h"
#include "hStreams_sink.h"
#include "hStreams_Logger.h"
#include "hStreams_internal_types_common.h"
#include "hStreams_internal_vars_common.h"
#include "hStreams_helpers_common.h"
#include "hStreams_exceptions.h"
#include "hStreams_MKLWrapper.h"

#ifndef HSTR_SOURCE
#include "hStreams_COIWrapper_sink.h"
#endif

#ifdef HSTR_BACKTRACE
#include <execinfo.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <vector>
#include <limits>

static void hStreams_Backtrace()
{
    void *array[20];
    size_t size;

    /* Get backtrace. */
    size = backtrace(array, 20);

    fflush(stdout);
    fflush(stderr);
    /* Show backtrace */
    fprintf(stderr, "Backtrace for thread: %p:\n", (void *)pthread_self());
    backtrace_symbols_fd(array, size, STDERR_FILENO);
    fflush(stderr);
    fprintf(stderr, "Now, attach gdb (pausing):\n");
    fflush(stderr);
    getchar();
}

static void hStreamsPageFaultHandler(int sn , siginfo_t *si , void *context)
{
    hStreams_Backtrace();
}
#endif

typedef   void  func19_t (uint64_t, uint64_t, uint64_t, uint64_t,   /*  1- 4 */
                          uint64_t, uint64_t, uint64_t, uint64_t, /*  5- 8 */
                          uint64_t, uint64_t, uint64_t, uint64_t, /*  9-12 */
                          uint64_t, uint64_t, uint64_t, uint64_t, /* 13-16 */
                          uint64_t, uint64_t, uint64_t,        /* 17-19 */
                          void *,                              /* return value */
                          uint16_t                             /* size of return value */
                         );

// we don't build main for the host-side streams
#ifndef HSTR_SOURCE
// main is automatically called whenever the source creates a process.
// However, once main exits, the process that was created exits.
int main(int argc, char **argv)
{
#if _BullseyeCoverage
#ifndef COVFILE_RUNTIME_LOCATION
#error Performing a coverage-enabled build but COVFILE_RUNTIME_LOCATION is not set in the preprocessor. Aborting
#endif
    if (setenv("COVFILE", hStreams_stringer(COVFILE_RUNTIME_LOCATION), 1)) {
        HSTR_ERROR(HSTR_INFO_TYPE_MISC)
                << "setenv didn't find sufficient space in the environment for "
                << "the COVFILE variable.\nCOVFILE_RUNTIME_LOCATION="
                << hStreams_stringer(COVFILE_RUNTIME_LOCATION);

        return HSTR_RESULT_OUT_OF_MEMORY;
    }
#endif

#ifdef HSTR_BACKTRACE
    static struct sigaction m, oldSigSEGV;
#endif
    UNUSED_ATTR HSTR_COIRESULT result;
    UNREFERENCED_PARAM(argc);
    UNREFERENCED_PARAM(argv);

#ifdef HSTR_BACKTRACE
    memset(&m, 0, sizeof(struct sigaction));
    memset(&oldSigSEGV, 0, sizeof(struct sigaction));
    m.sa_flags = SA_SIGINFO;
    m.sa_sigaction = hStreamsPageFaultHandler;
    sigaction(SIGSEGV, &m, &oldSigSEGV);
#endif
    // Functions enqueued on the sink side will not start executing until
    // you call hStreams_COIWrapper::COIPipelineStartExecutingRunFunctions(). This call is to
    // synchronize any initialization required on the sink side.

    result = hStreams_COIWrapper_sink::COIPipelineStartExecutingRunFunctions();

    assert(result == HSTR_COI_SUCCESS);

    // This call will wait until COIProcessDestroy() gets called on the source
    // side If COIProcessDestroy is called without force flag set, this call
    // will make sure all the functions enqueued are executed and does all
    // clean up required to exit gracefully.
    hStreams_COIWrapper_sink::COIProcessWaitForShutdown();

    return 0;
}

#endif // HSTR_SOURCE

namespace
{

void hStreams_returnErrorCodeThroughRetVal(
    HSTR_RESULT error_code,
    void *in_pReturnValue,
    uint16_t in_ReturnValueLength)
{
    if (in_pReturnValue != NULL && in_ReturnValueLength >= 8) {
        uint64_t *ret_ptr = (uint64_t *)in_pReturnValue;
        *ret_ptr = error_code;
    } else {
        HSTR_ERROR(HSTR_INFO_TYPE_SINK_INVOKE)
                << "Could not inform source-side about the error that occured on the sink. "
                << "Error code: " << error_code << ", i.e. " << hStreams_ResultGetName(error_code) << ".";
    }
}

} // anonymous namespace

/* FIXME: Need a way to communicate failures in hStreamsThunk() back to the host. */

// Common sink-side thunk that invokes a user-defined function
// These arguments conform to the COIPipelineRunFunction template,
//  since this function is invoked directly from COI's thunk
HSTREAMS_EXPORT
void hStreamsThunk(
    uint32_t         in_BufferCount,
    void           **in_ppBufferPointers,
    uint64_t        *in_pBufferLengths,
    void            *in_pMiscData,
    uint16_t         in_MiscDataLength,
    void            *in_pReturnValue,
    uint16_t         in_ReturnValueLength)
{
    // Buffer count and pointers are not used, since COIBuffers are unused, since
    //  buffers don't span multiple domains and deps are handled manually

    int i, j = 0;
    uint64_t     *misc_data = (uint64_t *)(in_pMiscData);
    uint64_t      all_args[HSTR_ARGS_SUPPORTED];
    uint64_t     *scalar_args;
    uint64_t     *obj_pointers; // these are really pointers, but are treated like values
    func19_t     *target_func19;

    if (in_MiscDataLength < 2 * sizeof(uint64_t)) {
        HSTR_ERROR(HSTR_INFO_TYPE_SINK_INVOKE) << "Misc data too small: " << in_MiscDataLength;

        /* FIXME: Need a way to communicate this failure back to the host.
         *          Maybe through return value ?*/
        return;
    }

    // Demarshall misc_data
    int num_scalar_args = misc_data[j++];
    int num_heap_args   = misc_data[j++];

    HSTR_DEBUG1(HSTR_INFO_TYPE_SINK_INVOKE)
            << "Arrived in hStreamThunk with " << num_scalar_args << "scalar args and "
            << num_heap_args << "heap args and " << in_MiscDataLength << "B of misc_data";

    // These look like separate arrays, but they are really one big arg array
    scalar_args = all_args;
    obj_pointers = (uint64_t *)(&all_args[num_scalar_args]);

    for (i = 0; i < num_scalar_args; i++) {
        scalar_args[i] = misc_data[j++];
        HSTR_DEBUG2(HSTR_INFO_TYPE_SINK_INVOKE)
                << "Received scalar arg " << i << " as 0x" << std::hex << scalar_args[i];
    }
    for (i = 0; i < num_heap_args; i++) {
        //    obj_pointers[i] = (uint64_t*)((uint64_t*)(misc_data[2+num_scalar_args+i*2]) + (uint64_t*)(misc_data[2+num_scalar_args+i*2+1]));
        obj_pointers[i] = misc_data[j++];
        HSTR_DEBUG2(HSTR_INFO_TYPE_SINK_INVOKE)
                << "Received heap arg " << i << " as 0x" << std::hex << obj_pointers[i];
    }
    target_func19 = (func19_t *)(misc_data[j++]);
    HSTR_DEBUG1(HSTR_INFO_TYPE_SINK_INVOKE)
            << "Target function address being called is " << (void *) target_func19;

    // By convention, if a return value is to be used, the called function
    //  is responsible for copying return data into the location pointed to
    //  by in_pReturnValue, and they need to not exceed the length specified
    //  by in_ReturnValueLength.  If fewer than 19 real arguments are needed,
    //  bogus arguments must be artificially inserted in order to receive
    //  the two return-related arguments.  in_ReturnValueLength is specified
    //  by the source-side caller, in EnqueueCompute.  The plumbing layer
    //  handles the memory allocation and copying of the returned data.
    // The called function must always have a void return type.
    if (target_func19 != 0) {
        (*target_func19)(all_args[0],  all_args[1],  all_args[2],  all_args[3],
                         all_args[4],  all_args[5],  all_args[6],  all_args[7],
                         all_args[8],  all_args[9],  all_args[10], all_args[11],
                         all_args[12], all_args[13], all_args[14], all_args[15],
                         all_args[16], all_args[17], all_args[18], in_pReturnValue,
                         in_ReturnValueLength);
    } else {
        HSTR_ERROR(HSTR_INFO_TYPE_SINK_INVOKE) << "Target func is NULL";
    }
}

// Initialization of the hStreams sink side.
// These arguments conform to the COIPipelineRunFunction template,
//  since this function is invoked directly from COI's thunk
HSTREAMS_EXPORT
void hStreams_init_sink(
    uint32_t         in_BufferCount,
    void           **in_ppBufferPointers,
    uint64_t        *in_pBufferLengths,
    void            *in_pMiscData,
    uint16_t         in_MiscDataLength,
    void            *in_pReturnValue,
    uint16_t         in_ReturnValueLength)
{
    try {
        if (in_MiscDataLength == sizeof(hStreams_InitSinkData)) {
            hStreams_InitSinkData *init_data = (hStreams_InitSinkData *)in_pMiscData;

            globals::logging_level = init_data->logging_level;
            globals::logging_bitmask = init_data->logging_bitmask;
            globals::logging_myphysdom = init_data->logging_myphysdom;
            globals::mkl_interface = init_data->mkl_interface;

            hStreams_SetOptions(&(init_data->options));

            // fetch cblas_*gemm addresses if needed
            if (globals::mkl_interface != HSTR_MKL_NONE) {
                MKLWrapper::cblas_sgemm_handler = (void *) hStreams_LibLoader::fetchExecFunctionAddress_nothrow("cblas_sgemm");
                MKLWrapper::cblas_dgemm_handler = (void *) hStreams_LibLoader::fetchExecFunctionAddress_nothrow("cblas_dgemm");
                MKLWrapper::cblas_cgemm_handler = (void *) hStreams_LibLoader::fetchExecFunctionAddress_nothrow("cblas_cgemm");
                MKLWrapper::cblas_zgemm_handler = (void *) hStreams_LibLoader::fetchExecFunctionAddress_nothrow("cblas_zgemm");
            }

            hStreams_returnErrorCodeThroughRetVal(HSTR_RESULT_SUCCESS, in_pReturnValue, in_ReturnValueLength);

        } else {
            HSTR_ERROR(HSTR_INFO_TYPE_SINK_INVOKE)
                    << "The size of input arguments for sink-side initialization function "
                    << "did not match what the function accepts. The function expected "
                    << sizeof(hStreams_InitSinkData) << " bytes and got " << in_MiscDataLength << " bytes.";

            hStreams_returnErrorCodeThroughRetVal(HSTR_RESULT_INTERNAL_ERROR, in_pReturnValue, in_ReturnValueLength);
        }
    } catch (...) {
        HSTR_ERROR(HSTR_INFO_TYPE_SINK_INVOKE) << "An exception occured during sink-side initialization.";
        hStreams_returnErrorCodeThroughRetVal(hStreams_handle_exception(), in_pReturnValue, in_ReturnValueLength);
    }
}

// Initialization the partitions as needed by OpenMP
// This function is called by the hStreams thunk
HSTREAMS_EXPORT
void hStreams_init_partition(
    uint64_t domain,
    uint64_t stream,
    uint64_t num_cores,
    uint64_t cpumask0,
    uint64_t /*cpumask1*/,
    uint64_t /*cpumask2*/,
    uint64_t /*cpumask3*/,
    uint64_t /*cpumask4*/,
    uint64_t /*cpumask5*/,
    uint64_t /*cpumask6*/,
    uint64_t /*cpumask7*/,
    uint64_t /*cpumask8*/,
    uint64_t /*cpumask9*/,
    uint64_t /*cpumask10*/,
    uint64_t /*cpumask11*/,
    uint64_t /*cpumask12*/,
    uint64_t /*cpumask13*/,
    uint64_t /*cpumask14*/,
    uint64_t /*cpumask15*/)
{
    //Based of pthread affinity (setup by COI or HostSideSinkWorker)
    //  this function is affinitazing openmp threads using openmp routine
    //  kmp_set_affinity to keep openmp library inform

    static bool defaultsAlreadySet = false;

    // Pick one of these
    if (!defaultsAlreadySet) {
        defaultsAlreadySet = true;
        // envirables are separated by ;
        kmp_set_defaults("KMP_AFFINITY=norespect,none"); // ignore default max_threads
        // Use KMP_AFFINITY=disabled if using sched_setaffinity
        // kmp_set_defaults("KMP_AFFINITY=norespect,disabled");
    }

    std::vector<int> hw_thread_IDs;

#ifndef _WIN32
    cpu_set_t stream_cpu_set;
    pthread_getaffinity_np(pthread_self(), sizeof(cpu_set_t), &stream_cpu_set);

    for (int i = 0; i < CPU_SETSIZE; ++i)
        if (CPU_ISSET(i, &stream_cpu_set)) {
            hw_thread_IDs.push_back(i);
        }
#else
    // FIXME: More than 64 threads on Windows aren't support by that function.
    for (int i = 0; i < 64; ++i)
        if (cpumask0 & (1ll) << i) {
            hw_thread_IDs.push_back(i);
        }
#endif

    omp_set_num_threads(hw_thread_IDs.size());

    HSTR_DEBUG1(HSTR_INFO_TYPE_MISC)
            << "Initializing sink partition, domain " << (int)(domain) << ", stream "
            << (int)(stream) << ", for " << hw_thread_IDs.size() << " threads.";

    #pragma omp parallel
    {
        //TODO: Add different bijection function for
        //HSTR_KMP_AFFINITY_SCATTER
        int core_id = omp_get_thread_num();

        kmp_affinity_mask_t cpu_mask;

        kmp_create_affinity_mask(&cpu_mask);
        kmp_set_affinity_mask_proc(hw_thread_IDs[core_id], &cpu_mask);

        int kmp_error = kmp_set_affinity(&cpu_mask);

        if (kmp_error != 0) {
            HSTR_WARN(HSTR_INFO_TYPE_MISC)
                    << "Couldn't affinitize openmp thread " << core_id << " to core "
                    << hw_thread_IDs[core_id] << " openmp error: " << kmp_error;
        }

        HSTR_DEBUG2(HSTR_INFO_TYPE_MISC)
                << "Thread " << core_id << " affinitized to core " << hw_thread_IDs[core_id];
    }

}

HSTREAMS_EXPORT
// This function is called by the hStreams thunk
void hStreams_null_func_1heap_arg(
    uint64_t addr)
{
    HSTR_DEBUG1(HSTR_INFO_TYPE_MISC) << "Arrived in null_func with no arguments.";
}

#ifndef HSTR_SOURCE
// Fetch the address of a sink-side function, and return same in in_pReturnValue:
// These arguments conform to the COIPipelineRunFunction template,
//  since this function is invoked directly from COI's thunk
HSTREAMS_EXPORT
void hStreams_fetchSinkFuncAddress(
    uint32_t         in_BufferCount,
    void           **in_ppBufferPointers,
    uint64_t        *in_pBufferLengths,
    void            *in_pMiscData,
    uint16_t         in_MiscDataLength,
    void            *in_pReturnValue,
    uint16_t         in_ReturnValueLength)
{

    /* Clear any current dl error: */
    char         *currentDlError = dlerror();
    func19_t     *target_func19  = (in_pMiscData == NULL) ?
                                   ((func19_t *)NULL) :
                                   ((func19_t *)dlsym(RTLD_DEFAULT, (const char *) in_pMiscData));

    /* Fetch any errors that happened from above call to dlsym(). See man page on dlsym() for
       the reason for this sequence. */
    currentDlError = dlerror();
    if (currentDlError) {
        HSTR_ERROR(HSTR_INFO_TYPE_SINK_INVOKE)
                << "dlsym() returned error: " << currentDlError << ", for function: " << in_pMiscData;

        target_func19 = ((func19_t *)NULL);
    }

    HSTR_DEBUG1(HSTR_INFO_TYPE_SINK_INVOKE)
            << "Address of target function name " << in_pMiscData << " is: " << (void *) target_func19;

    if (in_ReturnValueLength == sizeof(void *)) {
        memcpy(in_pReturnValue, &target_func19, sizeof(void *));
    }
}
#endif

