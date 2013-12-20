#!/bin/sh
# This shell script tests the output several previous tests.
# $Id: tst_output.sh,v 1.17 2010/05/14 16:21:15 ed Exp $

FILE=tst_pnetcdf.nc

echo ""
echo "*** Testing extended file format output."
set -e

echo "Test extended format output for a PNETCDF file"
rm -f tmp
../ncdump/ncdump -K $FILE >tmp
if ! fgrep 'PNETCDF' <tmp ; then
echo "*** Fail: extended format for a PNETCDF file"
fi
rm -f tmp
