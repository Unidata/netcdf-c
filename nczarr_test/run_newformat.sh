#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi 
. ../test_common.sh

. "$srcdir/test_nczarr.sh"

set -e

s3isolate "testdir_newformat"
THISDIR=`pwd`
cd $ISOPATH

echo ""
echo "*** Testing backward compatibility between nczarr meta data format V1 vs V2"

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

testcaseconsolidated() {
echo "*** Test old format to new format consolidated nczarr copy"
zext=$1
set -x
fileargs ${srcdir}/ref_oldformat
NCZARR_CONSOLIDATED=TRUE ${NCCOPY} "$fileurl" "file://tmp_newformat_consolidated.file#mode=nczarr,file"
test -f tmp_newformat_consolidated.file/.zmetadata
NCZARR_CONSOLIDATED=TRUE ${NCDUMP} -n ref_oldformat "file://tmp_newformat_consolidated.file#mode=nczarr,file" > ./tmp_oldformat_consolidated.cdl
diff -w ${srcdir}/ref_oldformat.cdl ./tmp_oldformat_consolidated.cdl
NCZARR_CONSOLIDATED=FALSE ${NCDUMP} -n ref_oldformat "file://tmp_newformat_consolidated.file#mode=nczarr,file" > ./tmp_oldformat_non_consolidated.cdl
diff -w ${srcdir}/ref_oldformat.cdl ./tmp_oldformat_non_consolidated.cdl
NCZARR_CONSOLIDATED=TRUE ${NCDUMP} -n ref_oldformat "file://tmp_newformat_consolidated.file#mode=zarr,file" > ./tmp_pure_consolidated.cdl
NCZARR_CONSOLIDATED=FALSE ${NCDUMP} -n ref_oldformat "file://tmp_newformat_consolidated.file#mode=zarr,file" > ./tmp_pure_non_consolidated.cdl
diff -w ./tmp_pure_consolidated.cdl ./tmp_pure_non_consolidated.cdl
rm ./tmp_pure_consolidated.cdl ./tmp_pure_non_consolidated.cdl
${NCDUMP} -n ref_oldformat "file://tmp_newformat_consolidated.file#mode=zarr,file,consolidated" > ./tmp_pure_consolidated.cdl
${NCDUMP} -n ref_oldformat "file://tmp_newformat_consolidated.file#mode=zarr,file" > ./tmp_pure_non_consolidated.cdl
diff -w ./tmp_pure_consolidated.cdl ./tmp_pure_non_consolidated.cdl
set +x
}

# Do zip tests only
if test "x$FEATURE_NCZARR_ZIP" = xyes ; then
    testcaseold zip
    testcasecvt zip
    testcasepure zip
    testcaseconsolidated zip
fi
