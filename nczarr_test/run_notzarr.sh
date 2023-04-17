#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi 
. ../test_common.sh

. "$srcdir/test_nczarr.sh"

# Test ability to detect NCZarr/Zarr files

URL="${NCZARR_S3_TEST_HOST}/${NCZARR_S3_TEST_BUCKET}"
KEY="/netcdf-c"

THISDIR=`pwd`
RESULTSDIR=tmp_notzarr
sometestfailed=

testfailed() {
  if test "x$1" != "x-51" ; then
    echo "*** Failed"
    sometestfailed=1
  fi
}

rm -fr ${RESULTSDIR}
mkdir -p ${RESULTSDIR}
cd ${RESULTSDIR}

# Make test sets
mkdir empty.file # empty
mkdir notzarr.file # non-empty, non-zarr
echo "random data" >notzarr.file/notzarr.txt
if test "x$FEATURE_NCZARR_ZIP" = xyes ; then
    mkdir empty
    zip -r empty.zip empty
    cp -r notzarr.file ./notzarr
    zip -r notzarr.zip notzarr
    rm -fr empty notzarr
fi
if test "x$FEATURE_S3TESTS" = xyes ; then
    cat /dev/null > empty.txt
    # not possible: ${execdir}/s3util -f notzarr.txt -u "https://${URL}" -k "/netcdf-c/empty.s3" upload
    ${execdir}/s3util -f notzarr.file/notzarr.txt -u "https://${URL}" -k "/netcdf-c/notzarr.s3/notzarr.txt" upload
fi

echo "Test empty file"
RET=`${execdir}/tst_notzarr "file://empty.file#mode=zarr,file"`
testfailed "$RET"
echo "Test non-zarr file"
RET=`${execdir}/tst_notzarr "file://notzarr.file#mode=zarr,file"`
testfailed "$RET"

if test "x$FEATURE_NCZARR_ZIP" = xyes ; then
echo "Test empty zip file"
RET=`${execdir}/tst_notzarr "file://empty.zip#mode=zarr,zip"`
testfailed "$RET"
echo "Test non-zarr zip file"
RET=`${execdir}/tst_notzarr "file://notzarr.zip#mode=zarr,zip"`
testfailed "$RET"
fi

if test "x$FEATURE_S3TESTS" = xyes ; then
if test 1 = 0 ; then
  # This test is NA for S3
  echo "Test empty S3 file"
  KEY="/netcdf-c/empty.s3"
  RET=`${execdir}/tst_notzarr "https://$URL${KEY}#mode=zarr,s3"`
  testfailed "$RET"
fi
echo "Test non-zarr S3 file"
RET=`${execdir}/tst_notzarr "https://$URL/netcdf-c/notzarr.s3#mode=zarr,s3"`
testfailed "$RET"
fi

cd ${THISDIR}

# Cleanup
rm -fr ${RESULTSDIR}
if test "x$FEATURE_S3TESTS" = xyes ; then
    awsdelete "/netcdf-c"
fi

exit 0
