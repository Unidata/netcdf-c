#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

NTHREADS=1
NCYCLES=1

set -e

echo "*** Testing netcdf-3 thread safe execution"
${execdir}/tst_threads -e3 -F "tmp_threadsafe_3_%d.nc" -T ${NTHREADS} -C ${NCYCLES}
echo "PASS: thread safe netcdf-3"
rm -f tmp_threadsafe*.nc

if test "x$FEATURE_HDF5" = xyes ; then
echo "*** Testing netcdf-4 thread safe execution"
${execdir}/tst_threads -4 -F "tmp_threadsafe_4_%d.nc" -T ${NTHREADS} -C ${NCYCLES}
echo "PASS: thread safe netcdf-4"
fi
rm -f tmp_threadsafe*.nc

exit
