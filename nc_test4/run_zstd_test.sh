#!/bin/sh

# This .in file is processed at build time into a shell that runs some
# zstd for netCDF.

# Ed Hartnett, 8/10/24

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

set -e

if test "x$TESTNCZARR" = x1 ; then
. "$srcdir/test_nczarr.sh"
s3isolate "testdir_specific_filters"
THISDIR=`pwd`
cd $ISOPATH
fi

# Load the findplugins function
. ${builddir}/findplugin.sh
echo "findplugin.sh loaded"

export HDF5_PLUGIN_PATH="${HDF5_PLUGIN_DIR}"
echo "HDF5_PLUGIN_PATH=$HDF5_PLUGIN_PATH"
ls -l "${HDF5_PLUGIN_PATH}"

echo "set HDF5_PLUGIN_PATH: ${HDF5_PLUGIN_PATH}"

./tst_zstd

echo "SUCCESS!"



