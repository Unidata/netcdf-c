#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi 
. ../test_common.sh

. "$srcdir/test_nczarr.sh"

# This shell script tests support for the NC_STRING type

set -e

testcase() {
zext=$1

echo "*** Test: nczarr string write then read; format=$zext"
# Get pure zarr args
fileargs tmp_string_zarr "mode=zarr,$zext"
zarrurl="$fileurl"
zarrfile="$file"
# Get nczarr args
fileargs tmp_string_nczarr "mode=nczarr,$zext"
nczarrurl="$fileurl"
nczarrfile="$file"

# setup
deletemap $zext $zarrfile
deletemap $zext $nczarrfile

# Create alternate ref files
echo "*** create pure zarr file"
${NCGEN} -4 -b -o "$zarrurl" $srcdir/ref_string.cdl
echo "*** create nczarr file"
${NCGEN} -4 -b -o "$nczarrurl" $srcdir/ref_string.cdl

echo "*** read purezarr"
${NCDUMP} -n ref_string $zarrurl > tmp_string_zarr_${zext}.cdl
${ZMD} -h $zarrurl > tmp_string_zarr_${zext}.txt
echo "*** read nczarr"
${NCDUMP} -n ref_string $nczarrurl > tmp_string_nczarr_${zext}.cdl
${ZMD} -h $nczarrurl > tmp_string_nczarr_${zext}.txt

echo "*** verify zarr output"
diff -bw ${srcdir}/ref_string_zarr.baseline tmp_string_zarr_${zext}.cdl

echo "*** verify nczarr output"
diff -bw ${srcdir}/ref_string_nczarr.baseline tmp_string_nczarr_${zext}.cdl
}

testcase file
if test "x$FEATURE_NCZARR_ZIP" = xyes ; then testcase zip; fi
if test "x$FEATURE_S3TESTS" = xyes ; then testcase s3; fi

exit 0
