#                                                                      #
# Copyright 2014-2016 Intel Corporation.                               #
#                                                                      #
# This file is subject to the Intel Sample Source Code License. A copy #
# of the Intel Sample Source Code License is included.                 #
#                                                                      #

_scriptpath="$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )"
_host_bin_dir="$( cd "$_scriptpath"/../../bin/host ; pwd -P)"
_dev_bin_dir="$( cd "$_scriptpath"/../../bin/dev ; pwd -P)"
_mpss_lib_dir="/opt/mpss/$(rpm --qf %{VERSION} -q mpss-daemon)/sysroots/k1om-mpss-linux/usr/lib64/"

export SINK_LD_LIBRARY_PATH=$_mpss_lib_dir:$_dev_bin_dir:$MIC_LD_LIBRARY_PATH
export HOST_SINK_LD_LIBRARY_PATH=$_host_bin_dir:$LD_LIBRARY_PATH

# Make this 4G for an 8G card
export MKL_MIC_MAX_MEMORY=8G
export MIC_USE_2MB_BUFFERS=64K
