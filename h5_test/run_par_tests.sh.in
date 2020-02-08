#!/bin/sh
# Copyright 2020 University Corporation for Atmospheric
# Research/Unidata. See netcdf-c/COPYRIGHT file for more info.

# This shell runs some parallel HDF5 tests.

# Ed Hartnett

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

echo ""
echo "Testing parallel I/O with HDF5..."

mpiexec -n 1 ./tst_h_par
mpiexec -n 2 ./tst_h_par
mpiexec -n 4 ./tst_h_par
echo "SUCCESS!!!"

exit 0
