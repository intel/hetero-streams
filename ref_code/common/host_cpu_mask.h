/*
 * Copyright 2014-2016 Intel Corporation.
 *
 * This file is subject to the Intel Sample Source Code License. A copy
 * of the Intel Sample Source Code License is included.
 */

#ifndef _WIN32
#include <sched.h>
#include <errno.h>
#else
#include <windows.h>
#endif
#include <iostream>

#ifndef _WIN32
typedef cpu_set_t host_cpu_set_t;
#else
typedef DWORD_PTR host_cpu_set_t;
#endif

struct HostCPUMask {
    host_cpu_set_t host_cpu_set;
    void cpu_zero();
    void cpu_set(unsigned int cpu);
    unsigned int cpu_isset(unsigned int cpu);
};
int setCurrentProcessAffinityMask(HostCPUMask &host_cpu_mask);

#ifndef _WIN32
void HostCPUMask::cpu_zero()
{
    CPU_ZERO(&host_cpu_set);
}
void HostCPUMask::cpu_set(unsigned int cpu)
{
    CPU_SET(cpu, &host_cpu_set);
}
unsigned int HostCPUMask::cpu_isset(unsigned int cpu)
{
    return CPU_ISSET(cpu, &host_cpu_set);
}
int setCurrentProcessAffinityMask(HostCPUMask &host_cpu_mask)
{
    int ret = sched_setaffinity(0, sizeof(cpu_set_t), &(host_cpu_mask.host_cpu_set));
    //On error, -1 is returned
    if (ret == -1) {
        std::cout << "sched_setaffinity was failed. errno returned: " << errno << std::endl;
        return 1;
    }
    return 0;
}
#else
void HostCPUMask::cpu_zero()
{
    host_cpu_set = 0;
}
void HostCPUMask::cpu_set(unsigned int cpu)
{
    if (cpu < 64) {
        host_cpu_set |= (1LL << cpu);
    }
}
unsigned int HostCPUMask::cpu_isset(unsigned int cpu)
{
    if (cpu < 64) {
        return host_cpu_set & (1LL << cpu);
    } else {
        return 0;
    }
}
int setCurrentProcessAffinityMask(HostCPUMask &host_cpu_mask)
{
    //On Windows call SetThreadAffinityMask (used by HostSideSinkWorker) after SetProccesAffinityMask caused error ERROR_INVALID_PARAMETER.
    //SetThreadAffinityMask is using instead to prevent this error.

    //If the function fails, the return value is zero
    if (SetThreadAffinityMask(GetCurrentThread(), host_cpu_mask.host_cpu_set) == 0) {
        std::cout << "SetThreadAffinityMask was failed. GetLastError returned: " << GetLastError() << std::endl;
        return 1;
    }
    return 0;
}
#endif


