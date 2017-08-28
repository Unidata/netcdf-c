#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

set -e

echo "*** Testing tst_szip "
./tst_szip
echo "***Passed"

echo "*** Testing h5testszip "
./h5testszip
echo "***Passed"

echo "*** Testing h5testszip on testszip.nc"
./h5testszip ./testszip.nc
echo "***Passed"


