REM #                                                                      #
REM # Copyright 2014-2016 Intel Corporation.                               #
REM #                                                                      #
REM # This file is subject to the Intel Sample Source Code License. A copy #
REM # of the Intel Sample Source Code License is included.                 #
REM #                                                                      #

set HSTREAMS_SINK=%INTEL_MPSS_HOME%\k1om-mpss-linux\usr\lib64;%INTEL_MPSS_HOME%\sdk\tutorials\hstreams\windows\cholesky\tiled_hstreams
set "SINK_LD_LIBRARY_PATH=%HSTREAMS_SINK%;%MIC_LD_LIBRARY_PATH%"

REM col major
.\x64\Release\tiled_hstreams.exe -m 4800 -t 6 -s 5 -l col -i 5

REM row major
.\x64\Release\tiled_hstreams.exe -m 4800 -t 6 -s 5 -l row -i 5

