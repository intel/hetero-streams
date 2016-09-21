#                                                                      #
# Copyright 2014-2016 Intel Corporation.                               #
#                                                                      #
# This file is subject to the Intel Sample Source Code License. A copy #
# of the Intel Sample Source Code License is included.                 #
#                                                                      #

scriptpath="$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )"
host_bin_dir="$( cd "$scriptpath"/../../bin/host ; pwd -P)"
knc_bin_dir="$( cd "$scriptpath"/../../bin/x100-card ; pwd -P)"
knl_bin_dir="$( cd "$scriptpath"/../../bin/x200-card ; pwd -P)"
mpss_lib_dir="/opt/mpss/$(rpm --qf %{VERSION} -q mpss-daemon)/sysroots/k1om-mpss-linux/usr/lib64/"

#Export SINK_LD_LIBRARY_PATH dependably on card type (x100 or x200)
if [ "$(cat /sys/class/mic/mic0/family)" == 'x100' ]; then
export SINK_LD_LIBRARY_PATH=$mpss_lib_dir:$knc_bin_dir:$knl_bin_dir:$MIC_LD_LIBRARY_PATH
elif [ "$(cat /sys/class/mic/mic0/family)" == 'x200' ]; then
export SINK_LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$knl_bin_dir
elif [ "$(cat /sys/class/mic/mic0/info/mic_family)" == 'x200' ]; then
export SINK_LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$knl_bin_dir
else
export SINK_LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$knl_bin_dir
fi

export HOST_SINK_LD_LIBRARY_PATH=$host_bin_dir:$LD_LIBRARY_PATH

# Make this 4G for an 8G card
export MKL_MIC_MAX_MEMORY=8G
export MIC_USE_2MB_BUFFERS=64K
