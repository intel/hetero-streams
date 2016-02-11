#                                                                      #
# Copyright 2014-2016 Intel Corporation.                               #
#                                                                      #
# This file is subject to the Intel Sample Source Code License. A copy #
# of the Intel Sample Source Code License is included.                 #
#                                                                      #

pushd ../../../bin/host > /dev/null

#scatter affinity set for host
export KMP_AFFINITY=scatter

./cholesky_tiled_host -m 4800 -t 6 -l col -i 5
./cholesky_tiled_host -m 4800 -t 6 -l row -i 5

popd > /dev/null