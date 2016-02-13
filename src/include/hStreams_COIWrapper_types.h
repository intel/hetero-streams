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

#ifndef HSTREAMS_COI_WRAPPER_TYPES_H
#define HSTREAMS_COI_WRAPPER_TYPES_H

#include "hStreams_types.h"
// hStreams types which imitate COI types

#define HSTR_COI_OPTIMIZE_SOURCE_READ           0x00000004
#define HSTR_COI_OPTIMIZE_SOURCE_WRITE          0x00000008
#define HSTR_COI_OPTIMIZE_SINK_READ             0x00000010
#define HSTR_COI_OPTIMIZE_SINK_WRITE            0x00000020
#define HSTR_COI_OPTIMIZE_NO_DMA                0x00000040
#define HSTR_COI_OPTIMIZE_HUGE_PAGE_SIZE        0x00000080

#define HSTR_COI_LOADLIBRARY_LOCAL              0x00000
#define HSTR_COI_LOADLIBRARY_GLOBAL             0x00100
#define HSTR_COI_LOADLIBRARY_LAZY               0x00001
#define HSTR_COI_LOADLIBRARY_NOW                0x00002
#define HSTR_COI_LOADLIBRARY_NOLOAD             0x00004
#define HSTR_COI_LOADLIBRARY_DEEPBIND           0x00008
#define HSTR_COI_LOADLIBRARY_NODELETE           0x01000
#define HSTR_COI_LOADLIBRARY_V1_FLAGS           (HSTR_COI_LOADLIBRARY_GLOBAL|HSTR_COI_LOADLIBRARY_NOW)

#define HSTR_COI_PROCESS_SOURCE                 ((HSTR_COIPROCESS)-1)


typedef struct coiprocess   *HSTR_COIPROCESS;
typedef struct coipipeline  *HSTR_COIPIPELINE;
typedef struct coifunction  *HSTR_COIFUNCTION;
typedef struct coiengine    *HSTR_COIENGINE;
typedef struct coibuffer    *HSTR_COIBUFFER;
typedef struct coilibrary   *HSTR_COILIBRARY;

typedef enum HSTR_COIRESULT {
    HSTR_COI_SUCCESS = 0,                  ///< The function succeeded without error.
    HSTR_COI_ERROR,                        ///< Unspecified error.
    HSTR_COI_NOT_INITIALIZED,              ///< The function was called before the
    ///< system was initialized.
    HSTR_COI_ALREADY_INITIALIZED,          ///< The function was called after the
    ///< system was initialized.
    HSTR_COI_ALREADY_EXISTS,               ///< Cannot complete the request due to
    ///< the existence of a similar object.
    HSTR_COI_DOES_NOT_EXIST,               ///< The specified object was not found.
    HSTR_COI_INVALID_POINTER,              ///< One of the provided addresses was not
    ///< valid.
    HSTR_COI_OUT_OF_RANGE,                 ///< One of the arguments contains a value
    ///< that is invalid.
    HSTR_COI_NOT_SUPPORTED,                ///< This function is not currently
    ///< supported as used.
    HSTR_COI_TIME_OUT_REACHED,             ///< The specified time out caused the
    ///< function to abort.
    HSTR_COI_MEMORY_OVERLAP,               ///< The source and destination range
    ///< specified overlaps for the same
    ///< buffer.
    HSTR_COI_ARGUMENT_MISMATCH,            ///< The specified arguments are not
    ///< compatible.
    HSTR_COI_SIZE_MISMATCH,                ///< The specified size does not match the
    ///< expected size.
    HSTR_COI_OUT_OF_MEMORY,                ///< The function was unable to allocate
    ///< the required memory.
    HSTR_COI_INVALID_HANDLE,               ///< One of the provided handles was not
    ///< valid.
    HSTR_COI_RETRY,                        ///< This function currently can't
    ///< complete, but might be able to later.
    HSTR_COI_RESOURCE_EXHAUSTED,           ///< The resource was not large enough.
    HSTR_COI_ALREADY_LOCKED,               ///< The object was expected to be
    ///< unlocked, but was locked.
    HSTR_COI_NOT_LOCKED,                   ///< The object was expected to be locked,
    ///< but was unlocked.
    HSTR_COI_MISSING_DEPENDENCY,           ///< One or more dependent components
    ///< could not be found.
    HSTR_COI_UNDEFINED_SYMBOL,             ///< One or more symbols the component
    ///< required was not defined in any
    ///< library.
    HSTR_COI_PENDING,                      ///< Operation is not finished
    HSTR_COI_BINARY_AND_HARDWARE_MISMATCH, ///< A specified binary will not run on
    ///< the specified hardware.
    HSTR_COI_PROCESS_DIED,
    HSTR_COI_INVALID_FILE,                 ///< The file is invalid for its intended
    ///< usage in the function.
    HSTR_COI_EVENT_CANCELED,               ///< Event wait on a user event that
    ///< was unregistered or is being
    ///< unregistered returns
    ///< HSTR_COI_EVENT_CANCELED.
    HSTR_COI_VERSION_MISMATCH,             ///< The version of Intel(R) Coprocessor
    ///< Offload Infrastructure on the host
    ///< is not compatible with the version
    ///< on the device.
    HSTR_COI_BAD_PORT,                     ///< The port that the host is set to
    ///< connect to is invalid.
    HSTR_COI_AUTHENTICATION_FAILURE,       ///< The daemon was unable to authenticate
    ///< the user that requested an engine.
    ///< Only reported if daemon is set up for
    ///< authorization. Is also reported in
    ///< Windows if host can not find user.
    HSTR_COI_NUM_RESULTS                   ///< Reserved, do not use.
}
HSTR_COIRESULT;

#define HSTR_COI_MAX_DRIVER_VERSION_STR_LEN 255
#define HSTR_COI_MAX_HW_THREADS 1024
///////////////////////////////////////////////////////////////////////////////
/// This enum defines miscellaneous information returned from the
/// COIGetEngineInfo() function.
///
typedef enum {
    HSTR_COI_ENG_ECC_DISABLED = 0,            //ECC is not enabled on this engine
    HSTR_COI_ENG_ECC_ENABLED = 0x00000001,    //ECC is enabled on this engine
    HSTR_COI_ENG_ECC_UNKNOWN = 0x00000002     //ECC is mode is unknown
} hstr_coi_eng_misc;

///////////////////////////////////////////////////////////////////////////////
/// This structure returns information about an Intel(R) Xeon Phi(TM)
/// coprocessor.
/// A pointer to this structure is passed into the COIGetEngineInfo() function,
/// which fills in the data before returning to the caller.
///
typedef struct HSTR_COI_ENGINE_INFO {
    /// The version string identifying the driver.
    uint32_t  DriverVersion[HSTR_COI_MAX_DRIVER_VERSION_STR_LEN];

    /// The ISA supported by the engine.
    HSTR_ISA_TYPE ISA;

    /// The number of cores on the engine.
    uint32_t     NumCores;

    /// Miscellaneous fields
    hstr_coi_eng_misc MiscFlags;

    /// The number of hardware threads on the engine.
    uint32_t     NumThreads;

    /// The maximum frequency (in MHz) of the cores on the engine.
    uint32_t     CoreMaxFrequency;

    /// The load percentage for each of the hardware threads on the engine.
    /// Currently this is limited to reporting out a maximum of 1024 HW threads
    uint32_t     Load[HSTR_COI_MAX_HW_THREADS];

    /// The amount of physical memory managed by the OS.
    uint64_t     PhysicalMemory;

    /// The amount of free physical memory in the OS.
    uint64_t     PhysicalMemoryFree;

    /// The amount of swap memory managed by the OS.
    uint64_t     SwapMemory;

    /// The amount of free swap memory in the OS.
    uint64_t     SwapMemoryFree;

    /// The pci config vendor id
    uint16_t     VendorId;

    /// The pci config device id
    uint16_t     DeviceId;

    /// The pci config subsystem id
    uint16_t     SubSystemId;

    /// The stepping of the board, A0, A1, C0, D0 etc.
    uint16_t     BoardStepping;

    /// The SKU of the stepping, EB, ED, etc.
    uint16_t     BoardSKU;
} HSTR_COI_ENGINE_INFO;

///////////////////////////////////////////////////////////////////////////////
/// The valid buffer types that may be created using COIBufferCreate.
typedef enum HSTR_COI_BUFFER_TYPE {
    /// Normal buffers exist as a single physical buffer in either Source or
    /// Sink physical memory. Mapping the buffer may stall the pipelines.
    HSTR_COI_BUFFER_NORMAL = 1,

    // Reserved values, not used by COI any more
    HSTR_COI_BUFFER_RESERVED_1,
    HSTR_COI_BUFFER_RESERVED_2,

    /// A pinned buffer exists in a shared memory region and is always
    /// available for read or write operations.
    /// Note: Pinned Buffers larger than 4KB are not supported in
    /// Windows 7 kernels.
    /// The value of HSTR_COI_BUFFER_PINNED is set to specific value
    /// to maintain compatibility with older versions of COI
    HSTR_COI_BUFFER_PINNED,

    /// OpenCL buffers are similar to Normal buffers except they don't
    /// stall pipelines and don't follow any read write dependencies.
    HSTR_COI_BUFFER_OPENCL

} HSTR_COI_BUFFER_TYPE;


//////////////////////////////////////////////////////////////////////////////
/// The valid copy operation types for the COIBufferWrite, COIBufferRead,
/// and COIBufferCopy APIs.
///
typedef enum HSTR_COI_COPY_TYPE {
    /// The runtime can pick the best suitable way to copy the data.
    HSTR_COI_COPY_UNSPECIFIED = 0,

    /// The runtime should use DMA to copy the data.
    HSTR_COI_COPY_USE_DMA,

    /// The runtime should use a CPU copy to copy the data.
    /// CPU copy is a synchronous copy. So the resulting operations are always
    /// blocking (even though a out_pCompletion event is specified).
    HSTR_COI_COPY_USE_CPU,

    /// Same as above, but forces moving entire buffer to target process in Ex
    /// extended APIs, even if the full buffer is not written.
    HSTR_COI_COPY_UNSPECIFIED_MOVE_ENTIRE,

    /// Same as above, but forces moving entire buffer to target process in Ex
    /// extended APIs, even if the full buffer is not written.
    HSTR_COI_COPY_USE_DMA_MOVE_ENTIRE,

    /// Same as above, but forces moving entire buffer to target process in Ex
    /// extended APIs, even if the full buffer is not written.
    HSTR_COI_COPY_USE_CPU_MOVE_ENTIRE

} HSTR_COI_COPY_TYPE;

//////////////////////////////////////////////////////////////////////////////
/// The buffer states used with COIBufferSetState call to indicate the new
/// state of the buffer on a given process
///
typedef enum {
    HSTR_COI_BUFFER_VALID = 0,      // Buffer is valid and up-to-date on the process
    HSTR_COI_BUFFER_INVALID ,       // Buffer is not valid, need valid data
    HSTR_COI_BUFFER_VALID_MAY_DROP, // Same as valid but will drop the content when
    // evicted to avoid overwriting the shadow
    // memory
    HSTR_COI_BUFFER_RESERVED        // Reserved for internal use
} HSTR_COI_BUFFER_STATE;

//////////////////////////////////////////////////////////////////////////////
/// The buffer move flags are used to indicate when a buffer should be moved
/// when it's state is changed. This is used with COIBufferSetState.
typedef enum {
    HSTR_COI_BUFFER_MOVE = 0,// Dirty data is moved if state change requires it
    HSTR_COI_BUFFER_NO_MOVE  // Change state without moving data
} HSTR_COI_BUFFER_MOVE_FLAG;

//////////////////////////////////////////////////////////////////////////////
/// These flags specify how a buffer will be used within a run function. They
/// allow the runtime to make optimizations in how it moves the data around.
/// These flags can affect the correctness of an application, so they must be
/// set properly. For example, if a buffer is used in a run function with the
/// COI_SINK_READ flag and then mapped on the source, the runtime may use a
/// previously cached version of the buffer instead of retrieving data from
/// the sink.
typedef enum HSTR_COI_ACCESS_FLAGS {
    /// Specifies that the run function will only read the associated buffer.
    HSTR_COI_SINK_READ = 1,

    /// Specifies that the run function will write to the associated buffer.
    HSTR_COI_SINK_WRITE,

    /// Specifies that the run function will overwrite the entire associated
    /// buffer and therefore the buffer will not be synchronized with the
    /// source before execution.
    HSTR_COI_SINK_WRITE_ENTIRE,

    /// Specifies that the run function will only read the associated buffer
    /// and will maintain the reference count on the buffer after
    /// run function exit.
    HSTR_COI_SINK_READ_ADDREF,

    /// Specifies that the run function will write to the associated buffer
    /// and will maintain the reference count on the buffer after
    /// run function exit.
    HSTR_COI_SINK_WRITE_ADDREF,

    /// Specifies that the run function will overwrite the entire associated
    /// buffer and therefore the buffer will not be synchronized with the
    /// source before execution and will maintain the reference count on the
    /// buffer after run function exit.
    HSTR_COI_SINK_WRITE_ENTIRE_ADDREF
} HSTR_COI_ACCESS_FLAGS;

#endif // HSTREAMS_COI_WRAPPER_TYPES_H

