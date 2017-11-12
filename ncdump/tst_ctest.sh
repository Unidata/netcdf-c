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
${NCGEN} -lc -o ctest0.nc $srcdir/c0.cdl > ctest.c
diff -b ctest.c $srcdir/ref_ctest.c

echo "*** Testing that ncgen with c0.cdl for 64-bit offset format."
${execdir}/ref_ctest64
${NCGEN}  -k2 -lc -o ctest0_64.nc $srcdir/c0.cdl > ctest64.c
diff -b ctest64.c $srcdir/ref_ctest64.c

echo "*** All tests of ncgen with ctest.c and ctest64.c passed!"
exit 0
