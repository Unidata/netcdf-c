#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi 
. ../test_common.sh

set -e

#CMD="valgrind --leak-check=full"
#GCMD="gdb --args"

URL="https://${S3ENDPOINT}/${S3TESTBUCKET}"

isolate "testdir_uts3sdk"

# Create an isolation path for S3; build on the isolation directory
S3ISODIR="$ISODIR"
S3ISOPATH="/${S3TESTSUBTREE}"
S3ISOPATH="${S3ISOPATH}/$S3ISODIR"

test_cleanup() {
${CMD} ${execdir}/../nczarr_test/s3util -u "${URL}" -k "${S3ISOPATH}" clear
}
if test "x$GITHUB_ACTIONS" != xtrue; then
trap test_cleanup EXIT
fi

THISDIR=`pwd`
cd $ISOPATH

echo "Running S3 AWSSDK Unit Tests."

echo "	o Checking ${URL} exists"
${CMD} ${execdir}/test_s3sdk -P "$S3ISOPATH" -u "${URL}"		     exists

echo "	o Checking write to ${URL}$S3ISOPATH/test_s3sdk.txt"
${CMD} ${execdir}/test_s3sdk -P "$S3ISOPATH" -u "${URL}" -k "/test_s3sdk.txt" write
echo "Status: $?"

echo "	o Checking read from ${URL}$S3ISOPATH/test_s3sdk.txt"
${CMD} ${execdir}/test_s3sdk -P "$S3ISOPATH" -u "${URL}" -k "/test_s3sdk.txt" read
echo "Status: $?"

echo "	o Checking size of ${URL}$S3ISOPATH/test_s3sdk.txt"
${CMD} ${execdir}/test_s3sdk -P "$S3ISOPATH" -u "${URL}" -k "/test_s3sdk.txt" size
echo "Status: $?"
		
echo "	o Checking list command for ${URL}"
${CMD} ${execdir}/test_s3sdk -P "$S3ISOPATH" -u "${URL}"		      list
echo "Status: $?"

echo "	o Checking listall command for ${URL}"
${CMD} ${execdir}/test_s3sdk -P "$S3ISOPATH" -u "${URL}"                      listall
echo "Status: $?"

echo "	o Checking delete command for ${URL}$S3ISOPATH/test_s3sdk.txt"
${CMD} ${execdir}/test_s3sdk -P "$S3ISOPATH" -u "${URL}" -k "/test_s3sdk.txt" delete
echo "Status: $?"

echo "	o Checking delete command for non-existent ${URL}$S3ISOPATH/test_s3sdk_x.txt"
${CMD} ${execdir}/test_s3sdk -P "$S3ISOPATH" -u "${URL}" -k "/test_s3sdk_x.txt" delete
echo "Status: $?"

if test "x$FEATURE_LARGE_TESTS" = xyes ; then
    echo "	o Checking longlist command for ${URL}"
    ${CMD} ${execdir}/test_s3sdk -P "$S3ISOPATH" -u "${URL}"                    longlist
    echo "Status: $?"
fi

echo "Finished"

if test "x$GITHUB_ACTIONS" = xtrue; then
# Cleanup on exit
test_cleanup
fi

exit

