::                                                                      ::

:: Copyright 2014-2016 Intel Corporation.                               ::

::                                                                      ::

:: This software is supplied under the terms of a license agreement or  ::

:: nondisclosure agreement with Intel Corporation and may not be copied ::

:: or disclosed except in accordance with the terms of that agreement.  ::

::                                                                      ::

CALL "C:\Program Files (x86)\Intel\Composer XE\bin\compilervars.bat" intel64

icl /Qmic -I../../../common/ -I%1 -fPIC -shared -mkl -O3 -D_GLIBCXX_USE_CXX11_ABI=0 -UHSTR_SOURCE -I../../../../include  -rdynamic -O3 -ggdb -o hello_world_mic.so ../../../hello_world/hello_world_sink.cpp