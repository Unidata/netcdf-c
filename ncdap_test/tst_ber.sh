#!/bin/sh

export SETX=1

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

/g/netcdf-c/build/ncdump/ncdump.exe '[log][cache]file:///g/netcdf-c/ncdap_test/testdata3/ber-2002-10-01.nc'

