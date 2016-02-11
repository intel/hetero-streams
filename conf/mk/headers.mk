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

HEADERS_TARGET_DIR:=/usr/include/hStreams/
HEADERS=$(wildcard $(EXT_HEADERS_DIR)*.h)

.PHONY: install-headers
install-headers:
	install -d $(DESTDIR)$(HEADERS_TARGET_DIR)
	install -m644 $(HEADERS) $(DESTDIR)$(HEADERS_TARGET_DIR)
