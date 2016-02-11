#                                                                      #
# Copyright 2014-2016 Intel Corporation.                               #
#                                                                      #
# This file is subject to the Intel Sample Source Code License. A copy #
# of the Intel Sample Source Code License is included.                 #
#                                                                      #

README for Cholesky Factorization Reference Code (16th January, 2015)

This file is for use for the cholesky on Linux only.

For information on windows, please see the README.txt file in
../windows/cholesky.

This file contains the following sections:

Contents
* INTRODUCTION
* HOW TO BUILD THE CHOLESKY REFERENCE CODE
* HOW TO RUN THE CHOLESKY REFERENCE CODE

INTRODUCTION

Cholesky decomposition is a decomposition of Hermitian, postive-definite matrix
into the product of a lower triangular matrix and its conjugate transpose,
useful for efficient numerical solutions and Monte Carlo simulations. When it
is applicable, the Cholesky decomposition is roughly twice as efficient as LU
decomposition for solving systems of linear equations [Ref. 1].

If we define a square matrix [A] having M rows by M columns, then the Cholesky
decomposition is of the form:
    [A] = [L] * [L^t]
where, [L] is a lower triangular matrix, and [L^t] is its transpose.
Every Hermitian positive-definite matrix (and thus also every real-valued
symmetric postive-definite matrix) has a unique Cholesky decomposition.

The computational cost of a Cholesky decomposition is (1/3)*M^3 floating point
operations. Typically only half of the matrix A is stored (lower triangular),
and this matrix is overwritten by the cholesky factor matrix. Thus the memory
footprint is (1/2)*M^2*sizeof(double) bytes (for double-precision real-valued
matrix)  (In the reference code we do save the complete matrix).  An algorithm
which is applied on elements of matrix [A] can only proceed one column at a
time as there is a data dependence which limits the overall concurrency.
Moreover, as one proceeds down the columns, the number of entries updated
reduces as we have implemented the right-looking Cholesky decomposition
algorithm [Ref. 2].

In this program, we do a tiled implementation of Cholesky decomposition (for
double-precision real-valued matrices) [Ref. 3] using fast level-3 BLAS
components. We use the hStreams framework to partition the card into 4 or 6
physical partitions and schedule BLAS3 operations (DPOTRF, DTRSM, DSYRK, DGEMM)
into compute-streams associated with these partitions in an as concurrent
manner as possible. The matrix is divided into T x T tiles each containing
(M/T * M/T elements). The outer-loop is iterated over 1:T. For the k-th outer-
loop iteration, we have 1 DPOTRF (of the k x k tile), T-k DTRSMs, T-k DSYRKs,
and (1/2)*(T-k-1)*(T-k) DGEMMs. The order of operations (and dependence) of
BLAS3 operations for a given k is: DPOTRF -> DTRSM -> DSYRK + DGEMM (i.e.
DSYRK and DGEMM depend on DTRSM, which in turn depends on DPOTRF). Thus the
max concurrency for a given k is : no. of DGEMMs + no. of DSYRKs = T-k +
(1/2)*(T-k-1)*(T-k). The concurrency is highest for k = 1, and is = T-1 +
(1/2)*(T-2)*(T-1). If T = 6, for example, then the max concurrency is
5 + 10 = 15.

It's best if this max concurrency is divisible by number of partitions on the
card. As a rule of thumb, we have observed that no. of partitions on card =
T - 1 gives the best results, and in particular, we have observed that T = 6,
and no. of partitions = 5, give best results for most cases.

There are two versions of the tiled-Cholesky program. One called tiled_host
performs the tiled-Cholesky on the host only (without using hStreams or the
card). The performance of this program is compared against the MKL DPOTRF
without automatic offload. The other version is called tiled_hstreams. This
uses both the host and the card, by using the hStreams library. The performance
of the tiled_hstreams version is compared against the MKL DPOTRF with automatic
offload. For the tiled_hstreams Cholesky decomposition, data transfer
to/from the card is interleaved with compute on the card using the concurrently
running streams.

To understand the data/compute dependencies, it's recommended that the user
first becomes familiar with the algorithm in the tiled_host example and then
it will be easier to understand the various synchronizations (in the form of
_event_wait) required in the tiled_hstreams example.

References:
1) Wikipedia. http://en.wikipedia.org/wiki/Cholesky_decomposition
2) Trefethen, Lloyd N. and Bau III, David, Numerical Linear Algebra,
SIAM (1997).
3) Jeannot, Emmanuel. Performance Analysis and Optimization of the Tiled
Cholesky Factorization on NUMA Machines. PAAP 2012-IEEE International Symposium
on Parallel Architectures, Algorithms and Programming. 2012.

In the description above, square brackets indicate a matrix
(e.g. [A]).  The symbol ^ indicates exponentiation by a power.
The symbol * indicates multiplication.

0. Set up the environment
All of the run_<version>.sh scripts set several environment variables, like
 export KMP_AFFINITY=scatter
and source the common setEnv.sh script.

To run any of the examples manually, they must be set up:
 source ../../common/setEnv.sh

Additionally, for MKL AO, the following should be set:
 export MIC_ENV_PREFIX=MIC
 export MIC_KMP_AFFINITY=balanced
 export MKL_MIC_MAX_MEMORY=8G
 export MIC_USE_2MB_BUFFERS=64K


1. To run the tiled_host example
See the setup sequence above, unless the run script is used.
Run the cholesky_tiled_host executable as follows:

./cholesky_tiled_host -m 4800 -t 6 -l col -i 5

the description of command line inputs is:
-m : matrix size per side
-t : no. of tiles the matrix is broken into per side
-l : row (or ROW) if ROW_MAJOR layout of matrix storage is used, otherwise COL_MAJOR
-i : no. of iterations to be performed

NOTE: the matrix size MUST be divisible by number of tiles

one can also use the run_tiled_host.sh script provided, which also
sets the KMP_AFFINITY.

2. To run the tiled_hstreams example
See the setup sequence above, unless the run script is used.
Run the cholesky_tiled_hstreams executable as follows:

./cholesky_tiled_hstreams -m 4800 -t 6 -s 5 -l col -i 5

the description of command line inputs is:
-m : matrix size per side
-t : no. of tiles the matrix is broken into per side
-s : no. of logical streams (= no. of physical partitions on the card)
-l : row (or ROW) if ROW_MAJOR layout of matrix storage is used, otherwise COL_MAJOR
-i : no. of iterations to be performed

NOTE: the matrix size MUST be divisible by number of tiles

one can also use the run_tiled_hstreams.sh script provided, which also
sets the KMP_AFFINITY on host and card.

3. To run the tiled_hstreams_host_multicard example
See the setup sequence above, unless the run script is used.

There are a number of switches available for controlling the executable:
 -m<N>: matrix size, in elements, for each dimension
 -t<N>: number of tiles on each side
 -s<N>: number of streams to use on MIC
 -l<S>: where layout S is ROW or row to use row major format, else col major
 -i<N>: iterations (default 5)
 -h<N>: 1 to use host, else 0
 -c<N>: number of MIC cards (<=2, must be present)
 -v<N>: 1 to verify

If the verbose flag is set, the number of threads reserved for each stream is
shown, along with the threads reserved for OS/COI use on the host.

Run the choleksy_tiled_hstreams_host_multicard executable as follows:

# target only host
./cholesky_tiled_hstreams_host_multicard -m 8000 -t 10 -s 3 -h 1 -c 0 -l col -i 5

# target only 1 card
./cholesky_tiled_hstreams_host_multicard -m 8000 -t 10 -s 3 -h 0 -c 1 -l col -i 5

# target 2 cards
./cholesky_tiled_hstreams_host_multicard -m 8000 -t 10 -s 3 -h 0 -c 2 -l col -i 5

# target host + 1 card
./cholesky_tiled_hstreams_host_multicard -m 8000 -t 10 -s 3 -h 1 -c 1 -l col -i 5

# target host + 2 cards
./cholesky_tiled_hstreams_host_multicard -m 8000 -t 10 -s 3 -h 1 -c 2 -l col -i 5

the description of command line inputs is:
-m : matrix size
-t : number of tiles
-s : number of streams (partitions on MIC)
-i : number of iterations
-l : layout (row or ROW for rowmajor; else is colMajor)
-h : using the host or not - can be 0 or 1
-c : number of cards used - can be 0 or 1 or 2

NOTE: the matrix size MUST be divisible by number of tiles

one can also use the run_tiled_hstreams_host_multicard.sh script
provided, which also sets the KMP_AFFINITY on host and card.


HOW TO BUILD THE CHOLESKY REFERENCE CODE

1. Install MPSS 3.6
2. Install the Intel Composer XE compiler
3. Copy the reference code to an empty temporary directory:
   $ cd
   $ rm -fr temp_ref_code
   $ mkdir temp_ref_code
   $ cd temp_ref_code
   $ cp -r /usr/share/doc/hStreams/ref_code .
4. Change directory to the ref_code/cholesky dir
   $ cd ref_code/cholesky
5. Set the environment variables for the Intel Composer XE compiler:

For example:

. /opt/mpss_toolchains/composer/composer_xe_2013/bin/compilervars.sh intel64
or
. /opt/intel/composerxe/bin/compilervars.sh intel64

(Your mileage may vary.  For example you probably will not have the Intel Composer
 XE compiler installed in /opt/mpss_toolchains).

5. First build the tiled_host version. Change directory to the tiled_host dir
   $ cd tiled_host
6. Type make:
   $ make

The make should build as follows:
$ make
[INFO] Building against system-wide Hetero Streams Library.
[INFO] In order to build against the library and headers from the repository, append in_repo=1 to make arguments
[INFO] Building release version. In order to build debug version, append CFG=DEBUG to make arguments
icpc -c /localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/cholesky/tiled_host/cholesky_tiled.cpp -o /localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/cholesky/tiled_host/cholesky_tiled.source.o -Wall -Werror-all -fPIC -DNDEBUG -O3 -diag-disable 13368 -diag-disable 15527 -I/localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/common -I/usr/include/hStreams  -mkl -openmp
icpc -c /localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/common/dtime.cpp -o /localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/common/dtime.source.o -Wall -Werror-all -fPIC -DNDEBUG -O3 -diag-disable 13368 -diag-disable 15527 -I/localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/common -I/usr/include/hStreams  -mkl -openmp
icpc -c /localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/common/matrices_generator.cpp -o /localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/common/matrices_generator.source.o -Wall -Werror-all -fPIC -DNDEBUG -O3 -diag-disable 13368 -diag-disable 15527 -I/localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/common -I/usr/include/hStreams  -mkl -openmp
icpc /localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/cholesky/tiled_host/cholesky_tiled.source.o /localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/common/dtime.source.o /localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/common/matrices_generator.source.o -o /localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/../bin/host/cholesky_tiled_host   -mkl -openmp


7. Now build the tiled_hstreams version. Change directory to the ../tiled_hstreams dir
   $ cd ../tiled_hstreams
8. Type make:
   $ make

The make should build as follows:
$ make
[INFO] Building against system-wide Hetero Streams Library.
[INFO] In order to build against the library and headers from the repository, append in_repo=1 to make arguments
[INFO] Building release version. In order to build debug version, append CFG=DEBUG to make arguments
icpc -c /localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/cholesky/tiled_hstreams/cholesky_tiled_hstreams.cpp -o /localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/cholesky/tiled_hstreams/cholesky_tiled_hstreams.source.o -Wall -Werror-all -fPIC -DNDEBUG -O3 -diag-disable 13368 -diag-disable 15527 -I/localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/common -I/usr/include/hStreams  -mkl -openmp
icpc -c /localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/common/dtime.cpp -o /localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/common/dtime.source.o -Wall -Werror-all -fPIC -DNDEBUG -O3 -diag-disable 13368 -diag-disable 15527 -I/localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/common -I/usr/include/hStreams  -mkl -openmp
icpc -c /localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/common/hStreams_custom.cpp -o /localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/common/hStreams_custom.source.o -Wall -Werror-all -fPIC -DNDEBUG -O3 -diag-disable 13368 -diag-disable 15527 -I/localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/common -I/usr/include/hStreams  -mkl -openmp
icpc -c /localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/common/matrices_generator.cpp -o /localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/common/matrices_generator.source.o -Wall -Werror-all -fPIC -DNDEBUG -O3 -diag-disable 13368 -diag-disable 15527 -I/localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/common -I/usr/include/hStreams  -mkl -openmp
icpc /localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/cholesky/tiled_hstreams/cholesky_tiled_hstreams.source.o /localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/common/dtime.source.o /localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/common/hStreams_custom.source.o /localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/common/matrices_generator.source.o -o /localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/../bin/host/cholesky_tiled_hstreams   -mkl -lhstreams_source -openmp
icpc -mmic -c /localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/common/hStreams_custom_kernels_sink.cpp -o /localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/common/hStreams_custom_kernels_sink.x100-sink.o -Wall -Werror-all -fPIC -DNDEBUG -O3 -diag-disable 13368 -diag-disable 15527 -I/localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/common -I/usr/include/hStreams  -mkl -openmp
icpc -mmic /localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/common/hStreams_custom_kernels_sink.x100-sink.o -o /localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/../bin/dev/cholesky_sink_1.so   -mkl -shared -Wl,-soname,cholesky_sink_1.so -openmp


9. Now build the tiled_hstreams_host_multicard version
Change directory to the ../tiled_hstreams_host_multicard dir
   $ cd ../tiled_hstreams_host_multicard
10. Type make:
   $ make

The make should build as follows:
$ make
[INFO] Building against system-wide Hetero Streams Library.
[INFO] In order to build against the library and headers from the repository, append in_repo=1 to make arguments
[INFO] Building release version. In order to build debug version, append CFG=DEBUG to make arguments
icpc -c /localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/cholesky/tiled_hstreams_host_multicard/cholesky_tiled_hstreams_host_multicard.cpp -o /localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/cholesky/tiled_hstreams_host_multicard/cholesky_tiled_hstreams_host_multicard.source.o -Wall -Werror-all -fPIC -DNDEBUG -O3 -diag-disable 13368 -diag-disable 15527 -I/localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/common -I/usr/include/hStreams  -mkl -openmp
icpc -c /localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/common/dtime.cpp -o /localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/common/dtime.source.o -Wall -Werror-all -fPIC -DNDEBUG -O3 -diag-disable 13368 -diag-disable 15527 -I/localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/common -I/usr/include/hStreams  -mkl -openmp
icpc -c /localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/common/hStreams_custom.cpp -o /localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/common/hStreams_custom.source.o -Wall -Werror-all -fPIC -DNDEBUG -O3 -diag-disable 13368 -diag-disable 15527 -I/localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/common -I/usr/include/hStreams  -mkl -openmp
icpc -c /localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/common/matrices_generator.cpp -o /localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/common/matrices_generator.source.o -Wall -Werror-all -fPIC -DNDEBUG -O3 -diag-disable 13368 -diag-disable 15527 -I/localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/common -I/usr/include/hStreams  -mkl -openmp
icpc /localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/cholesky/tiled_hstreams_host_multicard/cholesky_tiled_hstreams_host_multicard.source.o /localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/common/dtime.source.o /localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/common/hStreams_custom.source.o /localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/common/matrices_generator.source.o -o /localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/../bin/host/cholesky_tiled_hstreams_host_multicard   -mkl -lhstreams_source -openmp
icpc -c /localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/common/hStreams_custom_kernels_sink_host.cpp -o /localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/common/hStreams_custom_kernels_sink_host.host-sink.o -Wall -Werror-all -fPIC -DNDEBUG -O3 -diag-disable 13368 -diag-disable 15527 -I/localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/common -I/usr/include/hStreams  -mkl -openmp
icpc /localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/common/hStreams_custom_kernels_sink_host.host-sink.o -o /localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/../bin/host/cholesky_tiled_hstreams_host_multicard_host.so   -mkl -shared -Wl,-soname,mat_mult_host_multicard_host.so -openmp
icpc -mmic -c /localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/common/hStreams_custom_kernels_sink.cpp -o /localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/common/hStreams_custom_kernels_sink.x100-sink.o -Wall -Werror-all -fPIC -DNDEBUG -O3 -diag-disable 13368 -diag-disable 15527 -I/localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/common -I/usr/include/hStreams  -mkl -openmp
icpc -mmic /localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/common/hStreams_custom_kernels_sink.x100-sink.o -o /localdisk/work/wwasko/hstreams_workspace/hstreams/ref_code/../bin/host/cholesky_sink_1.so   -mkl -shared -Wl,-soname,cholesky_tiled_hstreams_host_multicard_host.so -openmp


HOW TO RUN THE Cholesky REFERENCE CODE


Run the tiled_host version
1. Change directory to the tiled_host dir, and run the run_tiled_host.sh script:

   $ cd ../tiled_host
   $ ./run_tiled_host.sh

This is how a typical output looks:

matrix is in Col major format
mat_size = 4800, num_tiles = 6, niter = 5


iter 0
time for Tiled Cholesky = 143.51 msec
time for MKL Cholesky (host, NO AO) = 98.98 msec
Tiled Cholesky successful

iter 1
time for Tiled Cholesky = 139.29 msec
time for MKL Cholesky (host, NO AO) = 98.79 msec
Tiled Cholesky successful

iter 2
time for Tiled Cholesky = 140.96 msec
time for MKL Cholesky (host, NO AO) = 98.99 msec
Tiled Cholesky successful

iter 3
time for Tiled Cholesky = 140.14 msec
time for MKL Cholesky (host, NO AO) = 99.72 msec
Tiled Cholesky successful

iter 4
time for Tiled Cholesky = 140.44 msec
time for MKL Cholesky (host, NO AO) = 98.32 msec
Tiled Cholesky successful

Tiled Cholesky, for 4 iterations (ignoring first),
mean Time = 140.21 msec, stdDev Time = 0.70 msec,
Mean Gflops (using mean Time) = 262.92

MKL Cholesky (host, NO AO), for 4 iterations (ignoring first),
mean Time = 98.95 msec, stdDev Time = 0.58 msec,
Mean Gflops (using mean Time) = 372.53

matrix is in Row major format
mat_size = 4800, num_tiles = 6, niter = 5


iter 0
time for Tiled Cholesky = 135.38 msec
time for MKL Cholesky (host, NO AO) = 256.18 msec
Tiled Cholesky successful

iter 1
time for Tiled Cholesky = 131.24 msec
time for MKL Cholesky (host, NO AO) = 255.67 msec
Tiled Cholesky successful

iter 2
time for Tiled Cholesky = 130.56 msec
time for MKL Cholesky (host, NO AO) = 255.50 msec
Tiled Cholesky successful

iter 3
time for Tiled Cholesky = 130.26 msec
time for MKL Cholesky (host, NO AO) = 255.88 msec
Tiled Cholesky successful

iter 4
time for Tiled Cholesky = 130.68 msec
time for MKL Cholesky (host, NO AO) = 256.60 msec
Tiled Cholesky successful

Tiled Cholesky, for 4 iterations (ignoring first),
mean Time = 130.68 msec, stdDev Time = 0.41 msec,
Mean Gflops (using mean Time) = 282.09

MKL Cholesky (host, NO AO), for 4 iterations (ignoring first),
mean Time = 255.91 msec, stdDev Time = 0.48 msec,
Mean Gflops (using mean Time) = 144.05


Run the tiled_hstreams version
1. Change directory to the tiled_hstreams dir, and run the
run_tiled_host.sh script:

   $ cd ../tiled_hstreams
   $ ./run_tiled_host.sh

This is how a typical output looks:

matrix is in Col major format
no. of streams (partitions) = 5, mat_size = 4800, num_tiles = 6, niter = 5


Iteration = 0
time for Tiled hstreams Cholesky for iteration 0 = 633.18 msec
time for MKL Cholesky (AO) for iteration 0 = 104.06 msec
Tiled Cholesky successful

Iteration = 1
time for Tiled hstreams Cholesky for iteration 1 = 175.23 msec
time for MKL Cholesky (AO) for iteration 1 = 99.91 msec
Tiled Cholesky successful

Iteration = 2
time for Tiled hstreams Cholesky for iteration 2 = 166.01 msec
time for MKL Cholesky (AO) for iteration 2 = 97.92 msec
Tiled Cholesky successful

Iteration = 3
time for Tiled hstreams Cholesky for iteration 3 = 171.43 msec
time for MKL Cholesky (AO) for iteration 3 = 98.70 msec
Tiled Cholesky successful

Iteration = 4
time for Tiled hstreams Cholesky for iteration 4 = 167.40 msec
time for MKL Cholesky (AO) for iteration 4 = 99.06 msec
Tiled Cholesky successful

Matrix size = 4800
Tiled hStreams Cholesky: for 4 iterations (ignoring first),
mean Time = 170.02 msec, stdDev Time = 4.17 msec,
Mean Gflops (using mean Time) = 216.83

MKL AO Cholesky: for 4 iterations (ignoring first),
mean Time = 98.89 msec, stdDev Time = 0.82 msec,
Mean Gflops (using meanTime) = 372.76

matrix is in Row major format
no. of streams (partitions) = 5, mat_size = 4800, num_tiles = 6, niter = 5


Iteration = 0
time for Tiled hstreams Cholesky for iteration 0 = 578.22 msec
time for MKL Cholesky (AO) for iteration 0 = 272.87 msec
Tiled Cholesky successful

Iteration = 1
time for Tiled hstreams Cholesky for iteration 1 = 175.35 msec
time for MKL Cholesky (AO) for iteration 1 = 259.23 msec
Tiled Cholesky successful

Iteration = 2
time for Tiled hstreams Cholesky for iteration 2 = 184.11 msec
time for MKL Cholesky (AO) for iteration 2 = 259.43 msec
Tiled Cholesky successful

Iteration = 3
time for Tiled hstreams Cholesky for iteration 3 = 182.33 msec
time for MKL Cholesky (AO) for iteration 3 = 257.85 msec
Tiled Cholesky successful

Iteration = 4
time for Tiled hstreams Cholesky for iteration 4 = 185.61 msec
time for MKL Cholesky (AO) for iteration 4 = 261.16 msec
Tiled Cholesky successful

Matrix size = 4800
Tiled hStreams Cholesky: for 4 iterations (ignoring first),
mean Time = 181.85 msec, stdDev Time = 4.53 msec,
Mean Gflops (using mean Time) = 202.72

MKL AO Cholesky: for 4 iterations (ignoring first),
mean Time = 259.42 msec, stdDev Time = 1.36 msec,
Mean Gflops (using meanTime) = 142.10



Run the tiled_hstreams_host_multicard version
1. Change directory to the tiled_hstreams_host_multicard dir, and run the
run_tiled_hstreams_host_multicard.sh script:

   $ cd ../tiled_hstreams_host_multicard
   $ ./run_tiled_hstreams_host_multicard.sh

If the verbose flag is turned on with -v1 (not shown), the output indicates
the width of each stream, and shows that there are overlapping streams:
stream 0 is full-width on the host, for potrf, while streams 1-3 overlap it.
We call overlapping sets of disjoint streams leagues.

This is how a typical output looks:

matrix is in Col major format
Using the host CPU for compute.. and
Using 0 MIC cards for compute..
number of streams used on host = 3
OMP: Warning #213: KMP_AFFINITY must be set prior to first parallel region or certain API calls; ignored.

Iteration = 0
time for Tiled hstreams Cholesky for iteration 0 = 491.42 msec
time for MKL Cholesky (AO) for iteration 0 = 443.25 msec
Tiled Cholesky successful

Iteration = 1
time for Tiled hstreams Cholesky for iteration 1 = 492.66 msec
time for MKL Cholesky (AO) for iteration 1 = 448.55 msec
Tiled Cholesky successful

Iteration = 2
time for Tiled hstreams Cholesky for iteration 2 = 483.76 msec
time for MKL Cholesky (AO) for iteration 2 = 446.69 msec
Tiled Cholesky successful

Iteration = 3
time for Tiled hstreams Cholesky for iteration 3 = 485.41 msec
time for MKL Cholesky (AO) for iteration 3 = 442.27 msec
Tiled Cholesky successful

Iteration = 4
time for Tiled hstreams Cholesky for iteration 4 = 489.98 msec
time for MKL Cholesky (AO) for iteration 4 = 448.06 msec
Tiled Cholesky successful

Matrix size = 8000
Tiled hStreams Cholesky: for 4 iterations (ignoring first),
mean Time = 487.95 msec, stdDev Time = 4.09 msec,
Mean Gflops (using mean Time) = 349.76

MKL AO Cholesky: for 4 iterations (ignoring first),
mean Time = 446.39 msec, stdDev Time = 2.86 msec,
Mean Gflops (using meanTime) = 382.33

[4]+  Terminated              ./cholesky_tiled_hstreams_host_multicard -m 8000 -t 10 -s 3 -h 1 -c 1 -l col -i 5
matrix is in Col major format
Using 1 MIC cards for compute..
number of streams used on mic = 3

Iteration = 0
time for Tiled hstreams Cholesky for iteration 0 = 885.48 msec
time for MKL Cholesky (AO) for iteration 0 = 446.06 msec
Tiled Cholesky successful

Iteration = 1
time for Tiled hstreams Cholesky for iteration 1 = 439.57 msec
time for MKL Cholesky (AO) for iteration 1 = 446.59 msec
Tiled Cholesky successful

Iteration = 2
time for Tiled hstreams Cholesky for iteration 2 = 468.49 msec
time for MKL Cholesky (AO) for iteration 2 = 448.75 msec
Tiled Cholesky successful

Iteration = 3
time for Tiled hstreams Cholesky for iteration 3 = 464.57 msec
time for MKL Cholesky (AO) for iteration 3 = 446.69 msec
Tiled Cholesky successful

Iteration = 4
time for Tiled hstreams Cholesky for iteration 4 = 463.26 msec
time for MKL Cholesky (AO) for iteration 4 = 447.25 msec
Tiled Cholesky successful

Matrix size = 8000
Tiled hStreams Cholesky: for 4 iterations (ignoring first),
mean Time = 458.97 msec, stdDev Time = 13.12 msec,
Mean Gflops (using mean Time) = 371.85

MKL AO Cholesky: for 4 iterations (ignoring first),
mean Time = 447.32 msec, stdDev Time = 1.00 msec,
Mean Gflops (using meanTime) = 381.53

matrix is in Col major format
Using the host CPU for compute.. and
Using 1 MIC cards for compute..
number of streams used on host = 3
number of streams used on mic = 3
OMP: Warning #213: KMP_AFFINITY must be set prior to first parallel region or certain API calls; ignored.

Iteration = 0
time for Tiled hstreams Cholesky for iteration 0 = 762.33 msec
time for MKL Cholesky (AO) for iteration 0 = 450.28 msec
Tiled Cholesky successful

Iteration = 1
time for Tiled hstreams Cholesky for iteration 1 = 306.12 msec
time for MKL Cholesky (AO) for iteration 1 = 448.82 msec
Tiled Cholesky successful

Iteration = 2
time for Tiled hstreams Cholesky for iteration 2 = 296.57 msec
time for MKL Cholesky (AO) for iteration 2 = 455.96 msec
Tiled Cholesky successful

Iteration = 3
time for Tiled hstreams Cholesky for iteration 3 = 312.85 msec
time for MKL Cholesky (AO) for iteration 3 = 478.48 msec
Tiled Cholesky successful

Iteration = 4
time for Tiled hstreams Cholesky for iteration 4 = 300.15 msec
time for MKL Cholesky (AO) for iteration 4 = 451.45 msec
Tiled Cholesky successful

Matrix size = 8000
Tiled hStreams Cholesky: for 4 iterations (ignoring first),
mean Time = 303.92 msec, stdDev Time = 7.14 msec,
Mean Gflops (using mean Time) = 561.54

MKL AO Cholesky: for 4 iterations (ignoring first),
mean Time = 458.68 msec, stdDev Time = 13.52 msec,
Mean Gflops (using meanTime) = 372.08


