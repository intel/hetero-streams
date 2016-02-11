#                                                                      #
# Copyright 2014-2016 Intel Corporation.                               #
#                                                                      #
# This file is subject to the Intel Sample Source Code License. A copy #
# of the Intel Sample Source Code License is included.                 #
#                                                                      #

README for basic_perf.cpp Reference Code (August 20, 2014) 

This file is for use of the basic_perf on Windows only.

For information on Linux, please see the README.txt file in ../../basic_perf.

This file contains the following sections:

BRING UP A DOS COMMAND WINDOW
HOW TO BUILD THE BASIC_PERF REFERENCE CODE
HOW TO RUN THE BASIC_PERF REFERENCE CODE
HOW TO INTERPRET RESULTS OF THE BASIC_PERF REFERENCE CODE
HOW TO GET HELP AND INTERPRET THE OPTIONS

Firstly, you must install either version '2013 SP1' or version '2015' release of the Intel Composer XE compiler.

--------------------------------------------------------------
BRING UP A DOS COMMAND WINDOW

Bring up DOS command window:

   - In Windows 7 or Windows Server 2008 go to:
       For version 2015 of Intel Composer XE compiler:
              All Programs / Intel Parallel Studio 2015 / Command prompt / Parallel Studio XE with Intel Compiler XE v15.0 / Intel 64 Visual Studio 2012 mode
       For version 2013 SP1 of Intel Composer XE compiler:
              All Programs / Intel Parallel Studio 2013 / Command prompt / Parallel Studio XE with Intel Compiler XE v14.0 / Intel 64 Visual Studio 2012 mode

   - In Windows 8, Windows 8.1, Windows Server 2012:
       On the Start screen, type Intel 64, and then choose Intel 64 Visual Studio 2012 mode. (To access the Start screen, press the Windows logo key on your keyboard.)

In the DOS command window type:

cd %INTEL_MPSS_HOST_SDK%\tutorials\hstreams\windows\basic_perf
--------------------------------------------------------------

HOW TO BUILD THE basic_perf REFERENCE CODE

A. Building the host-side of the basic_perf hstreams application:
1. Install MSVS 2012, and MPSS.
2. Bring up MSVS 2012, and open the solution file stored in %INTEL_MPSS_HOST_SDK%\tutorials\hstreams\windows\basic_perf\basic_perf.sln
3. Make sure the configuration selected is Debug/x64.
4. Build / Build solution.

Should build, and populate the directory with the following files:

%INTEL_MPSS_HOST_SDK%\tutorials\hstreams\windows\basic_perf>dir x64\Debug

 Directory of %INTEL_MPSS_HOST_SDK%\tutorials\hstreams\windows\basic_perf\x64\Debug

08/20/2014  02:05 PM    <DIR>          .
08/20/2014  02:05 PM    <DIR>          ..
08/20/2014  02:05 PM            48,640 basic_perf.exe
08/20/2014  02:05 PM           270,936 basic_perf.ilk
08/20/2014  02:05 PM           461,824 basic_perf.pdb
               3 File(s)        781,400 bytes
               2 Dir(s)  40,832,323,584 bytes free

(Do not exit from this DOS command window as it is referenced in
 the following instructions).

--------------------------------------------------------------

HOW TO RUN THE basic_perf REFERENCE CODE

1. Start the mic service.  In the above DOS command window, type:

cd %INTEL_MPSS_HOME%\bin
.\micctrl.exe --start

Should respond with:

The Intel(R) Xeon Phi(TM) Coprocessor is starting.

2. In the above DOS command window, type:

cd %INTEL_MPSS_HOST_SDK%\tutorials\hstreams\windows\basic_perf\
.\run_basic_perf.bat 

The system should respond with the following:

%INTEL_MPSS_HOST_SDK%\tutorials\hstreams\windows\basic_perf>.\run_basic_perf.bat

%INTEL_MPSS_HOST_SDK%\tutorials\hstreams\windows\basic_perf>set HSTREAMS_SINK=C:\Program Files\Intel\MPSS\\k1om-mpss-linux\usr\lib64

%INTEL_MPSS_HOST_SDK%\tutorials\hstreams\windows\basic_perf>set "SINK_LD_LIBRARY_PATH=C:\Program Files\Intel\MPSS\\k1om-mpss-linux\usr\lib64;C:\Progra
m Files (x86)\Intel\Composer XE 2013 SP1\compiler\lib\mic;C:\Program Files (x86)
\Intel\Composer XE 2013 SP1\mkl\lib\mic"

%INTEL_MPSS_HOST_SDK%\tutorials\hstreams\windows\basic_perf>.\x64\Debu
g\basic_perf.exe
>>init
>>create bufs
>>xfer mem to remote domain
Transfer A and B to remote domain: 19408.180 ms for 100 iterations, 1 streams
Host-card data rate (1 streams, 4096-square) = 0.346 GB/s
>>xfer mem from remote domain
Transfer A from remote domain: 14196.452 ms for 100 iterations, 1 streams
Card-host data rate (1 streams, 4096-square) = 0.473 GB/s
>>xgemm - sample fixed functionality
Remote domain xgemm time= 29411.267 ms for 100 iterations, using 1 streams
Remove domain dgem data rate (1 streams, 4096-square) = 0.114 GFl/s
>>fini
>>init
>>create bufs
>>xfer mem to remote domain
Transfer A and B to remote domain: 76932.500 ms for 100 iterations, 4 streams
Host-card data rate (4 streams, 4096-square) = 0.349 GB/s
>>xfer mem from remote domain
Transfer A from remote domain: 59086.330 ms for 100 iterations, 4 streams
Card-host data rate (4 streams, 4096-square) = 0.454 GB/s
>>xgemm - sample fixed functionality
Remote domain xgemm time= 112336.998 ms for 100 iterations, using 4 streams
Remove domain dgem data rate (4 streams, 4096-square) = 0.120 GFl/s
>>fini

--------------------------------------------------------------
