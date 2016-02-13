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

#ifndef HSTREAMS_COI_WRAPPER_H
#define HSTREAMS_COI_WRAPPER_H

#include "hStreams_types.h"
#include "hStreams_COIWrapper_types.h"
#include "hStreams_exceptions.h"

/// @brief Class load COI library and store handles to COI methods
class hStreams_COIWrapper
{
private:
    //COI function handlers' types
    typedef HSTR_COIRESULT(*COIEngineGetInfo_handler_t)(HSTR_COIENGINE, uint32_t, HSTR_COI_ENGINE_INFO *);
    typedef HSTR_COIRESULT(*COIEngineGetCount_handler_t)(HSTR_ISA_TYPE, uint32_t *);
    typedef HSTR_COIRESULT(*COIEngineGetHandle_handler_t)(HSTR_ISA_TYPE, uint32_t, HSTR_COIENGINE *);

    typedef HSTR_COIRESULT(*COIEventWait_handler_t)(uint16_t, const HSTR_EVENT *, int32_t, uint8_t, uint32_t *, uint32_t *);
    typedef HSTR_COIRESULT(*COIEventRegisterUserEvent_handler_t)(HSTR_EVENT *);
    typedef HSTR_COIRESULT(*COIEventSignalUserEvent_handler_t)(HSTR_EVENT);

    typedef HSTR_COIRESULT(*COIProcessCreateFromFile_handler_t)(HSTR_COIENGINE, const char *, int, const char **, uint8_t, const char **, uint8_t, const char *, uint64_t, const char *, HSTR_COIPROCESS *);
    typedef HSTR_COIRESULT(*COIProcessCreateFromMemory_handler_t)(HSTR_COIENGINE, const char *, const void *, uint64_t, int, const char **, uint8_t, const char **, uint8_t, const char *, uint64_t, const char *, const char *, uint64_t, HSTR_COIPROCESS *);

    typedef HSTR_COIRESULT(*COIProcessDestroy_handler_t)(HSTR_COIPROCESS, int32_t, uint8_t, int8_t *, uint32_t *);
    typedef HSTR_COIRESULT(*COIProcessGetFunctionHandles_handler_t)(HSTR_COIPROCESS, uint32_t, const char **, HSTR_COIFUNCTION *);
    typedef HSTR_COIRESULT(*COIProcessLoadLibraryFromFile_handler_t)(HSTR_COIPROCESS, const char *, const char *, const char *, uint32_t, HSTR_COILIBRARY *); //COIProcessLoadLibraryFromFile@COI_2.0
    typedef HSTR_COIRESULT(*COIProcessUnloadLibrary_handler_t)(HSTR_COIPROCESS, HSTR_COILIBRARY);

    typedef HSTR_COIRESULT(*COIBufferCreate_handler_t)(uint64_t , HSTR_COI_BUFFER_TYPE, uint32_t, const void *, uint32_t , const HSTR_COIPROCESS *, HSTR_COIBUFFER *);
    typedef HSTR_COIRESULT(*COIBufferCreateFromMemory_handler_t)(uint64_t , HSTR_COI_BUFFER_TYPE, uint32_t, void *, uint32_t, const HSTR_COIPROCESS *, HSTR_COIBUFFER *);
    typedef HSTR_COIRESULT(*COIBufferDestroy_handler_t)(HSTR_COIBUFFER);
    typedef HSTR_COIRESULT(*COIBufferGetSinkAddress_handler_t)(HSTR_COIBUFFER, uint64_t *);
    typedef HSTR_COIRESULT(*COIBufferWrite_handler_t)(HSTR_COIBUFFER, uint64_t, const void *, uint64_t, HSTR_COI_COPY_TYPE, uint32_t, const HSTR_EVENT *, HSTR_EVENT *);
    typedef HSTR_COIRESULT(*COIBufferCopy_handler_t)(HSTR_COIBUFFER, HSTR_COIBUFFER, uint64_t, uint64_t, uint64_t, HSTR_COI_COPY_TYPE, uint32_t, const HSTR_EVENT *, HSTR_EVENT *);
    typedef HSTR_COIRESULT(*COIBufferSetState_handler_t)(HSTR_COIBUFFER, HSTR_COIPROCESS, HSTR_COI_BUFFER_STATE, HSTR_COI_BUFFER_MOVE_FLAG, uint32_t, const HSTR_EVENT *, HSTR_EVENT *);
    typedef HSTR_COIRESULT(*COIBufferAddRefcnt_handler_t)(HSTR_COIPROCESS, HSTR_COIBUFFER, uint64_t);

    typedef HSTR_COIRESULT(*COIPipelineCreate_handler_t)(HSTR_COIPROCESS, HSTR_CPU_MASK, uint32_t, HSTR_COIPIPELINE *);
    typedef HSTR_COIRESULT(*COIPipelineDestroy_handler_t)(HSTR_COIPIPELINE);
    typedef HSTR_COIRESULT(*COIPipelineRunFunction_handler_t)(HSTR_COIPIPELINE, HSTR_COIFUNCTION, uint32_t, const HSTR_COIBUFFER *, const HSTR_COI_ACCESS_FLAGS *, uint32_t, const HSTR_EVENT *, const void *, uint16_t, void *, uint16_t, HSTR_EVENT *);

    typedef const char *(*COIResultGetName_handler_t)(HSTR_COIRESULT);

public:
    hStreams_COIWrapper();

    //COI function handlers
    static COIEngineGetInfo_handler_t                               COIEngineGetInfo;
    static COIEngineGetCount_handler_t                              COIEngineGetCount;
    static COIEngineGetHandle_handler_t                             COIEngineGetHandle;

    static COIEventWait_handler_t                                   COIEventWait;
    static COIEventRegisterUserEvent_handler_t                      COIEventRegisterUserEvent;
    static COIEventSignalUserEvent_handler_t                        COIEventSignalUserEvent;

    static COIProcessCreateFromFile_handler_t                       COIProcessCreateFromFile;
    static COIProcessCreateFromMemory_handler_t                     COIProcessCreateFromMemory;
    static COIProcessDestroy_handler_t                              COIProcessDestroy;
    static COIProcessGetFunctionHandles_handler_t                   COIProcessGetFunctionHandles;
    static COIProcessLoadLibraryFromFile_handler_t                  COIProcessLoadLibraryFromFile;
    static COIProcessUnloadLibrary_handler_t                        COIProcessUnloadLibrary;

    static COIBufferCreate_handler_t                                COIBufferCreate;
    static COIBufferCreateFromMemory_handler_t                      COIBufferCreateFromMemory;
    static COIBufferDestroy_handler_t                               COIBufferDestroy;
    static COIBufferGetSinkAddress_handler_t                        COIBufferGetSinkAddress;
    static COIBufferWrite_handler_t                                 COIBufferWrite;
    static COIBufferCopy_handler_t                                  COIBufferCopy;
    static COIBufferSetState_handler_t                              COIBufferSetState;
    static COIBufferAddRefcnt_handler_t                             COIBufferAddRefcnt;

    static COIPipelineCreate_handler_t                              COIPipelineCreate;
    static COIPipelineDestroy_handler_t                             COIPipelineDestroy;
    static COIPipelineRunFunction_handler_t                         COIPipelineRunFunction;

    static COIResultGetName_handler_t                               COIResultGetName;
};

#endif

