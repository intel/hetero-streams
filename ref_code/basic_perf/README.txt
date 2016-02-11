#                                                                      #
# Copyright 2014-2016 Intel Corporation.                               #
#                                                                      #
# This file is subject to the Intel Sample Source Code License. A copy #
# of the Intel Sample Source Code License is included.                 #
#                                                                      #

README for basic_perf.cpp Reference Code (Aug 15, 2014) This is a
reference application that does the most basic performance testing

This file is for use of the basic_perf on Linux only.

For information on windows, please see the README.txt file in ../windows/basic_perf.

This file contains the following sections:

* HOW TO BUILD THE BASIC_PERF REFERENCE CODE
* HOW TO RUN THE BASIC_PERF REFERENCE CODE
* HOW TO INTERPRET RESULTS OF THE BASIC_PERF REFERENCE CODE

**************************************************
**** HOW TO BUILD THE BASIC_PERF REFERENCE CODE
**************************************************

1. Install MPSS 3.4
2. Install the Intel Composer XE compiler
3. Copy the reference code to an empty temporary directory:
   $ cd
   $ rm -fr temp_ref_code
   $ mkdir temp_ref_code
   $ cd temp_ref_code
   $ cp -r /usr/share/doc/hStreams/ref_code .
4. Change directory to the ref_code/basic_perf dir
   $ cd ref_code/basic_perf
5. Set the environment variables for the Intel Composer XE compiler:

For example:

. /opt/mpss_toolchains/composer/composer_xe_2013/bin/compilervars.sh intel64
or
. /opt/intel/composerxe/bin/compilervars.sh intel64

(Your mileage may vary.  For example you probably will not have the Intel Composer
 XE compiler installed in /opt/mpss_toolchains).

5. Type make:
   make

You will see something like the following:

[INFO] Building against system-wide Hetero Streams Library.
[INFO] In order to build against the library and headers from the repository, append in_repo=1 to make arguments
[INFO] Building release version. In order to build debug version, append CFG=DEBUG to make arguments
icpc -c /localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/basic_perf/basic_perf.cpp -o /localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/basic_perf/basic_perf.source.o -Wall -Werror-all -fPIC -DNDEBUG -O3 -diag-disable 13368 -diag-disable 15527 -I/localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/common -I/usr/include/hStreams
icpc -c /localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/common/dtime.cpp -o /localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/common/dtime.source.o -Wall -Werror-all -fPIC -DNDEBUG -O3 -diag-disable 13368 -diag-disable 15527 -I/localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/common -I/usr/include/hStreams
icpc /localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/basic_perf/basic_perf.source.o /localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/common/dtime.source.o -o /localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/../bin/host/basic_perf   -lhstreams_source



**************************************************
**** HOW TO RUN THE BASIC_PERF REFERENCE CODE
**************************************************

What you can change: driver parameters in basic_perf.cpp
  #define SIZE 4096                               // size of each 2D matrix dimension
  #define DATA_TYPE double                        // double, float
  #define HSTREAMS_APP_XGEMM hStreams_app_dgemm   // *gemm type
  #define MAX_CONCURRENCY 4                       // number of streams per domain (card)
  #define ITERATIONS 100                          // timing iterations

If you change these, you will need to rebuild - see above.

There are no command line arguments.  Invoke the application with

./run_basic_perf.sh

Pay close attention to the setting for SINK_LD_LIBRARY_PATH, and
specifically the entries for /opt/mpss/ and the compiler.
There are multiple components:
  (a) mkl/lib/mic       : where to get the MKL libs for MIC side in composerxe
  (b) compiler/lib/mic  : where to get the OpenMP libs for MIC side
  (c) /opt/mpss/3.4/sysroots/k1om-mpss-linux/usr/lib64 : where to get hstreams
libs in production release

If you don't have /usr/lib64 in your host-side LD_LIBRARY_PATH, you may need
to add /usr/lib64.

**************************************************************
**** HOW TO INTERPRET RESULTS OF THE BASIC_PERF REFERENCE CODE
**************************************************************

A sample output of this app is as follows:

~/my_hstreams/hstreams-release-3.4/bin/host ~/my_hstreams/hstreams-release-3.4/ref_code/basic_perf
~/my_hstreams/hstreams-release-3.4/bin/dev ~/my_hstreams/hstreams-release-3.4/bin/host ~/my_hstreams/hstreams-release-3.4/ref_code/basic_perf
~/my_hstreams/hstreams-release-3.4/bin/host ~/my_hstreams/hstreams-release-3.4/ref_code/basic_perf
~/my_hstreams/hstreams-release-3.4/ref_code/basic_perf
>>init
>>create bufs
>>xfer mem to remote domain
Transfer A and B to remote domain: 7013.166 ms for 100 iterations, 1 streams
Host-card data rate (1 streams, 4096-square) = 0.957 GB/s
>>xfer mem from remote domain
Transfer A from remote domain: 7622.976 ms for 100 iterations, 1 streams
Card-host data rate (1 streams, 4096-square) = 0.880 GB/s
>>xgemm - sample fixed functionality
Remote domain xgemm time= 29657.001 ms for 100 iterations, using 1 streams
Remote domain dgem data rate (1 streams, 4096-square) = 0.113 GFl/s
>>fini
>>init
>>create bufs
>>xfer mem to remote domain
Transfer A and B to remote domain: 29912.661 ms for 100 iterations, 4 streams
Host-card data rate (4 streams, 4096-square) = 0.897 GB/s
>>xfer mem from remote domain
Transfer A from remote domain: 27308.315 ms for 100 iterations, 4 streams
Card-host data rate (4 streams, 4096-square) = 0.983 GB/s
>>xgemm - sample fixed functionality
Remote domain xgemm time= 112590.318 ms for 100 iterations, using 4 streams
Remote domain dgem data rate (4 streams, 4096-square) = 0.119 GFl/s
>>fini

This shows the rate of data transfer to and from the card,
 for the specified size (2D matrix dimension size of 4096)
 corrected for some number of timing iterations (100)
 for different numbers of streams: 1 and 4
For unidirectional independent transfers, DMAs themselves are not concurrent.
This result shows that DMAs in different channels don't slow down much,
 as expected.  If it goes faster with more streams, that's likely in the noise.

This also shows that concurrent dgemms in multiple streams are a bit faster
 than dgemms that are OpenMP-parallelized in a single stream.

Later versions of this app will show speedup from making computations
 concurrent, by using different streams, and by overlapping data transfer
 and computation.
