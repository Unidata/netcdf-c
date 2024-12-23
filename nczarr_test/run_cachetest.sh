#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi 
. ../test_common.sh

. "${builddir}/test_nczarr.sh"

set -e

s3isolate "testdir_cachtest"
THISDIR=`pwd`
cd $ISOPATH

# This shell script tests support for the NC_STRING type

testcase() {
zext=$1

echo "*** Test: cache operation"

# Get pure zarr args
fileargs tmp_scalar_zarr "mode=zarr,$zext"
zarrurl="$fileurl"
zarrfile="$file"

# setup
deletemap $zext $zarrfile

echo "*** write cache"
${execdir}/test_writecaching

echo "*** read cache"
${execdir}/test_readcaching
}

testcase file
if test "x$FEATURE_NCZARR_ZIP" = xyes ; then testcase zip; fi
if test "x$FEATURE_S3TESTS" = xyes ; then testcase s3; fi
