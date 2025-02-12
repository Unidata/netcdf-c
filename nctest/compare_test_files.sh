#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

set -e

WS=`${NCPATHCVT} -F ${srcdir}`

# Bit-by-bit comparsion is problematic.
# It should only be used for netcdf-3 files
# (classic and 64-bit-offset and maybe cdf5).
cmp nctest_classic.nc "$WS/ref_nctest_classic.nc"
cmp nctest_64bit_offset.nc "$WS/ref_nctest_64bit_offset.nc"

