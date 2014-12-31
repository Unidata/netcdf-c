#!/bin/sh
#set -x
set -e

if test "x$srcdir" = x ; then
srcdir=`pwd`
fi

# Create zip and bzip2 .nc
./tst_compress

# ncdump both files
../ncdump/ncdump -s -n compress zip.nc > zip.cdl
../ncdump/ncdump -s -n compress bzip2.nc > bzip2.cdl

# diff the two files
if diff -wBb zip.cdl bzip2.cdl ; then
  echo "***PASS: tst_compress"
  CODE=0
else
  echo "***FAIL: tst_compress"
  CODE=1
fi
rm -f zip.nc bzip2.nc zip.cdl bzip2.cdl

exit $CODE
