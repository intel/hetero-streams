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

# Note that we can't use Position Independent Executable
# on x100 Phis as COI only started supporting PIE executables
# with the 3.6 release
x100_CARD_BLD_DIR:=$(BLD_DIR)x100-card/
x100_CARD_BIN_DIR:=$(BIN_DIR)x100-card/

x100_CARD_EXE_TARGET:=$(x100_CARD_BIN_DIR)$(x100_CARD_PAYLOAD_NAME)

x100_CARD_EXE_SOURCE_FILES:= \
	hStreams_COIWrapper_sink.cpp \
	hStreams_Logger.cpp \
	hStreams_MKLWrapper.cpp \
	hStreams_app_api_sink.cpp \
	hStreams_common.cpp \
	hStreams_exceptions.cpp \
	hStreams_helpers_common.cpp \
	hStreams_internal_vars_common.cpp \
	hStreams_internal_vars_sink.cpp \
	hStreams_locks.cpp \
	hStreams_sink.cpp

x100_CARD_EXE_OBJS:=$(addprefix $(x100_CARD_BLD_DIR), $(x100_CARD_EXE_SOURCE_FILES:.cpp=.x100-card-exe.o))
x100_CARD_EXE_DEPS:=$(addprefix $(x100_CARD_BLD_DIR), $(x100_CARD_EXE_SOURCE_FILES:.cpp=.x100-card-exe.d))

# include flags
x100_CARD_INC_DIRS:=$(SRC_DIR)include/ $(SRC_DIR)../include/

x100_CARD_COMPILE_FLAGS:=$(CFLAGS) $(addprefix -I, $(x100_CARD_INC_DIRS)) \
  -UHSTR_SOURCE \
  -qopenmp -pthread \
  -rdynamic \
  $(STRICT_COMPILATION_FLAGS) \
  $(CONFIGURATION_FLAGS)

ifdef COVFILE_RUNTIME_LOCATION
x100_CARD_COMPILE_FLAGS += $(PREPROC_DEFINE)COVFILE_RUNTIME_LOCATION="$(COVFILE_RUNTIME_LOCATION)"
endif

x100_CARD_EXE_LINK_FLAGS:=$(LDFLAGS) -rdynamic -qopenmp -pthread -L$(x100_CARD_BIN_DIR) -Wl,--build-id -shared-intel -Wl,--version-script=$(SRC_DIR)linker_script_x100_card.map

$(x100_CARD_EXE_TARGET): $(x100_CARD_EXE_OBJS)
	$(dir_create)
	$(x100_CARD_CC) $^ $(x100_CARD_EXE_LINK_FLAGS) -o $@
	$(K1OM_STRIP) $(@)

$(x100_CARD_BLD_DIR)%.x100-card-exe.o: $(SRC_DIR)%.cpp
	$(dir_create)
	$(x100_CARD_CC) -MMD -MP -c $< $(x100_CARD_COMPILE_FLAGS) -o $@

$(x100_CARD_BLD_DIR)%.x100-card-lib.o: $(SRC_DIR)%.cpp
	$(dir_create)
	$(x100_CARD_CC) -MMD -MP -c $< $(x100_CARD_COMPILE_FLAGS) -o $@

build-x100-card: $(x100_CARD_EXE_TARGET)

.PHONY: install-x100-card
install-x100-card: build-x100-card

.PHONY: clean-x100-card
clean-x100-card:
	$(RM) $(x100_CARD_EXE_DEPS) $(x100_CARD_EXE_OBJS) $(x100_CARD_EXE_TARGET)

-include $(x100_CARD_DEPS)
