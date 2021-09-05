#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi 
. ../test_common.sh

. "$srcdir/test_nczarr.sh"

# This test uses a shared resource: the .rc files; so run in a special directory
# Create a special directory
# And enter it to execute tests
rm -fr rcmiscdir
mkdir rcmiscdir
cd rcmiscdir
WD=`pwd`

# This shell script provides a miscellaneous set of tests

set -e

cleanup() {
    resetrc
}

# Setup the .rc files

createrc() {
  RCP="${WD}/.ncrc"
  echo "Creating rc file $RCP"
  echo "ZARR.DIMENSION_SEPARATOR=/" >>$RCP
}

testcase1() {
zext=$1
echo "*** Test: use '/' as dimension separator for write then read"
fileargs tmp_dimsep "mode=nczarr,$zext"
deletemap $zext $file
cleanup
createrc
${NCGEN} -4 -lb -o $fileurl ${abs_srcdir}/ref_misc1.cdl
${NCDUMP} -n tmp_misc1 $fileurl > tmp_misc1_$zext.cdl
diff -bw ${abs_srcdir}/ref_misc1.cdl tmp_misc1_$zext.cdl
}

testcase2() {
zext=$1
echo "*** Test: '/' as dimension separator creates extra groups"
fileargs tmp_extra "mode=nczarr,$zext"
deletemap $zext $file
cleanup
createrc
${NCGEN} -4 -lb -o "$fileurl" ${abs_srcdir}/ref_misc2.cdl
${NCDUMP} -n tmp_misc2 $fileurl > tmp_misc2_$zext.cdl
diff -wb ${abs_srcdir}/ref_misc2.cdl tmp_misc2_$zext.cdl
}

testcase1 file
testcase2 file
exit 0
if test "x$FEATURE_NCZARR_ZIP" = xyes ; then
    testcase1 zip
    testcase2 zip
fi
if test "x$FEATURE_S3TESTS" = xyes ; then
    testcase1 s3
    testcase2 s3
fi

exit 0
