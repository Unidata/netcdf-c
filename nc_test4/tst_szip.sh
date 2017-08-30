#!/bin/sh

export SETX=1

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

set -e

rm -f testnc.h5 testszip.nc

echo "*** Testing tst_szip "
${execdir}/test_szip
echo "***Passed"

echo "*** Testing h5testszip "
${execdir}/h5testszip
echo "***Passed"

echo "*** Testing h5testszip on testszip.nc"
${execdir}/h5testszip ./testszip.nc
echo "***Passed"

rm -f testnc.h5 testszip.nc

exit
