#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi 
. ../test_common.sh

set -e

. "$srcdir/test_nczarr.sh"

s3isolate "testdir_external"
THISDIR=`pwd`
cd $ISOPATH

# This shell script tests reading of
# publically accessible S3 zarr datasets.

TESTCASES=
if test "x$FEATURE_BYTERANGE" = xyes && test "x$FEATURE_S3" = xyes && test "x$FP_ISCYGWIN" = x ; then
TESTCASES="${TESTCASES} OR_ABI;http://s3.amazonaws.com/noaa-goes16/ABI-L1b-RadF/2022/001/18/OR_ABI-L1b-RadF-M6C01_G16_s20220011800205_e20220011809513_c20220011809562.nc#mode=bytes,s3"
fi

testcase() {
NM=`echo "$1" | cut -d';' -f1`
URL=`echo "$1" | cut -d';' -f2`
echo "*** Test: $NM = $URL"
rm -f "tmp_external_$NM.cdl"
${NCDUMP} -h -n $NM $URL > "tmp_external_${NM}.cdl"
}

if test "x$FEATURE_S3" = xyes ; then
for t in $TESTCASES ; do
testcase "$t"
done
fi

if test "x$FEATURE_S3TESTS" = xyes ; then s3sdkdelete "/${S3ISOPATH}" ; fi # Cleanup
