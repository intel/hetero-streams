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
# on x200 Phis as COI only started supporting PIE executables
# with the 3.6 release
x200_CARD_BLD_DIR:=$(BLD_DIR)x200-card/
x200_CARD_BIN_DIR:=$(BIN_DIR)x200-card/

x200_CARD_EXE_TARGET:=$(x200_CARD_BIN_DIR)$(x200_CARD_PAYLOAD_NAME)

x200_CARD_EXE_SOURCE_FILES:= \
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

x200_CARD_EXE_OBJS:=$(addprefix $(x200_CARD_BLD_DIR), $(x200_CARD_EXE_SOURCE_FILES:.cpp=.x200-card-exe.o))
x200_CARD_EXE_DEPS:=$(addprefix $(x200_CARD_BLD_DIR), $(x200_CARD_EXE_SOURCE_FILES:.cpp=.x200-card-exe.d))

# include flags
x200_CARD_INC_DIRS:=$(SRC_DIR)include/ $(SRC_DIR)../include/

x200_CARD_COMPILE_FLAGS:=$(CFLAGS) $(addprefix -I, $(x200_CARD_INC_DIRS)) \
  -UHSTR_SOURCE \
  -qopenmp -pthread \
  -rdynamic \
  $(STRICT_COMPILATION_FLAGS) \
  $(CONFIGURATION_FLAGS)

ifdef COVFILE_RUNTIME_LOCATION
x200_CARD_COMPILE_FLAGS += $(PREPROC_DEFINE)COVFILE_RUNTIME_LOCATION="$(COVFILE_RUNTIME_LOCATION)"
endif

x200_CARD_EXE_LINK_FLAGS:=$(LDFLAGS) -rdynamic -qopenmp -pthread -L$(x200_CARD_BIN_DIR) -Wl,--build-id -shared-intel -Wl,--version-script=$(SRC_DIR)linker_script_x200_card.map

$(x200_CARD_EXE_TARGET): $(x200_CARD_EXE_OBJS)
	$(dir_create)
	$(x200_CARD_CC) $^ $(x200_CARD_EXE_LINK_FLAGS) -o $@
	$(K1OM_STRIP) $(@)

$(x200_CARD_BLD_DIR)%.x200-card-exe.o: $(SRC_DIR)%.cpp
	$(dir_create)
	$(x200_CARD_CC) -MMD -MP -c $< $(x200_CARD_COMPILE_FLAGS) -o $@

$(x200_CARD_BLD_DIR)%.x200-card-lib.o: $(SRC_DIR)%.cpp
	$(dir_create)
	$(x200_CARD_CC) -MMD -MP -c $< $(x200_CARD_COMPILE_FLAGS) -o $@

build-x200-card: $(x200_CARD_EXE_TARGET)

.PHONY: install-x200-card
install-x200-card: build-x200-card

.PHONY: clean-x200-card
clean-x200-card:
	$(RM) $(x200_CARD_EXE_DEPS) $(x200_CARD_EXE_OBJS) $(x200_CARD_EXE_TARGET)

-include $(x200_CARD_DEPS)
