#                                                                      #
# Copyright 2014-2016 Intel Corporation.                               #
#                                                                      #
# This file is subject to the Intel Sample Source Code License. A copy #
# of the Intel Sample Source Code License is included.                 #
#                                                                      #
. ../common/setEnv.sh
pushd ../../bin/host > /dev/null

./mat_mult -c -b500 -m1000 -k1500 -n2000 -i4

popd > /dev/null
