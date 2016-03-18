#                                                                      #
# Copyright 2014-2016 Intel Corporation.                               #
#                                                                      #
# This file is subject to the Intel Sample Source Code License. A copy #
# of the Intel Sample Source Code License is included.                 #
#                                                                      #

README for matMult on host and multicard Reference Code (September 3, 2015)

This file is for use of the matMult_host_multicard on Linux only.

For information on windows, please see the README.txt file in ../windows/matMult_host_multicard.

This file contains the following sections:

Contents
* INTRODUCTION
* HOW TO BUILD THE MATMUL REFERENCE CODE
* HOW TO RUN THE MATMUL REFERENCE CODE
* HOW TO INTERPRET RESULTS OF THE MATMUL REFERENCE CODE

INTRODUCTION

Matrix multiplication is an expensive mathematical operation used in
many software applications.  If we define two input matrices [A] and
[B] with [A] having M rows by K columns of numbers and [B] having K
rows by N columns of numbers, then the matrix product [C] is of size M
rows by N columns where each element of [C] is obtained by taking the
inner product (or dot product) of the corresponding row of [A] with
the corresponding column of [B].  The total number of multiplications
is (M*N*K) and the approximate number of additions is the same for a
total flops of (2*M*N*K).

If you try to use a high speed compute device to do matrix
multiplication, you need to send M*K*sizeof(double) bytes for matrix A
and then send K*N*sizeof(double) bytes for matrix B.  You typically
have to allocate enough memory (M*N*sizeof(double)) to hold the output
matrix first.  In certain common situations you have to initialize
that C output matrix to zero first. Then you start the matrix
multiplication operation on the high speed compute device.  And
finally you need to bring the output data (matrix C) back from the
device.

So the total data transfer is quantified as (M*K+K*N+M*N) double
precision numbers where double precision numbers require 8 bytes each.

If all three matrices in the expression [C] = [A]*[B] are square, then
the matrix multiplication operation requires 2*N^3 (2*Ncubed) Floating
Point Operations (flops) on the 24*N^2 (24*Nsquared) bytes.

If R_to is the data rate from the host to the compute device and
R_from is the data rate from the device back to the host, then the
"total data transfer time" T_xfer for square matrices is

       T_xfer = ((16*N^2 /R_to) + (8*N^2/ R_from)
         (bytes over bytes per second is seconds)

If G_host is the achievable flops per second on the host, then the
"time to compute on the host" is

       T_host = (2*N^3 /G_host)
         (flops divided by flops  per second is seconds)

If G_card is the achievable flops on the card form factor compute
device, then the time to compute on the card is

       T_card = (2*N^3 /G_card )
         (flops divided by flops  per second is seconds)

To decide to use a separate card to compute with, one needs to decide
if

       T_card + T_xfer < T_host

Expanding the terms and approximating R_to = R_from ~=~ R yields the
following:

       (2*N^3 /G_card) + (24*N^2/R) < (2*N^3/G_card)

Simplifying we get an equation with 3 reciprocals:

       (1/G_card) + (12/(R*N)) < (1/G_host)

as the criterion for when to use the high speed compute device to do
the matrix multiplication.

Now what if we were able to asynchronously transfer data back and
forth to a card while asynchronously executing computations.  Then we
could block the [A] & [B] matrices into a set of blocks, multiply the
blocks on the high-speed-compute device, and then pull the results
back to the host.

To see the advantage, let F_row be a row blocking factor (e.g. 2, 3,
4, 5, etc) and let F_col be the column blocking factor.  Then the
first block of [A] (we will call it [A_00]) would be (M/F_row) x
(K/F_col) in size.  Similarly, the first block of [B] (we will call it
[B_00]) will be (K/F_row)x(N/F_col) in size.  This makes the initial
data transfer requires the following amount of time for square matrix
size N and square block factor F:

       T_blkxfer = 24*N^2/(R*F^2)

So the initial data transfer time can go down with the square of the
blocking factor.  If the asynchronous compute device is partitioned
into F partitions, we will have the asynchronous compute device
completely busy in the time proportional to (1/F).  Similarly, there
will be some time at the end of the operation where the device is not
completely busy and the last matrix block results are being copied off
of the device.  But overall, the net throughput of the asynchronous
compute device can be optimized by doing block matrix multiplication
rather than full size matrix multiplication if you can successfully
overlap data transfer and computation with low enough overhead for the
blocking.

So that is why we have looked at the performance of block matrix
multiplication using h-streams. To experiment with h-streams on Xeon
Phi(TM), we have provided an initial implementation that carries out
operations as described above.

To run the matMult_host_multicard example after defining the necessary environment
variables, you would use the mat_mult executable as follows:

 $ mat_mult_host_multicard -b1000 -m 8000 -n4000 -k16000 -i5 -h1 -c0 -v

will do an (8k by 16k) (MxK) matrix times a (16k by 4k) (KxN) matrix
to yield an (8k by 4k) (MxN) output matrix using 1k by 1k submatrix
blocks using internally 4 h-streams on the Xeon Phi.  The i5 option
tell the executable to do 5 iterations and leave off the first
iteration owing to initialization overhead.

In the description above, square brackets indicate a matrix
(e.g. [A]).  The symbol ^ indicates exponentiation by a power. The
symbol * indicates multiplication.


HOW TO BUILD THE MATMUL REFERENCE CODE

1. Install MPSS 3.6
2. Install the Intel Composer XE compiler
3. Copy the reference code to an empty temporary directory:
   $ cd
   $ rm -fr temp_ref_code
   $ mkdir temp_ref_code
   $ cd temp_ref_code
   $ cp -r /usr/share/doc/hStreams/ref_code .
4. Change directory to the ref_code/matMult_host_multicard dir
   $ cd ref_code/matMult_host_multicard
5. Set the environment variables for the Intel Composer XE compiler:

For example:

. /opt/mpss_toolchains/composer/composer_xe_2013/bin/compilervars.sh intel64
or
. /opt/intel/composerxe/bin/compilervars.sh intel64

(Your mileage may vary.  For example you probably will not have the Intel Composer
 XE compiler installed in /opt/mpss_toolchains).

5. Type make:
   $ make

The make should build as follows:

[INFO] Building against system-wide Hetero Streams Library.
[INFO] In order to build against the library and headers from the repository, append in_repo=1 to make arguments
[INFO] Building release version. In order to build debug version, append CFG=DEBUG to make arguments
icpc -c [...]/temp_ref_code/hstreams/ref_code/matMult_host_multicard/matMult_host_multicard.cpp -o [...]/temp_ref_code/hstreams/ref_code/matMult_host_multicard/matMult_host_multicard.source.o -Wall -Werror-all -fPIC -DNDEBUG -O3 -diag-disable 13368 -diag-disable 15527 -I[...]/temp_ref_code/hstreams/ref_code/common -I/usr/include/hStreams  -mkl -DFP_DATA_TYPE=double
icpc -c [...]/temp_ref_code/hstreams/ref_code/common/dtime.cpp -o [...]/temp_ref_code/hstreams/ref_code/common/dtime.source.o -Wall -Werror-all -fPIC -DNDEBUG -O3 -diag-disable 13368 -diag-disable 15527 -I[...]/temp_ref_code/hstreams/ref_code/common -I/usr/include/hStreams  -mkl -DFP_DATA_TYPE=double
icpc -c [...]/temp_ref_code/hstreams/ref_code/common/hStreams_custom.cpp -o [...]/temp_ref_code/hstreams/ref_code/common/hStreams_custom.source.o -Wall -Werror-all -fPIC -DNDEBUG -O3 -diag-disable 13368 -diag-disable 15527 -I[...]/temp_ref_code/hstreams/ref_code/common -I/usr/include/hStreams  -mkl -DFP_DATA_TYPE=double
icpc [...]/temp_ref_code/hstreams/ref_code/matMult_host_multicard/matMult_host_multicard.source.o [...]/temp_ref_code/hstreams/ref_code/common/dtime.source.o [...]/temp_ref_code/hstreams/ref_code/common/hStreams_custom.source.o -o [...]/temp_ref_code/hstreams/ref_code/../bin/host/mat_mult_host_multicard   -mkl -lhstreams_source
icpc -c [...]/temp_ref_code/hstreams/ref_code/common/hStreams_custom_kernels_sink_host.cpp -o [...]/temp_ref_code/hstreams/ref_code/common/hStreams_custom_kernels_sink_host.host-sink.o -Wall -Werror-all -fPIC -DNDEBUG -O3 -diag-disable 13368 -diag-disable 15527 -I[...]/temp_ref_code/hstreams/ref_code/common -I/usr/include/hStreams  -mkl -DFP_DATA_TYPE=double -qopenmp
icpc [...]/temp_ref_code/hstreams/ref_code/common/hStreams_custom_kernels_sink_host.host-sink.o -o [...]/temp_ref_code/hstreams/ref_code/../bin/host/mat_mult_host_multicard_host.so   -mkl -shared -Wl,-soname,mat_mult_host_multicard_host.so -qopenmp


HOW TO RUN THE MATMULT Block Matrix Multiplier REFERENCE CODE for Multiple Cards

1. Examine the run_matMult_host_multicard.sh and ../common/setEnv.sh scripts.

You may need to edit it, depending on where you have the Intel
Composer XE compiler installed on your system.

Pay close attention to the setting for SINK_LD_LIBRARY_PATH, and
specifically the entries for /opt/mpss/ and the compiler.
There are multiple components:
  (a) mkl/lib/mic       : where to get the MKL libs for MIC side
  (b) compiler/lib/mic  : where to get the OpenMP libs for MIC side
  (c) /opt/mpss/3.6/sysroots/k1om-mpss-linux/usr/lib64 : where to get hstreams libs
  (d) /usr/lib64 : where to get host-side hstreams libs


2. Run the run_matMult_host_multicard.sh script:

   $ . ./run_matMult_host_multicard.sh

There are a number of switches available for controlling the executable:
 -b<N>: block size, in elements
 -m<N>: matrix size, in elements, for M dimension
 -n<N>: matrix size, in elements, for N dimension
 -k<N>: matrix size, in elements, for K dimension
 -i<N>: iterations (default 3)
 -h<N>: 1 to use host, else 0
 -c<N>: number of MIC cards (<=2, must be present)
 -l<N>: 1 to do load balancing across MICs and host, for when host slower than MIC,
   e.g. before the Haswell generation.
   This is set by default if cards > 0, host is selected; can be overridden explicitly
 -v: to make verbose

If the verbose flag is set, the number of threads reserved for each stream is
shown, along with the threads reserved for OS/COI use on the host.

It should nominally produce the following output, unless other tests in
the run script are un-commented:

*** C(mxn) = A(mxk) x B(kxn)
Load balancing across host and MIC cards is on.
Using the host CPU for compute.. and
Using 1 MIC cards for compute..
blocksize = 1000 x 1000, m = 3000, k = 3000, n = 3000
mblocks = 3, kblocks = 3, nblocks = 3
Matrix in blocks A(3,3), B(3,3), C(3,3)
number of streams used on host = 3
Perf: 270.115 Gflops/sec, Time= 199.915 msec
Perf: 781.960 Gflops/sec, Time= 69.057 msec
Perf: 761.765 Gflops/sec, Time= 70.888 msec
Perf: 758.842 Gflops/sec, Time= 71.161 msec
Perf: 744.150 Gflops/sec, Time= 72.566 msec
Size=, 3000, 3000, 3000, 3000, Pass=, 1, Iters=, 5, Max=, 781.96, GF/s, Avg_DGEMM=, 761.68, GFlop/s, StdDev=, 15.56, GFlop/s, 2.04, percent (Ignoring first iteration)
Computing result using host CPU...[If no MKLdgemm failures, then] Block Multiplication was successful.
MKL Host DGEMM Perf, 74.490, GFlops/sec, Time= 724.926 msec

HOW TO INTERPRET RESULTS OF THE MATMUL REFERENCE CODE

Note in the above, an estimated Gflops is computed.

To create a csv file to take to a spreadsheet program, do runs at different
sizes, redirecting output to a file like output.txt, then

    $ grep Size= output.txt > plot.csv

Note that significant run-run variance has been observed for the MKL Host DGEMM
performance, which is run only once and not warmed up.  It should not really
be used for performance comparison against the hStreams results.
