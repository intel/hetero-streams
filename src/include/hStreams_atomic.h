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

#ifndef __HSTR_ATOMIC_H__

#define __HSTR_ATOMIC_H__

#ifdef _WIN32
#include <Windows.h>
#endif

// Atomically, performs (in 32 bit):
//
// loc = loc + value;
//
// returns the value of loc BEFORE the operation.
//
static inline long hStreams_AtomicAdd(volatile long &loc, long value)
{
#ifdef _WIN32
    long retv = InterlockedExchangeAdd(&loc, value);
#else
    // __sync_fetch_and_add(&loc,value) is a builtin function that supports an interlocked add
    // operation.
    long retv = __sync_fetch_and_add(&loc, value);
#endif
    return retv;
}

// Atomically, performs (in 64 bit):
//
// loc = loc + value;
//
// returns the value of loc BEFORE the operation.
//
static inline int64_t hStreams_AtomicAdd64(volatile int64_t &loc, int64_t value)
{
#ifdef _WIN32
    return InterlockedExchangeAdd64(&loc, value);
#else
    // __sync_fetch_and_add(&loc,value) is a builtin function that supports an interlocked add
    // operation.
    return __sync_fetch_and_add(&loc, value);
#endif
}

// Atomically, performs:
//
// loc = value;
//
// returns the value of loc BEFORE the operation.
//
static inline long hStreams_AtomicStore(volatile long &loc, long value)
{
#ifdef _WIN32
    long retv = InterlockedExchange(&loc, value);
#else
    // __sync_lock_test_and_set(&loc,value) is a builtin function that supports an interlocked
    // memory fetch and store operation.
    long retv = __sync_lock_test_and_set(&loc, value);
#endif
    return retv;
}

class hStreams_AtomicCounter
{
public:
    hStreams_AtomicCounter() : mCounter(0)
    {
    };
    long Increment()
    {
        return hStreams_AtomicAdd(mCounter, 1);
    };
    long Decrement()
    {
        return hStreams_AtomicAdd(mCounter, -1);
    };
    long GetValue(void)
    {
        return hStreams_AtomicAdd(mCounter, 0);
    };
    long SetValue(long v)
    {
        return hStreams_AtomicStore(mCounter, v);
    };
private:
    volatile long mCounter;
};

template<class T>
class hStreams_AtomicEnum
{
public:
    hStreams_AtomicEnum() {};
    hStreams_AtomicEnum(T v) {
        SetValue(v);
    }

    T GetValue(void)
    {
        return (T)mac.GetValue();
    }
    T SetValue(T v)
    {
        return (T)mac.SetValue((long) v);
    }
    hStreams_AtomicEnum<T>& operator= (T v)
    {
        mac.SetValue((long) v);
        return *this;
    }
private:
    hStreams_AtomicCounter mac;
};

class hStreams_ScopedAtomicCounter
{
public:
    hStreams_ScopedAtomicCounter(hStreams_AtomicCounter &ac) : mAC(ac)
    {
        mAC.Increment();
    };

    ~hStreams_ScopedAtomicCounter()
    {
        mAC.Decrement();
    };
private:
    // Copy constructor is declared private, and never defined:
    hStreams_ScopedAtomicCounter(const hStreams_ScopedAtomicCounter &);
    // Assignment operator is declared private, and never defined:
    hStreams_ScopedAtomicCounter &operator=(const hStreams_ScopedAtomicCounter &);
    hStreams_AtomicCounter &mAC;
};

#endif  /* __HSTR_ATOMIC_H__ */
