# Hetero Streams Library
Hetero Streams Library supports a task-based parallelism programming model in which tasks
are submitted for asynchronous execution in streams.

--------------------------------------------------------------------------------
### Brief explanation of directory tree of source code distribution of the Hetero Streams Library.

Directory                 | Description
:-------------------------|:----------------------------------------------------
./include                 | External API declarations for the library (header files).
./ref_code                | Root of tree of reference code samples.
./src                     | Root of source code directory of hStreams
./doc                     | Documentation collateral

--------------------------------------------------------------------------------
### Requirements
Intel(R) Manycore Platform Software Stack recommended version is 3.8 for KNC card or 4.3.2 for x200 card. If using Intel(R)
MPSS in version prior to 3.8 see __Using Hetero Streams Library with Intel(R) Manycore
Platform Software Stack prior to version 3.7__ section.
#### Linux*
Name of Tool                              | Supported version
:-----------------------------------------|:-------------------:
Intel(R) Manycore Platform Software Stack | 3.4, 3.8, 4.3.2
Intel(R) C++ Compiler                     | 15.0, 16.0
Intel(R) Math Kernel Library              | 11.2, 11.3
Doxygen                                   | 1.6.3
Artistic Style                            | 2.05
pdflatex                                  | texlive-latex-bin-bin-svn14050.0-32.20130427_r30134.el7.noarch

#### Windows*
Name of Tool                              | Supported version
:-----------------------------------------|:-------------------:
Intel(R) Manycore Platform Software Stack | 3.4, 3.8
Intel(R) C++ Compiler                     | 15.0, 16.0
Intel(R) Math Kernel Library              | 11.2, 11.3
Doxygen (Optional[^Docs])                 | 1.6.3
MiKTeX (Optional[^Docs])                  | 2.9
Wix (Optional[^Installer])                | 3.10
Visual Studio                             | 11.0 (2012)

[^Docs]: Need to build documentation
[^Installer]: Need to build installer

--------------------------------------------------------------------------------
### Building and installing the Hetero Streams Library
#### Linux*
Set up the Intel (R) C++ Composer XE environment using commands like this:

    $ source /path/to/your/composer/directories/bin/compilervars.sh intel64

Build Hetero Streams Library using the command in root directory of repository:

    $ make artifacts

The make should generate RPMs in the rpmbuild/ directory:

    $ tree rpmbuild/*RPMS/
    rpmbuild/RPMS/
    └── x86_64
        ├── hstreams-1.2.0.DEVBOX-1.x86_64.rpm
        ├── hstreams-debuginfo-1.2.0.DEVBOX-1.x86_64.rpm
        ├── hstreams-devel-1.2.0.DEVBOX-1.x86_64.rpm
        └── hstreams-doc-1.2.0.DEVBOX-1.x86_64.rpm
    rpmbuild/SRPMS/
    └── hstreams-1.2.0.DEVBOX-1.src.rpm

These RPMs can be installed using your favourite package manager, e.g. rpm:

    $ sudo rpm -ihv rpmbuild/RPMS/x86_64/hstreams-*
    Preparing...                          ################################# [100%]
    Updating / installing...
       1:hstreams-1.2.0.DEVBOX-1          ################################# [ 25%]
       2:hstreams-devel-1.2.0.DEVBOX-1    ################################# [ 50%]
       3:hstreams-doc-1.2.0.DEVBOX-1      ################################# [ 75%]
       4:hstreams-debuginfo-1.2.0.DEVBOX-1################################# [100%]

#### Windows*
##### Build with installer and documentation
1. Open hstreams_source.sln placed in root of project.
2. Add MiKTeX (make sure that is pointing to bin directory in MiKTeX installation folder)
and Doxygen to PATH.
3. Run build solution in visual studio. Make sure selected configuration is Release/x64.
4. Run HeteroStreamsLibrary-1.2.0.msi and follow the instructions in the installer.

##### Build without installer and documentation
1. Open hstreams_source.sln placed in root of project.
2. Unload _hstreams_documentation_ and _hstreams_installer_ project in _hstreams_source_
solution.
3. Run build solution in visual studio. Make sure selected configuration is Release/x64.
4. Include library to your project using headers from _include_ directory and binaries
from _bin_ directory.

--------------------------------------------------------------------------------
### Building and running reference code sample applications.

Before starting this procedure, first make sure that you have followed the above procedure:
__Building and installing the Hetero Streams Library__

Next, there is a collection of reference code sample applications in this source code
distribution.
To build and run each, there are separate README files that provide instructions on
building them and running them:

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

### Using Hetero Streams Library with Intel(R) Manycore Platform Software Stack prior to version 3.7

Hetero Streams Library code was included in Intel(R) Manycore Platform Software Stack
releases 3.4 through 3.6. Some files from those versions may interfere with newest
version and must be removed or renamed manually.

####Clean previous version artifacts on Linux*

Previous host-side Hetero Streams Library packages will be removed automatically on
newest version installation. However, the mpss-sdk-k1om package contains headers which
might be picked up by the compiler when generating k1om binaries.

Run the command below to prevent headers conflict, replacing 3.6.1 with the appropriate
Intel(R) MPSS version:

    # rename ".h" ".h.bak" /opt/mpss/3.6.1/sysroots/k1om-mpss-linux/usr/include/hStreams_*.h

In case you should need these headers back for any reason, the following command will
restore them:

    # rename ".h.bak" ".h" /opt/mpss/3.6.1/sysroots/k1om-mpss-linux/usr/include/hStreams_*.h.bak

####Clean previous version artifacts on Windows*

In order to rename the headers, please run the following command:

    # pushd "C:\Program Files\Intel\MPSS\k1om-mpss-linux\usr\include\"
    # rename hStreams_*.h hStreams_*.h.bak

Windows* DLL search order depends on system configuration; to make sure that the old
version of the Hetero Streams Library will not be used, these files should be renamed or
removed:

    C:\Windows\System32\hstreams_source.dll
    %INTEL_MPSS_HOME%\sdk\lib\hstreams_source.lib

`%INTEL_MPSS_HOME%` is by default: `C:\Program Files\Intel\MPSS\`

Please keep in mind that a repair of the Intel(R) MPSS installation will restore these
files.

--------------------------------------------------------------------------------
<sub>*Some names and brands may be claimed as the property of others</sub>
