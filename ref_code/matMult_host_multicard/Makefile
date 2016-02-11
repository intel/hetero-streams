#                                                                      #
# Copyright 2014-2016 Intel Corporation.                               #
#                                                                      #
# This file is subject to the Intel Sample Source Code License. A copy #
# of the Intel Sample Source Code License is included.                 #
#                                                                      #

TOP_DIR:=$(dir $(abspath $(lastword $(MAKEFILE_LIST))))
REFCODE_DIR:=$(realpath $(TOP_DIR)../)/
include $(REFCODE_DIR)common/toolchain.mk

MATMULT_SOURCE_TARGET := $(BIN_HOST)mat_mult_host_multicard
MATMULT_HOST_SINK_TARGET := $(BIN_HOST)mat_mult_host_multicard_host.so

ADDITIONAL_SOURCE_CXXFLAGS := -mkl -DFP_DATA_TYPE=double
ADDITIONAL_SOURCE_LDFLAGS  := -mkl -lhstreams_source

ADDITIONAL_HOST_SINK_CXXFLAGS := -mkl -DFP_DATA_TYPE=double -openmp
ADDITIONAL_HOST_SINK_LDFLAGS  := -mkl -shared -Wl,-soname,mat_mult_host_multicard_host.so -openmp

MATMULT_SOURCE_SRCS    = $(TOP_DIR)matMult_host_multicard.cpp $(REFCODE_DIR)common/dtime.cpp $(REFCODE_DIR)common/hStreams_custom.cpp
MATMULT_HOST_SINK_SRCS = $(REFCODE_DIR)common/hStreams_custom_kernels_sink_host.cpp

MATMULT_SOURCE_OBJS    = $(MATMULT_SOURCE_SRCS:.cpp=.$(SOURCE_TAG).o)
MATMULT_HOST_SINK_OBJS = $(MATMULT_HOST_SINK_SRCS:.cpp=.$(HOST_SINK_TAG).o)

# The default "all" target - builds everything
all: $(MATMULT_SOURCE_TARGET) $(MATMULT_HOST_SINK_TARGET)

# If you're curious about the syntax below, please see 4.12.1 Syntax of Static Pattern Rules
# https://www.gnu.org/software/make/manual/html_node/Static-Usage.html#Static-Usage
$(MATMULT_SOURCE_OBJS): %.$(SOURCE_TAG).o: %.cpp
	$(dir_create)
	$(SOURCE_CXX) -c $^ -o $@ $(SOURCE_CXXFLAGS) $(ADDITIONAL_SOURCE_CXXFLAGS)

# If you're curious about the syntax below, please see 4.12.1 Syntax of Static Pattern Rules
# https://www.gnu.org/software/make/manual/html_node/Static-Usage.html#Static-Usage
$(MATMULT_HOST_SINK_OBJS): %.$(HOST_SINK_TAG).o: %.cpp
	$(dir_create)
	$(HOST_SINK_CXX) -c $^ -o $@ $(HOST_SINK_CXXFLAGS) $(ADDITIONAL_HOST_SINK_CXXFLAGS)

$(MATMULT_SOURCE_TARGET): $(MATMULT_SOURCE_OBJS)
	$(dir_create)
	$(SOURCE_CXX) $^ -o $@ $(SOURCE_LDFLAGS) $(ADDITIONAL_SOURCE_LDFLAGS)

$(MATMULT_HOST_SINK_TARGET): $(MATMULT_HOST_SINK_OBJS)
	$(dir_create)
	$(HOST_SINK_CXX) $^ -o $@ $(HOST_SINK_LDFLAGS) $(ADDITIONAL_HOST_SINK_LDFLAGS)

.PHONY: clean
clean:
	$(RM_rf) $(MATMULT_SOURCE_TARGET) $(MATMULT_SOURCE_OBJS) $(MATMULT_HOST_SINK_TARGET) $(MATMULT_HOST_SINK_OBJS)
