#!/bin/sh
#set -x
set -e

if test "x$srcdir" = x ; then
srcdir=`pwd`
fi

# Create zip and bzip2 .nc
if ! ./tst_compress ; then
  echo "***FAIL: tst_compress"
fi

# ncdump both files
if test -f bzip2.nc ; then
  ../ncdump/ncdump -s -n compress zip.nc > zip.cdl
  ../ncdump/ncdump -s -n compress bzip2.nc > bzip2.cdl
  # diff the two files
  if diff -wBb zip.cdl bzip2.cdl ; then CODE=0; else  CODE=1; fi
else
  CODE=0
fi

rm -f zip.nc bzip2.nc zip.cdl bzip2.cdl

if test "x$CODE" = "x0" ; then
    echo "***PASS: run_compress"
else
  echo "***FAIL: run_compress"
fi

exit $CODE
