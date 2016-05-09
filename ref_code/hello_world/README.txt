#
# Copyright 2014-2016 Intel Corporation.                               #
#                                                                      #
# This file is subject to the Intel Sample Source Code License. A copy #
# of the Intel Sample Source Code License is included.                 #
#                                                                      #

README for Hello_world Reference Code.

This file is for use of the Hello_world on Linux only.

For information on windows, please see the README.txt file in ../windows/hello_world

This file contains the following sections:

Contents
* INTRODUCTION
* HOW TO BUILD THE HELLO_WORLD REFERENCE CODE
* HOW TO RUN THE HELLO_WORLD REFERENCE CODE
* HOW TO INTERPRET RESULTS OF THE HELLO_WORLD REFERENCE CODE


INTRODUCTION:
----------------

 The goal of the test is to show how to write a simple hStream hello world program in hStreams.
 This file demonstrates the host(source) side code. It first initializes the hStreams library,
 providing the name for the sink/device side library to be called.
 It passes a double scaler and a pointer variable to the sink side by calling a sink side
 library function. It shows how to wrap a source proxy address (pointer) to a buffer, how
 to allocate a buffer on the sink side, how to transfer data in a buffer at sink-side
 and how to wait on a completion event
 The sink side library will print the value of the received value.

HOW TO BUILD:
---------------------
make


HOW TO RUN:
--------------------
./run_hello_world.sh

HOW TO INTERPRET RESULTS OF THE HELLO_WORLD REFERENCE CODE:
-------------------------------------------------------------
If you run the program you will see:

Hello hStreams world from sink side!!Received: 0.990000 and an array with 0.988000 as the first element from host!

This message has been print from the sink/mic side.
It says, the sink side received 0.99 as a scaler value from host and a buffer whose first element
was 0.988.

