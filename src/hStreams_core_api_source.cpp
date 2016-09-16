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

#include <stdarg.h>
#include <string.h>

#include "hStreams_internal_vars_source.h"
#include "hStreams_exceptions.h"
#include "hStreams_Logger.h"
#include "hStreams_core_api_workers_source.h"
#include "hStreams_internal.h"
#include "hStreams_common.h"
#include "hStreams_exceptions.h"
#include "hStreams_internal_vars_common.h"
#include "hStreams_Logger.h"

HSTR_EXPORT_IN_VERSION(
    HSTR_RESULT,
    hStreams_Init,
    HSTREAMS_1.0)()
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_CORE_API_CALLCOUNTER();
        detail::InitInVersion_impl_throw(HSTR_MAGIC_PRE1_0_0_VERSION_STRING);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_DEFAULT_VERSION(
    HSTR_RESULT,
    hStreams_InitInVersion)(
        const char *interface_version)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG_STR(interface_version);
        HSTR_CORE_API_CALLCOUNTER();
        detail::InitInVersion_impl_throw(interface_version);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_VERSION(
    HSTR_RESULT,
    hStreams_IsInitialized,
    HSTREAMS_1.0)()
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_CORE_API_CALLCOUNTER();
        HSTR_RETURN(detail::IsInitialized_impl_nothrow());
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_DEFAULT_VERSION(
    HSTR_RESULT,
    hStreams_IsInitialized)()
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_CORE_API_CALLCOUNTER();
        HSTR_RETURN(detail::IsInitialized_impl_nothrow());
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_VERSION(
    HSTR_RESULT,
    hStreams_Fini,
    HSTREAMS_1.0)()
{
    try {
        HSTR_TRACE_API_ENTER();
        detail::Fini_impl_throw();
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_DEFAULT_VERSION(
    HSTR_RESULT,
    hStreams_Fini)()
{
    try {
        HSTR_TRACE_API_ENTER();
        detail::Fini_impl_throw();
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_VERSION(
    HSTR_RESULT,
    hStreams_GetNumPhysDomains,
    HSTREAMS_1.0)(
        uint32_t          *out_pNumPhysDomains,
        uint32_t          *out_pNumActivePhysDomains,
        bool              *out_pHomogeneous)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(out_pNumPhysDomains);
        HSTR_TRACE_API_ARG(out_pNumActivePhysDomains);
        HSTR_TRACE_API_ARG(out_pHomogeneous);
        HSTR_CORE_API_CALLCOUNTER();

        detail::GetNumPhysDomains_impl_throw(out_pNumPhysDomains,
                                             out_pNumActivePhysDomains, out_pHomogeneous);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_DEFAULT_VERSION(
    HSTR_RESULT,
    hStreams_GetNumPhysDomains)(
        uint32_t          *out_pNumPhysDomains,
        uint32_t          *out_pNumActivePhysDomains,
        bool              *out_pHomogeneous)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(out_pNumPhysDomains);
        HSTR_TRACE_API_ARG(out_pNumActivePhysDomains);
        HSTR_TRACE_API_ARG(out_pHomogeneous);
        HSTR_CORE_API_CALLCOUNTER();

        detail::GetNumPhysDomains_impl_throw(out_pNumPhysDomains,
                                             out_pNumActivePhysDomains, out_pHomogeneous);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_VERSION(
    HSTR_RESULT,
    hStreams_GetPhysDomainDetails,
    HSTREAMS_1.0)(
        HSTR_PHYS_DOM       in_PhysDomain,
        uint32_t           *out_pNumThreads,
        HSTR_ISA_TYPE      *out_pISA,
        uint32_t           *out_pCoreMaxMHz,
        HSTR_CPU_MASK       out_MaxCPUmask,
        HSTR_CPU_MASK       out_AvoidCPUmask,
        uint64_t           *out_pSupportedMemTypes,
        uint64_t            out_pPhysicalBytesPerMemType[HSTR_MEM_TYPE_SIZE])
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_PhysDomain);
        HSTR_TRACE_API_ARG(out_pNumThreads);
        HSTR_TRACE_API_ARG(out_pISA);
        HSTR_TRACE_API_ARG(out_pCoreMaxMHz);
        HSTR_TRACE_API_ARG(out_MaxCPUmask);
        HSTR_TRACE_API_ARG(out_AvoidCPUmask);
        HSTR_TRACE_API_ARG(out_pSupportedMemTypes);
        HSTR_TRACE_API_ARG(out_pPhysicalBytesPerMemType);
        HSTR_CORE_API_CALLCOUNTER();

        detail::GetPhysDomainDetails_impl_throw(
            in_PhysDomain,
            out_pNumThreads,
            out_pISA,
            out_pCoreMaxMHz,
            out_MaxCPUmask,
            out_AvoidCPUmask,
            out_pSupportedMemTypes,
            out_pPhysicalBytesPerMemType);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_DEFAULT_VERSION(
    HSTR_RESULT,
    hStreams_GetPhysDomainDetails)(
        HSTR_PHYS_DOM       in_PhysDomain,
        uint32_t           *out_pNumThreads,
        HSTR_ISA_TYPE      *out_pISA,
        uint32_t           *out_pCoreMaxMHz,
        HSTR_CPU_MASK       out_MaxCPUmask,
        HSTR_CPU_MASK       out_AvoidCPUmask,
        uint64_t           *out_pSupportedMemTypes,
        uint64_t            out_pPhysicalBytesPerMemType[HSTR_MEM_TYPE_SIZE])
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_PhysDomain);
        HSTR_TRACE_API_ARG(out_pNumThreads);
        HSTR_TRACE_API_ARG(out_pISA);
        HSTR_TRACE_API_ARG(out_pCoreMaxMHz);
        HSTR_TRACE_API_ARG(out_MaxCPUmask);
        HSTR_TRACE_API_ARG(out_AvoidCPUmask);
        HSTR_TRACE_API_ARG(out_pSupportedMemTypes);
        HSTR_TRACE_API_ARG(out_pPhysicalBytesPerMemType);
        HSTR_CORE_API_CALLCOUNTER();

        detail::GetPhysDomainDetails_impl_throw(
            in_PhysDomain,
            out_pNumThreads,
            out_pISA,
            out_pCoreMaxMHz,
            out_MaxCPUmask,
            out_AvoidCPUmask,
            out_pSupportedMemTypes,
            out_pPhysicalBytesPerMemType);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}


HSTR_EXPORT_IN_VERSION(
    HSTR_RESULT,
    hStreams_GetOversubscriptionLevel,
    HSTREAMS_1.0)(
        HSTR_PHYS_DOM   in_PhysDomainID,
        uint32_t        in_NumThreads,
        uint32_t       *out_pOversubscriptionArray)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_PhysDomainID);
        HSTR_TRACE_API_ARG(in_NumThreads);
        HSTR_TRACE_API_ARG(out_pOversubscriptionArray);
        HSTR_CORE_API_CALLCOUNTER();

        detail::GetOversubscriptionLevel_impl_throw(in_PhysDomainID,
                in_NumThreads, out_pOversubscriptionArray);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_DEFAULT_VERSION(
    HSTR_RESULT,
    hStreams_GetOversubscriptionLevel)(
        HSTR_PHYS_DOM   in_PhysDomainID,
        uint32_t        in_NumThreads,
        uint32_t       *out_pOversubscriptionArray)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_PhysDomainID);
        HSTR_TRACE_API_ARG(in_NumThreads);
        HSTR_TRACE_API_ARG(out_pOversubscriptionArray);
        HSTR_CORE_API_CALLCOUNTER();

        detail::GetOversubscriptionLevel_impl_throw(in_PhysDomainID,
                in_NumThreads, out_pOversubscriptionArray);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_VERSION(
    HSTR_RESULT,
    hStreams_GetAvailable,
    HSTREAMS_1.0)(
        HSTR_PHYS_DOM       in_PhysDomainID,
        HSTR_CPU_MASK       out_AvailableCPUmask)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_PhysDomainID);
        HSTR_TRACE_API_ARG(out_AvailableCPUmask);
        HSTR_CORE_API_CALLCOUNTER();

        detail::GetAvailable_impl_throw(in_PhysDomainID,
                                        out_AvailableCPUmask);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_DEFAULT_VERSION(
    HSTR_RESULT,
    hStreams_GetAvailable)(
        HSTR_PHYS_DOM       in_PhysDomainID,
        HSTR_CPU_MASK       out_AvailableCPUmask)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_PhysDomainID);
        HSTR_TRACE_API_ARG(out_AvailableCPUmask);
        HSTR_CORE_API_CALLCOUNTER();

        detail::GetAvailable_impl_throw(in_PhysDomainID,
                                        out_AvailableCPUmask);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_VERSION(
    HSTR_RESULT,
    hStreams_AddLogDomain,
    HSTREAMS_1.0)(
        HSTR_PHYS_DOM      in_PhysDomainID,
        HSTR_CPU_MASK      in_CPUmask,
        HSTR_LOG_DOM      *out_pLogDomainID,
        HSTR_OVERLAP_TYPE *out_pOverlap)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_PhysDomainID);
        HSTR_TRACE_API_ARG(in_CPUmask);
        HSTR_TRACE_API_ARG(out_pLogDomainID);
        HSTR_TRACE_API_ARG(out_pOverlap);
        HSTR_CORE_API_CALLCOUNTER();

        detail::AddLogDomain_impl_throw(in_PhysDomainID,
                                        in_CPUmask, out_pLogDomainID, out_pOverlap);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_DEFAULT_VERSION(
    HSTR_RESULT,
    hStreams_AddLogDomain)(
        HSTR_PHYS_DOM      in_PhysDomainID,
        HSTR_CPU_MASK      in_CPUmask,
        HSTR_LOG_DOM      *out_pLogDomainID,
        HSTR_OVERLAP_TYPE *out_pOverlap)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_PhysDomainID);
        HSTR_TRACE_API_ARG(in_CPUmask);
        HSTR_TRACE_API_ARG(out_pLogDomainID);
        HSTR_TRACE_API_ARG(out_pOverlap);
        HSTR_CORE_API_CALLCOUNTER();

        detail::AddLogDomain_impl_throw(in_PhysDomainID,
                                        in_CPUmask, out_pLogDomainID, out_pOverlap);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_VERSION(
    HSTR_RESULT,
    hStreams_RmLogDomains,
    HSTREAMS_1.0)(
        uint32_t       in_NumLogDomains,
        HSTR_LOG_DOM  *in_pLogDomainIDs)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_NumLogDomains);
        HSTR_TRACE_API_ARG(in_pLogDomainIDs);
        HSTR_CORE_API_CALLCOUNTER();
        detail::RmLogDomains_impl_throw(in_NumLogDomains, in_pLogDomainIDs);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_DEFAULT_VERSION(
    HSTR_RESULT,
    hStreams_RmLogDomains)(
        uint32_t       in_NumLogDomains,
        HSTR_LOG_DOM  *in_pLogDomainIDs)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_NumLogDomains);
        HSTR_TRACE_API_ARG(in_pLogDomainIDs);
        HSTR_CORE_API_CALLCOUNTER();
        detail::RmLogDomains_impl_throw(in_NumLogDomains, in_pLogDomainIDs);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_VERSION(
    HSTR_RESULT,
    hStreams_GetNumLogDomains,
    HSTREAMS_1.0)(
        HSTR_PHYS_DOM   in_PhysDomainID,
        uint32_t       *out_pNumLogDomains)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_PhysDomainID);
        HSTR_TRACE_API_ARG(out_pNumLogDomains);
        HSTR_CORE_API_CALLCOUNTER();
        detail::GetNumLogDomains_impl_throw(in_PhysDomainID, out_pNumLogDomains);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_DEFAULT_VERSION(
    HSTR_RESULT,
    hStreams_GetNumLogDomains)(
        HSTR_PHYS_DOM   in_PhysDomainID,
        uint32_t       *out_pNumLogDomains)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_PhysDomainID);
        HSTR_TRACE_API_ARG(out_pNumLogDomains);
        HSTR_CORE_API_CALLCOUNTER();
        detail::GetNumLogDomains_impl_throw(in_PhysDomainID, out_pNumLogDomains);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_VERSION(
    HSTR_RESULT,
    hStreams_GetLogDomainIDList,
    HSTREAMS_1.0)(
        HSTR_PHYS_DOM   in_PhysDomainID,
        uint32_t        in_NumLogDomains,
        HSTR_LOG_DOM   *out_pLogDomainIDs)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_PhysDomainID);
        HSTR_TRACE_API_ARG(in_NumLogDomains);
        HSTR_TRACE_API_ARG(out_pLogDomainIDs);
        HSTR_CORE_API_CALLCOUNTER();
        detail::GetLogDomainIDList_impl_throw(in_PhysDomainID, in_NumLogDomains,
                                              out_pLogDomainIDs);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_DEFAULT_VERSION(
    HSTR_RESULT,
    hStreams_GetLogDomainIDList)(
        HSTR_PHYS_DOM   in_PhysDomainID,
        uint32_t        in_NumLogDomains,
        HSTR_LOG_DOM   *out_pLogDomainIDs)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_PhysDomainID);
        HSTR_TRACE_API_ARG(in_NumLogDomains);
        HSTR_TRACE_API_ARG(out_pLogDomainIDs);
        HSTR_CORE_API_CALLCOUNTER();
        detail::GetLogDomainIDList_impl_throw(in_PhysDomainID, in_NumLogDomains,
                                              out_pLogDomainIDs);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_VERSION(
    HSTR_RESULT,
    hStreams_GetLogDomainDetails,
    HSTREAMS_1.0)(
        HSTR_LOG_DOM   in_LogDomainID,
        HSTR_PHYS_DOM *out_pPhysDomainID,
        HSTR_CPU_MASK  out_CPUmask)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_LogDomainID);
        HSTR_TRACE_API_ARG(out_pPhysDomainID);
        HSTR_TRACE_API_ARG(out_CPUmask);
        HSTR_CORE_API_CALLCOUNTER();

        detail::GetLogDomainDetails_impl_throw(in_LogDomainID, out_pPhysDomainID,
                                               out_CPUmask);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_DEFAULT_VERSION(
    HSTR_RESULT,
    hStreams_GetLogDomainDetails)(
        HSTR_LOG_DOM   in_LogDomainID,
        HSTR_PHYS_DOM *out_pPhysDomainID,
        HSTR_CPU_MASK  out_CPUmask)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_LogDomainID);
        HSTR_TRACE_API_ARG(out_pPhysDomainID);
        HSTR_TRACE_API_ARG(out_CPUmask);
        HSTR_CORE_API_CALLCOUNTER();

        detail::GetLogDomainDetails_impl_throw(in_LogDomainID, out_pPhysDomainID,
                                               out_CPUmask);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_VERSION(
    HSTR_RESULT,
    hStreams_StreamCreate,
    HSTREAMS_1.0)(
        HSTR_LOG_STR        in_LogStreamID,
        HSTR_LOG_DOM        in_LogDomainID,
        const HSTR_CPU_MASK in_CPUmask)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_LogStreamID);
        HSTR_TRACE_API_ARG(in_LogDomainID);
        HSTR_TRACE_API_ARG(in_CPUmask);
        HSTR_CORE_API_CALLCOUNTER();

        detail::StreamCreate_impl_throw(in_LogStreamID, in_LogDomainID,
                                        in_CPUmask);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_DEFAULT_VERSION(
    HSTR_RESULT,
    hStreams_StreamCreate)(
        HSTR_LOG_STR        in_LogStreamID,
        HSTR_LOG_DOM        in_LogDomainID,
        const HSTR_CPU_MASK in_CPUmask)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_LogStreamID);
        HSTR_TRACE_API_ARG(in_LogDomainID);
        HSTR_TRACE_API_ARG(in_CPUmask);
        HSTR_CORE_API_CALLCOUNTER();

        detail::StreamCreate_impl_throw(in_LogStreamID, in_LogDomainID,
                                        in_CPUmask);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_VERSION(
    HSTR_RESULT,
    hStreams_StreamDestroy,
    HSTREAMS_1.0)(
        HSTR_LOG_STR    in_LogStreamID)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_LogStreamID);
        HSTR_CORE_API_CALLCOUNTER();
        detail::StreamDestroy_impl_throw(in_LogStreamID);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_DEFAULT_VERSION(
    HSTR_RESULT,
    hStreams_StreamDestroy)(
        HSTR_LOG_STR    in_LogStreamID)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_LogStreamID);
        HSTR_CORE_API_CALLCOUNTER();
        detail::StreamDestroy_impl_throw(in_LogStreamID);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_VERSION(
    HSTR_RESULT,
    hStreams_GetNumLogStreams,
    HSTREAMS_1.0)(
        HSTR_LOG_DOM   in_LogDomainID,
        uint32_t      *out_pNumLogStreams)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_LogDomainID);
        HSTR_TRACE_API_ARG(out_pNumLogStreams);
        HSTR_CORE_API_CALLCOUNTER();
        detail::GetNumLogStreams_impl_throw(in_LogDomainID, out_pNumLogStreams);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_DEFAULT_VERSION(
    HSTR_RESULT,
    hStreams_GetNumLogStreams)(
        HSTR_LOG_DOM   in_LogDomainID,
        uint32_t      *out_pNumLogStreams)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_LogDomainID);
        HSTR_TRACE_API_ARG(out_pNumLogStreams);
        HSTR_CORE_API_CALLCOUNTER();
        detail::GetNumLogStreams_impl_throw(in_LogDomainID, out_pNumLogStreams);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_VERSION(
    HSTR_RESULT,
    hStreams_GetLogStreamIDList,
    HSTREAMS_1.0)(
        HSTR_LOG_DOM  in_LogDomainID,
        uint32_t      in_NumLogStreams,
        HSTR_LOG_STR *out_pLogStreamIDs)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_LogDomainID);
        HSTR_TRACE_API_ARG(in_NumLogStreams);
        HSTR_TRACE_API_ARG(out_pLogStreamIDs);
        HSTR_CORE_API_CALLCOUNTER();
        detail::GetLogStreamIDList_impl_throw(in_LogDomainID, in_NumLogStreams,
                                              out_pLogStreamIDs);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_DEFAULT_VERSION(
    HSTR_RESULT,
    hStreams_GetLogStreamIDList)(
        HSTR_LOG_DOM  in_LogDomainID,
        uint32_t      in_NumLogStreams,
        HSTR_LOG_STR *out_pLogStreamIDs)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_LogDomainID);
        HSTR_TRACE_API_ARG(in_NumLogStreams);
        HSTR_TRACE_API_ARG(out_pLogStreamIDs);
        HSTR_CORE_API_CALLCOUNTER();
        detail::GetLogStreamIDList_impl_throw(in_LogDomainID, in_NumLogStreams,
                                              out_pLogStreamIDs);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_VERSION(
    HSTR_RESULT,
    hStreams_GetLogStreamDetails,
    HSTREAMS_1.0)(
        HSTR_LOG_STR      in_LogStreamID,
        HSTR_LOG_DOM      /*in_LogDomainID*/,
        HSTR_CPU_MASK     out_CPUmask)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_LogStreamID);
        HSTR_TRACE_API_ARG(out_CPUmask);
        HSTR_CORE_API_CALLCOUNTER();
        detail::GetLogStreamDetails_impl_throw(in_LogStreamID, out_CPUmask);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_DEFAULT_VERSION(
    HSTR_RESULT,
    hStreams_GetLogStreamDetails)(
        HSTR_LOG_STR      in_LogStreamID,
        HSTR_LOG_DOM      /*in_LogDomainID*/,
        HSTR_CPU_MASK     out_CPUmask)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_LogStreamID);
        HSTR_TRACE_API_ARG(out_CPUmask);
        HSTR_CORE_API_CALLCOUNTER();
        detail::GetLogStreamDetails_impl_throw(in_LogStreamID, out_CPUmask);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_VERSION(
    HSTR_RESULT,
    hStreams_EnqueueCompute,
    HSTREAMS_1.0)(
        HSTR_LOG_STR        in_LogStreamID,
        const char         *in_pFunctionName,
        uint32_t            in_numScalarArgs,
        uint32_t            in_numHeapArgs,
        uint64_t           *in_pArgs,
        HSTR_EVENT         *out_pEvent,
        void               *out_ReturnValue,
        uint32_t            in_ReturnValueSize)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_LogStreamID);
        HSTR_TRACE_API_ARG_STR(in_pFunctionName);
        HSTR_TRACE_API_ARG(in_numScalarArgs);
        HSTR_TRACE_API_ARG(in_numHeapArgs);
        HSTR_TRACE_API_ARG(in_pArgs);
        HSTR_TRACE_API_ARG(out_pEvent);
        HSTR_TRACE_API_ARG(out_ReturnValue);
        HSTR_TRACE_API_ARG(in_ReturnValueSize);
        HSTR_CORE_API_CALLCOUNTER();

        detail::EnqueueCompute_impl_throw(in_LogStreamID,
                                          in_pFunctionName,
                                          in_numScalarArgs,
                                          in_numHeapArgs,
                                          in_pArgs,
                                          out_pEvent,
                                          out_ReturnValue,
                                          in_ReturnValueSize);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_VERSION(
    HSTR_RESULT,
    hStreams_EnqueueCompute,
    HSTREAMS_2.0)(
        HSTR_LOG_STR        in_LogStreamID,
        const char         *in_pFunctionName,
        uint32_t            in_numScalarArgs,
        uint32_t            in_numHeapArgs,
        uint64_t           *in_pArgs,
        HSTR_EVENT         *out_pEvent,
        void               *out_ReturnValue,
        uint16_t            in_ReturnValueSize)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_LogStreamID);
        HSTR_TRACE_API_ARG_STR(in_pFunctionName);
        HSTR_TRACE_API_ARG(in_numScalarArgs);
        HSTR_TRACE_API_ARG(in_numHeapArgs);
        HSTR_TRACE_API_ARG(in_pArgs);
        HSTR_TRACE_API_ARG(out_pEvent);
        HSTR_TRACE_API_ARG(out_ReturnValue);
        HSTR_TRACE_API_ARG(in_ReturnValueSize);
        HSTR_CORE_API_CALLCOUNTER();

        detail::EnqueueCompute_impl_throw(in_LogStreamID,
                                          in_pFunctionName,
                                          in_numScalarArgs,
                                          in_numHeapArgs,
                                          in_pArgs,
                                          out_pEvent,
                                          out_ReturnValue,
                                          in_ReturnValueSize);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_DEFAULT_VERSION(
    HSTR_RESULT,
    hStreams_EnqueueCompute)(
        HSTR_LOG_STR        in_LogStreamID,
        const char         *in_pFunctionName,
        uint32_t            in_numScalarArgs,
        uint32_t            in_numHeapArgs,
        uint64_t           *in_pArgs,
        HSTR_EVENT         *out_pEvent,
        void               *out_ReturnValue,
        uint16_t            in_ReturnValueSize)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_LogStreamID);
        HSTR_TRACE_API_ARG_STR(in_pFunctionName);
        HSTR_TRACE_API_ARG(in_numScalarArgs);
        HSTR_TRACE_API_ARG(in_numHeapArgs);
        HSTR_TRACE_API_ARG(in_pArgs);
        HSTR_TRACE_API_ARG(out_pEvent);
        HSTR_TRACE_API_ARG(out_ReturnValue);
        HSTR_TRACE_API_ARG(in_ReturnValueSize);
        HSTR_CORE_API_CALLCOUNTER();

        detail::EnqueueCompute_impl_throw(in_LogStreamID,
                                          in_pFunctionName,
                                          in_numScalarArgs,
                                          in_numHeapArgs,
                                          in_pArgs,
                                          out_pEvent,
                                          out_ReturnValue,
                                          in_ReturnValueSize);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_VERSION(
    HSTR_RESULT,
    hStreams_EnqueueData1D,
    HSTREAMS_1.0)(
        HSTR_LOG_STR        in_LogStreamID,
        void               *in_pWriteAddr,
        void               *in_pReadAddr,
        uint64_t            in_size,
        HSTR_XFER_DIRECTION in_XferDirection,
        HSTR_EVENT         *out_pEvent)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_LogStreamID);
        HSTR_TRACE_API_ARG(in_pWriteAddr);
        HSTR_TRACE_API_ARG(in_pReadAddr);
        HSTR_TRACE_API_ARG(in_size);
        HSTR_TRACE_API_ARG(in_XferDirection);
        HSTR_TRACE_API_ARG(out_pEvent);
        HSTR_CORE_API_CALLCOUNTER();

        detail::EnqueueData1D_impl_throw(in_LogStreamID,
                                         in_pWriteAddr,
                                         in_pReadAddr,
                                         in_size,
                                         in_XferDirection,
                                         out_pEvent);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_DEFAULT_VERSION(
    HSTR_RESULT,
    hStreams_EnqueueData1D)(
        HSTR_LOG_STR        in_LogStreamID,
        void               *in_pWriteAddr,
        void               *in_pReadAddr,
        uint64_t            in_size,
        HSTR_XFER_DIRECTION in_XferDirection,
        HSTR_EVENT         *out_pEvent)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_LogStreamID);
        HSTR_TRACE_API_ARG(in_pWriteAddr);
        HSTR_TRACE_API_ARG(in_pReadAddr);
        HSTR_TRACE_API_ARG(in_size);
        HSTR_TRACE_API_ARG(in_XferDirection);
        HSTR_TRACE_API_ARG(out_pEvent);
        HSTR_CORE_API_CALLCOUNTER();

        detail::EnqueueData1D_impl_throw(in_LogStreamID,
                                         in_pWriteAddr,
                                         in_pReadAddr,
                                         in_size,
                                         in_XferDirection,
                                         out_pEvent);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_VERSION(
    HSTR_RESULT,
    hStreams_EnqueueDataXDomain1D,
    HSTREAMS_1.0)(
        HSTR_LOG_STR        in_LogStreamID,
        void               *in_pWriteAddr,
        void               *in_pReadAddr,
        uint64_t            in_size,
        HSTR_LOG_DOM        in_dstLogDomain,
        HSTR_LOG_DOM        in_srcLogDomain,
        HSTR_EVENT         *out_pEvent)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_LogStreamID);
        HSTR_TRACE_API_ARG(in_pWriteAddr);
        HSTR_TRACE_API_ARG(in_pReadAddr);
        HSTR_TRACE_API_ARG(in_size);
        HSTR_TRACE_API_ARG(in_dstLogDomain);
        HSTR_TRACE_API_ARG(in_srcLogDomain);
        HSTR_TRACE_API_ARG(out_pEvent);
        HSTR_CORE_API_CALLCOUNTER();

        detail::EnqueueDataXDomain1D_impl_throw(in_LogStreamID,
                                                in_pWriteAddr,
                                                in_pReadAddr,
                                                in_size,
                                                in_dstLogDomain,
                                                in_srcLogDomain,
                                                out_pEvent);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_DEFAULT_VERSION(
    HSTR_RESULT,
    hStreams_EnqueueDataXDomain1D)(
        HSTR_LOG_STR        in_LogStreamID,
        void               *in_pWriteAddr,
        void               *in_pReadAddr,
        uint64_t            in_size,
        HSTR_LOG_DOM        in_dstLogDomain,
        HSTR_LOG_DOM        in_srcLogDomain,
        HSTR_EVENT         *out_pEvent)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_LogStreamID);
        HSTR_TRACE_API_ARG(in_pWriteAddr);
        HSTR_TRACE_API_ARG(in_pReadAddr);
        HSTR_TRACE_API_ARG(in_size);
        HSTR_TRACE_API_ARG(in_dstLogDomain);
        HSTR_TRACE_API_ARG(in_srcLogDomain);
        HSTR_TRACE_API_ARG(out_pEvent);
        HSTR_CORE_API_CALLCOUNTER();

        detail::EnqueueDataXDomain1D_impl_throw(in_LogStreamID,
                                                in_pWriteAddr,
                                                in_pReadAddr,
                                                in_size,
                                                in_dstLogDomain,
                                                in_srcLogDomain,
                                                out_pEvent);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}


HSTR_EXPORT_IN_VERSION(
    HSTR_RESULT,
    hStreams_StreamSynchronize,
    HSTREAMS_1.0)(
        HSTR_LOG_STR      in_LogStreamID)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_LogStreamID);
        HSTR_CORE_API_CALLCOUNTER();
        detail::StreamSynchronize_impl_throw(in_LogStreamID);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_DEFAULT_VERSION(
    HSTR_RESULT,
    hStreams_StreamSynchronize)(
        HSTR_LOG_STR      in_LogStreamID)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_LogStreamID);
        HSTR_CORE_API_CALLCOUNTER();
        detail::StreamSynchronize_impl_throw(in_LogStreamID);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_VERSION(
    HSTR_RESULT,
    hStreams_ThreadSynchronize,
    HSTREAMS_1.0)()
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_CORE_API_CALLCOUNTER();
        detail::ThreadSynchronize_impl_throw();
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_DEFAULT_VERSION(
    HSTR_RESULT,
    hStreams_ThreadSynchronize)()
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_CORE_API_CALLCOUNTER();
        detail::ThreadSynchronize_impl_throw();
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_VERSION(
    HSTR_RESULT,
    hStreams_EventWait,
    HSTREAMS_1.0)(
        uint32_t           in_NumEvents,
        HSTR_EVENT        *in_pEvents,
        bool               in_WaitForAll,
        int32_t            in_TimeOutMilliSeconds,
        uint32_t          *out_pNumSignaled,
        uint32_t          *out_pSignaledIndices)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_NumEvents);
        HSTR_TRACE_API_ARG(in_pEvents);
        HSTR_TRACE_API_ARG(in_WaitForAll);
        HSTR_TRACE_API_ARG(in_TimeOutMilliSeconds);
        HSTR_TRACE_API_ARG(out_pNumSignaled);
        HSTR_TRACE_API_ARG(out_pSignaledIndices);
        HSTR_CORE_API_CALLCOUNTER();
        detail::EventWait_impl_throw(in_NumEvents, in_pEvents, in_WaitForAll,
                                     in_TimeOutMilliSeconds, out_pNumSignaled, out_pSignaledIndices);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_DEFAULT_VERSION(
    HSTR_RESULT,
    hStreams_EventWait)(
        uint32_t           in_NumEvents,
        HSTR_EVENT        *in_pEvents,
        bool               in_WaitForAll,
        int32_t            in_TimeOutMilliSeconds,
        uint32_t          *out_pNumSignaled,
        uint32_t          *out_pSignaledIndices)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_NumEvents);
        HSTR_TRACE_API_ARG(in_pEvents);
        HSTR_TRACE_API_ARG(in_WaitForAll);
        HSTR_TRACE_API_ARG(in_TimeOutMilliSeconds);
        HSTR_TRACE_API_ARG(out_pNumSignaled);
        HSTR_TRACE_API_ARG(out_pSignaledIndices);
        HSTR_CORE_API_CALLCOUNTER();
        detail::EventWait_impl_throw(in_NumEvents, in_pEvents, in_WaitForAll,
                                     in_TimeOutMilliSeconds, out_pNumSignaled, out_pSignaledIndices);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_VERSION(
    HSTR_RESULT,
    hStreams_EventStreamWait,
    HSTREAMS_1.0)(
        HSTR_LOG_STR       in_LogStreamID,
        uint32_t           in_NumEvents,
        HSTR_EVENT        *in_pEvents,
        int32_t            in_NumAddresses,
        void             **in_pAddresses,
        HSTR_EVENT        *out_pEvent)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_LogStreamID);
        HSTR_TRACE_API_ARG(in_NumEvents);
        HSTR_TRACE_API_ARG(in_pEvents);
        HSTR_TRACE_API_ARG(in_NumAddresses);
        HSTR_TRACE_API_ARG(in_pAddresses);
        HSTR_TRACE_API_ARG(out_pEvent);
        HSTR_CORE_API_CALLCOUNTER();
        detail::EventStreamWait_impl_throw(
            in_LogStreamID,
            in_NumEvents,
            in_pEvents,
            in_NumAddresses,
            in_pAddresses,
            out_pEvent);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_DEFAULT_VERSION(
    HSTR_RESULT,
    hStreams_EventStreamWait)(
        HSTR_LOG_STR       in_LogStreamID,
        uint32_t           in_NumEvents,
        HSTR_EVENT        *in_pEvents,
        int32_t            in_NumAddresses,
        void             **in_pAddresses,
        HSTR_EVENT        *out_pEvent)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_LogStreamID);
        HSTR_TRACE_API_ARG(in_NumEvents);
        HSTR_TRACE_API_ARG(in_pEvents);
        HSTR_TRACE_API_ARG(in_NumAddresses);
        HSTR_TRACE_API_ARG(in_pAddresses);
        HSTR_TRACE_API_ARG(out_pEvent);
        HSTR_CORE_API_CALLCOUNTER();
        detail::EventStreamWait_impl_throw(
            in_LogStreamID,
            in_NumEvents,
            in_pEvents,
            in_NumAddresses,
            in_pAddresses,
            out_pEvent);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_VERSION(
    HSTR_RESULT,
    hStreams_Alloc1D,
    HSTREAMS_1.0)(
        void               *in_BaseAddress,
        uint64_t            in_size)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_BaseAddress);
        HSTR_TRACE_API_ARG(in_size);
        HSTR_CORE_API_CALLCOUNTER();

        // Use default Alloc1D properties and create buffer for all logical domains
        HSTR_BUFFER_PROPS buffer_props = HSTR_BUFFER_PROPS_INITIAL_VALUES;
        detail::Alloc1DEx_impl_throw(
            in_BaseAddress,
            in_size,
            &buffer_props,
            -1,
            NULL);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_DEFAULT_VERSION(
    HSTR_RESULT,
    hStreams_Alloc1D)(
        void               *in_BaseAddress,
        uint64_t            in_size)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_BaseAddress);
        HSTR_TRACE_API_ARG(in_size);
        HSTR_CORE_API_CALLCOUNTER();

        // Use default Alloc1D properties and create buffer for all logical domains
        HSTR_BUFFER_PROPS buffer_props = HSTR_BUFFER_PROPS_INITIAL_VALUES;
        detail::Alloc1DEx_impl_throw(
            in_BaseAddress,
            in_size,
            &buffer_props,
            -1,
            NULL);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_VERSION(
    HSTR_RESULT,
    hStreams_Alloc1DEx,
    HSTREAMS_1.0)(
        void                 *in_BaseAddress,
        uint64_t              in_Size,
        HSTR_BUFFER_PROPS    *in_pBufferProps,
        int64_t               in_NumLogDomains,
        HSTR_LOG_DOM         *in_pLogDomainIDs)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_BaseAddress);
        HSTR_TRACE_API_ARG(in_Size);
        HSTR_TRACE_API_ARG(in_pBufferProps);
        HSTR_TRACE_API_ARG(in_NumLogDomains);
        HSTR_TRACE_API_ARG(in_pLogDomainIDs);
        HSTR_CORE_API_CALLCOUNTER();

        detail::Alloc1DEx_impl_throw(
            in_BaseAddress,
            in_Size,
            in_pBufferProps,
            in_NumLogDomains,
            in_pLogDomainIDs);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_DEFAULT_VERSION(
    HSTR_RESULT,
    hStreams_Alloc1DEx)(
        void                 *in_BaseAddress,
        uint64_t              in_Size,
        HSTR_BUFFER_PROPS    *in_pBufferProps,
        int64_t               in_NumLogDomains,
        HSTR_LOG_DOM         *in_pLogDomainIDs)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_BaseAddress);
        HSTR_TRACE_API_ARG(in_Size);
        HSTR_TRACE_API_ARG(in_pBufferProps);
        HSTR_TRACE_API_ARG(in_NumLogDomains);
        HSTR_TRACE_API_ARG(in_pLogDomainIDs);
        HSTR_CORE_API_CALLCOUNTER();

        detail::Alloc1DEx_impl_throw(
            in_BaseAddress,
            in_Size,
            in_pBufferProps,
            in_NumLogDomains,
            in_pLogDomainIDs);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_VERSION(
    HSTR_RESULT,
    hStreams_AddBufferLogDomains,
    HSTREAMS_1.0)(
        void            *in_Address,
        uint64_t         in_NumLogDomains,
        HSTR_LOG_DOM    *in_pLogDomainIDs)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_Address);
        HSTR_TRACE_API_ARG(in_NumLogDomains);
        HSTR_TRACE_API_ARG(in_pLogDomainIDs);
        HSTR_CORE_API_CALLCOUNTER();

        detail::AddBufferLogDomains_impl_throw(
            in_Address,
            in_NumLogDomains,
            in_pLogDomainIDs);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_DEFAULT_VERSION(
    HSTR_RESULT,
    hStreams_AddBufferLogDomains)(
        void            *in_Address,
        uint64_t         in_NumLogDomains,
        HSTR_LOG_DOM    *in_pLogDomainIDs)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_Address);
        HSTR_TRACE_API_ARG(in_NumLogDomains);
        HSTR_TRACE_API_ARG(in_pLogDomainIDs);
        HSTR_CORE_API_CALLCOUNTER();

        detail::AddBufferLogDomains_impl_throw(
            in_Address,
            in_NumLogDomains,
            in_pLogDomainIDs);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}


HSTR_EXPORT_IN_VERSION(
    HSTR_RESULT,
    hStreams_RmBufferLogDomains,
    HSTREAMS_1.0)(
        void            *in_Address,
        int64_t          in_NumLogDomains,
        HSTR_LOG_DOM    *in_pLogDomainIDs)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_Address);
        HSTR_TRACE_API_ARG(in_NumLogDomains);
        HSTR_TRACE_API_ARG(in_pLogDomainIDs);
        HSTR_CORE_API_CALLCOUNTER();

        detail::RmBufferLogDomains_impl_throw(
            in_Address,
            in_NumLogDomains,
            in_pLogDomainIDs);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_DEFAULT_VERSION(
    HSTR_RESULT,
    hStreams_RmBufferLogDomains)(
        void            *in_Address,
        int64_t          in_NumLogDomains,
        HSTR_LOG_DOM    *in_pLogDomainIDs)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_Address);
        HSTR_TRACE_API_ARG(in_NumLogDomains);
        HSTR_TRACE_API_ARG(in_pLogDomainIDs);
        HSTR_CORE_API_CALLCOUNTER();

        detail::RmBufferLogDomains_impl_throw(
            in_Address,
            in_NumLogDomains,
            in_pLogDomainIDs);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}


HSTR_EXPORT_IN_VERSION(
    HSTR_RESULT,
    hStreams_GetBufferNumLogDomains,
    HSTREAMS_1.0)(
        void                *in_Address,
        uint64_t            *out_NumLogDomains)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_Address);
        HSTR_TRACE_API_ARG(out_NumLogDomains);
        HSTR_CORE_API_CALLCOUNTER();
        detail::GetBufferNumLogDomains_impl_throw(
            in_Address,
            out_NumLogDomains);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}


HSTR_EXPORT_IN_DEFAULT_VERSION(
    HSTR_RESULT,
    hStreams_GetBufferNumLogDomains)(
        void                *in_Address,
        uint64_t            *out_NumLogDomains)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_Address);
        HSTR_TRACE_API_ARG(out_NumLogDomains);
        HSTR_CORE_API_CALLCOUNTER();
        detail::GetBufferNumLogDomains_impl_throw(
            in_Address,
            out_NumLogDomains);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_VERSION(
    HSTR_RESULT,
    hStreams_GetBufferLogDomains,
    HSTREAMS_1.0)(
        void                *in_Address,
        uint64_t             in_NumLogDomains,
        HSTR_LOG_DOM        *out_pLogDomains,
        uint64_t            *out_pNumLogDomains)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_Address);
        HSTR_TRACE_API_ARG(in_NumLogDomains);
        HSTR_TRACE_API_ARG(out_pLogDomains);
        HSTR_TRACE_API_ARG(out_pNumLogDomains);
        HSTR_CORE_API_CALLCOUNTER();
        detail::GetBufferLogDomains_impl_throw(
            in_Address,
            in_NumLogDomains,
            out_pLogDomains,
            out_pNumLogDomains);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_DEFAULT_VERSION(
    HSTR_RESULT,
    hStreams_GetBufferLogDomains)(
        void                *in_Address,
        uint64_t             in_NumLogDomains,
        HSTR_LOG_DOM        *out_pLogDomains,
        uint64_t            *out_pNumLogDomains)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_Address);
        HSTR_TRACE_API_ARG(in_NumLogDomains);
        HSTR_TRACE_API_ARG(out_pLogDomains);
        HSTR_TRACE_API_ARG(out_pNumLogDomains);
        HSTR_CORE_API_CALLCOUNTER();
        detail::GetBufferLogDomains_impl_throw(
            in_Address,
            in_NumLogDomains,
            out_pLogDomains,
            out_pNumLogDomains);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_VERSION(
    HSTR_RESULT,
    hStreams_GetBufferProps,
    HSTREAMS_1.0)(
        void                *in_Address,
        HSTR_BUFFER_PROPS   *out_BufferProps)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_Address);
        HSTR_TRACE_API_ARG(out_BufferProps);
        HSTR_CORE_API_CALLCOUNTER();
        detail::GetBufferProps_impl_throw(
            in_Address,
            out_BufferProps);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_DEFAULT_VERSION(
    HSTR_RESULT,
    hStreams_GetBufferProps)(
        void                *in_Address,
        HSTR_BUFFER_PROPS   *out_BufferProps)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_Address);
        HSTR_TRACE_API_ARG(out_BufferProps);
        HSTR_CORE_API_CALLCOUNTER();
        detail::GetBufferProps_impl_throw(
            in_Address,
            out_BufferProps);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_VERSION(
    HSTR_RESULT,
    hStreams_DeAlloc,
    HSTREAMS_1.0)(
        void               *in_Address)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_Address);
        HSTR_CORE_API_CALLCOUNTER();
        detail::DeAlloc_impl_throw(in_Address);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_DEFAULT_VERSION(
    HSTR_RESULT,
    hStreams_DeAlloc)(
        void               *in_Address)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_Address);
        HSTR_CORE_API_CALLCOUNTER();
        detail::DeAlloc_impl_throw(in_Address);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_VERSION(
    HSTR_RESULT,
    hStreams_GetLastError,
    HSTREAMS_1.0)()
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_CORE_API_CALLCOUNTER();
        return (HSTR_RESULT) hstr_proc.lastError;
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_DEFAULT_VERSION(
    HSTR_RESULT,
    hStreams_GetLastError)()
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_CORE_API_CALLCOUNTER();
        return (HSTR_RESULT) hstr_proc.lastError;
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_VERSION(
    void,
    hStreams_ClearLastError,
    HSTREAMS_1.0)()
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_CORE_API_CALLCOUNTER();
        hStreams_AtomicStore(hstr_proc.lastError, HSTR_RESULT_SUCCESS);
    } catch (...) {
        // do nothing
    }
}

HSTR_EXPORT_IN_DEFAULT_VERSION(
    void,
    hStreams_ClearLastError)()
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_CORE_API_CALLCOUNTER();
        hStreams_AtomicStore(hstr_proc.lastError, HSTR_RESULT_SUCCESS);
    } catch (...) {
        // do nothing
    }
}

HSTR_EXPORT_IN_DEFAULT_VERSION(
    HSTR_RESULT,
    hStreams_Cfg_SetLogLevel)(
        HSTR_LOG_LEVEL in_loglevel)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_loglevel);
        HSTR_CORE_API_CALLCOUNTER();
        detail::Cfg_SetLogLevel(in_loglevel);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_DEFAULT_VERSION(
    HSTR_RESULT,
    hStreams_Cfg_SetLogInfoType)(
        uint64_t in_info_type_mask)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_info_type_mask);
        HSTR_CORE_API_CALLCOUNTER();
        detail::Cfg_SetLogInfoType(in_info_type_mask);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_DEFAULT_VERSION(
    HSTR_RESULT,
    hStreams_Cfg_SetMKLInterface)(
        HSTR_MKL_INTERFACE in_MKLInterface)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_MKLInterface);
        HSTR_CORE_API_CALLCOUNTER();
        detail::Cfg_SetMKLInterface(in_MKLInterface);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_VERSION(
    uint32_t,
    hStreams_GetVerbose,
    HSTREAMS_1.0)()
{
    // That was the default in HSTR_OPTIONS
    return 0;
}

HSTR_EXPORT_IN_DEFAULT_VERSION(
    uint32_t,
    hStreams_GetVerbose)()
{
    return 0;
}

HSTR_EXPORT_IN_VERSION(
    HSTR_RESULT,
    hStreams_SetVerbose,
    HSTREAMS_1.0)(int target_verbosity)
{
    // Do nothing
    return HSTR_RESULT_SUCCESS;
}

HSTR_EXPORT_IN_DEFAULT_VERSION(
    HSTR_RESULT,
    hStreams_SetVerbose)(int target_verbosity)
{
    return HSTR_RESULT_SUCCESS;
}

HSTR_EXPORT_IN_VERSION(
    HSTR_RESULT,
    hStreams_GetCurrentOptions,
    HSTREAMS_1.0)(HSTR_OPTIONS *pCurrentOptions, uint64_t buffSize)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(pCurrentOptions);
        HSTR_TRACE_API_ARG(buffSize);
        HSTR_CORE_API_CALLCOUNTER();
        detail::GetCurrentOptions_impl_throw(pCurrentOptions, buffSize);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_DEFAULT_VERSION(
    HSTR_RESULT,
    hStreams_GetCurrentOptions)(HSTR_OPTIONS *pCurrentOptions, uint64_t buffSize)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(pCurrentOptions);
        HSTR_TRACE_API_ARG(buffSize);
        HSTR_CORE_API_CALLCOUNTER();
        detail::GetCurrentOptions_impl_throw(pCurrentOptions, buffSize);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}


HSTR_EXPORT_IN_VERSION(
    HSTR_RESULT,
    hStreams_SetOptions,
    HSTREAMS_1.0)(const HSTR_OPTIONS *in_options)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_options);
        detail::SetOptions_impl_throw(in_options);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_DEFAULT_VERSION(
    HSTR_RESULT,
    hStreams_SetOptions)(const HSTR_OPTIONS *in_options)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_options);
        detail::SetOptions_impl_throw(in_options);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_DEFAULT_VERSION(
    HSTR_RESULT,
    hStreams_SetLibrariesToLoad)(
        HSTR_ISA_TYPE in_isaType,
        uint32_t in_NumLibNames,
        char **in_ppLibNames,
        int *in_pLibFlags)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(in_isaType);
        HSTR_TRACE_API_ARG(in_NumLibNames);
        HSTR_TRACE_API_ARG(in_ppLibNames);
        HSTR_TRACE_API_ARG(in_pLibFlags);
        detail::SetLibrariesToLoad_impl_throw(
            in_isaType,
            in_NumLibNames,
            in_ppLibNames,
            in_pLibFlags
        );
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_VERSION(
    HSTR_RESULT,
    hStreams_GetVersionStringLen,
    HSTREAMS_1.0)(uint32_t *out_pVersionStringLen)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(out_pVersionStringLen);
        detail::GetVersionStringLen_impl_throw(out_pVersionStringLen);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_DEFAULT_VERSION(
    HSTR_RESULT,
    hStreams_GetVersionStringLen)(uint32_t *out_pVersionStringLen)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG(out_pVersionStringLen);
        detail::GetVersionStringLen_impl_throw(out_pVersionStringLen);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_VERSION(
    HSTR_RESULT,
    hStreams_Version,
    HSTREAMS_1.0)(char *buff, uint32_t buffLength)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG((void *)buff);
        HSTR_TRACE_API_ARG(buffLength);
        detail::Version_impl_throw(buff, buffLength);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_DEFAULT_VERSION(
    HSTR_RESULT,
    hStreams_Version)(char *buff, uint32_t buffLength)
{
    try {
        HSTR_TRACE_API_ENTER();
        HSTR_TRACE_API_ARG((void *)buff);
        HSTR_TRACE_API_ARG(buffLength);
        detail::Version_impl_throw(buff, buffLength);
        HSTR_RETURN(HSTR_RESULT_SUCCESS);
    } catch (...) {
        HSTR_RETURN(hStreams_handle_exception());
    }
}

HSTR_EXPORT_IN_VERSION(
    int,
    _hStreams_EmitMessage,
    HSTREAMS_1.0)
(HSTR_SEVERITY ols, const char *funcName, const char *format, ...)
{
    // does nothing
    return 0;
}

HSTR_EXPORT_IN_VERSION(
    int,
    _hStreams_EmitMessage,
    HSTREAMS_2.0)
(HSTR_SEVERITY ols, const char *funcName, const char *format, ...)
{
    // does nothing
    return 0;
}
