#!/bin/sh
# This shell script runs the examples for netCDF4.
# Ed Hartnett

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../../test_common.sh

echo "*** Running examples for netCDF-4."
set -e

echo "*** running simple_nc4 examples..."
${execdir}/simple_nc4_wr
${execdir}/simple_nc4_rd

echo "*** running simple_xy_nc4 examples..."
${execdir}/simple_xy_nc4_wr
${execdir}/simple_xy_nc4_rd

echo "*** running test_filter example..."
HDF5_PLUGIN_PATH=`pwd`
export HDF5_PLUGIN_PATH="${HDF5_PLUGIN_PATH}/plugins"
rm -f ./bzip2.nc
${execdir}/test_filter
#rm -f ./bzip2.nc

echo "*** Examples successful!"
exit 0
