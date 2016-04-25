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

HOST_BLD_DIR=$(BLD_DIR)host/
HOST_BIN_DIR=$(BIN_DIR)host/
HOST_ONTHEFLY_SRC_DIR=$(HOST_BLD_DIR)generated/

# We do not version the SONAME of our library due to the
# legacy that was released earlier with SONAME mistakenly set
# to libhstreams_source.so
HOST_LIBNAME=libhstreams_source.so
HOST_TARGET=$(HOST_BIN_DIR)$(HOST_LIBNAME)

HOST_SOURCE_FILES= \
	hStreams_COIWrapper.cpp \
	hStreams_HostSideSinkWorker.cpp \
	hStreams_LogBuffer.cpp \
	hStreams_LogBufferCollection.cpp \
	hStreams_LogDomain.cpp \
	hStreams_LogDomainCollection.cpp \
	hStreams_LogStream.cpp \
	hStreams_LogStreamCollection.cpp \
	hStreams_Logger.cpp \
	hStreams_MKLWrapper.cpp \
	hStreams_PhysBuffer.cpp \
	hStreams_PhysBufferHost.cpp \
	hStreams_PhysDomain.cpp \
	hStreams_PhysDomainCOI.cpp \
	hStreams_PhysDomainCollection.cpp \
	hStreams_PhysDomainHost.cpp \
	hStreams_PhysStream.cpp \
	hStreams_PhysStreamCOI.cpp \
	hStreams_PhysStreamHost.cpp \
	hStreams_RefCountDestroyed.cpp \
	hStreams_app_api_sink.cpp \
	hStreams_app_api_source.cpp \
	hStreams_app_api_workers_source.cpp \
	hStreams_common.cpp \
	hStreams_core_api_source.cpp \
	hStreams_core_api_workers_source.cpp \
	hStreams_exceptions.cpp \
	hStreams_helpers_common.cpp \
	hStreams_helpers_source.cpp \
	hStreams_internal.cpp \
	hStreams_internal_vars_common.cpp \
	hStreams_internal_vars_source.cpp \
	hStreams_locks.cpp \
	hStreams_sink.cpp \
	hStreams_threading.cpp

HOST_SRCS=$(addprefix $(SRC_DIR), $(HOST_SOURCE_FILES))
HOST_OBJS=$(addprefix $(HOST_BLD_DIR), $(HOST_SOURCE_FILES:.cpp=.host.o))
HOST_DEPS=$(addprefix $(HOST_BLD_DIR), $(HOST_SOURCE_FILES:.cpp=.host.d))

HOST_x100_CARD_STARTUP_SERIALIZED = hStreams_x100_card.cpp
ABS_HOST_x100_CARD_STARTUP_SERIALIZED = $(HOST_ONTHEFLY_SRC_DIR)hStreams_x100_card.cpp

HOST_ONTHEFLY_FILES = \
	$(HOST_x100_CARD_STARTUP_SERIALIZED)

HOST_ONTHEFLY_SRCS=$(addprefix $(HOST_ONTHEFLY_SRC_DIR), $(HOST_ONTHEFLY_FILES))
HOST_ONTHEFLY_OBJS=$(addprefix $(HOST_BLD_DIR), $(HOST_ONTHEFLY_FILES:.cpp=.host.o))
HOST_ONTHEFLY_DEPS=$(addprefix $(HOST_BLD_DIR), $(HOST_ONTHEFLY_FILES:.cpp=.host.d))

# include flags
HOST_INC_DIRS=$(SRC_DIR)include/ $(SRC_DIR)../include/

HOST_COMPILE_FLAGS=$(CFLAGS) $(addprefix -I, $(HOST_INC_DIRS)) \
  -DHSTR_SOURCE \
  -fPIC -qopenmp -pthread \
  -static-intel \
  $(STRICT_COMPILATION_FLAGS) \
  $(CONFIGURATION_FLAGS)

HOST_LINK_FLAGS=$(LDFLAGS) -shared -fPIC -qopenmp -pthread \
  -Wl,-soname,$(HOST_LIBNAME)\
  -Wl,--version-script=$(SRC_DIR)linker_script_source.map

HOST_INSTALL_DIR:=/usr/lib64/

ifeq ($(COVERAGE),y)
    HOST_CC := export PATH=$(bullseye_path):$(PATH) ; cov01 --on -v ; $(HOST_CC)
endif


$(HOST_TARGET): $(HOST_OBJS) $(HOST_ONTHEFLY_OBJS)
	$(dir_create)
	$(HOST_CC) $^ $(HOST_LINK_FLAGS) -o $@

$(HOST_BLD_DIR)%.host.o: $(SRC_DIR)%.cpp
	$(dir_create)
	$(HOST_CC) -MMD -MP -c $< $(HOST_COMPILE_FLAGS) -o $@

$(HOST_BLD_DIR)%.host.o: $(HOST_ONTHEFLY_SRC_DIR)%.cpp
	$(dir_create)
	$(HOST_CC) -MMD -MP -c $< $(HOST_COMPILE_FLAGS) -o $@

$(ABS_HOST_x100_CARD_STARTUP_SERIALIZED): $(x100_CARD_EXE_TARGET)
	$(dir_create)
	python $(INCBIN) $< $@ x100_card_startup


build-host: $(HOST_TARGET)

.PHONY: install-host
install-host: build-host install-headers install-doc
	install -d $(DESTDIR)$(HOST_INSTALL_DIR)
	install $(HOST_TARGET) $(DESTDIR)$(HOST_INSTALL_DIR)

.PHONY: clean-host
clean-host:
	$(RM) $(HOST_DEPS) $(HOST_OBJS) $(HOST_TARGET)

-include $(HOST_DEPS)

