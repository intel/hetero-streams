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

# compilation switches and stuff
DEBUG_FLAGS:=-g -O0 -D_DEBUG -DHSTREAMS_BACKTRACE
RELEASE_FLAGS:=-O3 -ggdb

STRICT_COMPILATION_FLAGS:=-pedantic-errors -Wextra -Wall -Winit-self -Woverloaded-virtual -Wuninitialized -Werror -D_FORTIFY_SOURCE=2

ifndef CFG
  CFG=RELEASE
endif

ifeq (RELEASE,$(CFG))
  CONFIGURATION_FLAGS = $(RELEASE_FLAGS)
else ifeq (DEBUG,$(CFG))
  CONFIGURATION_FLAGS = $(DEBUG_FLAGS)
else
  $(error Unknown configuration: $(CFG))
endif

mpss_toolchains := /opt/mpss_toolchains

ifeq ($(COVERAGE),y)
    bullseye_coverage := $(mpss_toolchains)/coverage/bullseye
    bullseye_path := $(bullseye_coverage)/bin
endif

#
# compiler
ifdef ICC
  $(info ICC is defined, will use that.)
  HOST_CC := $(ICC) -std=c++11
  x100_CARD_CC := $(ICC) -mmic
  x200_CARD_CC := $(ICC) -xMIC-AVX512
else
  HOST_CC := icc -std=c++11
  x100_CARD_CC := icc -mmic
  x200_CARD_CC := icc -xMIC-AVX512
endif

# dir creation guard, to prevent explicitly adding directories as targets
# put $(dir_create) as a first command in the target definition
dir_create=@mkdir -p $(@D)

RM_rf:=rm -rf

# Compilation paths. We prefer an out-of-source build. Currently, we assume that the
# user is running "make" from the root directory of the project. In order to be able
# to relax that requirement, it is important that all targets define their sources/
# objects/targets through the following variables.

# Here's where the sources reside.
# We actually don't expand those to absolute paths as we use __FILE__ in our
# exception handling and absolute paths look terrible in the logs.
SRC_DIR:=src/
# Here's where the external headers reside.
EXT_HEADERS_DIR:=$(TOP_DIR)include/
# Where to put intermediate build artifacts
BLD_DIR:=$(TOP_DIR)build/
# Where to put final build artifacts
BIN_DIR:=$(TOP_DIR)bin/
CFG_DIR:=$(TOP_DIR)conf/
CFG_SPEC_DIR:=$(CFG_DIR)spec/
DOC_DIR:=$(TOP_DIR)doc/
REF_CODE_DIR:=$(TOP_DIR)ref_code/
TUT_CODE_DIR:=$(TOP_DIR)tutorial/
SH_DIR:=$(TOP_DIR)conf/sh/
PY_DIR:=$(TOP_DIR)conf/py/

INCBIN:=$(PY_DIR)bin2inc.py

K1OM_STRIP:=/usr/linux-k1om-4.7/bin/x86_64-k1om-linux-strip

HOST_BUILD_DIR:=build/host/
HOST_BIN_DIR:=bin/host/

# astyle
ASTYLE_DIR:=$(mpss_toolchains)/astyle
ASTYLE_VER:=2.05.1
ASTYLE_OPTIONS_FILE:=$(CFG_DIR)astyle.rc
ASTYLE_OPTIONS:=--options=$(ASTYLE_OPTIONS_FILE) -n --recursive "$(TOP_DIR)*.h" "$(TOP_DIR)*.cpp"

ASTYLE:=$(ASTYLE_DIR)/$(ASTYLE_VER)/astyle
ifeq ("$(wildcard $(ASTYLE))","")
$(info [INFO] Running with system-installed astyle)
ASTYLE:=astyle
endif

DOXYGEN_DIR=$(mpss_toolchains)/doxygen
DOXYGEN_VER=1.6.3
DOXYGEN:=$(DOXYGEN_DIR)/$(DOXYGEN_VER)/bin/doxygen
ifeq ("$(wildcard $(DOXYGEN))","")
$(info [INFO] Running with system-installed doxygen)
DOXYGEN:=doxygen
SYSTEM_DOXYGEN_VER:= $(strip $(shell $(DOXYGEN) --version))
ifneq ($(SYSTEM_DOXYGEN_VER), $(DOXYGEN_VER))
$(warning [WARN] Please be advised that the doxygen version that will be used is $(SYSTEM_DOXYGEN_VER).)
$(warning [WARN] It is recommended to use doxygen $(DOXYGEN_VER) for generating the documentation.)
endif
endif


# funnily enough, we install the payloads and card-side .so to regular
# /usr/lib64 since that will be prefixed through DESTDIR to point to
# k1om sysroot.
CARD_LIBRARY_INSTALL_DIR:=/usr/lib64/
CARD_PAYLOAD_INSTALL_DIR:=/usr/lib64/

x100_CARD_PAYLOAD_NAME:=hstreams_start_x100_card

x200_CARD_PAYLOAD_NAME:=hstreams_start_x200_card

# RPM SPEC file
SPEC_FILE:=$(CFG_SPEC_DIR)hstreams.spec


MANIFEST_FILE:=$(TOP_DIR)MANIFEST

ALL_FILES := $(shell cat $(MANIFEST_FILE))

ifdef OUTPUT_DIR
RPM_TOPDIR:=$(abspath $(OUTPUT_DIR))/hstreams/linux-rpm/
$(info [INFO] Running with OUTPUT_DIR=$(OUTPUT_DIR))
$(info [INFO] RPM's TOPDIR is $(RPM_TOPDIR))
else # def OUTPUT_DIR
RPM_TOPDIR:=$(TOP_DIR)rpmbuild/
endif # def OUTPUT_DIR

RPM_SOURCESDIR=$(RPM_TOPDIR)SOURCES/
RPM_BUILDROOT_SUBDIRS:= $(RPM_TOPDIR)BUILD/ \
  $(RPM_TOPDIR)RPMS/ \
  $(RPM_SOURCESDIR) \
  $(RPM_TOPDIR)SPECS/ \
  $(RPM_TOPDIR)SRPMS/

TARGZ_FILE:=$(RPM_SOURCESDIR)$(name)-$(version).tar.gz
