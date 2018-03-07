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

if test -f ${builddir}/findplugin.sh ; then
echo "*** running test_filter example..."
. ${builddir}/findplugin.sh

# Locate the plugin path and the library names; argument order is critical
# Find bzip2 and capture
findplugin bzip2
BZIP2PATH="${HDF5_PLUGIN_PATH}/${HDF5_PLUGIN_LIB}"
# Verify
if ! test -f ${BZIP2PATH} ; then echo "Unable to locate ${BZIP2PATH}"; exit 1; fi
export HDF5_PLUGIN_PATH

echo "*** running filter_example..."
rm -f ./bzip2.nc
${execdir}/filter_example
#rm -f ./bzip2.nc

fi # Filter enabled

echo "*** Examples successful!"
exit 0
