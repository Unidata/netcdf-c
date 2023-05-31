#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

set -x
set -e

NTHREADS=8
NCYCLES=4

isolate "testdir_nc_tests_threads"

# Move into test directory
cd $ISOPATH

echo "*** Testing netcdf-3 thread safe execution"
#gdb -batch -ex run -ex bt --args
${execdir}/tst_threads -3 -F 'tmp_threadsafe_3_%d.nc' -T ${NTHREADS} -C ${NCYCLES}
echo "PASS: thread safe netcdf-3"
rm -f tmp_threadsafe*.nc

if test "x$FEATURE_HDF5" = xyes ; then
echo "*** Testing netcdf-4 thread safe execution"
#gdb -batch -ex run -ex bt --args
${execdir}/tst_threads -4 -F 'tmp_threadsafe_4_%d.nc' -T ${NTHREADS} -C ${NCYCLES}
echo "PASS: thread safe netcdf-4"
fi
rm -f tmp_threadsafe*.nc

exit
