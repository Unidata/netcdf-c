#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi 
. ../test_common.sh

set -e

#CMD="valgrind --leak-check=full"

URL="https://s3.us-east-1.amazonaws.com/unidata-zarr-test-data"

isolate "testdir_uts3sdk"

# Create an isolation path for S3; build on the isolation directory
S3ISODIR="$ISODIR"
S3ISOPATH="/netcdf-c"
S3ISOPATH="${S3ISOPATH}/$S3ISODIR"

test_cleanup() {
${CMD} ${execdir}/../nczarr_test/s3util -u "${URL}" -k "${S3ISOPATH}" clear
}
if test "x$GITHUB_ACTIONS" != xtrue; then
trap test_cleanup EXIT
fi

THISDIR=`pwd`
cd $ISOPATH

${CMD} ${execdir}/test_s3sdk -u "${URL}"                                  exists
${CMD} ${execdir}/test_s3sdk -u "${URL}" -k "${S3ISOPATH}/test_s3sdk.txt" write
${CMD} ${execdir}/test_s3sdk -u "${URL}" -k "${S3ISOPATH}/test_s3sdk.txt" read
${CMD} ${execdir}/test_s3sdk -u "${URL}" -k "${S3ISOPATH}/test_s3sdk.txt" size
${CMD} ${execdir}/test_s3sdk -u "${URL}" -k "${S3ISOPATH}"                list
${CMD} ${execdir}/test_s3sdk -u "${URL}" -k "${S3ISOPATH}"                search
${CMD} ${execdir}/test_s3sdk -u "${URL}" -k "${S3ISOPATH}/test_s3sdk.txt" delete
if test "x$FEATURE_LARGE_TESTS" = xyes ; then
${CMD} ${execdir}/test_s3sdk -u "${URL}" -k "${S3ISOPATH}"                longlist
fi

exit
if test "x$GITHUB_ACTIONS" = xtrue; then
# Cleanup on exit
test_cleanup
fi

exit

