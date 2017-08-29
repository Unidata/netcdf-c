#!/bin/sh

export SETX=1

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

set -e

rm -f testnc.h5 testszip.nc

echo "*** Testing tst_szip "
./test_szip
echo "***Passed"

echo "*** Testing h5testszip "
./h5testszip
echo "***Passed"

echo "*** Testing h5testszip on testszip.nc"
./h5testszip ./testszip.nc
echo "***Passed"

rm -f testnc.h5 testszip.nc

exit
