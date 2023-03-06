#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi 
. ../test_common.sh

. "$srcdir/test_nczarr.sh"

set -e

echo "*** Test: Github issues #2063, #2062, #2059"

testcase2059() {
zext=$1
echo "*** Test: Github issue #2059"
fileargs tmp_groups_regular "mode=zarr,$zext"
deletemap $zext $file
${NCCOPY} ${srcdir}/ref_groups.h5 "$fileurl"
rm -f tmp_nczfill.cdl
${ZMD} -h "$fileurl"
${NCDUMP} -s -n tmp_groups_regular "$fileurl" > tmp_nczfill.cdl
sclean tmp_nczfill.cdl tmp_groups_regular_$zext.cdl
diff -wb ${srcdir}/ref_groups_regular.cdl tmp_groups_regular_$zext.cdl
}

testcase2062() {
zext=$1
echo "*** Test: Github issue #2062"
rm -fr ref_byte.zarr
unzip ${srcdir}/ref_byte.zarr.zip
rm -fr tmp_nczfill.cdl
${ZMD} -h "file://ref_byte.zarr#mode=zarr,$zext"
${NCDUMP} -s "file://ref_byte.zarr#mode=zarr,$zext" > tmp_nczfill.cdl
sclean tmp_nczfill.cdl tmp_byte_$zext.cdl
diff -wb ${srcdir}/ref_byte.cdl tmp_byte_$zext.cdl
rm -fr ref_byte.zarr
}

testcase2063() {
zext=$1
echo "*** Test: Github issue #2063"
rm -fr ref_byte_fill_value_null.zarr
unzip ${srcdir}/ref_byte_fill_value_null.zarr.zip
rm -fr tmp_nczfill.cdl
${ZMD} -h "file://ref_byte_fill_value_null.zarr#mode=zarr,$zext"
${NCDUMP} -s "file://ref_byte_fill_value_null.zarr#mode=zarr,$zext" > tmp_nczfill.cdl
sclean tmp_nczfill.cdl tmp_byte_fill_value_null_$zext.cdl
diff -wb ${srcdir}/ref_byte_fill_value_null.cdl tmp_byte_fill_value_null_$zext.cdl
rm -fr ref_byte_fill_value_null.zarr
}


testcase2062 file
testcase2063 file
if test "x$FEATURE_HDF5" = xyes ; then
  testcase2059 file
  if test "x$FEATURE_NCZARR_ZIP" = xyes ; then
    testcase2059 zip
  fi
  if test "x$FEATURE_S3TESTS" = xyes ; then
    testcase2059 s3
  fi
fi

exit 0
