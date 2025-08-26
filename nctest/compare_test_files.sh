#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

set -e

WS=`${NCPATHCVT} -F ${srcdir}`

cmp nctest_classic.nc "$WS/ref_nctest_classic.nc"
cmp nctest_64bit_offset.nc "$WS/ref_nctest_64bit_offset.nc"

