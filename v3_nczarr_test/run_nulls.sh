#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi 
. ../test_common.sh

. "${srcdir}/test_nczarr.sh"

set -x
set -e

s3isolate "testdir_nulls"
THISDIR=`pwd`
cd $ISOPATH

# This shell script tests support for special cases of NC_CHAR

testcase() {
zext=$1

echo "*** Test: nczarr string write then read; format=$zext"
# Get pure zarr args
fileargs tmp_nulls_zarr "mode=zarr,$zext"
zarrurl="$fileurl"
zarrfile="$file"
# Get nczarr args
fileargs tmp_nulls_nczarr "mode=nczarr,$zext"
nczarrurl="$fileurl"
nczarrfile="$file"

# setup
deletemap $zext $zarrfile
deletemap $zext $nczarrfile

# Create alternate ref files
echo "*** create pure zarr file"
${NCGEN} -4 -b -o "$zarrurl" $srcdir/../nczarr_test/ref_nulls.cdl
echo "*** create nczarr file"
${NCGEN} -4 -b -o "$nczarrurl" $srcdir/../nczarr_test/ref_nulls.cdl

echo "*** read purezarr"
${NCDUMP} -n ref_nulls $zarrurl > tmp_nulls_zarr_${zext}.cdl
${ZMD} -h $zarrurl > tmp_nulls_zarr_${zext}.txt
echo "*** read nczarr"
${NCDUMP} -n ref_nulls $nczarrurl > tmp_nulls_nczarr_${zext}.cdl
${ZMD} -h $nczarrurl > tmp_nulls_nczarr_${zext}.txt

echo "*** verify zarr output"
diff -bw ${srcdir}/ref_nulls_zarr.baseline tmp_nulls_zarr_${zext}.cdl

echo "*** verify nczarr output"
diff -bw ${srcdir}/ref_nulls_nczarr.baseline tmp_nulls_nczarr_${zext}.cdl
}

testcase file
if test "x$FEATURE_NCZARR_ZIP" = xyes ; then testcase zip; fi
if test "x$FEATURE_S3TESTS" = xyes ; then testcase s3; fi
