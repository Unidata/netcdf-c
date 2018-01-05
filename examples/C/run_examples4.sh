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

if test -f ${execdir}/test_filter${ext} ; then
echo "*** running test_filter example..."
echo "ISCYGWIN=${ISCYGWIN}"
if test "x$ISCYGWIN" != x ; then
PLUGIN=cygbzip2.dll
else
PLUGIN=libbzip2.so
fi

HDF5_PLUGIN_PATH=`pwd`
HDF5_PLUGIN_PATH="${HDF5_PLUGIN_PATH}/plugins"
if test -f "${HDF5_PLUGIN_PATH}/.libs/${PLUGIN}" ; then
HDF5_PLUGIN_PATH="${HDF5_PLUGIN_PATH}/.libs"
fi
export HDF5_PLUGIN_PATH
rm -f ./bzip2.nc
${execdir}/test_filter
#rm -f ./bzip2.nc
fi

echo "*** Examples successful!"
exit 0
