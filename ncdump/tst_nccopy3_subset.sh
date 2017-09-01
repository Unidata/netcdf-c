#!/bin/bash

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

# For a netCDF-3 build, test nccopy on netCDF files in this directory

set -e
echo ""
