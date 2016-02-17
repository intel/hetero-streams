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
KNC_CARD_BLD_DIR:=$(BLD_DIR)knc-card/
KNC_CARD_BIN_DIR:=$(BIN_DIR)knc-card/


KNC_CARD_EXE_TARGET:=$(KNC_CARD_BIN_DIR)$(KNC_CARD_PAYLOAD_NAME)

KNC_CARD_EXE_SOURCE_FILES:= \
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

KNC_CARD_EXE_OBJS:=$(addprefix $(KNC_CARD_BLD_DIR), $(KNC_CARD_EXE_SOURCE_FILES:.cpp=.knc-card-exe.o))
KNC_CARD_EXE_DEPS:=$(addprefix $(KNC_CARD_BLD_DIR), $(KNC_CARD_EXE_SOURCE_FILES:.cpp=.knc-card-exe.d))

# include flags
KNC_CARD_INC_DIRS:=$(SRC_DIR)include/ $(SRC_DIR)../include/

KNC_CARD_COMPILE_FLAGS:=$(CFLAGS) $(addprefix -I, $(KNC_CARD_INC_DIRS)) \
  -UHSTR_SOURCE \
  -openmp -pthread \
  -rdynamic \
  $(STRICT_COMPILATION_FLAGS) \
  $(CONFIGURATION_FLAGS)

ifdef COVFILE_RUNTIME_LOCATION
KNC_CARD_COMPILE_FLAGS += $(PREPROC_DEFINE)COVFILE_RUNTIME_LOCATION="$(COVFILE_RUNTIME_LOCATION)"
endif

KNC_CARD_EXE_LINK_FLAGS:=$(LDFLAGS) -rdynamic -openmp -pthread -L$(KNC_CARD_BIN_DIR) -Wl,--build-id -shared-intel -Wl,--version-script=$(SRC_DIR)linker_script_KNC.map

$(KNC_CARD_EXE_TARGET): $(KNC_CARD_EXE_OBJS)
	$(dir_create)
	$(KNC_CARD_CC) $^ $(KNC_CARD_EXE_LINK_FLAGS) -o $@
	$(K1OM_STRIP) $(@)

$(KNC_CARD_BLD_DIR)%.knc-card-exe.o: $(SRC_DIR)%.cpp
	$(dir_create)
	$(KNC_CARD_CC) -MMD -MP -c $< $(KNC_CARD_COMPILE_FLAGS) -o $@

$(KNC_CARD_BLD_DIR)%.knc-card-lib.o: $(SRC_DIR)%.cpp
	$(dir_create)
	$(KNC_CARD_CC) -MMD -MP -c $< $(KNC_CARD_COMPILE_FLAGS) -o $@

build-knc-card: $(KNC_CARD_EXE_TARGET)

.PHONY: install-knc-card
install-knc-card: build-knc-card

.PHONY: clean-knc-card
clean-knc-card:
	$(RM) $(KNC_CARD_EXE_DEPS) $(KNC_CARD_EXE_OBJS) $(KNC_CARD_EXE_TARGET)

-include $(KNC_CARD_DEPS)
