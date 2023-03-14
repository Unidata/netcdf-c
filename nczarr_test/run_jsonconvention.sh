#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi 
. ../test_common.sh

. "$srcdir/test_nczarr.sh"

# This shell script tests support for:
# read/write using json convention

set -e

testcase() {
zext=$1

echo "*** Test: write then read using json convention"
fileargs tmp_jsonconvention "mode=nczarr,$zext"
deletemap $zext $file
${NCGEN} -4 -b -o "$fileurl" $srcdir/ref_jsonconvention.cdl
${NCDUMP} $fileurl > tmp_jsonconvention_${zext}.cdl
${ZMD} -h $fileurl > tmp_jsonconvention_${zext}.txt
# | sed -e 's/,key1=value1|key2=value2//' -e '/"_NCProperties"/ s/(378)/(354)/'
# Clean up extraneous changes so comparisons work
# remove '\n' from ref file before comparing
sed -e 's|\\n||g' < ${srcdir}/ref_jsonconvention.cdl > tmp_jsonconvention_clean.cdl
sed -e 's|\\n||g' < ${srcdir}/ref_jsonconvention.zmap > tmp_jsonconvention_clean.zmap
sed -e '' < tmp_jsonconvention_${zext}.cdl > tmp_jsonconvention_clean_${zext}.cdl 
sed -e 's|"_NCProperties": "version=2,netcdf=[^,]*,nczarr=2.0.0",||' < tmp_jsonconvention_${zext}.txt > tmp_jsonconvention_clean_${zext}.txt 
diff -b tmp_jsonconvention_clean.cdl tmp_jsonconvention_clean_${zext}.cdl
diff -b ${srcdir}/tmp_jsonconvention_clean.zmap tmp_jsonconvention_clean_${zext}.txt
}

testcase file
#if test "x$FEATURE_NCZARR_ZIP" = xyes ; then testcase zip; fi
#if test "x$FEATURE_S3TESTS" = xyes ; then testcase s3; fi

exit 0
