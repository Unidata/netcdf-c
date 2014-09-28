#!/bin/sh
#set -x
set -e

#S="-s"

if test "x$srcdir" = x ; then
srcdir=`pwd`
fi

# Start clean
rm -f zip.nc bzip2.nc szip.nc zip.cdl bzip2.cdl szip.cdl

# Create {zip,bzip2,szip}.nc
if ! ./tst_compress ; then
  echo "***FAIL: tst_compress"
fi

# ncdump both files
if test -f bzip2.nc ; then
  ../ncdump/ncdump $S -n compress zip.nc > zip.cdl
  ../ncdump/ncdump $S -n compress bzip2.nc > bzip2.cdl
  # diff the two files
  if diff -wBb zip.cdl bzip2.cdl ; then CODE=0; else  CODE=1; fi
  if test -f szip.nc ; then
    ../ncdump/ncdump $S -n compress szip.nc > szip.cdl
    if diff -wBb zip.cdl szip.cdl ; then CODE=${CODE}; else  CODE=1; fi
  fi
else
  CODE=0
fi

#Cleanup
rm -f zip.nc bzip2.nc szip.nc zip.cdl bzip2.cdl szip.cdl

if test "x$CODE" = "x0" ; then
    echo "***PASS: run_compress"
else
  echo "***FAIL: run_compress"
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
>>>>>>> Add support for multiple compression algorithms.

if test "x$CODE" = "x0" ; then
    echo "***PASS: run_compress"
else
  echo "***FAIL: run_compress"
fi

exit $CODE
