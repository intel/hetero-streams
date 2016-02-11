REM #                                                                      #
REM # Copyright 2014-2016 Intel Corporation.                               #
REM #                                                                      #
REM # This file is subject to the Intel Sample Source Code License. A copy #
REM # of the Intel Sample Source Code License is included.                 #
REM #                                                                      #

icl -Qmic -openmp -mkl -fPIC -shared ..\..\..\common\hStreams_custom_kernels_sink.cpp -lhstreams_sink -static-intel -o cholesky_sink_1.so

