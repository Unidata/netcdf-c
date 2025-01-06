#!/bin/sh

# Test various kinds of corrupted files


if test "x$srcdir" = x ; then srcdir=`pwd`; fi 
. ../test_common.sh

. "${srcdir}/test_nczarr.sh"

set -e

s3isolate "testdir_corrupt"
THISDIR=`pwd`
cd $ISOPATH

export NCLOGGING=WARN

testnoshape1() {
  zext=file
  unzip ${srcdir}/ref_noshape.file.zip
  fileargs ${ISOPATH}/ref_noshape "mode=zarr,$zext"
  rm -f tmp_noshape1_${zext}.cdl
  find "${ISOPATH}"
  ${ZMD} -h -t int $fileurl
  ${NCDUMP} $flags $fileurl > tmp_noshape1_${zext}.cdl
}

testnoshape2() {
  # Test against the original issue URL
  rm -f tmp_noshape2_gs.cdl
  fileurl="https://storage.googleapis.com/cmip6/CMIP6/CMIP/NASA-GISS/GISS-E2-1-G/historical/r1i1p1f1/day/tasmin/gn/v20181015/#mode=zarr,s3&aws.profile=no"
  ${ZMD} -h $fileurl > tmp_noshape2_gs.zmap
  diff -wb ${srcdir}/ref_cmip6.zmap tmp_noshape2_gs.zmap
  #  ${NCDUMP} -h $flags $fileurl > tmp_noshape2_gs.cdl -- will fail
}
 
testnoshape1
if test "x$FEATURE_S3TESTS" = xyes && test "x$FEATURE_S3_INTERNAL" = xyes ; then
    # The aws-sdk-cpp driver does not support google storage
    testnoshape2
fi
