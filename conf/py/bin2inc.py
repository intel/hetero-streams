#!/usr/bin/python

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

import sys

def main(argv):
    if len(argv) != 3:
        print "Usage: python bin2inc.py INPUT OUTPUT SYMBOL_NAME";
        exit(1);

    in_file = open(argv[0], "rb");
    out_file = open(argv[1], "w+");
    symbol_name = argv[2];

    out_file.write("#include <stdint.h>\n\n");

    out_file.write("extern const uint8_t ");
    out_file.write(symbol_name);
    out_file.write("[] = {\n    ");

    how_many_byte_read = 0;

    while 1:
        buf = in_file.read(1);
        if not buf:
            break;

        if how_many_byte_read != 0:
            out_file.write(", ");

        out_file.write("0x");

        out_file.write("{0:00x}".format(ord(buf[0])));


        how_many_byte_read += 1;

        if how_many_byte_read % 16 == 0:
            out_file.write("\n    ");



    out_file.write("\n};\n\n");
    out_file.write("extern const uint64_t ");
    out_file.write(symbol_name);
    out_file.write("_size = sizeof(");
    out_file.write(symbol_name);
    out_file.write(");\n");

if __name__ == "__main__":
    main(sys.argv[1:])
