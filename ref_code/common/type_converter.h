/*
 * Copyright 2014-2016 Intel Corporation.
 *
 * This file is subject to the Intel Sample Source Code License. A copy
 * of the Intel Sample Source Code License is included.
 */

#include <mkl.h>
#include <string.h>

#define STATIC_ASSERT( condition, name ) typedef char assert_failed_ ## name [ (condition) ? 1 : -1 ]

// The ConvertToUint64_t template is used to allow data to safely get passed the calling conventions
// in the hstreams API by marshalling/demartialing data from/to source/sink execution environments.
// For an example of its use, see ref_code/common/hStreams_custom.cpp and
// ref_code/common/hStreams_custom_kernels_sink.cpp
template<class TYPE, const size_t sz>
class ConvertToUint64_t
{
public:

    // We abort compilation if the expression is not true, which is that the size of the TYPE parameter is less than
    // or equal to the sz parameter timessizeof(uint64_t).
    STATIC_ASSERT(sizeof(TYPE) <= sz *sizeof(uint64_t), sizeof_TYPE_must_be_less_than_or_equal_to_sz_times_sizeof_uint64_t);

    typedef uint64_t conversion_t[sz];

    ConvertToUint64_t()
    {
        memset(this, 0, sizeof(*this));
    }

    void Set(const TYPE &t)
    {
        mUnion.t = t;
    }

    void Set_uint64_t(uint64_t u64, size_t idx = 0)
    {
        mUnion.c[idx] = u64;
    }

    TYPE &Get(void)
    {
        return mUnion.t;
    }

    uint64_t Get_uint64_t(size_t idx = 0)
    {
        return mUnion.c[idx];
    }

private:
    union {
        TYPE         t;
        conversion_t c;
    } mUnion;
};

typedef ConvertToUint64_t<char, 1>           charToUint64_t;
typedef ConvertToUint64_t<float, 1>          floatToUint64_t;
typedef ConvertToUint64_t<double, 1>         doubleToUint64_t;
typedef ConvertToUint64_t<MKL_Complex8, 1>   MKL_Complex8ToUint64_t;
typedef ConvertToUint64_t<MKL_Complex16, 2>  MKL_Complex16ToUint64_t;
