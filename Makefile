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

name    := hstreams
version_major := 1
version_minor := 0
version_micro := 0
ifdef BUILDNO
version_build := $(BUILDNO)
else
version_build := DEVBOX
endif

version := $(version_major).$(version_minor).$(version_micro).$(version_build)

# To not build the documentation, pass NODOC=1 to the make line
ALL_TARGET := build-host build-x100-card
ifneq "$(NODOC)" "1"
ALL_TARGET += build-doc
endif

.PHONY: all
all: $(ALL_TARGET)
.PHONY: install
install: install-x100-card

# Here's the root directory
TOP_DIR:=$(dir $(abspath $(lastword $(MAKEFILE_LIST))))

include $(TOP_DIR)conf/mk/toolchain.mk
include $(TOP_DIR)conf/mk/x100-card.mk
include $(TOP_DIR)conf/mk/host.mk
include $(TOP_DIR)conf/mk/headers.mk
include $(TOP_DIR)conf/mk/doc.mk
include $(TOP_DIR)conf/mk/artifacts.mk

# Check formatting (only if we're not doing formatting right now). Also do not check
# if prepping for rpm creation
ifeq "$(or $(findstring $(MAKECMDGOALS),format),$(findstring $(MAKECMDGOALS),artifacts))" ""
    ifneq ($(strip $(shell $(ASTYLE) $(ASTYLE_OPTIONS) --dry-run | grep ^Formatted)),)
        $(warning [WARN] The code is not formatted properly. Please run make format)
    endif # ifneq ($(strip $(shell $(ASTYLE) $(ASTYLE_OPTIONS) --dry-run | grep ^Formatted)),)
endif

format:
	$(ASTYLE) $(ASTYLE_OPTIONS)

.PHONY: install-x100-card
install-x100-card: install-host install-x100-card install-headers install-doc

.PHONY: clean
clean: clean-host clean-x100-card clean-doc
