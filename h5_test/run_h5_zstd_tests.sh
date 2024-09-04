#!/bin/sh
# Copyright 2024 University Corporation for Atmospheric
# Research/Unidata. See netcdf-c/COPYRIGHT file for more info.

# This shell runs the zstandard tests.

# Ed Hartnett

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

# Load the findplugins function
. ${builddir}/findplugin.sh
export HDF5_PLUGIN_PATH="${HDF5_PLUGIN_DIR}"
echo "HDF5_PLUGIN_PATH=$HDF5_PLUGIN_PATH"

findplugin h5zstd
echo ""
echo "Testing zstandard I/O with HDF5..."

./tst_h_zstd
echo "SUCCESS!!!"

exit 0
