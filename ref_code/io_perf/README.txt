#                                                                      #
# Copyright 2014-2016 Intel Corporation.                               #
#                                                                      #
# This file is subject to the Intel Sample Source Code License. A copy #
# of the Intel Sample Source Code License is included.                 #
#                                                                      #

README for io_perf.cpp, IO performance benchmark for HSTREAMS.
This file is for use of the io_perf on Linux only.


**************************************************
**** HOW TO BUILD IO_PERF
**************************************************

1. Install MPSS 3.4
2. Install the Intel Composer XE compiler
3. Copy the reference code to an empty temporary directory:
   $ cd
   $ rm -fr temp_ref_code
   $ mkdir temp_ref_code
   $ cd temp_ref_code
   $ cp -r /usr/share/doc/hStreams/ref_code .
4. Change directory to the ref_code/io_perf dir
   $ cd ref_code/io_perf
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
icpc -c /localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/io_perf/io_perf.cpp -o /localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/io_perf/io_perf.source.o -Wall -Werror-all -fPIC -DNDEBUG -O3 -diag-disable 13368 -diag-disable 15527 -I/localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/common -I/usr/include/hStreams  -qopenmp
icpc -c /localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/common/dtime.cpp -o /localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/common/dtime.source.o -Wall -Werror-all -fPIC -DNDEBUG -O3 -diag-disable 13368 -diag-disable 15527 -I/localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/common -I/usr/include/hStreams  -qopenmp
icpc /localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/io_perf/io_perf.source.o /localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/common/dtime.source.o -o /localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/../bin/host/io_perf   -lhstreams_source -qopenmp


**************************************************
**** HOW TO RUN IO_PERF
**************************************************

The simplest way is to invoke the application with

./run_io_perf.sh

Command line arguments:
    -b <number>    buffer size (default 1MB).
    -s <number>     number of concurrent streams (default 1).
    -i <number>     iterations (default 1000).
    -f              transfer from card to host (default host-to-card).
    -v              verbose output.

Pay close attention to the setting for SINK_LD_LIBRARY_PATH, and
specifically the entries for /opt/mpss/ and the compiler.
There are multiple components:
  (a) mkl/lib/mic       : where to get the MKL libs for MIC side in composerxe
  (b) compiler/lib/mic  : where to get the OpenMP libs for MIC side
  (c) /opt/mpss/3.4/sysroots/k1om-mpss-linux/usr/lib64 : where to get hstreams
libs in production release

If you don't have /usr/lib64 in your host-side LD_LIBRARY_PATH, you may need
to add /usr/lib64.
