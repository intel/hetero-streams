#                                                                      #
# Copyright 2014-2016 Intel Corporation.                               #
#                                                                      #
# This file is subject to the Intel Sample Source Code License. A copy #
# of the Intel Sample Source Code License is included.                 #
#                                                                      #

TOP_DIR:=$(dir $(abspath $(lastword $(MAKEFILE_LIST))))
REFCODE_DIR:=$(realpath $(TOP_DIR)../)/
include $(REFCODE_DIR)common/toolchain.mk

IO_PERF_TARGET := $(BIN_HOST)io_perf

ADDITIONAL_SOURCE_CXXFLAGS := -openmp
ADDITIONAL_SOURCE_LDFLAGS  := -lhstreams_source -openmp

IO_PERF_SOURCE_SRCS := $(TOP_DIR)io_perf.cpp $(REFCODE_DIR)common/dtime.cpp
IO_PERF_SOURCE_OBJS := $(IO_PERF_SOURCE_SRCS:.cpp=.$(SOURCE_TAG).o)

# The default "all" target - builds everything
all: $(IO_PERF_TARGET)

# If you're curious about the syntax below, please see 4.12.1 Syntax of Static Pattern Rules
# https://www.gnu.org/software/make/manual/html_node/Static-Usage.html#Static-Usage
$(IO_PERF_SOURCE_OBJS): %.$(SOURCE_TAG).o: %.cpp
	$(dir_create)
	$(SOURCE_CXX) -c $^ -o $@ $(SOURCE_CXXFLAGS) $(ADDITIONAL_SOURCE_CXXFLAGS)

$(IO_PERF_TARGET): $(IO_PERF_SOURCE_OBJS)
	$(dir_create)
	$(SOURCE_CXX) $^ -o $@ $(SOURCE_LDFLAGS) $(ADDITIONAL_SOURCE_LDFLAGS)

.PHONY: clean
clean:
	$(RM_rf) $(IO_PERF_TARGET) $(IO_PERF_SOURCE_OBJS)

