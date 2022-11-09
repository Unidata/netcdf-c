#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi 
. ../test_common.sh

. "$top_srcdir/nczarr_test/test_nczarr.sh"

# This shell script tests support for the NC_STRING type

set -e

zarrscalar() {
    rm -f $2
    sed -e '/dimensions:/d' -e '/_scalar_ =/d' -e '/int v/ s|(_scalar_)||' <$1 >$2
}

testcase() {
zext=$1

echo "*** Test: scalar write/read"

# Get pure zarr args
fileargs tmp_scalar_zarr "mode=zarr,$zext"
zarrurl="$fileurl"
zarrfile="$file"
# Get nczarr args
fileargs tmp_scalar_nczarr "mode=nczarr,$zext"
nczarrurl="$fileurl"
nczarrfile="$file"

# setup
deletemap $zext $zarrfile
deletemap $zext $nczarrfile

# Create alternate ref files
echo "*** create pure zarr file"
${NCGEN} -4 -b -o "$zarrurl" $top_srcdir/nczarr_test/ref_scalar.cdl
echo "*** create nczarr file"
${NCGEN} -4 -b -o "$nczarrurl" $top_srcdir/nczarr_test/ref_scalar.cdl

echo "*** read purezarr"
${NCDUMP} -n ref_scalar $zarrurl > tmp_scalar_zarr0_${zext}.cdl
${ZMD} -h $zarrurl > tmp_scalar_zarr_${zext}.txt
echo "*** read nczarr"
${NCDUMP} -n ref_scalar $nczarrurl > tmp_scalar_nczarr_${zext}.cdl
${ZMD} -h $nczarrurl > tmp_scalar_nczarr_${zext}.txt

echo "*** verify"
diff -bw $top_srcdir/nczarr_test/ref_scalar.cdl tmp_scalar_nczarr_${zext}.cdl

# Fixup
zarrscalar tmp_scalar_zarr0_${zext}.cdl tmp_scalar_zarr_${zext}.cdl
diff -bw $top_srcdir/nczarr_test/ref_scalar.cdl tmp_scalar_zarr_${zext}.cdl
}

testcase file
if test "x$FEATURE_NCZARR_ZIP" = xyes ; then testcase zip; fi
if test "x$FEATURE_S3TESTS" = xyes ; then testcase s3; fi

exit 0
