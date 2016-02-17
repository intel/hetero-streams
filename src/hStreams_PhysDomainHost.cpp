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

#include "hStreams_PhysDomainHost.h"
#include "hStreams_types.h"
#include "hStreams_COIWrapper.h"
#include "hStreams_helpers_common.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif // _WIN32

#include <string>
#include <sstream>
#include <iostream>

namespace
{
// Will write CPUID information into registers
// See: http://www.intel.com/content/www/us/en/processors/processor-identification-cpuid-instruction-note.html
void cpuid(int eax_in, uint32_t *registers)
{
#ifdef _WIN32
    __cpuid((int *)registers, (int)eax_in);
#else
    asm volatile
    ("cpuid" : "=a"(*registers), "=b"(*(registers+1)), "=c"(*(registers+2)), "=d"(*(registers+3))
     : "a"(eax_in), "c"(0));
#endif
}
uint32_t getMaxCoreFrequency()
{
    char cpu_info[48];
    cpuid(0x80000002, (uint32_t *)cpu_info);
    cpuid(0x80000003, (uint32_t *)(cpu_info + 16));
    cpuid(0x80000004, (uint32_t *)(cpu_info + 32));

    double multiplier = 0;
    // We're interested in results in Mhz, hence the scaled multipliers
    // The fourty-fourth letter in the cpuid is the multiplier
    switch (cpu_info[44]) {
    case 'T':
        multiplier = 1e6;
        break;
    case 'G':
        multiplier = 1e3;
        break;
    case 'M':
        multiplier = 1e0;
        break;
    }
    // Look for the blank, making our way backwards
    // The format is x.yzGhz. We need to start backwards from the multiplier letter
    int blank_idx = 43;
    while (cpu_info[blank_idx] != ' ') {
        --blank_idx;
    }
    std::string freq_str(&cpu_info[blank_idx + 1], 43 - blank_idx);
    std::stringstream freq_ss(freq_str);
    double freq;
    freq_ss >> freq;

    freq *= multiplier;
    return (uint32_t) freq;
}

uint64_t getAvailableMemory()
{
#ifdef _WIN32
    MEMORYSTATUSEX mem_status;
    mem_status.dwLength = sizeof(mem_status);
    GlobalMemoryStatusEx(&mem_status);
    return (uint64_t) mem_status.ullTotalPhys;
#else
    uint64_t num_pages = (uint64_t) sysconf(_SC_PHYS_PAGES);
    uint64_t page_size = (uint64_t) sysconf(_SC_PAGESIZE);
    return num_pages * page_size;
#endif // _WIN32
}

hStreams_CPUMask generateMaxCPUMask()
{
    int num_cpus;
#ifdef _WIN32
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    num_cpus = (int) sysinfo.dwNumberOfProcessors;
#else
    num_cpus = (int) sysconf(_SC_NPROCESSORS_ONLN);
#endif // _WIN32

    hStreams_CPUMask cpu_mask;
    HSTR_CPU_MASK_ZERO(cpu_mask.mask);
    for (int i = 0; i < num_cpus; ++i) {
        HSTR_CPU_MASK_SET(i, cpu_mask.mask);
    }
    return cpu_mask;
}

// Implementation assumes that no CPUs on the host are reserved
// and should be avoided.
hStreams_CPUMask generateAvoidCPUMask()
{
    hStreams_CPUMask cpu_mask;
    HSTR_CPU_MASK_ZERO(cpu_mask.mask);
    return cpu_mask;
}
} // anonymous namespace

hStreams_PhysDomainHost::hStreams_PhysDomainHost(HSTR_COIPROCESS coi_proc, const std::vector<LIB_HANDLER::handle_t> &loaded_libs_handles)
    : hStreams_PhysDomain(
          HSTR_SRC_PHYS_DOMAIN,
          HSTR_ISA_x86_64,
          getMaxCoreFrequency(),
          getAvailableMemory(),
          generateMaxCPUMask(),
          generateAvoidCPUMask()
      ),
      coi_proc_(coi_proc), loaded_libs_handles_(loaded_libs_handles)
{

}

hStreams_PhysDomainHost::~hStreams_PhysDomainHost()
{
    for (std::vector<LIB_HANDLER::handle_t>::const_iterator it = loaded_libs_handles_.cbegin(); it != loaded_libs_handles_.cend(); ++it) {
        hStreams_LibLoader::unload_nothrow(*it);
    }
}

HSTR_COIPROCESS hStreams_PhysDomainHost::impl_getCOIProcess() const
{
    return coi_proc_;
}

hStreams_PhysStream *hStreams_PhysDomainHost::impl_createNewPhysStream(hStreams_LogDomain &log_dom, hStreams_CPUMask const &cpu_mask)
{
    return new hStreams_PhysStreamHost(log_dom, cpu_mask);
}

uint64_t hStreams_PhysDomainHost::impl_fetchSinkFunctionAddress(std::string const &func_name)
{
    uint64_t handle;
    for (std::vector<LIB_HANDLER::handle_t>::const_iterator it = loaded_libs_handles_.cbegin(); it != loaded_libs_handles_.cend(); ++it) {
        handle = hStreams_LibLoader::fetchFunctionAddress_nothrow(*it, func_name);

        if (handle != NULL) {
            return handle;
        }
    }

    // Try for current process as well
    handle = hStreams_LibLoader::fetchExecFunctionAddress_nothrow(func_name);
    if (handle != NULL) {
        return handle;
    }

    // At least try fetch global function
    return hStreams_LibLoader::fetchGlobalFunctionAddress_nothrow(func_name);
}
