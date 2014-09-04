Summary: Rexx Translation Library
Name: RexxTrans
Version: 1.4
Release: 1
Copyright: LGPL
Group: Development/Languages
Vendor: Mark Hessling
Source: ftp://prdownloads.sourceforge.net/rexxtrans/RexxTrans-1.4.tar.gz
%description
Rexx/Trans is a dynamically loaded library that translates Rexx API calls
from an application to one of several Rexx interpreters.  This library allows
an application to run unmodified with a number of different Rexx interpreter;
determined at runtime.
For more information on Rexx/Trans, visit http://rexxtrans.sf.net
For more information on Rexx, visit http://www.rexxla.org
%prep
%setup

%build
./configure
make

%install
make install

%files
/usr/include/rexxtrans.h
/usr/lib/librexxtrans.so.1.4
/usr/bin/rexxtrans-config

%post
cd /usr/lib
ln -sf ./librexxtrans.so.1.4 ./librexxtrans.so.1
ln -sf ./librexxtrans.so.1.4 ./librexxtrans.so
ldconfig /usr/lib
