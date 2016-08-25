::
:: Copyright 2014-2016 Intel Corporation.
::
:: This file is subject to the Intel Sample Source Code License. A copy
:: of the Intel Sample Source Code License is included.
::

set HSTREAMS_SINK=%INTEL_MPSS_HOME%\k1om-mpss-linux\usr\lib64;%INTEL_MPSS_HOME%\sdk\tutorials\hstreams\windows\cholesky\tiled_hstreams_host_multicard
set "SINK_LD_LIBRARY_PATH=%HSTREAMS_SINK%;%MIC_LD_LIBRARY_PATH%;%cd%"

REM col major
.\x64\Release\tiled_hstreams_host_multicard.exe -m 4800 -t 6 -s 5 -l col -i 5

REM row major
.\x64\Release\tiled_hstreams_host_multicard.exe -m 4800 -t 6 -s 5 -l row -i 5

