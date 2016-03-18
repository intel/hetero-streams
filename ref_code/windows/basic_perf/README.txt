#                                                                      #
# Copyright 2014-2016 Intel Corporation.                               #
#                                                                      #
# This file is subject to the Intel Sample Source Code License. A copy #
# of the Intel Sample Source Code License is included.                 #
#                                                                      #

README for basic_perf.cpp Reference Code (August 20, 2014)
This file is for use of the basic_perf on Windows only. For information on Linux, please see the README.txt file in ../../basic_perf.


This file contains the following sections:
    REQUIREMENTS
    SETTING UP ENVIRONMENT VARIABLES
    HOW TO BUILD THE BASIC_PERF REFERENCE CODE
    HOW TO RUN THE BASIC_PERF REFERENCE CODE


REQUIREMENTS
--------------------------------------------------------------
Below software must be installed on Windows, before we start:
    Intel(R) Composer XE Compiler 2015
    Intel(R) Manycore Platform Software Stack
    Microsoft Visual Studio 2012


SETTING UP ENVIRONMENT VARIABLES
--------------------------------------------------------------
If you use Windows command prompt, you have to set up Intel(R) Composer XE environment variables, by run command:
    "C:\path\to\Intel\Composer XE\bin\compilervars.bat" intel64
   With default path it is:
    "C:\Program Files (x86)\Intel\Composer XE\bin\compilervars.bat" intel64

You can also use "Intel 64 Visual Studio 2012 mode" command prompt:
   - In Windows 7 or Windows Server 2008 go to:
       For version 2015 of Intel Composer XE compiler:
              All Programs / Intel Parallel Studio 2015 / Command prompt / Parallel Studio XE with Intel Compiler XE v15.0 / Intel 64 Visual Studio 2012 mode

   - In Windows 8, Windows 8.1, Windows Server 2012:
       On the Start screen, type Intel 64, and then choose Intel 64 Visual Studio 2012 mode. (To access the Start screen, press the Windows logo key on your keyboard.)

THIS COMMAND PROMPT WINDOW WILL BE NEEDED IN NEXT STEPS.


HOW TO BUILD THE BASIC_PERF REFERENCE CODE
--------------------------------------------------------------
Building the host-side of the basic_perf hstreams application:
    1. Bring up MSVS 2012, and open the solution file stored in %HSTREAMS_HOME%\ref_code\windows\basic_perf\basic_perf.sln
    2. Make sure the configuration selected is Release/x64.
    3. Build / Build solution.

    OR

    1. In command prompt go to basic_perf directory:
        cd %HSTREAMS_HOME%\ref_code\windows\basic_perf
    2. Build solution:
        "C:\path\to\Microsoft Visual Studio 11.0\Common7\IDE\devenv.com" basic_perf.sln /Build Release
       With default path it is:
        "C:\Program Files (x86)\Microsoft Visual Studio 11.0\Common7\IDE\devenv.com" basic_perf.sln /Build Release

Should build, and populate the directory with the following files:
    %HSTREAMS_HOME%\ref_code\windows\basic_perf>dir x64\Release

        Directory of %HSTREAMS_HOME%\ref_code\windows\basic_perf\x64\Release

        08/20/2014  02:05 PM    <DIR>          .
        08/20/2014  02:05 PM    <DIR>          ..
        08/20/2014  02:05 PM            48,640 basic_perf.exe
        08/20/2014  02:05 PM           270,936 basic_perf.ilk
        08/20/2014  02:05 PM           461,824 basic_perf.pdb
                    3 File(s)        781,400 bytes
                    2 Dir(s)  40,832,323,584 bytes free


HOW TO RUN THE BASIC_PERF REFERENCE CODE
--------------------------------------------------------------
1. Start the mic service.  In the command window, type:
    cd %INTEL_MPSS_HOME%\bin
    .\micctrl.exe --start

    Should respond with:
    The Intel(R) Xeon Phi(TM) Coprocessor is starting.

2. In the command window type below command to run basic_perf:
    cd %HSTREAMS_HOME%\ref_code\windows\basic_perf
    .\run_basic_perf.bat

The system should respond with the following:

    %HSTREAMS_HOME%\ref_code\windows\basic_perf>.\run_basic_perf.bat

    %HSTREAMS_HOME%\ref_code\windows\basic_perf>set HSTREAMS_SINK=C:\Program Files\Intel\MPSS\\k1om-mpss-linux\usr\lib64

    %HSTREAMS_HOME%\ref_code\windows\basic_perf>set "SINK_LD_LIBRARY_PATH=C:\Program Files\Intel\MPSS\\k1om-mpss-linux\usr\lib64;
    C:\Program Files (x86)\Intel\Composer XE 2013 SP1\compiler\lib\mic;C:\Program Files (x86)\Intel\Composer XE 2013 SP1\mkl\lib\mic"

    %HSTREAMS_HOME%\ref_code\windows\basic_perf>.\x64\Release\basic_perf.exe
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
