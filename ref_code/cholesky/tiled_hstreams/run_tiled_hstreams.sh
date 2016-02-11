#                                                                      #
# Copyright 2014-2016 Intel Corporation.                               #
#                                                                      #
# This file is subject to the Intel Sample Source Code License. A copy #
# of the Intel Sample Source Code License is included.                 #
#                                                                      #
#scatter affinity set for host
export KMP_AFFINITY=scatter

#for MKL AO run
export MIC_ENV_PREFIX=MIC
export MIC_KMP_AFFINITY=balanced

source ../../common/setEnv.sh

pushd ../../../bin/host > /dev/null

#m - matrix size; t - no. of tiles; s - no. of streams (partitions on MIC); i - no. of iterations
#l - layout (row or ROW for rowmajor; else is colMajor)

./cholesky_tiled_hstreams -m 4800 -t 6 -s 5 -l col -i 5
./cholesky_tiled_hstreams -m 4800 -t 6 -s 5 -l row -i 5

popd > /dev/null
