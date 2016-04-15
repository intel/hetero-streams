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
 *   computes the average, min and max and then
 *   copies the result in the host side memory proxy address.
 **/
HSTREAMS_EXPORT
void average_min_max_sink_1(uint64_t a, uint64_t b, uint64_t, uint64_t,   /*  1- 4 */
                            uint64_t, uint64_t, uint64_t, uint64_t,       /*  5- 8 */
                            uint64_t, uint64_t, uint64_t, uint64_t,       /*  9-12 */
                            uint64_t, uint64_t, uint64_t, uint64_t,       /* 13-16 */
                            uint64_t, uint64_t, uint64_t,                 /* 17-19 */
                            void *ret_val,                                /* return value */
                            uint16_t ret_val_size                         /* size of return value */
                           )
{
    printf("On sink side received: %d, and %d from host!\n", (int)a, (int)b);

    //!!c Set up the blob you wish to return
    double return_args[3];
    return_args[0] = ((int) a + (int) b) / 2.0;
    return_args[1] = (a > b) ? b : a;
    return_args[2] = (a > b) ? a : b;
    if (ret_val_size < sizeof(double) * 3) {
        printf("Value returned from sink side corrupted if not big enough\n");
    }
    //!!c Copy the blob to the space that the user passed in
    // hStreams does the magic to copy that blob back to the source
    memcpy(ret_val, return_args, ret_val_size);
}
