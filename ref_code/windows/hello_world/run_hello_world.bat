::
:: Copyright 2014-2016 Intel Corporation.
::
:: This file is subject to the Intel Sample Source Code License. A copy
:: of the Intel Sample Source Code License is included.
::

set HSTREAMS_SINK=%INTEL_MPSS_HOME%\k1om-mpss-linux\usr\lib64;%cd%;.\hello_world_sink\;
set "SINK_LD_LIBRARY_PATH=%HSTREAMS_SINK%;%MIC_LD_LIBRARY_PATH%"
.\x64\Release\hello_world.exe
