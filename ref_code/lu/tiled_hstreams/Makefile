#                                                                      #
# Copyright 2014-2016 Intel Corporation.                               #
#                                                                      #
# This file is subject to the Intel Sample Source Code License. A copy #
# of the Intel Sample Source Code License is included.                 #
#                                                                      #

TOP_DIR:=$(dir $(abspath $(lastword $(MAKEFILE_LIST))))
REFCODE_DIR:=$(realpath $(TOP_DIR)../../)/
include $(REFCODE_DIR)common/toolchain.mk

LU_SOURCE_TARGET := $(BIN_HOST)lu_tiled_hstreams
LU_x100_SINK_TARGET := $(BIN_x100)lu_sink_1.so

ADDITIONAL_SOURCE_CXXFLAGS := -mkl -qopenmp
ADDITIONAL_SOURCE_LDFLAGS  := -mkl -lhstreams_source -qopenmp

ADDITIONAL_x100_SINK_CXXFLAGS := -mkl -qopenmp
ADDITIONAL_x100_SINK_LDFLAGS  := -mkl -shared -Wl,-soname,lu_sink_1.so -qopenmp

LU_SOURCE_SRCS    = $(TOP_DIR)lu_tile_hstreams.cpp $(REFCODE_DIR)common/dtime.cpp $(REFCODE_DIR)common/hStreams_custom.cpp $(REFCODE_DIR)common/matrices_generator.cpp
LU_x100_SINK_SRCS = $(REFCODE_DIR)common/hStreams_custom_kernels_sink_host.cpp

LU_SOURCE_OBJS    = $(LU_SOURCE_SRCS:.cpp=.$(SOURCE_TAG).o)
LU_x100_SINK_OBJS = $(LU_x100_SINK_SRCS:.cpp=.$(x100_SINK_TAG).o)

# The default "all" target - builds everything
all: $(LU_SOURCE_TARGET) $(LU_x100_SINK_TARGET)

# If you're curious about the syntax below, please see 4.12.1 Syntax of Static Pattern Rules
# https://www.gnu.org/software/make/manual/html_node/Static-Usage.html#Static-Usage
$(LU_SOURCE_OBJS): %.$(SOURCE_TAG).o: %.cpp
	$(dir_create)
	$(SOURCE_CXX) -c $^ -o $@ $(SOURCE_CXXFLAGS) $(ADDITIONAL_SOURCE_CXXFLAGS)

# If you're curious about the syntax below, please see 4.12.1 Syntax of Static Pattern Rules
# https://www.gnu.org/software/make/manual/html_node/Static-Usage.html#Static-Usage
$(LU_x100_SINK_OBJS): %.$(x100_SINK_TAG).o: %.cpp
	$(dir_create)
	$(x100_SINK_CXX) -c $^ -o $@ $(x100_SINK_CXXFLAGS) $(ADDITIONAL_x100_SINK_CXXFLAGS)

$(LU_SOURCE_TARGET): $(LU_SOURCE_OBJS)
	$(dir_create)
	$(SOURCE_CXX) $^ -o $@ $(SOURCE_LDFLAGS) $(ADDITIONAL_SOURCE_LDFLAGS)

$(LU_x100_SINK_TARGET): $(LU_x100_SINK_OBJS)
	$(dir_create)
	$(x100_SINK_CXX) $^ -o $@ $(x100_SINK_LDFLAGS) $(ADDITIONAL_x100_SINK_LDFLAGS)

.PHONY: clean
clean:
	$(RM_rf) $(LU_SOURCE_TARGET) $(LU_SOURCE_OBJS) $(LU_x100_SINK_TARGET) $(LU_x100_SINK_OBJS)
