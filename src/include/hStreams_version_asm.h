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
/*
 Version for Symbols (only Functions currently versioned)
 Only the Linux Host Side code is versioned currently
 When there are multiple versions, only last has @@
*/
#if (! defined _WIN32) && (defined HSTR_SOURCE)

/*Initialization/finalization*/
__asm__(".symver hStreams_Init1,hStreams_Init@@HSTREAMS_1.0");
__asm__(".symver hStreams_Fini1,hStreams_Fini@@HSTREAMS_1.0");
__asm__(".symver hStreams_IsInitialized1,hStreams_IsInitialized@@HSTREAMS_1.0");

/*Resources enumeration*/
__asm__(".symver hStreams_GetNumPhysDomains1,hStreams_GetNumPhysDomains@@HSTREAMS_1.0");
__asm__(".symver hStreams_GetAvailable1,hStreams_GetAvailable@@HSTREAMS_1.0");
__asm__(".symver hStreams_GetPhysDomainDetails1,hStreams_GetPhysDomainDetails@@HSTREAMS_1.0");
__asm__(".symver hStreams_AddLogDomain1,hStreams_AddLogDomain@@HSTREAMS_1.0");
__asm__(".symver hStreams_RmLogDomains1,hStreams_RmLogDomains@@HSTREAMS_1.0");
__asm__(".symver hStreams_GetNumLogDomains1,hStreams_GetNumLogDomains@@HSTREAMS_1.0");
__asm__(".symver hStreams_GetLogDomainIDList1,hStreams_GetLogDomainIDList@@HSTREAMS_1.0");
__asm__(".symver hStreams_GetLogDomainDetails1,hStreams_GetLogDomainDetails@@HSTREAMS_1.0");
__asm__(".symver hStreams_GetNumLogStreams1,hStreams_GetNumLogStreams@@HSTREAMS_1.0");
__asm__(".symver hStreams_GetLogStreamIDList1,hStreams_GetLogStreamIDList@@HSTREAMS_1.0");
__asm__(".symver hStreams_GetLogStreamDetails1,hStreams_GetLogStreamDetails@@HSTREAMS_1.0");
__asm__(".symver hStreams_GetOversubscriptionLevel1,hStreams_GetOversubscriptionLevel@@HSTREAMS_1.0");

/*Stream management*/
__asm__(".symver hStreams_StreamCreate1,hStreams_StreamCreate@@HSTREAMS_1.0");
__asm__(".symver hStreams_StreamDestroy1,hStreams_StreamDestroy@@HSTREAMS_1.0");

/*Stream usage*/
__asm__(".symver hStreams_EnqueueCompute1,hStreams_EnqueueCompute@HSTREAMS_1.0");
__asm__(".symver hStreams_EnqueueCompute2,hStreams_EnqueueCompute@@HSTREAMS_2.0");
__asm__(".symver hStreams_EnqueueData1D1,hStreams_EnqueueData1D@@HSTREAMS_1.0");
__asm__(".symver hStreams_EnqueueDataXDomain1D1,hStreams_EnqueueDataXDomain1D@@HSTREAMS_1.0");

/*Synchronization*/
__asm__(".symver hStreams_StreamSynchronize1,hStreams_StreamSynchronize@@HSTREAMS_1.0");
__asm__(".symver hStreams_ThreadSynchronize1,hStreams_ThreadSynchronize@@HSTREAMS_1.0");
__asm__(".symver hStreams_EventWait1,hStreams_EventWait@@HSTREAMS_1.0");
__asm__(".symver hStreams_EventStreamWait1,hStreams_EventStreamWait@@HSTREAMS_1.0");

/*Memory management*/
__asm__(".symver hStreams_Alloc1D1,hStreams_Alloc1D@@HSTREAMS_1.0");
__asm__(".symver hStreams_Alloc1DEx1,hStreams_Alloc1DEx@@HSTREAMS_1.0");
__asm__(".symver hStreams_AddBufferLogDomains1,hStreams_AddBufferLogDomains@@HSTREAMS_1.0");
__asm__(".symver hStreams_RmBufferLogDomains1,hStreams_RmBufferLogDomains@@HSTREAMS_1.0");
__asm__(".symver hStreams_DeAlloc1,hStreams_DeAlloc@@HSTREAMS_1.0");
__asm__(".symver hStreams_GetBufferNumLogDomains1,hStreams_GetBufferNumLogDomains@@HSTREAMS_1.0");
__asm__(".symver hStreams_GetBufferLogDomains1,hStreams_GetBufferLogDomains@@HSTREAMS_1.0");
__asm__(".symver hStreams_GetBufferProps1,hStreams_GetBufferProps@@HSTREAMS_1.0");

/*Those that pertain to error handling*/
__asm__(".symver hStreams_GetLastError1,hStreams_GetLastError@@HSTREAMS_1.0");
__asm__(".symver hStreams_ClearLastError1,hStreams_ClearLastError@@HSTREAMS_1.0");

__asm__(".symver hStreams_SetOptions1,hStreams_SetOptions@@HSTREAMS_1.0");
__asm__(".symver hStreams_GetCurrentOptions1,hStreams_GetCurrentOptions@@HSTREAMS_1.0");
__asm__(".symver hStreams_GetVerbose1,hStreams_GetVerbose@@HSTREAMS_1.0");
__asm__(".symver hStreams_SetVerbose1,hStreams_SetVerbose@@HSTREAMS_1.0");
__asm__(".symver hStreams_Version1,hStreams_Version@@HSTREAMS_1.0");
__asm__(".symver hStreams_GetVersionStringLen1,hStreams_GetVersionStringLen@@HSTREAMS_1.0");
__asm__(".symver hStreams_ResultGetName1,hStreams_ResultGetName@@HSTREAMS_1.0");

/*Those that are part of the app API*/
__asm__(".symver hStreams_app_init1,hStreams_app_init@@HSTREAMS_1.0");
__asm__(".symver hStreams_app_init_domains1,hStreams_app_init_domains@@HSTREAMS_1.0");
__asm__(".symver hStreams_app_fini1,hStreams_app_fini@@HSTREAMS_1.0");
__asm__(".symver hStreams_app_create_buf1,hStreams_app_create_buf@@HSTREAMS_1.0");
__asm__(".symver hStreams_app_xfer_memory1,hStreams_app_xfer_memory@HSTREAMS_1.0");
__asm__(".symver hStreams_app_xfer_memory2,hStreams_app_xfer_memory@@HSTREAMS_2.0");

__asm__(".symver hStreams_app_stream_sync1,hStreams_app_stream_sync@@HSTREAMS_1.0");
__asm__(".symver hStreams_app_thread_sync1,hStreams_app_thread_sync@@HSTREAMS_1.0");
__asm__(".symver hStreams_app_event_wait1,hStreams_app_event_wait@@HSTREAMS_1.0");
__asm__(".symver hStreams_app_event_wait_in_stream1,hStreams_app_event_wait_in_stream@@HSTREAMS_1.0");
__asm__(".symver hStreams_app_invoke1,hStreams_app_invoke@HSTREAMS_1.0");
__asm__(".symver hStreams_app_invoke2,hStreams_app_invoke@@HSTREAMS_2.0");
__asm__(".symver hStreams_app_memset1,hStreams_app_memset@HSTREAMS_1.0");
__asm__(".symver hStreams_app_memset2,hStreams_app_memset@@HSTREAMS_2.0");
__asm__(".symver hStreams_app_memcpy1,hStreams_app_memcpy@HSTREAMS_1.0");
__asm__(".symver hStreams_app_memcpy2,hStreams_app_memcpy@@HSTREAMS_2.0");
__asm__(".symver hStreams_app_sgemm1,hStreams_app_sgemm@HSTREAMS_1.0");
__asm__(".symver hStreams_app_sgemm2,hStreams_app_sgemm@HSTREAMS_2.0");
__asm__(".symver hStreams_app_sgemm3,hStreams_app_sgemm@@HSTREAMS_3.0");
__asm__(".symver hStreams_app_dgemm1,hStreams_app_dgemm@HSTREAMS_1.0");
__asm__(".symver hStreams_app_dgemm2,hStreams_app_dgemm@HSTREAMS_2.0");
__asm__(".symver hStreams_app_dgemm3,hStreams_app_dgemm@@HSTREAMS_3.0");
__asm__(".symver hStreams_app_cgemm1,hStreams_app_cgemm@HSTREAMS_1.0");
__asm__(".symver hStreams_app_cgemm2,hStreams_app_cgemm@HSTREAMS_2.0");
__asm__(".symver hStreams_app_cgemm3,hStreams_app_cgemm@@HSTREAMS_3.0");
__asm__(".symver hStreams_app_zgemm1,hStreams_app_zgemm@HSTREAMS_1.0");
__asm__(".symver hStreams_app_zgemm2,hStreams_app_zgemm@HSTREAMS_2.0");
__asm__(".symver hStreams_app_zgemm3,hStreams_app_zgemm@@HSTREAMS_3.0");

/* Configuration APIs */
__asm__(".symver hStreams_Cfg_SetLogLevel1,hStreams_Cfg_SetLogLevel@@HSTREAMS_2.0");
__asm__(".symver hStreams_Cfg_SetLogInfoType1,hStreams_Cfg_SetLogInfoType@@HSTREAMS_2.0");
__asm__(".symver hStreams_Cfg_SetMKLInterface1,hStreams_Cfg_SetMKLInterface@@HSTREAMS_3.0");

#endif
