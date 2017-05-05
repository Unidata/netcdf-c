#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi 
. ../test_common.sh

# This shell script tests the output several previous tests.

ECODE=0

echo ""
echo "*** Testing extended file format output."
set -e
echo "Test extended format output for a netcdf-3 file"
rm -f tmp
${NCGEN} -k nc3 -b -o ./test.nc $srcdir/ref_tst_small.cdl
${NCDUMP} -K test.nc >tmp
if ! grep 'classic mode=00000000' <tmp ; then
echo "*** Fail: extended format for a classic file"
ECODE=1
fi

echo "Test extended format output for a 64-bit offset netcdf-3 file"
rm -f tmp
${NCGEN} -k nc6 -b -o ./test.nc $srcdir/ref_tst_small.cdl
${NCDUMP} -K test.nc >tmp
if ! grep '64-bit offset mode=00000200' <tmp ; then
echo "*** Fail: extended format for a 64-bit classic file"
ECODE=1
fi

echo "Test extended format output for a 64-bit CDF-5 classic file"
rm -f tmp
${NCGEN} -k5 -b -o ./test.nc $srcdir/ref_tst_small.cdl
${NCDUMP} -K test.nc >tmp
if ! grep -F '64-bit data mode=00000020' <tmp ; then
echo "*** Fail: extended format for a 64-bit CDF-5 classic file"
ECODE=1
fi

rm -f tmp test.nc

exit $ECODE


