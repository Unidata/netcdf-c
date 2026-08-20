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


test_numpy_scalars() {
  # in zarr + numpy an adimensional array is a scalar - check issue #3108
  # it tests these cases
  # >>> np.array(1).shape
  # ()
  # >>> np.array(1.).shape
  # ()

  name='numpy_scalars.zarr'
  
  mkdir -p $name
  cat > "$name/.zmetadata" <<-'EOF'
{
    "metadata": {
        ".zgroup": { "zarr_format": 2 },
        "float_scalar/.zarray": {
            "chunks": [], "compressor": null, "dtype": "<f8", "fill_value": 0.0, "filters": null, "order": "C",
            "shape": [], "zarr_format": 2
        },
        "int_scalar/.zarray": {
            "chunks": [], "compressor": null, "dtype": "<i8", "fill_value": 0, "filters": null, "order": "C",
            "shape": [], "zarr_format": 2
        }
    },
    "zarr_consolidated_format": 1
}
EOF

cat > expected_${name}.out <<-'EOF'
netcdf numpy_scalars {
variables:
   double float_scalar ;
           float_scalar:_FillValue = 0. ;
   int64 int_scalar ;
           int_scalar:_FillValue = 0LL ;
}
EOF

  # Ensure variable is shown
  ${NCDUMP} -h ${flags} "file://${name}#mode=zarr,file,consolidated" > result_${name}.out

  diff -w result_${name}.out expected_${name}.out

  # Test read and write
  if test "x${FEATURE_HDF5}" = xyes; then
    ${NCCOPY} "file://${name}#mode=zarr,file,consolidated" "${name}.nc"
    ${NCDUMP} -n numpy_scalars -h "${name}.nc" > copy_result_${name}.out
    diff -w copy_result_${name}.out expected_${name}.out
  fi
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

# run tests
test_bad_dtype
test_numpy_scalars
testnoshape1
if test "x$FEATURE_S3TESTS" = xyes && test "x$FEATURE_S3_INTERNAL" = xyes ; then
    # The aws-sdk-cpp driver does not support google storage
    testnoshape2
fi
