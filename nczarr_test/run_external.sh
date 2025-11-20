#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi 
. ../test_common.sh

set -e

. "$srcdir/test_nczarr.sh"

s3isolate "testdir_external"
THISDIR=`pwd`
cd $ISOPATH

# This shell script tests reading of
# publicly accessible S3 zarr datasets.

TESTCASES=
if test "x$FEATURE_BYTERANGE" = xyes && test "x$FEATURE_S3" = xyes && test "x$FP_ISCYGWIN" = x ; then
TESTCASES="${TESTCASES} OR_ABI;;;http://s3.amazonaws.com/noaa-goes16/ABI-L1b-RadF/2022/001/18/OR_ABI-L1b-RadF-M6C01_G16_s20220011800205_e20220011809513_c20220011809562.nc#mode=bytes,s3"
TESTCASES="${TESTCASES} cesmLE;blosc;;http://s3.us-west-2.amazonaws.com/ncar-cesm-lens/atm/daily/cesmLE-20C-FLNS.zarr#mode=zarr,s3"
# Test TEST_REGION defaulting and s3 inferencing
TESTCASES="${TESTCASES} wtk;;us-west-2;https://nrel-pds-wtk.s3.amazonaws.com/wtk-techno-economic/pywtk-data/met_data/0/0.nc#mode=bytes"
fi

testcase() {
NM=`echo "$1" | cut -d';' -f1`
FILT=`echo "$1" | cut -d';' -f2`
REG=`echo "$1" | cut -d';' -f3`
URL=`echo "$1" | cut -d';' -f4`
echo "*** Test: $NM = $URL"
rm -f "tmp_external_$NM.cdl"
if test "x$FILT" != x ; then
  if avail $FILT; then
    if test "x$REG" != x ; then export TEST_REGION="$REG"; fi
    ${NCDUMP} -h -s -n $NM $URL > "tmp_external_${NM}.cdl"
    unset TEST_REGION
  fi
fi
}

if test "x$FEATURE_S3" = xyes ; then
for t in $TESTCASES ; do
testcase "$t"
done
fi
