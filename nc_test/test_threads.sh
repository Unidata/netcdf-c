#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

set -x
#set -e

echo ""

echo "*** Testing netcdf-3 thread safe execution"
${execdir}/tst_threads 3 "tmp_threadsafe_3_%d.nc"
echo "PASS: thread safe netcdf-3"

if test "x$FEATURE_HDF5" = xyes ; then
echo "*** Testing netcdf-4 thread safe execution"
${execdir}/tst_threads 4 "tmp_threadsafe_4_%d.nc"
echo "PASS: thread safe netcdf-4"
fi

# Cleanup
rm -f tmp_threadsafe*.nc

exit
