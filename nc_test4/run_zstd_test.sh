#!/bin/sh

# This .in file is processed at build time into a shell that runs some
# zstd for netCDF.

# Ed Hartnett, 8/10/24

set -e

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

# Load the findplugins function
. ${builddir}/findplugin.sh
echo "findplugin.sh loaded"
echo "${HDF5_PLUGIN_DIR}"

findplugin h5zstd
echo "HDF5_PLUGIN_DIR=$HDF5_PLUGIN_DIR"
./tst_zstd

echo
echo "Testing Zstandard compression..."

export HDF5_PLUGIN_PATH="${HDF5_PLUGIN_DIR}"
echo "set HDF5_PLUGIN_PATH: ${HDF5_PLUGIN_PATH}"

echo "SUCCESS!"



