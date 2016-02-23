rem #                                                                            #
rem # Hetero Streams Library - A streaming library for heterogeneous platforms   #
rem # Copyright (c) 2014 - 2016, Intel Corporation.                              #
rem #                                                                            #
rem # This program is free software; you can redistribute it and/or modify it    #
rem # under the terms and conditions of the GNU Lesser General Public License,   #
rem # version 2.1, as published by the Free Software Foundation.                 #
rem #                                                                            #
rem # This program is distributed in the hope it will be useful, but WITHOUT ANY #
rem # WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS  #
rem # FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for   #
rem # more details.                                                              #
rem #                                                                            #

SET PATH=%PATH%;M:\doxygen-win\1.6.3\;M:\miktex-win\2.9\miktex\bin\;
SET FAILED=0

cd /D %1
call doxygen conf\doxygen.rc
cd /D %1
copy "conf\make.bat" "build\doxygen\latex\" || set FAILED=%ERRORLEVEL%
cd /D "build\doxygen\latex" || set FAILED=%ERRORLEVEL%
call make || set FAILED=%ERRORLEVEL%
cd /D %1
copy "build\doxygen\latex\refman.pdf" "doc\hStreams_Reference.pdf" || set FAILED=%ERRORLEVEL%

if NOT %FAILED% == 0 (
    echo "Error occured, error: %FAILED%."
    exit /b %FAILED%
)
