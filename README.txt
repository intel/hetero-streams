README.txt

This file includes information on the source code distribution of mpss-hStreams:

A. Brief explanation of directory tree of source code distribution of mpss-hStreams.
B. Building host-side and card-side of hstreams
C. Building and running reference code sample applications.

------------------------------------------------------------------------------------------------------------------

A. Brief explanation of directory tree of source code distribution of mpss-hStreams.

./mpss-hstreams-<version>/include                               External API declarations for hStreams (header files).
./mpss-hstreams-<version>/ref_code                              Root of tree of reference code samples.
./mpss-hstreams-<version>/src                                   Root of source code directory of hStreams
./mpss-hstreams-<version>/src/docs_config                       Root of directory tree used for generating PDF files.
./mpss-hstreams-<version>/doc                                   Released documents
./mpss-hstreams-<version>/doc/tools/ExtractManPages             Source code for generating man pages for hStreams.

------------------------------------------------------------------------------------------------------------------

B. Building host-side and card-side of hstreams:

1. To build: set up the composer xe environment using commands like this:

$ export INTEL_LICENSE_FILE=/path/to/your/composer/licenses/directory
$ . /path/to/your/composer/directories/.../composer_xe_2013/bin/compilervars.sh intel64

2. Build using the commands:
$ cd ./mpss-hstreams-<version>/src
$ make

The make should generate five files that can be verified using the commands:

$ cd ./mpss-hstreams-<version>/
$ find bin
bin
bin/dev
bin/dev/libhstreams_sink.so
bin/dev/libhstreams_sink.so.0
bin/dev/libhstreams_mic
bin/host
bin/host/libhstreams_source.so.0
bin/host/libhstreams_source.so

------------------------------------------------------------------------------------------------------------------

C. Building and running reference code sample applications.

Before starting this procedure, first make sure that you have followed the above procedure:
B. Building host-side and card-side of hstreams.

Next, there are a collection of reference code sample applications in this source code distribution.

To build and run each, there are six separate README files that provide instructions on building them
and running them:

$ cd ./mpss-hstreams-<version>/ref_code
$ find . -iname '*README*'
./basic_perf/README.txt
./test_app/README.txt
./lu/README.txt
./io_perf/README.txt
./matMult/README.txt
./cholesky/README.txt

Follow the instuctions in those files, after you have built hStreams following section B above.
