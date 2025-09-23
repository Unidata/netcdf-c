#!/bin/sh
#set -x
#set -e
if test "x$srcdir" = x ; then srcdir=`pwd`; fi 
. ../test_common.sh

. "$srcdir/test_nczarr.sh"

# Build both ISOPATH and S3ISOPATH
s3isolate "testdir_notzarr"
THISDIR=`pwd`
cd $ISOPATH

# Test ability to detect NCZarr/Zarr files

URL="${NCZARR_S3_TEST_HOST}/${NCZARR_S3_TEST_BUCKET}"
KEY="${S3ISOPATH}"

sometestfailed=

testfailed() {
  if test "x$1" != "x-51" ; then
    echo "*** Failed"
    sometestfailed=1
  fi
}

# Make test sets
rm -fr ./ref_notzarr.tar.gz ./ref_notzarr.tar
cp ${srcdir}/ref_notzarr.tar.gz .
gunzip ref_notzarr.tar.gz
tar -xf ref_notzarr.tar
if test "x$FEATURE_S3TESTS" = xyes ; then
    ${execdir}/s3util -f notzarr.file/notzarr.txt -u "https://${URL}" -k "${S3ISOPATH}/notzarr.s3/notzarr.txt" upload
fi

echo "Test empty file"
RET=`${execdir}/test_notzarr "file://empty.file#mode=zarr,file"`
testfailed "$RET"
echo "Test non-zarr file"
RET=`${execdir}/test_notzarr "file://notzarr.file#mode=zarr,file"`
testfailed "$RET"

if test "x$FEATURE_NCZARR_ZIP" = xyes ; then
echo "Test empty zip file"
RET=`${execdir}/test_notzarr "file://empty.zip#mode=zarr,zip"`
testfailed "$RET"
echo "Test non-zarr zip file"
RET=`${execdir}/test_notzarr "file://notzarr.zip#mode=zarr,zip"`
testfailed "$RET"
fi

if test "x$FEATURE_S3TESTS" = xyes ; then
if test 1 = 0 ; then
  # This test is NA for S3
  echo "Test empty S3 file"
  KEY2="${KEY}/empty.s3"
  RET=`${execdir}/test_notzarr "https://$URL${KEY2}#mode=zarr,s3"`
  testfailed "$RET"
fi
echo "Test non-zarr S3 file"
RET=`${execdir}/test_notzarr "https://$URL/${S3ISOPATH}/notzarr.s3#mode=zarr,s3"`
testfailed "$RET"
fi
