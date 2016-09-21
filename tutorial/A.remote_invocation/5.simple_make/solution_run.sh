#                                                                      #
# Copyright 2014-2016 Intel Corporation.                               #
#                                                                      #
# This file is subject to the Intel Sample Source Code License. A copy #
# of the Intel Sample Source Code License is included.                 #
#                                                                      #

# some or all of these arguments may be missing, and will be ignored

mpss_lib_dir="/opt/mpss/$(rpm --qf %{VERSION} -q mpss-daemon)/sysroots/k1om-mpss-linux/usr/lib64/"


#!! b Shows how to run the executable while properly setting up the lib paths,
# in the simplest way.
# For KNC card you should:
#  export SINK_LD_LIBRARY_PATH=$mpss_lib_dir:$MIC_LD_LIBRARY_PATH:$(pwd)
# For x200 card you should:
#  export SINK_LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(pwd)

export HOST_SINK_LD_LIBRARY_PATH=`pwd`:$LD_LIBRARY_PATH

#Export SINK_LD_LIBRARY_PATH dependably on card type (x100 or x200)
if [ "$(cat /sys/class/mic/mic0/family)" == 'x100' ]; then
export SINK_LD_LIBRARY_PATH=$mpss_lib_dir:$MIC_LD_LIBRARY_PATH:$(pwd)
elif [ "$(cat /sys/class/mic/mic0/family)" == 'x200' ]; then
export SINK_LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(pwd)
elif [ "$(cat /sys/class/mic/mic0/info/mic_family)" == 'x200' ]; then
export SINK_LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(pwd)
else
export SINK_LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(pwd)
fi

./hello_hStreams_world_solution $1 $2 $3 $4 $5 $6 $7 $8

