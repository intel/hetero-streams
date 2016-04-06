#                                                                      #
# Copyright 2014-2016 Intel Corporation.                               #
#                                                                      #
# This file is subject to the Intel Sample Source Code License. A copy #
# of the Intel Sample Source Code License is included.                 #
#                                                                      #

README for matMult_host_multicard.cpp Reference Code (August 20, 2014)
This file is for use of the matMult_host_multicard app on Windows only. For information on Linux, please see the README.txt file in ../../matMult_host_multicard.

This file contains the following sections:
    REQUIREMENTS
    SETTING UP ENVIRONMENT VARIABLES
    HOW TO BUILD THE MATMULT REFERENCE CODE
    HOW TO RUN THE MATMULT REFERENCE CODE


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
       For version 2013 SP1 of Intel Composer XE compiler:
              All Programs / Intel Parallel Studio 2013 / Command prompt / Parallel Studio XE with Intel Compiler XE v14.0 / Intel 64 Visual Studio 2012 mode

   - In Windows 8, Windows 8.1, Windows Server 2012:
       On the Start screen, type Intel 64, and then choose Intel 64 Visual Studio 2012 mode. (To access the Start screen, press the Windows logo key on your keyboard.)

THIS COMMAND PROMPT WINDOW WILL BE NEEDED IN NEXT STEPS.


HOW TO BUILD THE MATMULT REFERENCE CODE
--------------------------------------------------------------
Building the host-side of the matMult hstreams application:
    1. Bring up MSVS 2012, and open the solution file stored in %HSTREAMS_HOME%\ref_code\windows\matMult_host_multicard\matMult_host_multicard.sln
    2. Make sure the configuration selected is Release/x64.
    3. Build / Build solution.

    OR

    1. In command prompt go to matMult_host_multicard directory:
        cd %HSTREAMS_HOME%\ref_code\windows\matMult_host_multicard
    2. Build solution:
        "C:\path\to\Microsoft Visual Studio 11.0\Common7\IDE\devenv.com" matMult_host_multicard.sln /Build Release
       With default path it is:
        "C:\Program Files (x86)\Microsoft Visual Studio 11.0\Common7\IDE\devenv.com" matMult_host_multicard.sln /Build Release

    Should build, and populate the directory with the following files:
    %HSTREAMS_HOME%\ref_code\windows\matMult_host_multicard>dir x64\Release

    Directory of %HSTREAMS_HOME%\ref_code\windows\matMult_host_multicard
    \x64\Release

    08/20/2014  10:50 AM    <DIR>          .
    08/20/2014  10:50 AM    <DIR>          ..
    08/20/2014  10:50 AM            86,016 matMult_host_multicard.exe
    08/20/2014  10:50 AM           445,440 matMult_host_multicard.pdb
                3 File(s)        958,840 bytes
                2 Dir(s)  40,901,140,480 bytes free

    4. Build the card side of the ref_code:
    cd %HSTREAMS_HOME%\ref_code\windows\matMult_host_multicard
    .\build_cs.bat

    [TODO] Add example outputs


HOW TO RUN THE MATMULT REFERENCE CODE
--------------------------------------------------------------
1. Start the mic service.  In the above DOS command window, type:
    cd %INTEL_MPSS_HOME%\bin
    .\micctrl.exe --start

    Should respond with:
    The Intel(R) Xeon Phi(TM) Coprocessor is starting.

2. Type the following command in the above DOS command window:
    cd %HSTREAMS_HOME%\ref_code\windows\matMult_host_multicard\
    .\run_matMult_host_multicard.bat

The system should respond with the following::
    *** C(mxn) = A(mxk) x B(kxn)
    Load balancing across host and MIC cards is on.
    Using the host CPU for compute.. and
    Using 1 MIC cards for compute..
    blocksize = 1000 x 1000, m = 3000, k = 3000, n = 3000
    mblocks = 3, kblocks = 3, nblocks = 3
    Matrix in blocks A(3,3), B(3,3), C(3,3)
    number of streams used on host = 3
    Perf: 270.115 Gflops/sec, Time= 199.915 msec
    Perf: 781.960 Gflops/sec, Time= 69.057 msec
    Perf: 761.765 Gflops/sec, Time= 70.888 msec
    Perf: 758.842 Gflops/sec, Time= 71.161 msec
    Perf: 744.150 Gflops/sec, Time= 72.566 msec
    Size=, 3000, 3000, 3000, 3000, Pass=, 1, Iters=, 5, Max=, 781.96, GF/s, Avg_DGEMM=, 761.68, GFlop/s, StdDev=, 15.56, GFlop/s, 2.04, percent (Ignoring first iteration)
    Computing result using host CPU...[If no MKLdgemm failures, then] Block Multiplication was successful.
    MKL Host DGEMM Perf, 74.490, GFlops/sec, Time= 724.926 msec

--------------------------------------------------------------
