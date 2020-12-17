#!/bin/sh

# Run (tst_chunks,tst_chunks2) X (nzf,nc4,s3)


if test "x$srcdir" = x ; then srcdir=`pwd`; fi 
. ../test_common.sh

. "$srcdir/test_nczarr.sh"

set -e

# Functions

ittest() {
extfor $1
if test "x$2" != x ; then CLOUD="-c $2"; fi
${execdir}/tst_chunks -e $1 $CLOUD
${execdir}/tst_chunks2 -e $1 $CLOUD
}

ittest nzf
if test "xFEATURE_HDF5" = xyes ; then ittest nz4; fi
if test "x$FEATURE_S3TESTS" = xyes ; then ittest s3 'https://stratus.ucar.edu/unidata-netcdf-zarr-testing'; fi
}


