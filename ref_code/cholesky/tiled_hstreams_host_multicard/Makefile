#                                                                      #
# Copyright 2014-2016 Intel Corporation.                               #
#                                                                      #
# This file is subject to the Intel Sample Source Code License. A copy #
# of the Intel Sample Source Code License is included.                 #
#                                                                      #

TOP_DIR:=$(dir $(abspath $(lastword $(MAKEFILE_LIST))))
REFCODE_DIR:=$(realpath $(TOP_DIR)../../)/
include $(REFCODE_DIR)common/toolchain.mk

CHOLESKY_SOURCE_TARGET := $(BIN_HOST)cholesky_tiled_hstreams_host_multicard
CHOLESKY_x100_SINK_TARGET := $(BIN_x100)cholesky_sink_1.so
CHOLESKY_x200_SINK_TARGET := $(BIN_x200)cholesky_sink_2.so
CHOLESKY_HOST_SINK_TARGET := $(BIN_HOST)cholesky_tiled_hstreams_host_multicard_host.so

ADDITIONAL_SOURCE_CXXFLAGS := -mkl -qopenmp
ADDITIONAL_SOURCE_LDFLAGS  := -mkl -lhstreams_source -qopenmp

ADDITIONAL_x100_SINK_CXXFLAGS := -mkl -qopenmp
ADDITIONAL_x100_SINK_LDFLAGS  := -mkl -shared -Wl,-soname,cholesky_sink_1.so -qopenmp

ADDITIONAL_x200_SINK_CXXFLAGS := -mkl -qopenmp
ADDITIONAL_x200_SINK_LDFLAGS  := -mkl -shared -Wl,-soname,cholesky_sink_2.so -qopenmp

ADDITIONAL_HOST_SINK_CXXFLAGS := -mkl -qopenmp
ADDITIONAL_HOST_SINK_LDFLAGS  := -mkl -shared -Wl,-soname,cholesky_tiled_hstreams_host_multicard_host.so -qopenmp

CHOLESKY_SOURCE_SRCS    = $(TOP_DIR)cholesky_tiled_hstreams_host_multicard.cpp $(REFCODE_DIR)common/dtime.cpp $(REFCODE_DIR)common/hStreams_custom.cpp $(REFCODE_DIR)common/matrices_generator.cpp
CHOLESKY_x100_SINK_SRCS = $(REFCODE_DIR)common/hStreams_custom_kernels_sink.cpp
CHOLESKY_x200_SINK_SRCS = $(REFCODE_DIR)common/hStreams_custom_kernels_sink.cpp
CHOLESKY_HOST_SINK_SRCS = $(REFCODE_DIR)common/hStreams_custom_kernels_sink_host.cpp

CHOLESKY_SOURCE_OBJS    = $(CHOLESKY_SOURCE_SRCS:.cpp=.$(SOURCE_TAG).o)
CHOLESKY_x100_SINK_OBJS = $(CHOLESKY_x100_SINK_SRCS:.cpp=.$(x100_SINK_TAG).o)
CHOLESKY_x200_SINK_OBJS = $(CHOLESKY_x200_SINK_SRCS:.cpp=.$(x200_SINK_TAG).o)
CHOLESKY_HOST_SINK_OBJS = $(CHOLESKY_HOST_SINK_SRCS:.cpp=.$(HOST_SINK_TAG).o)

# The default "all" target - builds everything
ifeq "$(TARGET)" "knc"
all: $(CHOLESKY_SOURCE_TARGET) $(CHOLESKY_HOST_SINK_TARGET) $(CHOLESKY_x100_SINK_TARGET)
else ifeq "$(TARGET)" "x200"
all: $(CHOLESKY_SOURCE_TARGET) $(CHOLESKY_HOST_SINK_TARGET) $(CHOLESKY_x200_SINK_TARGET)
endif

# If you're curious about the syntax below, please see 4.12.1 Syntax of Static Pattern Rules
# https://www.gnu.org/software/make/manual/html_node/Static-Usage.html#Static-Usage
$(CHOLESKY_SOURCE_OBJS): %.$(SOURCE_TAG).o: %.cpp
	$(dir_create)
	$(SOURCE_CXX) -c $^ -o $@ $(SOURCE_CXXFLAGS) $(ADDITIONAL_SOURCE_CXXFLAGS)

# If you're curious about the syntax below, please see 4.12.1 Syntax of Static Pattern Rules
# https://www.gnu.org/software/make/manual/html_node/Static-Usage.html#Static-Usage
$(CHOLESKY_HOST_SINK_OBJS): %.$(HOST_SINK_TAG).o: %.cpp
	$(dir_create)
	$(HOST_SINK_CXX) -c $^ -o $@ $(HOST_SINK_CXXFLAGS) $(ADDITIONAL_HOST_SINK_CXXFLAGS)

# If you're curious about the syntax below, please see 4.12.1 Syntax of Static Pattern Rules
# https://www.gnu.org/software/make/manual/html_node/Static-Usage.html#Static-Usage
$(CHOLESKY_x100_SINK_OBJS): %.$(x100_SINK_TAG).o: %.cpp
	$(dir_create)
	$(x100_SINK_CXX) -c $^ -o $@ $(x100_SINK_CXXFLAGS) $(ADDITIONAL_x100_SINK_CXXFLAGS)

# If you're curious about the syntax below, please see 4.12.1 Syntax of Static Pattern Rules
# https://www.gnu.org/software/make/manual/html_node/Static-Usage.html#Static-Usage
$(CHOLESKY_x200_SINK_OBJS): %.$(x200_SINK_TAG).o: %.cpp
	$(dir_create)
	$(x200_SINK_CXX) -c $^ -o $@ $(x200_SINK_CXXFLAGS) $(ADDITIONAL_x200_SINK_CXXFLAGS)

$(CHOLESKY_SOURCE_TARGET): $(CHOLESKY_SOURCE_OBJS)
	$(dir_create)
	$(SOURCE_CXX) $^ -o $@ $(SOURCE_LDFLAGS) $(ADDITIONAL_SOURCE_LDFLAGS)

$(CHOLESKY_x100_SINK_TARGET): $(CHOLESKY_x100_SINK_OBJS)
	$(dir_create)
	$(x100_SINK_CXX) $^ -o $@ $(x100_SINK_LDFLAGS) $(ADDITIONAL_x100_SINK_LDFLAGS)

$(CHOLESKY_x200_SINK_TARGET): $(CHOLESKY_x200_SINK_OBJS)
	$(dir_create)
	$(x200_SINK_CXX) $^ -o $@ $(x200_SINK_LDFLAGS) $(ADDITIONAL_x200_SINK_LDFLAGS)

$(CHOLESKY_HOST_SINK_TARGET): $(CHOLESKY_HOST_SINK_OBJS)
	$(dir_create)
	$(HOST_SINK_CXX) $^ -o $@ $(HOST_SINK_LDFLAGS) $(ADDITIONAL_HOST_SINK_LDFLAGS)

.PHONY: clean
clean:
	$(RM_rf) $(CHOLESKY_SOURCE_TARGET) $(CHOLESKY_SOURCE_OBJS) $(CHOLESKY_HOST_SINK_TARGET) $(CHOLESKY_HOST_SINK_OBJS) $(CHOLESKY_x100_SINK_TARGET) $(CHOLESKY_x100_SINK_OBJS) $(CHOLESKY_x200_SINK_TARGET) $(CHOLESKY_x200_SINK_OBJS)
