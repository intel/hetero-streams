#                                                                      #
# Copyright 2014-2016 Intel Corporation.                               #
#                                                                      #
# This file is subject to the Intel Sample Source Code License. A copy #
# of the Intel Sample Source Code License is included.                 #
#                                                                      #

README for cholesky Reference Code February 3, 2015)
This file is for use of the lu apps on Windows only. For information on Linux, please see the README.txt file in ../../lu.

This file contains the following sections:
    REQUIREMENTS
    SETTING UP ENVIRONMENT VARIABLES
    HOW TO BUILD THE LU REFERENCE CODE APPS
    HOW TO RUN THE LU REFERENCE CODE


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


HOW TO BUILD THE LU REFERENCE CODE APPS
--------------------------------------------------------------
A. Building the tiled_host of the lu ref code:
    1. Bring up MSVS 2012, and in MSVS:
        3a If you have v 15.0 of Composer XE installed open the solution file stored in %HSTREAMS_HOME%\ref_code\windows\lu\tiled_host\tiled_host.sln
        3b If you have v 14.0 of Composer XE installed open the solution file stored in %HSTREAMS_HOME%\ref_code\windows\lu\tiled_host\tiled_host_14.0.sln
    2. Make sure the configuration selected is Release/x64.
    3. Build / Build solution.

    OR

    1. In command prompt go to lu\tiled_host directory:
        cd %HSTREAMS_HOME%\ref_code\windows\lu\tiled_host
    2a. Build solution with Composer XE 15.0:
        "C:\path\to\Microsoft Visual Studio 11.0\Common7\IDE\devenv.com" tiled_host.sln /Build Release
       With default path it is:
        "C:\Program Files (x86)\Microsoft Visual Studio 11.0\Common7\IDE\devenv.com" tiled_host.sln /Build Release
    2b. Build solution with Composer XE 14.0:
        "C:\path\to\Microsoft Visual Studio 11.0\Common7\IDE\devenv.com" tiled_host_14.0.sln /Build Release
       With default path it is:
        "C:\Program Files (x86)\Microsoft Visual Studio 11.0\Common7\IDE\devenv.com" tiled_host_14.0.sln /Build Release

    Should build, and populate the directory with the following files:
        %HSTREAMS_HOME%\ref_code\windows\lu\tiled_host>dir x64\Release

        Directory of %HSTREAMS_HOME%\ref_code\windows\lu\tiled_host\x64\Release

        08/20/2014  10:50 AM    <DIR>          .
        08/20/2014  10:50 AM    <DIR>          ..
        08/20/2014  10:50 AM            86,016 tiled_host.exe
        08/20/2014  10:50 AM           427,384 tiled_host.ilk
        08/20/2014  10:50 AM           445,440 tiled_host.pdb
                    3 File(s)        958,840 bytes
                    2 Dir(s)  40,901,140,480 bytes free

B. Building the tiled_hstreams version of the lu hstreams ref code:
    1. Bring up MSVS 2012, and in MSVS:
        3a If you have v 15.0 of Composer XE installed open the solution file stored in %HSTREAMS_HOME%\ref_code\windows\lu\tiled_hstreams\tiled_hstreams.sln
        3b If you have v 14.0 of Composer XE installed open the solution file stored in %HSTREAMS_HOME%\ref_code\windows\lu\tiled_hstreams\tiled_hstreams_14.0.sln
    2. Make sure the configuration selected is Release/x64.
    3. Build / Build solution.

    OR

    1. In command prompt go to lu\tiled_hstreams directory:
        cd %HSTREAMS_HOME%\ref_code\windows\lu\tiled_hstreams
    2a. Build solution with Composer XE 15.0:
        "C:\path\to\Microsoft Visual Studio 11.0\Common7\IDE\devenv.com" tiled_hstreams.sln /Build Release
       With default path it is:
        "C:\Program Files (x86)\Microsoft Visual Studio 11.0\Common7\IDE\devenv.com" tiled_hstreams.sln /Build Release
    2b. Build solution with Composer XE 14.0:
        "C:\path\to\Microsoft Visual Studio 11.0\Common7\IDE\devenv.com" tiled_hstreams_14.0.sln /Build Release
       With default path it is:
        "C:\Program Files (x86)\Microsoft Visual Studio 11.0\Common7\IDE\devenv.com" tiled_hstreams_14.0.sln /Build Release

    Should build, and populate the directory with the following files:
        %HSTREAMS_HOME%\ref_code\windows\lu\tiled_hstreams>dir x64\Release

        Directory of %HSTREAMS_HOME%\ref_code\windows\lu\tiled_hstreams\x64\Release

        02/03/2015  11:56 AM    <DIR>          .
        02/03/2015  11:56 AM    <DIR>          ..
        02/03/2015  11:56 AM            91,136 tiled_hstreams.exe
        02/03/2015  11:56 AM           515,832 tiled_hstreams.ilk
        02/03/2015  11:56 AM           461,824 tiled_hstreams.pdb
                    3 File(s)      1,068,792 bytes
                    2 Dir(s)  30,768,435,200 bytes free

    3. Build the card side of the ref_code:
    3a. cd %HSTREAMS_HOME%\ref_code\windows\lu\tiled_hstreams
    3b. .\buildcs.bat
    3c. The batch script should respond in the following manner:
        %HSTREAMS_HOME%\ref_code\windows\lu\tiled_hstreams>icl -Qmic -mkl -qopenmp -fPIC -shared ..\..\..\lu\tiled_hstreams\hStreams_custom_kernels_sink.cpp -lhstreams_sink -static-intel -o lu_sink_1.so
        Intel(R) C++ Intel(R) 64 Compiler XE for applications running on Intel(R) 64, Version 15.0.0.108 Build 20140726
        Copyright (C) 1985-2014 Intel Corporation.  All rights reserved.
        icc: warning #10342: -liomp5 linked in dynamically, static library not available
        for Intel(R) MIC Architecture
        icc: warning #10342: -liomp5 linked in dynamically, static library not available
        for Intel(R) MIC Architecture
        icc: warning #10342: -liomp5 linked in dynamically, static library not available
        for Intel(R) MIC Architecture
        icc: warning #10237: -lcilkrts linked in dynamically, static library not available

    3d. Dir should show one file generated as a result of the above script:
        %HSTREAMS_HOME%\ref_code\windows\lu\tiled_hstreams>dir lu_sink_1.so

        Directory of %HSTREAMS_HOME%\ref_code\windows\lu\tiled_hstreams

        02/03/2015  12:24 PM         1,325,918 lu_sink_1.so
                    1 File(s)      1,325,918 bytes
                    0 Dir(s)  30,649,065,472 bytes free


HOW TO RUN THE LU REFERENCE CODE APPS
--------------------------------------------------------------
1. Start the mic service.  In the command window, type:
    cd %INTEL_MPSS_HOME%\bin
    .\micctrl.exe --start

Should respond with:
    The Intel(R) Xeon Phi(TM) Coprocessor is starting.

2. In the command window type below command to run tiled_host ref code:
    cd %HSTREAMS_HOME%\ref_code\windows\lu\tiled_host
    .\runit.bat

    This is how a typical output looks:

    matrix is in Row major format
    mat_size = 4800, num_tiles = 6, niter = 5


    iter 0
    time for Tiled LU = 521.655083 msec
    time for MKL LU (dgetrf) (host, NO AO) = 383.312941 msec
    Tiled LU successful

    iter 1
    time for Tiled LU = 486.261845 msec
    time for MKL LU (dgetrf) (host, NO AO) = 350.229979 msec
    Tiled LU successful

    iter 2
    time for Tiled LU = 479.424953 msec
    time for MKL LU (dgetrf) (host, NO AO) = 377.222061 msec
    Tiled LU successful

    iter 3
    time for Tiled LU = 480.629921 msec
    time for MKL LU (dgetrf) (host, NO AO) = 382.603168 msec
    Tiled LU successful

    iter 4
    time for Tiled LU = 479.327917 msec
    time for MKL LU (dgetrf) (host, NO AO) = 366.101980 msec
    Tiled LU successful

    Tiled LU, for 4 iterations (ignoring first),
    mean Time = 481.41 msec, stdDev Time = 3.29 msec,
    Mean Gflops (using mean Time) = 153.15

    MKL LU, For 4 iterations (ignoring first),
    mean Time = 369.04 msec, stdDev Time = 14.30 msec,
    Mean Gflops (using mean Time) = 199.78

3. In the command window type below command to run  in the above DOS command window:
    cd %HSTREAMS_HOME%\ref_code\windows\lu\tiled_hstreams
    .\runit.bat

    This is how a typical output looks:

    matrix is in Row major format
    no. of streams (partitions) = 5, mat_size = 4800, num_tiles = 6, niter = 5


    Iteration = 0, Tbegin
    time for Tiled hstreams LU for iteration 0 = 818.50 msec
    time for MKL LU (dgetrf) (AO) for iteration 0 = 276.64 msec
    Tiled LU successful

    Iteration = 1, Tbegin
    time for Tiled hstreams LU for iteration 1 = 336.69 msec
    time for MKL LU (dgetrf) (AO) for iteration 1 = 334.06 msec
    Tiled LU successful

    Iteration = 2, Tbegin
    time for Tiled hstreams LU for iteration 2 = 388.07 msec
    time for MKL LU (dgetrf) (AO) for iteration 2 = 382.38 msec
    Tiled LU successful

    Iteration = 3, Tbegin
    time for Tiled hstreams LU for iteration 3 = 346.46 msec
    time for MKL LU (dgetrf) (AO) for iteration 3 = 360.69 msec
    Tiled LU successful

    Iteration = 4, Tbegin
    time for Tiled hstreams LU for iteration 4 = 471.69 msec
    time for MKL LU (dgetrf) (AO) for iteration 4 = 377.19 msec
    Tiled LU successful

    Matrix size = 4800
    Tiled hStreams LU: for 4 iterations (ignoring first),
    mean Time = 385.73 msec, stdDev Time = 61.49 msec,
    Mean Gflops (using mean Time) = 191.14

    MKL AO LU (dgetrf): For 4 iterations (ignoring first),
    mean Time = 363.58 msec, stdDev Time = 21.74 msec,
    Mean Gflops (using mean Time) = 202.78

--------------------------------------------------------------
