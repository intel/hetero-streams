Hetero Streams Library - A streaming library for heterogeneous platforms.

A. Brief explanation of directory tree of source code distribution of the Hetero Streams Library.
B. Building and installing the Hetero Streams Library
C. Building and running reference code sample applications.

------------------------------------------------------------------------------------------------------------------

A. Brief explanation of directory tree of source code distribution of the Hetero Streams Library.

./include                           External API declarations for the library (header files).
./ref_code                          Root of tree of reference code samples.
./src                               Root of source code directory of hStreams
./doc                               Documentation collateral

------------------------------------------------------------------------------------------------------------------

B. Building and installing the Hetero Streams Library

St up the Intel (R) C++ Composer XE environment using commands like this:

    $ source /path/to/your/composer/directories/bin/compilervars.sh intel64

Build Hetero Streams Library using the command:

    $ make artifacts

The make should generate RPMs in the rpmbuild/ directory:

    $ tree rpmbuild/*RPMS/
    rpmbuild/RPMS/
    └── x86_64
        ├── hstreams-1.0.0.DEVBOX-1.x86_64.rpm
        ├── hstreams-debuginfo-1.0.0.DEVBOX-1.x86_64.rpm
        ├── hstreams-devel-1.0.0.DEVBOX-1.x86_64.rpm
        └── hstreams-doc-1.0.0.DEVBOX-1.x86_64.rpm
    rpmbuild/SRPMS/
    └── hstreams-1.0.0.DEVBOX-1.src.rpm

These RPMs can be installed using your favourite package manager, e.g. rpm:

    $ sudo rpm -ihv rpmbuild/RPMS/x86_64/hstreams-*
    Preparing...                          ################################# [100%]
    Updating / installing...
       1:hstreams-1.0.0.DEVBOX-1          ################################# [ 25%]
       2:hstreams-devel-1.0.0.DEVBOX-1    ################################# [ 50%]
       3:hstreams-doc-1.0.0.DEVBOX-1      ################################# [ 75%]
       4:hstreams-debuginfo-1.0.0.DEVBOX-1################################# [100%]

------------------------------------------------------------------------------------------------------------------

C. Building and running reference code sample applications.

Before starting this procedure, first make sure that you have followed the above procedure:
B. Building and installing the Hetero Streams Library

Next, there is a collection of reference code sample applications in this source code distribution.
To build and run each, there are separate README files that provide instructions on building them
and running them:

    $ cd ref_code
    $ find . -name README.txt
    ./basic_perf/README.txt
    ./cholesky/README.txt
    ./io_perf/README.txt
    ./lu/README.txt
    ./matMult_host_multicard/README.txt
    ./matMult/README.txt
    ./windows/basic_perf/README.txt
    ./windows/cholesky/README.txt
    ./windows/io_perf/README.txt
    ./windows/lu/README.txt
    ./windows/matMult/README.txt

Follow the instructions in those files.
