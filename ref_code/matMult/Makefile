#                                                                      #
# Copyright 2014-2016 Intel Corporation.                               #
#                                                                      #
# This file is subject to the Intel Sample Source Code License. A copy #
# of the Intel Sample Source Code License is included.                 #
#                                                                      #

TOP_DIR:=$(dir $(abspath $(lastword $(MAKEFILE_LIST))))
REFCODE_DIR:=$(realpath $(TOP_DIR)../)/
include $(REFCODE_DIR)common/toolchain.mk

MATMULT_TARGET := $(BIN_HOST)mat_mult

ADDITIONAL_SOURCE_CXXFLAGS := -mkl -DFP_DATA_TYPE=double
ADDITIONAL_SOURCE_LDFLAGS  := -mkl -lhstreams_source

MATMULT_SOURCE_SRCS := $(TOP_DIR)matMultapi.cpp $(REFCODE_DIR)common/dtime.cpp
MATMULT_SOURCE_OBJS := $(MATMULT_SOURCE_SRCS:.cpp=.$(SOURCE_TAG).o)

# The default "all" target - builds everything
all: $(MATMULT_TARGET)

# If you're curious about the syntax below, please see 4.12.1 Syntax of Static Pattern Rules
# https://www.gnu.org/software/make/manual/html_node/Static-Usage.html#Static-Usage
$(MATMULT_SOURCE_OBJS): %.$(SOURCE_TAG).o: %.cpp
	$(dir_create)
	$(SOURCE_CXX) -c $^ -o $@ $(SOURCE_CXXFLAGS) $(ADDITIONAL_SOURCE_CXXFLAGS)

$(MATMULT_TARGET): $(MATMULT_SOURCE_OBJS)
	$(dir_create)
	$(SOURCE_CXX) $^ -o $@ $(SOURCE_LDFLAGS) $(ADDITIONAL_SOURCE_LDFLAGS)

.PHONY: clean
clean:
	$(RM_rf) $(MATMULT_TARGET) $(MATMULT_SOURCE_OBJS)
