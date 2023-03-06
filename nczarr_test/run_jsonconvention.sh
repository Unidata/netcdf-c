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
# Clean up extraneous changes wrt _NCProperties
${ZMD} -h $fileurl | sed -e 's/,key1=value1|key2=value2//' -e '/"_NCProperties"/ s/(378)/(354)/' > tmp_jsonconvention_${zext}.txt
${NCDUMP} $fileurl > tmp_jsonconvention_${zext}.cdl
# remove '\n' from ref file before comparing
rm -f tmp_jsonconvention.cdl
sed -e 's|\\n||g' < ${srcdir}/ref_jsonconvention.cdl > tmp_jsonconvention.cdl
diff -b tmp_jsonconvention.cdl tmp_jsonconvention_${zext}.cdl
diff -b ${srcdir}/ref_jsonconvention.zmap tmp_jsonconvention_${zext}.txt
}

testcase file
if test "x$FEATURE_NCZARR_ZIP" = xyes ; then testcase zip; fi
if test "x$FEATURE_S3TESTS" = xyes ; then testcase s3; fi

exit 0
