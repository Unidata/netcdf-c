#!/bin/sh
# This shell script runs the examples for netCDF4.
# Ed Hartnett

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../../test_common.sh

set -e

echo "*** Running test_filter example..."
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

echo "*** Example successful!"
exit 0
