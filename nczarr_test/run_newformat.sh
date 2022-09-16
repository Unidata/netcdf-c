#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi 
. ../test_common.sh

. "$srcdir/test_nczarr.sh"

set -e

echo ""
echo "*** Testing backward compatibilty between nczarr meta data format V1 vs V2"

testcaseold() {
echo "*** Test old format support"
zext=$1
fileargs ${srcdir}/ref_oldformat
${NCDUMP} -n ref_oldformat "$fileurl" > ./tmp_oldformat.cdl
diff -w ${srcdir}/ref_oldformat.cdl ./tmp_oldformat.cdl
}

testcasecvt() {
echo "*** Test old format to new format nczarr copy"
zext=$1
fileargs ${srcdir}/ref_oldformat
${NCCOPY} "$fileurl" "file://tmp_newformat.file#mode=nczarr,file"
${NCDUMP} -n ref_oldformat "file://tmp_newformat.file#mode=nczarr,file" > ./tmp_newformat.cdl
diff -w ${srcdir}/ref_oldformat.cdl ./tmp_newformat.cdl
}

testcasepure() {
echo "*** Test old format to new format pure zarr copy"
zext=$1
fileargs ${srcdir}/ref_oldformat
${NCCOPY} "$fileurl" "file://tmp_newformat.file#mode=nczarr,file"
${NCDUMP} -n ref_oldformat "file://tmp_newformat.file#mode=zarr,file" > ./tmp_newpure.cdl
diff -w ${srcdir}/ref_newformatpure.cdl ./tmp_newpure.cdl
}

# Do zip tests only
if test "x$FEATURE_NCZARR_ZIP" = xyes ; then
    testcaseold zip
    testcasecvt zip
    testcasepure zip
fi

exit 0
