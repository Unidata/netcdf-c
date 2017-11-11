#!/bin/sh
# This shell script tests that running ncgen can use c0.cdl and
# produce the correct code, which matches ref_ctest.c.
# Ed Hartnett

if test "x$srcdir" = x ; then srcdir=`pwd`; fi 
. ../test_common.sh

set -e

echo ""
echo "*** Testing that ncgen produces correct C code from c0.cdl."
${execdir}/ref_ctest
${NCGEN} -lc -o ctest0.nc c0.cdl > ctest.c
diff -b ctest.c $srcdir/ref_ctest.c

echo "*** All ncgen and ncdump test output for netCDF-4 format passed!"
exit 0
