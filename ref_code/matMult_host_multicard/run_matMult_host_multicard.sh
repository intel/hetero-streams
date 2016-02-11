#                                                                      #
# Copyright 2014-2016 Intel Corporation.                               #
#                                                                      #
# This file is subject to the Intel Sample Source Code License. A copy #
# of the Intel Sample Source Code License is included.                 #
#                                                                      #

source ../common/setEnv.sh
pushd ../../bin/host > /dev/null

#There are a number of switches available for controlling the executable:
# -b<N>: block size, in elements
# -m<N>: matrix size, in elements, for M dimension
# -n<N>: matrix size, in elements, for N dimension
# -k<N>: matrix size, in elements, for K dimension
# -i<N>: iterations (default 3)
# -s<N>: Number of streams - only use if computing on only host or only MICs
# -h<N>: 1 to use host, else 0
# -c<N>: number of MIC cards (<=2, must be present)
# -l<N>: 1 to do load balancing across MICs and host, for when host slower than MIC,
#   e.g. before the Haswell generation.
#   This is set by default if cards > 0, host is selected; can be overridden explicitly
# -v: to make verbose


#./mat_mult_host_multicard  -b1000 -m3000 -k3000 -n3000 -i5 -s3 -h1 -c0 -v

# target only 1 card
#./mat_mult_host_multicard  -b1000 -m3000 -k3000 -n3000 -i5 -s3 -h0 -c1 -v

# target 2 cards
# tiling must be 2 x 2
#./mat_mult_host_multicard  -b1500 -m3000 -k3000 -n3000 -i5 -s2 -h0 -c2 -v

# target host + 1 card
# Note if load balancing is enabled, tiling must be 3 x 3
# else, tiling must be 2 x 2
# Load balancing on
./mat_mult_host_multicard  -b1000 -m3000 -k3000 -n3000 -i5 -h1 -c1 -l1
# Load balancing off
#./mat_mult_host_multicard  -b1500 -m3000 -k3000 -n3000 -i5 -h1 -c1 -v -l0

# target host + 2 cards
# Note if load balancing is enabled, tiling must be 5 x 5
# else, tiling must be 3 x 3
# Load balancing on
#./mat_mult_host_multicard  -b600 -m3000 -k3000 -n3000 -i5 -h1 -c2 -v -l1
# Load balancing off
#./mat_mult_host_multicard  -b1000 -m3000 -k3000 -n3000 -i5 -h1 -c2 -v -l0

popd > /dev/null
