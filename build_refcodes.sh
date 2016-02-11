#!/bin/bash

#                                                                            #
# Hetero Streams Library - A streaming library for heterogeneous platforms   #
# Copyright (c) 2014 - 2016, Intel Corporation.                              #
#                                                                            #
# This program is free software; you can redistribute it and/or modify it    #
# under the terms and conditions of the GNU Lesser General Public License,   #
# version 2.1, as published by the Free Software Foundation.                 #
#                                                                            #
# This program is distributed in the hope it will be useful, but WITHOUT ANY #
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS  #
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for   #
# more details.                                                              #
#                                                                            #

TOPDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
REF_CODES=( basic_perf                     \
    cholesky/tiled_host                    \
    cholesky/tiled_hstreams                \
    cholesky/tiled_hstreams_host_multicard \
    io_perf                                \
    lu/tiled_host                          \
    lu/tiled_hstreams                      \
    matMult                                \
    matMult_host_multicard )
for ref_code in "${REF_CODES[@]}"
do
    echo "************************************************************************"
    echo "************************************************************************"
    echo "    Building $ref_code."
    echo "************************************************************************"
    echo "************************************************************************"
    pushd $TOPDIR/ref_code/$ref_code
    make -j 5
    if [ $? -ne 0 ];
    then
        echo "########################################################################"
        echo "########################################################################"
        echo "    Building $ref_code FAILED!"
        echo "########################################################################"
        echo "########################################################################"
        exit $?
    fi
    popd
done

