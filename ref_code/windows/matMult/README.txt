#                                                                      #
# Copyright 2014-2016 Intel Corporation.                               #
#                                                                      #
# This file is subject to the Intel Sample Source Code License. A copy #
# of the Intel Sample Source Code License is included.                 #
#                                                                      #

README for matMult.cpp Reference Code (August 20, 2014) 

This file is for use of the matMult app on Windows only.

For information on Linux, please see the README.txt file in ../../matMult.

This file contains the following sections:

BRING UP A DOS COMMAND WINDOW
HOW TO BUILD THE MATMULT REFERENCE CODE
HOW TO RUN THE MATMULT REFERENCE CODE
HOW TO INTERPRET RESULTS OF THE MATMULT REFERENCE CODE
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

cd %INTEL_MPSS_HOST_SDK%\tutorials\hstreams\windows\matMult

--------------------------------------------------------------

HOW TO BUILD THE MATMULT REFERENCE CODE

A. Building the host-side of the matMult hstreams application:
1. Install MSVS 2012, and MPSS.
2. Install either version 14.0 or 15.0 of the Intel Composer XE compiler for windows.
3. Bring up MSVS 2012, and in MSVS:
3a If you have v 15.0 of Composer XE installed open the solution file stored in %INTEL_MPSS_HOST_SDK%\tutorials\hstreams\windows\matMult\matMult.sln
3b If you have v 14.0 of Composer XE installed open the solution file stored in %INTEL_MPSS_HOST_SDK%\tutorials\hstreams\windows\matMult\matMult_14.0.sln
4. Make sure the configuration selected is Debug/x64.
5. Build / Build solution.

Should build, and populate the directory with the following files:

%INTEL_MPSS_HOST_SDK%\tutorials\windows\matMult>dir x64\Debug

 Directory of %INTEL_MPSS_HOST_SDK%\tutorials\windows\matMult
\x64\Debug

08/20/2014  10:50 AM    <DIR>          .
08/20/2014  10:50 AM    <DIR>          ..
08/20/2014  10:50 AM            86,016 matMult.exe
08/20/2014  10:50 AM           427,384 matMult.ilk
08/20/2014  10:50 AM           445,440 matMult.pdb
               3 File(s)        958,840 bytes
               2 Dir(s)  40,901,140,480 bytes free

(Do not exit from this DOS command window as it is referenced in
 the following instructions).

--------------------------------------------------------------

HOW TO RUN THE MATMULT REFERENCE CODE

1. Start the mic service.  In the above DOS command window, type:

cd %INTEL_MPSS_HOME%\bin
.\micctrl.exe --start

Should respond with:

The Intel(R) Xeon Phi(TM) Coprocessor is starting.

2. Type the following command inin the above DOS command window:

cd %INTEL_MPSS_HOST_SDK%\tutorials\hstreams\windows\matMult\
.\run_matmult.bat 

The system should respond with the following (note that this output corresponds to version 2013 of composer xe, if you have using version 2015
the output will be slightly different):

%INTEL_MPSS_HOST_SDK%\tutorials\hstreams\windows\matMult>set HSTREAMS_SINK=C:\Program Files\Intel\MPSS\\k1om-mpss-linux\usr\lib64

%INTEL_MPSS_HOST_SDK%\tutorials\hstreams\windows\matMult>set "SINK_LD_LIBRARY_PATH=C:\Program Files\Intel\MPSS\\k1om-mpss-linux\usr\lib64;C:\Program F
iles (x86)\Intel\Composer XE 2013 SP1\compiler\lib\mic;C:\Program Files (x86)\In
tel\Composer XE 2013 SP1\mkl\lib\mic"

%INTEL_MPSS_HOST_SDK%\tutorials\hstreams\windows\matMult>.\x64\Debug\m
atMult.exe -b500 -m1000 -k1500 -n2000 -i3 -v0
*** C(mxn) = A(mxk) x B(kxn)
blocksize = 500 x 500, m = 1000, k = 1500, n = 2000
mblocks = 2, kblocks = 3, nblocks = 4
Matrix in blocks A(2,3), B(3,4), C(2,4)
Perf: 40.972 Gflops/sec, Time= 146.442 msec
Perf: 80.547 Gflops/sec, Time= 74.491 msec
Perf: 79.150 Gflops/sec, Time= 75.805 msec
Size=, 1000, 1500, 1500, 2000, Pass=, 1, Iters=, 3, Max=, 80.55, GF/s, Avg_DGEMM
=, 79.85, GFlop/s, StdDev=, 0.99, GFlop/s, 1.24, percent (Ignoring first iterati
on)
Computing result using host CPU...[If no MKLdgemm failures, then] Block Multipli
cation was successful.
MKL Host DGEMM Perf, 33.904, GFlops/sec, Time= 176.969 msec

--------------------------------------------------------------
