#!/bin/sh

# Test various kinds of corrupted files


if test "x$srcdir" = x ; then srcdir=`pwd`; fi 
. ../test_common.sh

. "$srcdir/test_nczarr.sh"

set -e

s3isolate "testdir_corrupt"
THISDIR=`pwd`
cd $ISOPATH

export NCLOGGING=WARN

testnoshape1() {
  echo "*** testing no shape (zip)"
  zext=file
  rm -fr ./ref_noshape.file
  unzip ${srcdir}/ref_noshape.file.zip
  fileargs ${ISOPATH}/ref_noshape "mode=zarr,$zext"
  rm -f tmp_noshape1_${zext}.cdl
  ${NCDUMP} $flags $fileurl > tmp_noshape1_${zext}.cdl
}

testnoshape2() {
  echo "*** testing no shape (s3)"
  # Test against the original issue URL
  rm -f tmp_noshape2_gs.cdl
  fileurl="https://storage.googleapis.com/cmip6/CMIP6/CMIP/NASA-GISS/GISS-E2-1-G/historical/r1i1p1f1/day/tasmin/gn/v20181015/#mode=zarr,s3&aws.profile=no"
  ${NCDUMP} -h $flags $fileurl > tmp_noshape2_gs.cdl
}

test_bad_dtype() {
  echo "*** testing bad dtype"
  name='bad_dtype'
  mkdir -p "${name}.zarr/id"
  cat > "${name}.zarr/.zgroup" <<-EOF
{"zarr_format":2}
EOF
cat > "${name}.zarr/id/.zarray" <<-EOF
{
  "chunks": [225,265],
  "compressor": null,
  "dtype": "|O",
  "fill_value": null,
  "filters": null,
  "order": "C",
  "shape": [1799,1059],
  "zarr_format": 2
}
EOF
  set +e
  "${NCDUMP}" -h "file://${name}.zarr#mode=zarr" > ncdump_dtype_output.txt 2>&1
  ret=$?
  set -e
  if test $ret -ne 0 && grep -q 'ERR: Unsupported data type detected in variable "id" (dtype="|O").' ncdump_dtype_output.txt; then
    return 0;
  fi
  return 1;
}

test_bad_dtype
testnoshape1
if test "x$FEATURE_S3TESTS" = xyes && test "x$FEATURE_S3_INTERNAL" = xyes ; then
    # The aws-sdk-cpp driver does not support google storage
    testnoshape2
fi
