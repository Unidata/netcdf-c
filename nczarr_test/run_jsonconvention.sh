#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi 
. ../test_common.sh

. "$srcdir/test_nczarr.sh"

set -e

s3isolate "testdir_jsonconvention"
THISDIR=`pwd`
cd $ISOPATH

# This shell script tests support for:
# read/write using json convention

testcase() {
zext=$1
host_le=`printf '\001\000\000\000' | od -An -t d4 | tr -d ' \n'`
expected_zmap="${srcdir}/ref_jsonconvention.zmap"

echo "*** Test: write then read using json convention"
fileargs tmp_jsonconvention "mode=nczarr,$zext"
deletemap $zext $file
${NCGEN} -4 -b -o "$fileurl" $srcdir/ref_jsonconvention.cdl
${NCDUMP} $fileurl > tmp_jsonconvention_${zext}.cdl
${ZMD} -h $fileurl > tmp_jsonconvention_${zext}.txt
# Clean up extraneous changes so comparisons work
cat < tmp_jsonconvention_${zext}.cdl > tmp_jsonconvention_clean_${zext}.cdl
cat < tmp_jsonconvention_${zext}.txt > tmp_jsonconvention_clean_${zext}.txt
sed -i.bak -e 's|"_NCProperties": "version=[0-9],[^"]*",||' tmp_jsonconvention_clean_${zext}.txt 
sed -i.bak -e 's|\(.z[a-z][a-z]*\) : ([0-9][0-9]*)|\1 : ()|g' tmp_jsonconvention_clean_${zext}.txt
if test "x${host_le}" != x1 ; then
  expected_zmap="${srcdir}/ref_jsonconvention_be.zmap"
fi
# compare
diff -b ${srcdir}/ref_jsonconvention.cdl tmp_jsonconvention_clean_${zext}.cdl
diff -b ${expected_zmap} tmp_jsonconvention_clean_${zext}.txt
}

testcase file
if test "x$FEATURE_NCZARR_ZIP" = xyes ; then testcase zip; fi
if test "x$FEATURE_S3TESTS" = xyes ; then testcase s3; fi
