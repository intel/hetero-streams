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

export HOST_SINK_LD_LIBRARY_PATH=`pwd`:$LD_LIBRARY_PATH
export SINK_LD_LIBRARY_PATH=$mpss_lib_dir:$MIC_LD_LIBRARY_PATH:$(pwd)

./hello_hStreams_world_solution $1 $2 $3 $4 $5 $6 $7 $8

