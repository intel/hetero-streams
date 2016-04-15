#                                                                      #
# Copyright 2014-2016 Intel Corporation.                               #
#                                                                      #
# This file is subject to the Intel Sample Source Code License. A copy #
# of the Intel Sample Source Code License is included.                 #
#                                                                      #
export NAME=`cat rootname.mk`
. $TUT_INSTALL/ref_code/common/setEnv.sh
pushd $TUT_INSTALL/bin/host > /dev/null

# some or all of these arguments may be missing, and will be ignored
./$NAME"_solution" 1.5 $2 $3 $4 $5 $6 $7 $8

popd > /dev/null
