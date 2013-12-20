#!/bin/sh
# This shell script tests the output several previous tests.
# $Id: tst_output.sh,v 1.17 2010/05/14 16:21:15 ed Exp $


echo ""
echo "*** Testing extended file format output."
set -e
echo "Test extended format output for a netcdf-4 file"
rm -f tmp
../ncgen/ncgen -k3 -b -o ./test.nc ref_tst_small.cdl
./ncdump -K test.nc >tmp
if ! fgrep 'HDF5' <tmp ; then
echo "*** Fail: extended format for a netcdf-4 file"
fi

echo "Test extended format output for a classic netcdf-4 file"
rm -f tmp
../ncgen/ncgen -k4 -b -o ./test.nc ref_tst_small.cdl
./ncdump -K test.nc >tmp
if ! fgrep 'HDF5' <tmp ; then
echo "*** Fail: extended format for a classic netcdf-4 file"
fi

rm -f tmp
