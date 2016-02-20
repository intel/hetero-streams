REM #                                                                      #
REM # Copyright 2014-2016 Intel Corporation.                               #
REM #                                                                      #
REM # This file is subject to the Intel Sample Source Code License. A copy #
REM # of the Intel Sample Source Code License is included.                 #
REM #                                                                      #

icl -Qmic -mkl -qopenmp -fPIC -shared ..\..\..\common\hStreams_custom_kernels_sink.cpp -lhstreams_sink -static-intel -o lu_sink_1.so

