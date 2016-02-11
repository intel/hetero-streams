#                                                                      #
# Copyright 2014-2016 Intel Corporation.                               #
#                                                                      #
# This file is subject to the Intel Sample Source Code License. A copy #
# of the Intel Sample Source Code License is included.                 #
#                                                                      #
#scatter affinity set for host
#export KMP_AFFINITY=compact,granularity=fine,1,0
export KMP_AFFINITY=scatter

#for MKL AO run
export MIC_ENV_PREFIX=MIC
export MIC_KMP_AFFINITY=balanced
export MKL_MIC_MAX_MEMORY=8G
export MIC_USE_2MB_BUFFERS=64K

source ../../common/setEnv.sh

pushd ../../../bin/host > /dev/null



#There are a number of switches available for controlling the executable:
# -m<N>: matrix size, in elements, for each dimension
# -t<N>: number of tiles on each side
# -s<N>: number of streams to use on MIC
# -l<S>: where layout S is ROW or row to use row major format, else col major
# -i<N>: iterations (default 5)
# -h<N>: 1 to use host, else 0
# -c<N>: number of MIC cards (<=2, must be present)
# -v<N>: 1 to verify

# target only host
./cholesky_tiled_hstreams_host_multicard -m 8000 -t 10 -s 3 -h 1 -c 0 -l col -i 5

# target only 1 card
./cholesky_tiled_hstreams_host_multicard -m 8000 -t 10 -s 3 -h 0 -c 1 -l col -i 5

# target 2 cards
#./cholesky_tiled_hstreams_host_multicard -m 8000 -t 10 -s 3 -h 0 -c 2 -l col -i 5

# target host + 1 card
./cholesky_tiled_hstreams_host_multicard -m 8000 -t 10 -s 3 -h 1 -c 1 -l col -i 5

# target host + 2 cards
#./cholesky_tiled_hstreams_host_multicard -m 8000 -t 10 -s 3 -h 1 -c 2 -l col -i 5

popd > /dev/null
