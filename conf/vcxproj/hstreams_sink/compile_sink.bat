rem #                                                                            #
rem # Hetero Streams Library - A streaming library for heterogeneous platforms   #
rem # Copyright (c) 2014 - 2016, Intel Corporation.                              #
rem #                                                                            #
rem # This program is free software; you can redistribute it and/or modify it    #
rem # under the terms and conditions of the GNU Lesser General Public License,   #
rem # version 2.1, as published by the Free Software Foundation.                 #
rem #                                                                            #
rem # This program is distributed in the hope it will be useful, but WITHOUT ANY #
rem # WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS  #
rem # FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for   #
rem # more details.                                                              #
rem #                                                                            #

CALL "C:\Program Files (x86)\Intel\Composer XE\bin\compilervars.bat" intel64
cd %1
icl /Qmic -D_GLIBCXX_USE_CXX11_ABI=0 -UHSTR_SOURCE -I../include -I./include -openmp -pthread -mkl -rdynamic -O3 -ggdb -o %2\hstreams_sink.so hStreams_COIWrapper_sink.cpp hStreams_MKLWrapper.cpp hStreams_sink.cpp hStreams_exceptions.cpp hStreams_app_api_sink.cpp hStreams_internal_vars_common.cpp hStreams_internal_vars_sink.cpp hStreams_common.cpp hStreams_locks.cpp hStreams_Logger.cpp hStreams_helpers_common.cpp
