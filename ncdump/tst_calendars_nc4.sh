#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi 
. ../test_common.sh

# This shell script tests ncdump -t option for CF calendar attributes using netcdf4 format

set -e
echo ""
echo "*** Testing ncdump -t output for times with CF calendar attribute, netcdf4 format"
echo "*** creating netcdf4 file tst_calendars.nc from tst_calendars.cdl..."
${NCGEN} -b -k nc4 -o tst_calendars_nc4.nc $srcdir/tst_calendars_nc4.cdl
echo "*** creating tst_times_nc4.cdl from tst_calendars.nc with ncdump -t ..."
${NCDUMP} -n tst_times_nc4 -t tst_calendars_nc4.nc > tst_times_nc4.cdl
echo "*** comparing tst_times_nc4.cdl with ref_times_nc4.cdl..."
diff -b tst_times_nc4.cdl $srcdir/ref_times_nc4.cdl
echo ""
echo "*** All ncdump test output (netcdf4 format) for -t option with CF calendar atts passed!"

exit 0
