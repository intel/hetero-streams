#                                                                            #
# Hetero Streams Library - A streaming library for heterogeneous platforms   #
# Copyright (c) 2014 - 2016, Intel Corporation.                              #
#                                                                            #
# This program is free software; you can redistribute it and/or modify it    #
# under the terms and conditions of the GNU Lesser General Public License,   #
# version 2.1, as published by the Free Software Foundation.                 #
#                                                                            #
# This program is distributed in the hope it will be useful, but WITHOUT ANY #
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS  #
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for   #
# more details.                                                              #
#                                                                            #
Name:		hstreams
Version:	%{version}
Release:	1
Summary:	Hetero Streams Library

Group:		System Environment/Libraries
License:	LGPLv2.1 Intel-Sample-Code-License
URL:		https://github.com/01org/hetero-streams
Source0:	%{name}-%{version}.tar.gz
AutoReqProv: no
Obsoletes:	mpss-hstreams

BuildRequires:	gcc make
#Requires:

%post
/sbin/ldconfig
if test -n "$(find /opt/mpss/3.*/sysroots/k1om-mpss-linux/usr/include/ -name hStreams*.h -print -quit 2>/dev/null)"
then
    echo ""
    echo "It appears the system has an older installation of Intel(R) Manycore Platform Software Stack"
    echo "containing legacy installation of Hetero Streams Library which is incompatible with this release"
    echo "and may cause issues with compilation of k1om binaries. Please see section X.Y of the Hetero"
    echo "Streams Library User's Guide for instructions on how to resolve this issue."
    echo ""
    echo "Alternatively, you might consider upgrading to Intel(R) Manycore Platform Software Stack version >= 3.7"
    echo ""
fi

%postun
/sbin/ldconfig

%description
hetero-streams provides a simple streaming abstraction for task concurrency.

%package devel
Summary: Hetero Streams Library - development files
Group: System Environment/Libraries
Requires: %{name} = %{version}

%description devel
Development files for Hetero Streams Library

%package doc
Summary: Hetero Streams Library - documentation files
Group: Documentation
Requires: %{name} = %{version}

%description doc
Documentation files for Hetero Streams Library

%prep
%setup -q -n %{name}-%{version}

%build
make %{?_smp_mflags}

%install
%make_install

%clean
make clean


%files
%{_libdir}/lib*.so

%files devel
%{_includedir}/*

%files doc
%{_defaultdocdir}/hStreams/
%{_mandir}/man3/*

%changelog

