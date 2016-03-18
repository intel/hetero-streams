#                                                                      #
# Copyright 2014-2016 Intel Corporation.                               #
#                                                                      #
# This file is subject to the Intel Sample Source Code License. A copy #
# of the Intel Sample Source Code License is included.                 #
#                                                                      #

README for matMult.cpp Reference Code (August 20, 2014)
This file is for use of the matMult app on Windows only. For information on Linux, please see the README.txt file in ../../matMult.

This file contains the following sections:
    REQUIREMENTS
    SETTING UP ENVIRONMENT VARIABLES
    HOW TO BUILD THE MATMULT REFERENCE CODE
    HOW TO RUN THE MATMULT REFERENCE CODE


REQUIREMENTS
--------------------------------------------------------------
Below software must be installed on Windows, before we start:
    Intel(R) Composer XE Compiler in version '2013 SP1' or '2015'
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
       For version 2013 SP1 of Intel Composer XE compiler:
              All Programs / Intel Parallel Studio 2013 / Command prompt / Parallel Studio XE with Intel Compiler XE v14.0 / Intel 64 Visual Studio 2012 mode

   - In Windows 8, Windows 8.1, Windows Server 2012:
       On the Start screen, type Intel 64, and then choose Intel 64 Visual Studio 2012 mode. (To access the Start screen, press the Windows logo key on your keyboard.)

THIS COMMAND PROMPT WINDOW WILL BE NEEDED IN NEXT STEPS.



HOW TO BUILD THE MATMULT REFERENCE CODE
--------------------------------------------------------------
Building the host-side of the matMult hstreams application:
    1. Bring up MSVS 2012, and in MSVS:
    1a If you have v 15.0 of Composer XE installed open the solution file stored in %HSTREAMS_HOME%\ref_code\windows\matMult\matMult.sln
    1b If you have v 14.0 of Composer XE installed open the solution file stored in %HSTREAMS_HOME%\ref_code\windows\matMult\matMult_14.0.sln
    2. Make sure the configuration selected is Release/x64.
    3. Build / Build solution.

    OR

    1. In command prompt go to matMult directory:
        cd %HSTREAMS_HOME%\ref_code\windows\matMult
    2a. Build solution with Composer XE 15.0:
        "C:\path\to\Microsoft Visual Studio 11.0\Common7\IDE\devenv.com" matMult.sln /Build Release
       With default path it is:
        "C:\Program Files (x86)\Microsoft Visual Studio 11.0\Common7\IDE\devenv.com" matMult.sln /Build Release
    2b. Build solution with Composer XE 14.0:
        "C:\path\to\Microsoft Visual Studio 11.0\Common7\IDE\devenv.com" matMult_14.0.sln /Build Release
       With default path it is:
        "C:\Program Files (x86)\Microsoft Visual Studio 11.0\Common7\IDE\devenv.com" matMult_14.0.sln /Build Release

    Should build, and populate the directory with the following files:
    %HSTREAMS_HOME%\ref_code\windows\matMult>dir x64\Release

    Directory of %HSTREAMS_HOME%\ref_code\windows\matMult
    \x64\Release

    08/20/2014  10:50 AM    <DIR>          .
    08/20/2014  10:50 AM    <DIR>          ..
    08/20/2014  10:50 AM            86,016 matMult.exe
    08/20/2014  10:50 AM           427,384 matMult.ilk
    08/20/2014  10:50 AM           445,440 matMult.pdb
                3 File(s)        958,840 bytes
                2 Dir(s)  40,901,140,480 bytes free


HOW TO RUN THE MATMULT REFERENCE CODE
--------------------------------------------------------------
1. Start the mic service.  In the above DOS command window, type:
    cd %INTEL_MPSS_HOME%\bin
    .\micctrl.exe --start

    Should respond with:
    The Intel(R) Xeon Phi(TM) Coprocessor is starting.

2. Type the following command inin the above DOS command window:
    cd %HSTREAMS_HOME%\ref_code\windows\matMult\
    .\run_matmult.bat

The system should respond with the following (note that this output corresponds to version 2013 of composer xe, if you have using version 2015 the output will be slightly different):
    %HSTREAMS_HOME%\ref_code\windows\matMult>set HSTREAMS_SINK=C:\Program Files\Intel\MPSS\\k1om-mpss-linux\usr\lib64

    %HSTREAMS_HOME%\ref_code\windows\matMult>set "SINK_LD_LIBRARY_PATH=C:\Program Files\Intel\MPSS\\k1om-mpss-linux\usr\lib64;C:\Program Files (x86)\Intel\Composer XE 2013 SP1\compiler\lib\mic;C:\Program Files (x86)\Intel\Composer XE 2013 SP1\mkl\lib\mic"

    %HSTREAMS_HOME%\ref_code\windows\matMult>.\x64\Release\matMult.exe -b500 -m1000 -k1500 -n2000 -i3 -v0
    *** C(mxn) = A(mxk) x B(kxn)
    blocksize = 500 x 500, m = 1000, k = 1500, n = 2000
    mblocks = 2, kblocks = 3, nblocks = 4
    Matrix in blocks A(2,3), B(3,4), C(2,4)
    Perf: 40.972 Gflops/sec, Time= 146.442 msec
    Perf: 80.547 Gflops/sec, Time= 74.491 msec
    Perf: 79.150 Gflops/sec, Time= 75.805 msec
    Size=, 1000, 1500, 1500, 2000, Pass=, 1, Iters=, 3, Max=, 80.55, GF/s, Avg_DGEMM
    =, 79.85, GFlop/s, StdDev=, 0.99, GFlop/s, 1.24, percent (Ignoring first iteration)
    Computing result using host CPU...[If no MKLdgemm failures, then] Block Multiplication was successful.
    MKL Host DGEMM Perf, 33.904, GFlops/sec, Time= 176.969 msec

--------------------------------------------------------------
