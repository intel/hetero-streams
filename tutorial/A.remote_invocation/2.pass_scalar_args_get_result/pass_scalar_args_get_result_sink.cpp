/*
 *    Copyright 2014-2016 Intel Corporation.
 *
 *    This file is subject to the Intel Sample Source Code License. A copy
 *    of the Intel Sample Source Code License is included.
 *
 */

// Basic hStreams sink side library
#include <hStreams_sink.h>

// C standard input/output library
#include <stdio.h>
#include <string.h>
/*
 *  This file is an example of a device (sink) side code.
 *   It receives the values passed to it from the host,
 *   computes the average and then
 *   copies the result in the host side memory proxy address.
 **/
//!!c This form is general enough to handle up to 19 arguments
//  The unused args may be omitted unless return values are used.
//  We seek to offer a simpler and more comprehensive interface in the future
HSTREAMS_EXPORT
void
average_sink_1(uint64_t a, uint64_t b, uint64_t, uint64_t, /*  1- 4 */
               uint64_t, uint64_t, uint64_t, uint64_t,     /*  5- 8 */
               uint64_t, uint64_t, uint64_t, uint64_t,     /*  9-12 */
               uint64_t, uint64_t, uint64_t, uint64_t,     /* 13-16 */
               uint64_t, uint64_t, uint64_t,               /* 17-19 */
               void *ret_val,                              /* return value */
               uint16_t ret_val_size                       /* size of return */
              )
{
    double average = ((int) a + (int) b) / 2.0;
    if (ret_val_size < sizeof(average)) {
        printf("Value returned from sink side corrupted if not big enough\n");
    }
    memcpy(ret_val, &average, ret_val_size);
    printf("On sink side received: %d, and %d, and computed average %f !\n",
           (int)a, (int)b, average);
}
