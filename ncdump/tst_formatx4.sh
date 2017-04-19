#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi 
. ../test_common.sh

# This shell script tests the output several previous tests.

ECODE=0
echo ""
echo "*** Testing extended file format output."
set -e
echo "Test extended format output for a netcdf-4 file"
rm -f tmp
${NCGEN} -k nc4 -b -o ./test.nc $srcdir/ref_tst_small.cdl
${NCDUMP} -K test.nc >tmp
if ! grep 'HDF5 mode=00001000' <tmp ; then
echo "*** Fail: extended format for a netcdf-4 file"
ECODE=1
fi

echo "Test extended format output for a classic netcdf-4 file"
rm -f tmp
${NCGEN} -k nc7 -b -o ./test.nc $srcdir/ref_tst_small.cdl
${NCDUMP} -K test.nc >tmp
if ! grep 'HDF5 mode=00001000' <tmp ; then
echo "*** Fail: extended format for a classic netcdf-4 file"
ECODE=1
fi

rm -f tmp test.nc

exit $ECODE

