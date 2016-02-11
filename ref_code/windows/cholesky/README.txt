#                                                                      #
# Copyright 2014-2016 Intel Corporation.                               #
#                                                                      #
# This file is subject to the Intel Sample Source Code License. A copy #
# of the Intel Sample Source Code License is included.                 #
#                                                                      #

README for cholesky Reference Code (January 27, 2015) 

This file is for use of the cholesky apps on Windows only.

For information on Linux, please see the README.txt file in ../../cholesky.

This file contains the following sections:

BRING UP A DOS COMMAND WINDOW
HOW TO BUILD THE CHOLESKY REFERENCE CODE APPS
HOW TO RUN THE CHOLESKY REFERENCE CODE
HOW TO INTERPRET RESULTS OF THE CHOLESKY REFERENCE CODE
HOW TO GET HELP AND INTERPRET THE OPTIONS

--------------------------------------------------------------
BRING UP A DOS COMMAND WINDOW

Bring up DOS command window:

   - In Windows 7 or Windows Server 2008 go to:
       All Programs / Intel Parallel Studio XE 2015 / Command prompt / Parallel Studio XE with Intel Compiler XE v15.0 / Intel 64 Visual Studio 2012 mode   
       (or, if you have v14.0 of Composer XE installed,:
       All Programs / Intel Parallel Studio XE 2013 / Command prompt / Parallel Studio XE with Intel Compiler XE v14.0 / Intel 64 Visual Studio 2012 mode
       )
   - In Windows 8, Windows 8.1, Windows Server 2012:
       On the Start screen, type Intel 64, and then choose Intel 64 Visual Studio 2012 mode. (To access the Start screen, press the Windows logo key on your keyboard.)

In the DOS command window type:

cd %INTEL_MPSS_HOST_SDK%\tutorials\hstreams\windows\cholesky

--------------------------------------------------------------

HOW TO BUILD THE CHOLESKY REFERENCE CODE APPS

A. Building the tiled_host of the cholesky hstreams ref code:
1. Install MSVS 2012, and MPSS.
2. Install either version 14.0 or 15.0 of the Intel Composer XE compiler for windows.
3. Bring up MSVS 2012, and in MSVS:
3a If you have v 15.0 of Composer XE installed open the solution file stored in %INTEL_MPSS_HOST_SDK%\tutorials\hstreams\windows\cholesky\tiled_host\tiled_host.sln
3b If you have v 14.0 of Composer XE installed open the solution file stored in %INTEL_MPSS_HOST_SDK%\tutorials\hstreams\windows\cholesky\tiled_host\tiled_host_14.0.sln
4. Make sure the configuration selected is Debug/x64.
5. Build / Build solution.

Should build, and populate the directory with the following files:

%INTEL_MPSS_HOST_SDK%\tutorials\windows\cholesky\tiled_host>dir x64\Debug

 Directory of %INTEL_MPSS_HOST_SDK%\tutorials\windows\cholesky\tiled_host\x64\Debug

08/20/2014  10:50 AM    <DIR>          .
08/20/2014  10:50 AM    <DIR>          ..
08/20/2014  10:50 AM            86,016 tiled_host.exe
08/20/2014  10:50 AM           427,384 tiled_host.ilk
08/20/2014  10:50 AM           445,440 tiled_host.pdb
               3 File(s)        958,840 bytes
               2 Dir(s)  40,901,140,480 bytes free

B. Building the tiled_hstreams of the cholesky hstreams ref code:
1. Install MSVS 2012, and MPSS.
2. Install either version 14.0 or 15.0 of the Intel Composer XE compiler for windows.
3. Bring up MSVS 2012, and in MSVS:
3a If you have v 15.0 of Composer XE installed open the solution file stored in %INTEL_MPSS_HOST_SDK%\tutorials\hstreams\windows\cholesky\tiled_hstreams\tiled_hstreams.sln
3b If you have v 14.0 of Composer XE installed open the solution file stored in %INTEL_MPSS_HOST_SDK%\tutorials\hstreams\windows\cholesky\tiled_hstreams\tiled_hstreams_14.0.sln
4. Make sure the configuration selected is Debug/x64.
5. Build / Build solution.

Should build, and populate the directory with the following files:

 Directory of C:\Program Files\Intel\MPSS\sdk\tutorials\hstreams\windows\cholesky\tiled_hstreams\x64\Debug

02/03/2015  11:56 AM    <DIR>          .
02/03/2015  11:56 AM    <DIR>          ..
02/03/2015  11:56 AM            91,136 tiled_hstreams.exe
02/03/2015  11:56 AM           515,832 tiled_hstreams.ilk
02/03/2015  11:56 AM           461,824 tiled_hstreams.pdb
               3 File(s)      1,068,792 bytes
               2 Dir(s)  30,768,435,200 bytes free
6. Build the card side of the ref_code:
6a. cd to the ... sdk\tutorials\hstreams\windows\cholesky\tiled_hstreams directory
6b. .\build_cs.bat
6c. The batch script should respond in the following manner:

C:\Program Files\Intel\MPSS\sdk\tutorials\hstreams\windows\cholesky\tiled_hstreams>icl -Qmic -mkl  -fPIC -
shared ..\..\..\cholesky\tiled_hstreams\hStreams_custom_kernels_sink.cpp -lhstreams_sink -static-intel -o cholesky_sink_1.so
Intel(R) C++ Intel(R) 64 Compiler XE for applications running on Intel(R) 64, Version 15.0.0.108 Build 20140726
Copyright (C) 1985-2014 Intel Corporation.  All rights reserved.
icc: warning #10342: -liomp5 linked in dynamically, static library not available
 for Intel(R) MIC Architecture
icc: warning #10342: -liomp5 linked in dynamically, static library not available
 for Intel(R) MIC Architecture
icc: warning #10237: -lcilkrts linked in dynamically, static library not available

6d. Dir should show one file generated as a result of the above script:

C:\Program Files\Intel\MPSS\sdk\tutorials\hstreams\windows\cholesky\tiled_hstreams>dir cholesky_sink_1.so
 Volume in drive C has no label.
 Volume Serial Number is 1AEE-4BBA

 Directory of C:\Program Files\Intel\MPSS\sdk\tutorials\hstreams\windows\cholesk
y\tiled_hstreams

02/03/2015  12:02 PM         1,314,853 cholesky_sink_1.so
               1 File(s)      1,314,853 bytes
               0 Dir(s)  30,768,230,400 bytes free

(Do not exit from this DOS command window as it is referenced in
 the following instructions).

--------------------------------------------------------------

HOW TO RUN THE CHOLESKY REFERENCE CODE APPS

1. Start the mic service.  In the above DOS command window, type:

cd %INTEL_MPSS_HOME%\bin
.\micctrl.exe --start

Should respond with:

The Intel(R) Xeon Phi(TM) Coprocessor is starting.

2.  Running the tiled_host ref code:
2a  Type the following command in the above DOS command window:

cd %INTEL_MPSS_HOST_SDK%\tutorials\hstreams\windows\cholesky\tiled_host
.\runit.bat 

The system should respond with the following (note that this output corresponds to version 2013 of composer xe, if you have using version 2015
the output will be slightly different):

3. Running the tiled_hstreams version of the ref_code 

3a. Type the following command in the above DOS command window:

cd %INTEL_MPSS_HOST_SDK%\tutorials\hstreams\windows\cholesky\tiled_hstreams
.\runit.bat 

The system should respond with output similar to the output recorded in the README file for Linux (../../cholesky/README.txt).

--------------------------------------------------------------
