/*
 * Copyright 2014-2016 Intel Corporation.
 *
 * This file is subject to the Intel Sample Source Code License. A copy
 * of the Intel Sample Source Code License is included.
 */

#include <time.h>
// this file requires no license
//=======================timer stuff ===============
// Timing Prototypes: Include where needed
void  dtimeInit();
double dtimeGet();
double dtimeSince(double t1, char *str);
double dtimeElapse(double t1);
