#                                                                      #
# Copyright 2014-2016 Intel Corporation.                               #
#                                                                      #
# This file is subject to the Intel Sample Source Code License. A copy #
# of the Intel Sample Source Code License is included.                 #
#                                                                      #

# There are two kinds of targets:
#  user - working code that lab participant modifies
#  solution - reference code that user code will become like

# This Makefile should be consistent across all installations and directories
# Customization
#   ../../hStreams.mk defines HSRT_INSTALL: standard path to hstreams
#     and whether code is generated for MIC
#   ./rootname.mk contains a string which is the rootname for each directory
#     Example: hello_hStreams_world, for 1.hello_hStreams_world

# Loads base names for files and executables, and flags to indicate whether MIC
# (i.e., Xeon Phi) is being used or not.
include names.mk

# Dynamically determines the top level directory.
TOP_DIR:=$(dir $(abspath $(lastword $(MAKEFILE_LIST))))

include ../../hStreams.mk

# Location for ref_code directory
REFCODE_DIR:=$(TUT_INSTALL)/ref_code/

# Sets up compiler flags and commands
include $(REFCODE_DIR)common/toolchain.mk

TARGET_USER := $(BIN_HOST)$(NAME)
TARGET_SOLN := $(BIN_HOST)$(NAME)_solution

# Normally, the convention of a _mic extension is used
#  But if you want a different name you need to tell hStreams what to load
#  with HSTR_OPTIONS - see A.remote_invocation/4.optional.hello_extra
x100_SINK_TARGET_USER := $(BIN_x100)$(SINK_NAME).so
x100_SINK_TARGET_SOLN := $(BIN_x100)$(SINK_SOLN_NAME).so

# Here are some additional compiler flags that have been used to improve program
# runtime.  These have nothing in particular to do with hStreams.
# -qopenmp if your program uses openmp pragmas
# -ansi-alias pointers do not alias
# -opt-subscript-in-range the loop subscripts are in int range
# -AVX enable avx vectorization
# -ip enables inter procedure optimizations
# -parallel requests for automatic parallelizations
# -restrict needs to compiler programs if some pointers have restrict qualifiers
# restrict means pointers do not overlap
ADDITIONAL_SOURCE_CXXFLAGS := -qopenmp -ansi-alias -opt-subscript-in-range \
 -AVX -ip -parallel -restrict
ADDITIONAL_x100_SINK_CXXFLAGS := -qopenmp -restrict

# Additional dynamically loaded libraries
#  -lhstreams_source is required for hStreams
ADDITIONAL_SOURCE_LDFLAGS  := -lhstreams_source -qopenmp
# -shared is a must keyword to add here.
# The soname is a string, which is used as a "logical name" describing the
# functionality of the object
ADDITIONAL_x100_SINK_USER_LDFLAGS:= -shared \
  -Wl,-soname,$(x100_SINK_TARGET_USER) -qopenmp
ADDITIONAL_x100_SINK_SOLN_LDFLAGS:= -shared \
  -Wl,-soname,$(x100_SINK_TARGET_SOLN) -qopenmp

# Sources and objects
SOURCE_SRCS := $(TOP_DIR)$(NAME)_src.cpp $(TOP_DIR)$(NAME)_src_solution.cpp \
$(REFCODE_DIR)common/dtime.cpp
USER_SRCS := $(TOP_DIR)$(NAME)_src.cpp $(REFCODE_DIR)common/dtime.cpp
SOLN_SRCS := $(TOP_DIR)$(NAME)_src_solution.cpp $(REFCODE_DIR)common/dtime.cpp
SOURCE_OBJS := $(SOURCE_SRCS:.cpp=.$(SOURCE_TAG).o)
USER_OBJS := $(USER_SRCS:.cpp=.$(SOURCE_TAG).o)
SOLN_OBJS := $(SOLN_SRCS:.cpp=.$(SOURCE_TAG).o)

# Sink-side source and object files
x100_SINK_SRCS = $(TOP_DIR)$(NAME)_sink.cpp
x100_SINK_OBJS = $(x100_SINK_SRCS:.cpp=.$(x100_SINK_TAG).o)

ifdef IS_MIC
# Makefile user-modified version
user: $(TARGET_USER) $(x100_SINK_TARGET_USER)

# Makefile pre-defined solution version
solution: $(TARGET_SOLN) $(x100_SINK_TARGET_SOLN)
else
# Makefile user-modified version
user: $(TARGET_USER)

# Makefile user-modified version
solution: $(TARGET_SOLN)
endif

# The default "all" target - builds everything
all: user solution

# If you're curious about the syntax below, please see 4.12.1,
#   Syntax of Static Pattern Rules
# https://www.gnu.org/software/make/manual/html_node/Static-Usage.html#Static-Usage
$(SOURCE_OBJS): %.$(SOURCE_TAG).o: %.cpp
	$(dir_create)
	$(SOURCE_CXX) -c $^ -o $@ $(SOURCE_CXXFLAGS) $(ADDITIONAL_SOURCE_CXXFLAGS)

$(x100_SINK_OBJS): %.$(x100_SINK_TAG).o: %.cpp
	$(dir_create)
	$(x100_SINK_CXX) -c $^ -o $@ $(x100_SINK_CXXFLAGS) \
	$(ADDITIONAL_x100_SINK_CXXFLAGS)

$(x100_SINK_TARGET_USER): $(x100_SINK_OBJS)
	$(dir_create)
	$(x100_SINK_CXX) $^ -o $@ $(x100_SINK_LDFLAGS) \
	$(ADDITIONAL_x100_SINK_USER_LDFLAGS)

$(x100_SINK_TARGET_SOLN): $(x100_SINK_OBJS)
	$(dir_create)
	$(x100_SINK_CXX) $^ -o $@ $(x100_SINK_LDFLAGS) \
	$(ADDITIONAL_x100_SINK_SOLN_LDFLAGS)

$(TARGET_USER): $(USER_OBJS)
	$(dir_create)
	$(SOURCE_CXX) $^ -o $@ $(SOURCE_LDFLAGS) \
	$(ADDITIONAL_SOURCE_LDFLAGS)

$(TARGET_SOLN): $(SOLN_OBJS)
	$(dir_create)
	$(SOURCE_CXX) $^ -o $@ $(SOURCE_LDFLAGS) \
	$(ADDITIONAL_SOURCE_LDFLAGS)

.PHONY: clean
clean:
	$(RM_rf) $(TARGET_USER) $(TARGET_SOLN) $(SOURCE_OBJS) \
	$(x100_SINK_USER_TARGET) $(x100_SINK_SOLN_TARGET) $(x100_SINK_OBJS)
