#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi 
. ../test_common.sh

. "$srcdir/test_nczarr.sh"

# This shell script tests bug reported in github issue 
# https://github.com/Unidata/netcdf-c/issues/1826

set -e

echo ""
echo "*** Testing data conversions when a variable has fill value but never written"

testcase() {
zext=$1
fileargs tmp_fillonly
deletemap $zext $file
${NCGEN} -4 -b -o "$fileurl" $srcdir/ref_fillonly.cdl
${execdir}/tst_fillonlyz${ext} "$fileurl"
}

testcase file
if test "x$FEATURE_NCZARR_ZIP" = xyes ; then testcase zip; fi
if test "x$FEATURE_S3TESTS" = xyes ; then testcase s3; fi

exit 0
