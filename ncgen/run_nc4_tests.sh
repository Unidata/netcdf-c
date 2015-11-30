#!/bin/sh
# This shell script runs the ncdump tests.
# $Id: run_nc4_tests.sh,v 1.4 2010/05/18 20:05:23 dmh Exp $

if test "x$srcdir" = x ; then srcdir="."; fi

echo "*** Testing ncgen for netCDF-4."
set -e

echo "*** creating netCDF-4 file c0_4.nc from c0_4.cdl..."
./ncgen -k nc4 -b -o c0_4.nc $srcdir/c0_4.cdl

echo "*** creating netCDF-4 classic model file c0_4c.nc from c0.cdl..."
./ncgen -k nc7 -b -o c0_4c.nc $srcdir/c0.cdl

echo "*** creating C code for CAM file ref_camrun.cdl..."
./ncgen -lc $srcdir/ref_camrun.cdl >ref_camrun.c

echo "*** test for jira NCF-199 bug"
./ncgen -k nc4 $srcdir/ncf199.cdl

echo "*** Test successful!"
exit 0
