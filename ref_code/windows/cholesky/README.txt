#                                                                      #
# Copyright 2014-2016 Intel Corporation.                               #
#                                                                      #
# This file is subject to the Intel Sample Source Code License. A copy #
# of the Intel Sample Source Code License is included.                 #
#                                                                      #

README for cholesky Reference Code (January 27, 2015)
This file is for use of the cholesky apps on Windows only. For information on Linux, please see the README.txt file in ../../cholesky.

This file contains the following sections:
    REQUIREMENTS
    SETTING UP ENVIRONMENT VARIABLES
    HOW TO BUILD THE CHOLESKY REFERENCE CODE APPS
    HOW TO RUN THE CHOLESKY REFERENCE CODE


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

THIS COMMAND PROMPT WINDOW WILL BE NEED IN NEXT STEPS.


HOW TO BUILD THE CHOLESKY REFERENCE CODE APPS
--------------------------------------------------------------
A. Building the tiled_host of the cholesky hstreams ref code:
    1. Bring up MSVS 2012, and open the solution file stored in %HSTREAMS_HOME%\ref_code\windows\cholesky\tiled_host\tiled_host.sln
    2. Make sure the configuration selected is Release/x64.
    3. Build / Build solution.

    Should build, and populate the directory with the following files:
    %HSTREAMS_HOME%\ref_code\windows\cholesky\tiled_host>dir x64\Release

    Directory of %HSTREAMS_HOME%\ref_code\windows\cholesky\tiled_host\x64\Release

    08/20/2014  10:50 AM    <DIR>          .
    08/20/2014  10:50 AM    <DIR>          ..
    08/20/2014  10:50 AM            86,016 tiled_host.exe
    08/20/2014  10:50 AM           427,384 tiled_host.ilk
    08/20/2014  10:50 AM           445,440 tiled_host.pdb
                3 File(s)        958,840 bytes
                2 Dir(s)  40,901,140,480 bytes free

B. Building the tiled_hstreams of the cholesky hstreams ref code:
    1. Bring up MSVS 2012, and in MSVS:
    1a If you have v 15.0 of Composer XE installed open the solution file stored in %HSTREAMS_HOME%\ref_code\windows\cholesky\tiled_hstreams\tiled_hstreams.sln
    1b If you have v 14.0 of Composer XE installed open the solution file stored in %HSTREAMS_HOME%\ref_code\windows\cholesky\tiled_hstreams\tiled_hstreams_14.0.sln
    2. Make sure the configuration selected is Release/x64.
    3. Build / Build solution.

    Should build, and populate the directory with the following files:
    %HSTREAMS_HOME%\ref_code\windows\cholesky\tiled_hstreams>dir x64\Release

    Directory of %HSTREAMS_HOME%\ref_code\windows\cholesky\tiled_hstreams\x64\Release

    02/03/2015  11:56 AM    <DIR>          .
    02/03/2015  11:56 AM    <DIR>          ..
    02/03/2015  11:56 AM            91,136 tiled_hstreams.exe
    02/03/2015  11:56 AM           515,832 tiled_hstreams.ilk
    02/03/2015  11:56 AM           461,824 tiled_hstreams.pdb
                3 File(s)      1,068,792 bytes
                2 Dir(s)  30,768,435,200 bytes free
4. Build the card side of the ref_code:
    cd %HSTREAMS_HOME%\ref_code\windows\cholesky\cholesky\tiled_hstreams
    .\build_cs.bat

    The batch script should respond in the following manner:

    %HSTREAMS_HOME%\ref_code\windows\cholesky\tiled_hstreams>icl -Qmic -mkl  -fPIC -
    shared ..\..\..\cholesky\tiled_hstreams\hStreams_custom_kernels_sink.cpp -lhstreams_sink -static-intel -o cholesky_sink_1.so
    Intel(R) C++ Intel(R) 64 Compiler XE for applications running on Intel(R) 64, Version 15.0.0.108 Build 20140726
    Copyright (C) 1985-2014 Intel Corporation.  All rights reserved.
    icc: warning #10342: -liomp5 linked in dynamically, static library not available
    for Intel(R) MIC Architecture
    icc: warning #10342: -liomp5 linked in dynamically, static library not available
    for Intel(R) MIC Architecture
    icc: warning #10237: -lcilkrts linked in dynamically, static library not available


    Dir should show one file generated as a result of the above script:

    %HSTREAMS_HOME%\ref_code\windows\cholesky\tiled_hstreams>dir cholesky_sink_1.so
    Volume in drive C has no label.
    Volume Serial Number is 1AEE-4BBA

    Directory of %HSTREAMS_HOME%\ref_code\windows\cholesky\tiled_hstreams

    02/03/2015  12:02 PM         1,314,853 cholesky_sink_1.so
                1 File(s)      1,314,853 bytes
                0 Dir(s)  30,768,230,400 bytes free

C. Building the tiled_hstreams_host_multicard of the cholesky hstreams ref code:
    1. Bring up MSVS 2012, and open the solution file stored in %HSTREAMS_HOME%\ref_code\windows\cholesky\tiled_hstreams_host_multicard\tiled_hstreams_host_multicard.sln
    2. Make sure the configuration selected is Release/x64.
    3. Build / Build solution.

    Should build, and populate the directory with the following files:
        %HSTREAMS_HOME%\ref_code\windows\cholesky\tiled_hstreams_host_multicard>dir x64\Release

        Directory of %HSTREAMS_HOME%\ref_code\windows\cholesky\tiled_hstreams_host_multicard\x64\Release

        02/03/2015  11:56 AM    <DIR>          .
        02/03/2015  11:56 AM    <DIR>          ..
        02/03/2015  11:56 AM            91,136 tiled_hstreams.exe
        02/03/2015  11:56 AM           515,832 tiled_hstreams.ilk
        02/03/2015  11:56 AM           461,824 tiled_hstreams.pdb
                    3 File(s)      1,068,792 bytes
                    2 Dir(s)  30,768,435,200 bytes free

    4. Build the card side of the ref_code:
        cd %HSTREAMS_HOME%\ref_code\windows\cholesky\cholesky\tiled_hstreams_host_multicard
        .\build_cs.bat

    The batch script should respond in the following manner:

        %HSTREAMS_HOME%\ref_code\windows\cholesky\tiled_hstreams>icl -Qmic -mkl  -fPIC -
        shared ..\..\..\cholesky\tiled_hstreams\hStreams_custom_kernels_sink.cpp -lhstreams_sink -static-intel -o cholesky_sink_1.so
        Intel(R) C++ Intel(R) 64 Compiler XE for applications running on Intel(R) 64, Version 15.0.0.108 Build 20140726
        Copyright (C) 1985-2014 Intel Corporation.  All rights reserved.
        icc: warning #10342: -liomp5 linked in dynamically, static library not available
        for Intel(R) MIC Architecture
        icc: warning #10342: -liomp5 linked in dynamically, static library not available
        for Intel(R) MIC Architecture
        icc: warning #10237: -lcilkrts linked in dynamically, static library not available

    Dir should show one file generated as a result of the above script:

        %HSTREAMS_HOME%\ref_code\windows\cholesky\tiled_hstreams>dir cholesky_sink_1.so

        Directory of %HSTREAMS_HOME%\ref_code\windows\cholesky\tiled_hstreams

        02/03/2015  12:02 PM         1,314,853 cholesky_sink_1.so
                    1 File(s)      1,314,853 bytes
                    0 Dir(s)  30,768,230,400 bytes free

HOW TO RUN THE CHOLESKY REFERENCE CODE APPS
--------------------------------------------------------------
1. Start the mic service.  In the command window, type:
    cd %INTEL_MPSS_HOME%\bin
    .\micctrl.exe --start

Should respond with:
    The Intel(R) Xeon Phi(TM) Coprocessor is starting.

2. In the command window, type:
    cd %HSTREAMS_HOME%\ref_code\windows\cholesky\tiled_host
    .\runit.bat

3. For running the tiled_hstreams version of the ref_code:
    cd %HSTREAMS_HOME%\ref_code\windows\cholesky\tiled_hstreams
    .\runit.bat

4. For running the tiled_hstreams version of the ref_code:
    cd %HSTREAMS_HOME%\ref_code\windows\cholesky\tiled_hstreams_host_multicard
    .\runit.bat

They should respond with similar output:
    matrix is in Col major format
    no. of streams (partitions) = 5, mat_size = 4800, num_tiles = 6, niter = 5


    Iteration = 0
    time for Tiled hstreams Cholesky for iteration 0 = 633.18 msec
    time for MKL Cholesky (AO) for iteration 0 = 104.06 msec
    Tiled Cholesky successful

    Iteration = 1
    time for Tiled hstreams Cholesky for iteration 1 = 175.23 msec
    time for MKL Cholesky (AO) for iteration 1 = 99.91 msec
    Tiled Cholesky successful

    Iteration = 2
    time for Tiled hstreams Cholesky for iteration 2 = 166.01 msec
    time for MKL Cholesky (AO) for iteration 2 = 97.92 msec
    Tiled Cholesky successful

    Iteration = 3
    time for Tiled hstreams Cholesky for iteration 3 = 171.43 msec
    time for MKL Cholesky (AO) for iteration 3 = 98.70 msec
    Tiled Cholesky successful

    Iteration = 4
    time for Tiled hstreams Cholesky for iteration 4 = 167.40 msec
    time for MKL Cholesky (AO) for iteration 4 = 99.06 msec
    Tiled Cholesky successful

    Matrix size = 4800
    Tiled hStreams Cholesky: for 4 iterations (ignoring first),
    mean Time = 170.02 msec, stdDev Time = 4.17 msec,
    Mean Gflops (using mean Time) = 216.83

    MKL AO Cholesky: for 4 iterations (ignoring first),
    mean Time = 98.89 msec, stdDev Time = 0.82 msec,
    Mean Gflops (using meanTime) = 372.76

    matrix is in Row major format
    no. of streams (partitions) = 5, mat_size = 4800, num_tiles = 6, niter = 5


    Iteration = 0
    time for Tiled hstreams Cholesky for iteration 0 = 578.22 msec
    time for MKL Cholesky (AO) for iteration 0 = 272.87 msec
    Tiled Cholesky successful

    Iteration = 1
    time for Tiled hstreams Cholesky for iteration 1 = 175.35 msec
    time for MKL Cholesky (AO) for iteration 1 = 259.23 msec
    Tiled Cholesky successful

    Iteration = 2
    time for Tiled hstreams Cholesky for iteration 2 = 184.11 msec
    time for MKL Cholesky (AO) for iteration 2 = 259.43 msec
    Tiled Cholesky successful

    Iteration = 3
    time for Tiled hstreams Cholesky for iteration 3 = 182.33 msec
    time for MKL Cholesky (AO) for iteration 3 = 257.85 msec
    Tiled Cholesky successful

    Iteration = 4
    time for Tiled hstreams Cholesky for iteration 4 = 185.61 msec
    time for MKL Cholesky (AO) for iteration 4 = 261.16 msec
    Tiled Cholesky successful

    Matrix size = 4800
    Tiled hStreams Cholesky: for 4 iterations (ignoring first),
    mean Time = 181.85 msec, stdDev Time = 4.53 msec,
    Mean Gflops (using mean Time) = 202.72

    MKL AO Cholesky: for 4 iterations (ignoring first),
    mean Time = 259.42 msec, stdDev Time = 1.36 msec,
    Mean Gflops (using meanTime) = 142.10

--------------------------------------------------------------
