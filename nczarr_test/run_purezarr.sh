#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi 
. ../test_common.sh

. "${builddir}/test_nczarr.sh"

set -e

s3isolate "testdir_purezarr"
THISDIR=`pwd`
cd $ISOPATH

# This shell script tests support for:
# 1. pure zarr -- without xarray -- read/write
# 2. xarray read/write

testcase() {
zext=$1

echo "*** Test: pure zarr write then read; format=$zext; without xarray" 
fileargs tmp_purezarr "mode=zarr,noxarray,$zext"
deletemap $zext $file
${NCGEN} -4 -b -o "$fileurl" $srcdir/ref_purezarr_base.cdl
${NCDUMP} $fileurl > tmp_purezarr_${zext}.cdl
diff -b ${srcdir}/ref_purezarr.cdl tmp_purezarr_${zext}.cdl

echo "*** Test: xarray zarr write then read; format=$zext"
fileargs tmp_xarray "mode=zarr,$zext"
#deletemap $zext $file
${NCGEN} -4 -b -o "$fileurl" $srcdir/ref_purezarr_base.cdl
${NCDUMP} $fileurl > tmp_xarray_${zext}.cdl
diff -b ${srcdir}/ref_xarray.cdl tmp_xarray_${zext}.cdl

echo "*** Test: pure zarr reading nczarr; format=$zext; without xarray"
fileargs tmp_nczarr "mode=nczarr,noxarray,$zext"
deletemap $zext $file
${NCGEN} -4 -b -o "$fileurl" $srcdir/ref_whole.cdl
fileargs tmp_nczarr "mode=zarr,$zext"
${NCDUMP} -n nczarr2zarr $fileurl > tmp_nczarr_${zext}.cdl
diff -b ${srcdir}/ref_nczarr2zarr.cdl tmp_nczarr_${zext}.cdl
}

testcase file
if test "x$FEATURE_NCZARR_ZIP" = xyes ; then testcase zip; fi
if test "x$FEATURE_S3TESTS" = xyes ; then testcase s3; fi

