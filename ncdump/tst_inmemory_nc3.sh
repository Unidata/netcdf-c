#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi 
. ../test_common.sh

verbose=1
set -e

# Setup
PASS=1

# Define the .cdl files to test
CLASSIC="small ref_tst_nans ref_tst_utf8"
EXTENDED="ref_nc_test_netcdf4 ref_tst_comp ref_tst_opaque_data"

rm -fr ./results
mkdir ./results

# Dump classic files two ways and compare
dotest() {
K=$1
for f in $2 ; do
  echo "Testing ${f}"
  ${NCGEN} -$K -o ./results/${f}.nc ${srcdir}/${f}.cdl
  ${NCDUMP} ./results/${f}.nc > ./results/${f}.cdl
  ${NCDUMP} -Xm ./results/${f}.nc > ./results/${f}.cdx
  diff -w ./results/${f}.cdl ./results/${f}.cdx &> ./results/${f}.diff
  if test -s ./results/${f}.diff ; then
    echo "***FAIL: $f"
    PASS=0
  fi
done
}

dotest "3" "$CLASSIC"

# Cleanup
rm -fr results

if test "x$PASS" = x1 ; then
  echo "*** PASS all tests"
  CODE=0
else
  CODE=1
fi
exit $CODE
