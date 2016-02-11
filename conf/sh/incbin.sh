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

# This script will serialize an input file into a C++ array of uint8_t
# with the name SYMBOL_NAME. Additionally, the array's size will be
# made available through an uint64_t variable SYMBOL_NAME_size.
#
# The output of this script is a C++ file which can be compiled.

set -e

help () {
    echo Usage: ./incbin.sh INPUT OUTPUT.cpp SYMBOL_NAME
    exit 206
}

[ -z "$1" ] && help
[ -z "$2" ] && help
[ -z "$3" ] && help
serialized=$(od -An -t x1 -w1 -v "$1")

(
    echo "#include <stdint.h>"
    echo
    echo "extern const uint8_t $3[] = {"

    num_lines=$(echo "$serialized" | wc -l)
    idx=0
    count=0
    count_modulo=16
    for byte in $serialized ; do
        if [ $count -eq 0 ] ; then
            echo -n "    "  # Tabbing
        fi
        printf " 0x%s" "$byte"
        if [ $((idx+1)) -ne $num_lines ]; then
            echo -n ","          # Comma separation
        fi
        count=$((count+1))
        idx=$((idx+1))
        if [ $count -eq $count_modulo ] ; then
            echo
            count=0
        fi
    done
    echo
    echo "};"
    echo
    echo "extern const uint64_t $3_size = sizeof($3);"
    echo
) > "$2"
