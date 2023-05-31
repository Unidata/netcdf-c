#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

set -e

NTHREADS=4
NMSGS=4

isolate "testdir_nc_tests_threads"

# Move into test directory
cd $ISOPATH

if test 1 = 1 ; then
echo "*** Testing netcdf-3 thread safe execution"
${execdir}/thread_nc_test -T ${NTHREADS} -M ${NMSGS}
echo "PASS: thread safe netcdf-3"
rm -f tmp_threadsafe*.nc
fi

if test 1 = 1 ; then
if test "x$FEATURE_HDF5" = xyes ; then
echo "*** Testing netcdf-4 thread safe execution"
#export NETCDF_LOG_LEVEL=5
${execdir}/thread_nc_test -T ${NTHREADS} -M ${NMSGS}
echo "PASS: thread safe netcdf-4"
fi
rm -f tmp_threadsafe*.nc
fi

exit
