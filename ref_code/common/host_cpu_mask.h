/*
 * Copyright 2014-2016 Intel Corporation.
 *
 * This file is subject to the Intel Sample Source Code License. A copy
 * of the Intel Sample Source Code License is included.
 */

#ifndef _WIN32
#include <sched.h>
#else
#include <windows.h>
#endif


#ifndef _WIN32
typedef cpu_set_t host_cpu_set_t;
#else
typedef DWORD_PTR host_cpu_set_t;
#endif

struct HostCPUMask {
    host_cpu_set_t host_cpu_set;
    void cpu_zero();
    void cpu_set(int cpu);
    int cpu_isset(int cpu);
};
void setCurrentProcessAffinityMask(HostCPUMask &host_cpu_mask);
void getCurrentProcessAffinityMask(HostCPUMask &host_cpu_mask);

#ifndef _WIN32
void HostCPUMask::cpu_zero()
{
    CPU_ZERO(&host_cpu_set);
}
void HostCPUMask::cpu_set(int cpu)
{
    CPU_SET(cpu, &host_cpu_set);
}
int HostCPUMask::cpu_isset(int cpu)
{
    return CPU_ISSET(cpu, &host_cpu_set);
}
void setCurrentProcessAffinityMask(HostCPUMask &host_cpu_mask)
{
    sched_setaffinity(0, sizeof(cpu_set_t), &(host_cpu_mask.host_cpu_set));
}
void getCurrentProcessAffinityMask(HostCPUMask &host_cpu_mask)
{
    sched_getaffinity(0, sizeof(cpu_set_t), &host_cpu_mask.host_cpu_set);
}
#else
void HostCPUMask::cpu_zero()
{
    host_cpu_set = 0;
}
void HostCPUMask::cpu_set(int cpu)
{
    host_cpu_set |= (1LL << cpu);
}
int HostCPUMask::cpu_isset(int cpu)
{
    return host_cpu_set && (1LL << cpu);
}
void setCurrentProcessAffinityMask(HostCPUMask &host_cpu_mask)
{
    SetProcessAffinityMask(GetCurrentProcess(), host_cpu_mask.host_cpu_set);
}
void getCurrentProcessAffinityMask(HostCPUMask &host_cpu_mask)
{
    host_cpu_set_t unneeded_system_mask;
    GetProcessAffinityMask(GetCurrentProcess(), &host_cpu_mask.host_cpu_set, &unneeded_system_mask);
}
#endif


