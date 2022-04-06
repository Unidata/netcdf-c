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
# remove '\n' from ref file before comparing
rm -f tmp_jsonconvention.cdl
sed -e 's|\\n||g' < ref_jsonconvention.cdl > tmp_jsonconvention.cdl
diff -b ${srcdir}/tmp_jsonconvention.cdl tmp_jsonconvention_${zext}.cdl
}

testcase file
if test "x$FEATURE_NCZARR_ZIP" = xyes ; then testcase zip; fi
if test "x$FEATURE_S3TESTS" = xyes ; then testcase s3; fi

exit 0
