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
ulimit -c unlimited
if ${execdir}/tst_threads 3 "tmp_threadsafe_4_%d.nc" ; then
fail=0
else
fail=1
fi
ttpid=$!
echo $ttpid
ls -l ~/Library/Logs/DiagnosticReports
ls -ld /cores
ls -lrt /cores
touch /cores/test && rm /cores/test
echo "PASS: thread safe netcdf-4"
fi

# Cleanup
rm -f tmp_threadsafe*.nc

exit
