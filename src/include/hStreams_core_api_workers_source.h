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

#ifndef HSTREAMS_CORE_API_WORKERS_SOURCE_H
#define HSTREAMS_CORE_API_WORKERS_SOURCE_H

#include "hStreams_types.h"
#include "hStreams_COIWrapper.h"
#include <string>

/**
 * All workers reside in the detail namespace.
 *
 * Ideally we'd want the workers to be void functions
 * which throw on error so that the API function would look like this:
 *
 *      extern "C"
 *      HSTR_RESULT
 *      hStreams_my_function(<the arguments>)
 *      {
 *          try {
 *              HSTR_TRACE_API_ENTER();
 *              HSTR_TRACE_API_ARG(<first arguments>);
 *
 *              detail::my_function(<the arguments>);
 *              HSTR_RETURN(HSTR_RESULT_SUCCESS);
 *          } catch (...) {
 *              HSTR_RETURN(hStreams_handle_exception());
 *          }
 *      }
 *
 */

namespace detail
{

void
InitInVersion_impl_throw(const char *interface_version);

void
InitPhysicalDomains_impl_throw(HSTR_ISA_TYPE isa_type, std::string executableFileName,
                               std::string library_name, void *sink_startup_ptr, uint64_t sink_startup_size,
                               uint32_t &active_domains, HSTR_COIPROCESS &dummy_process, uint32_t &num_phys_domains);

void
IsInitialized_impl_throw();

HSTR_RESULT
IsInitialized_impl_nothrow();

void
Fini_impl_throw();

void
GetNumPhysDomains_impl_throw(
    uint32_t          *out_pNumPhysDomains,
    uint32_t          *out_pNumActivePhysDomains,
    bool              *out_pHomogeneous);

void
GetPhysDomainDetails_impl_throw(
    HSTR_PHYS_DOM       in_PhysDomainID,
    uint32_t           *out_pNumThreads,
    HSTR_ISA_TYPE      *out_pISA,
    uint32_t           *out_pCoreMaxMHz,
    HSTR_CPU_MASK       out_MaxCPUmask,
    HSTR_CPU_MASK       out_AvoidCPUmask,
    uint64_t           *out_pSupportedMemTypes,
    uint64_t            out_pPhysicalBytesPerMemType[HSTR_MEM_TYPE_SIZE]);

void
GetOversubscriptionLevel_impl_throw(
    HSTR_PHYS_DOM   in_PhysDomainID,
    uint32_t        in_NumThreads,
    uint32_t       *out_pOversubscriptionArray);

void
GetAvailable_impl_throw(
    HSTR_PHYS_DOM in_PhysDomainID,
    HSTR_CPU_MASK out_AvailableCPUmask);

void
AddLogDomain_impl_throw(
    HSTR_PHYS_DOM      in_PhysDomainID,
    HSTR_CPU_MASK      in_CPUmask,
    HSTR_LOG_DOM      *out_pLogDomainID,
    HSTR_OVERLAP_TYPE *out_pOverlap);

void
RmLogDomains_impl_throw(
    uint32_t       in_NumLogDomains,
    HSTR_LOG_DOM  *in_pLogDomainIDs);

void
GetNumLogDomains_impl_throw(
    HSTR_PHYS_DOM   in_PhysDomainID,
    uint32_t       *out_pNumLogDomains);

void
GetLogDomainIDList_impl_throw(
    HSTR_PHYS_DOM   in_PhysDomainID,
    uint32_t        in_NumLogDomains,
    HSTR_LOG_DOM   *out_pLogDomainIDs);

void
GetLogDomainDetails_impl_throw(
    HSTR_LOG_DOM   in_LogDomainID,
    HSTR_PHYS_DOM *out_pPhysDomainID,
    HSTR_CPU_MASK  out_CPUmask);

void
StreamCreate_impl_throw(
    HSTR_LOG_STR        in_LogStreamID,
    HSTR_LOG_DOM        in_LogDomainID,
    const HSTR_CPU_MASK in_CPUmask);

void
StreamDestroy_impl_throw(HSTR_LOG_STR in_LogStreamID);

void
GetNumLogStreams_impl_throw(
    HSTR_LOG_DOM   in_LogDomainID,
    uint32_t      *out_pNumLogStreams);

void
GetLogStreamIDList_impl_throw(
    HSTR_LOG_DOM  in_LogDomainID,
    uint32_t      in_NumLogStreams,
    HSTR_LOG_STR *out_pLogStreamIDs);

void
GetLogStreamDetails_impl_throw(
    HSTR_LOG_STR      in_LogStreamID,
    HSTR_CPU_MASK     out_CPUmask);

void
EnqueueCompute_impl_throw(
    HSTR_LOG_STR        in_LogStreamID,
    const char         *in_pFunctionName,
    uint32_t            in_numScalarArgs,
    uint32_t            in_numHeapArgs,
    uint64_t           *in_pArgs,
    HSTR_EVENT         *out_pEvent,
    void               *out_ReturnValue,
    uint16_t            in_ReturnValueSize);

void
EnqueueData1D_impl_throw(
    HSTR_LOG_STR        in_LogStreamID,
    void               *in_pWriteAddr,
    void               *in_pReadAddr,
    uint64_t            in_size,
    HSTR_XFER_DIRECTION in_XferDirection,
    HSTR_EVENT         *out_pEvent);

void
EnqueueDataXDomain1D_impl_throw(
    HSTR_LOG_STR        in_LogStreamID,
    void               *in_pWriteAddr,
    void               *in_pReadAddr,
    uint64_t            in_size,
    HSTR_LOG_DOM        in_dstLogDomain,
    HSTR_LOG_DOM        in_srcLogDomain,
    HSTR_EVENT         *out_pEvent);

void
StreamSynchronize_impl_throw(HSTR_LOG_STR in_LogStreamID);

void
ThreadSynchronize_impl_throw();

void
EventWait_impl_throw(
    uint32_t           in_NumEvents,
    HSTR_EVENT        *in_pEvents,
    bool               in_WaitForAll,
    int32_t            in_TimeOutMilliSeconds,
    uint32_t          *out_pNumSignaled,
    uint32_t          *out_pSignaledIndices);

void
EventStreamWait_impl_throw(
    HSTR_LOG_STR       in_LogStreamID,
    uint32_t           in_NumEvents,
    HSTR_EVENT        *in_pEvents,
    int32_t            in_NumAddresses,
    void             **in_pAddresses,
    HSTR_EVENT        *out_pEvent);

void
Alloc1DEx_impl_throw(
    void                    *in_BaseAddress,
    uint64_t                 in_Size,
    const HSTR_BUFFER_PROPS *in_pBufferProps,
    int64_t                  in_NumLogDomains,
    HSTR_LOG_DOM            *in_pLogDomainIDs);

void
AddBufferLogDomains_impl_throw(
    void            *in_Address,
    uint64_t         in_NumLogDomains,
    HSTR_LOG_DOM    *in_pLogDomainIDs);

void
RmBufferLogDomains_impl_throw(
    void               *in_Address,
    int64_t             in_NumLogDomains,
    HSTR_LOG_DOM       *in_pLogDomainIDs);

void
GetBufferNumLogDomains_impl_throw(
    void                *in_Address,
    uint64_t            *out_NumLogDomains);

void
GetBufferLogDomains_impl_throw(
    void                *in_Address,
    uint64_t             in_NumLogDomains,
    HSTR_LOG_DOM        *out_pLogDomains,
    uint64_t            *out_pNumLogDomains);

void
GetBufferProps_impl_throw(
    void                *in_Address,
    HSTR_BUFFER_PROPS   *out_BufferProps);

void
DeAlloc_impl_throw(void *in_Address);

void
Cfg_SetLogLevel(HSTR_LOG_LEVEL in_loglevel);


void
Cfg_SetLogInfoType(uint64_t in_info_type_mask);

void
Cfg_SetMKLInterface(HSTR_MKL_INTERFACE in_MKLInterface);

void
GetCurrentOptions_impl_throw(HSTR_OPTIONS *pCurrentOptions, uint64_t buffSize);

void
SetOptions_impl_throw(const HSTR_OPTIONS *in_options);

void
GetVersionStringLen_impl_throw(uint32_t *out_pVersionStringLen);

void
Version_impl_throw(char *buff, uint32_t buffLength);

} // namespace detail

#endif /* HSTREAMS_CORE_API_WORKERS_SOURCE_H */
