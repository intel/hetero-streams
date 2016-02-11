#                                                                      #
# Copyright 2014-2016 Intel Corporation.                               #
#                                                                      #
# This file is subject to the Intel Sample Source Code License. A copy #
# of the Intel Sample Source Code License is included.                 #
#                                                                      #

# We use "in_repo" to denote that we're building against local sources from the source
# code repository. If you want to build the reference code against the headers and library
# from the source code repo, do a
#
#     make in_repo=1
#
ifneq ($(in_repo),1)
$(info [INFO] Building against system-wide Hetero Streams Library.)
$(info [INFO] In order to build against the library and headers from the repository, append in_repo=1 to make arguments)
in_repo=0
else # in_repo == 1
# let's make sure there's something to run against
ifeq ("$(wildcard $(REFCODE_DIR)../bin/host/libhstreams_source.so)","")
$(warning [ERROR] Build against in-repo Hetero Streams Library requested but the library is not present.)
$(warning [ERROR] Have you built the library?)
$(error [ERROR] Aborting.)
endif # libhstreams_source.so exists
$(info [INFO] Building against repository-local Hetero Streams Library)
endif # libhstreams_source.so exists

# Two configurations can be built: DEBUG or RELEASE.
# By default, RELEASE config is built.
# To create DEBUG config, make CFG=DEBUG
DEBUG_FLAGS = -g -O0 -D_DEBUG
RELEASE_FLAGS = -DNDEBUG -O3

COMMON_CXXFLAGS := -Wall -Werror-all -fPIC
ifeq ($(CFG),DEBUG)
$(info [INFO] Building debug version.)
COMMON_CXXFLAGS += $(DEBUG_FLAGS)
else
$(info [INFO] Building release version. In order to build debug version, append CFG=DEBUG to make arguments)
COMMON_CXXFLAGS += $(RELEASE_FLAGS)
endif
# Disabling some vectorization-related diagnostics as ComposerXE 2015 would
# abort compilation upon failing to vectorize loops which include malloc() if
# -O3 or -O2 is used
COMMON_CXXFLAGS += -diag-disable 13368
COMMON_CXXFLAGS += -diag-disable 15527
COMMON_CXXFLAGS += -I$(realpath $(REFCODE_DIR)common)

ifneq ($(in_repo),1)
COMMON_CXXFLAGS += -I/usr/include/hStreams
else # in_repo == 1
# includes
__local_inc_dir := $(realpath $(REFCODE_DIR)../include)
LOCAL_SOURCE_CXXFLAGS    := -I$(__local_inc_dir)/
LOCAL_x100_SINK_CXXFLAGS := -I$(__local_inc_dir)/
LOCAL_HOST_SINK_CXXFLAGS := -I$(__local_inc_dir)/
# libs
__local_lib_dir := $(realpath $(REFCODE_DIR)../bin/host)
LOCAL_SOURCE_LDFLAGS    := -L$(__local_lib_dir)/
endif # in_repo == 1

SOURCE_CXXFLAGS    := $(COMMON_CXXFLAGS) $(LOCAL_SOURCE_CXXFLAGS)
x100_SINK_CXXFLAGS := $(COMMON_CXXFLAGS) $(LOCAL_x100_SINK_CXXFLAGS)
HOST_SINK_CXXFLAGS := $(COMMON_CXXFLAGS) $(LOCAL_HOST_SINK_CXXFLAGS)

SOURCE_LDFLAGS    := $(COMMON_LDFLAGS) $(LOCAL_SOURCE_LDFLAGS)
x100_SINK_LDFLAGS := $(COMMON_LDFLAGS) $(LOCAL_x100_SINK_LDFLAGS)
HOST_SINK_LDFLAGS := $(COMMON_LDFLAGS) $(LOCAL_HOST_SINK_LDFLAGS)

# We use icpc for compilation
SOURCE_CXX    := icpc
HOST_SINK_CXX := icpc
x100_SINK_CXX := icpc -mmic

# Tags which to use for namng the intermediate object files Helps avoid name
# clashes among compilations for different targets from one source file.
SOURCE_TAG    := source
HOST_SINK_TAG := host-sink
x100_SINK_TAG := x100-sink

# dir creation guard, to prevent explicitly adding directories as targets
# put $(dir_create) as a first command in the target definition
dir_create=@mkdir -p $(@D)

RM_rf := rm -rf

BIN_HOST = $(REFCODE_DIR)../bin/host/
BIN_DEV  = $(REFCODE_DIR)../bin/dev/
