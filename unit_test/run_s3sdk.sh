#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi 
. ../test_common.sh

set -e

#CMD="valgrind --leak-check=full"

URL="https://s3.us-east-1.amazonaws.com/${S3TESTBUCKET}"

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

echo -e "Running S3 AWSSDK Unit Tests."
echo -e "\to Checking ${URL} exists"
${CMD} ${execdir}/test_s3sdk -u "${URL}"                                  exists

echo -e "\to Checking write to ${URL}"
${CMD} ${execdir}/test_s3sdk -u "${URL}" -k "${S3ISOPATH}/test_s3sdk.txt" write
echo "Status: $?"

echo -e "\to Checking read from ${URL}"
${CMD} ${execdir}/test_s3sdk -u "${URL}" -k "${S3ISOPATH}/test_s3sdk.txt" read
echo "Status: $?"

echo -e "\to Checking size of ${URL}/test_s3sdk.txt"
${CMD} ${execdir}/test_s3sdk -u "${URL}" -k "${S3ISOPATH}/test_s3sdk.txt" size
echo "Status: $?"

echo -e "\to Checking list command for ${URL}"
${CMD} ${execdir}/test_s3sdk -u "${URL}" -k "${S3ISOPATH}"                list
echo "Status: $?"

echo -e "\to Checking search command for ${URL}"
${CMD} ${execdir}/test_s3sdk -u "${URL}" -k "/object_store"               listall
echo "Status: $?"

echo -e "\to Checking delete command for ${URL}/test_s3sdk.txt"
${CMD} ${execdir}/test_s3sdk -u "${URL}" -k "${S3ISOPATH}/test_s3sdk.txt" delete
echo "Status: $?"

echo -e "\to Checking delete command for non-existent ${URL}/test_s3sdk_x.txt"
${CMD} ${execdir}/test_s3sdk -u "${URL}" -k "${S3ISOPATH}/test_s3sdk_x.txt" delete
echo "Status: $?"

if test "x$FEATURE_LARGE_TESTS" = xyes ; then
    echo -e "\to Checking longlist command for ${URL}"
    ${CMD} ${execdir}/test_s3sdk -u "${URL}" -k "${S3ISOPATH}"                longlist
    echo "Status: $?"
fi

echo -e "Finished"

if test "x$GITHUB_ACTIONS" = xtrue; then
# Cleanup on exit
test_cleanup
fi

exit

