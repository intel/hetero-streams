#                                                                      #
# Copyright 2014-2016 Intel Corporation.                               #
#                                                                      #
# This file is subject to the Intel Sample Source Code License. A copy #
# of the Intel Sample Source Code License is included.                 #
#                                                                      #

TOP_DIR:=$(dir $(abspath $(lastword $(MAKEFILE_LIST))))
REFCODE_DIR:=$(realpath $(TOP_DIR)../)/
include $(REFCODE_DIR)common/toolchain.mk

TARGET := $(BIN_HOST)hello_world
# By default, the sink side library name is source side executable + _mic.so
x100_SINK_TARGET := $(BIN_x100)hello_world_mic.so

ADDITIONAL_SOURCE_CXXFLAGS :=
ADDITIONAL_SOURCE_LDFLAGS  := -lhstreams_source

ADDITIONAL_x100_SINK_CXXFLAGS :=
ADDITIONAL_x100_SINK_LDFLAGS  := -shared -Wl,-soname,hello_world_mic.so

SOURCE_SRCS := $(TOP_DIR)hello_world_src.cpp
SOURCE_OBJS := $(SOURCE_SRCS:.cpp=.$(SOURCE_TAG).o)

x100_SINK_SRCS = $(TOP_DIR)hello_world_sink.cpp
x100_SINK_OBJS = $(x100_SINK_SRCS:.cpp=.$(x100_SINK_TAG).o)

# The default "all" target - builds everything
all: $(TARGET) $(x100_SINK_TARGET)

# If you're curious about the syntax below, please see 4.12.1 Syntax of Static Pattern Rules
# https://www.gnu.org/software/make/manual/html_node/Static-Usage.html#Static-Usage
$(SOURCE_OBJS): %.$(SOURCE_TAG).o: %.cpp
	$(dir_create)
	$(SOURCE_CXX) -c $^ -o $@ $(SOURCE_CXXFLAGS) $(ADDITIONAL_SOURCE_CXXFLAGS)

$(x100_SINK_OBJS): %.$(x100_SINK_TAG).o: %.cpp
	$(dir_create)
	$(x100_SINK_CXX) -c $^ -o $@ $(x100_SINK_CXXFLAGS) $(ADDITIONAL_x100_SINK_CXXFLAGS)

$(x100_SINK_TARGET): $(x100_SINK_OBJS)
	$(dir_create)
	$(x100_SINK_CXX) $^ -o $@ $(x100_SINK_LDFLAGS) $(ADDITIONAL_x100_SINK_LDFLAGS)

$(TARGET): $(SOURCE_OBJS)
	$(dir_create)
	$(SOURCE_CXX) $^ -o $@ $(SOURCE_LDFLAGS) $(ADDITIONAL_SOURCE_LDFLAGS)

.PHONY: clean
clean:
	$(RM_rf) $(TARGET) $(SOURCE_OBJS) $(x100_SINK_TARGET) $(x100_SINK_OBJS)
