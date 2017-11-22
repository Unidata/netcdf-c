#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi 
. ../test_common.sh

# This shell script runs an ncdump bug test for netcdf-4

echo ""
echo "*** Running ncdump bug test."

${execdir}/tst_fillbug
# echo "*** dumping tst_fillbug.nc to tst_fillbug.cdl..."
${NCDUMP} tst_fillbug.nc > tst_fillbug.cdl
# echo "*** comparing tst_fillbug.cdl with ref_tst_fillbug.cdl..."
diff -b tst_fillbug.cdl $srcdir/ref_tst_fillbug.cdl

echo "*** All ncdump bug test output for netCDF-4 format passed!"
exit 0
