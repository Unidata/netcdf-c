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
  zext=file
  rm -fr ./ref_noshape.file
  unzip ${srcdir}/ref_noshape.file.zip
  fileargs ${ISOPATH}/ref_noshape "mode=zarr,$zext"
  rm -f tmp_noshape1_${zext}.cdl
  ${NCDUMP} $flags $fileurl > tmp_noshape1_${zext}.cdl
}

testnoshape2() {
  # Test against the original issue URL
  rm -f tmp_noshape2_gs.cdl
  fileurl="https://storage.googleapis.com/cmip6/CMIP6/CMIP/NASA-GISS/GISS-E2-1-G/historical/r1i1p1f1/day/tasmin/gn/v20181015/#mode=zarr,s3&aws.profile=no"
  ${NCDUMP} -h $flags $fileurl > tmp_noshape2_gs.cdl
}


test_default_numpy_scalars_and_empty() {
  # in zarr + numpy an adimensional array is a scalar - check issue #3108
  # it tests these cases
  # >>> np.array(1).shape
  # ()
  # >>> np.array(1.).shape
  # ()
  # >>> np.array([]).shape
  # (0,)
  # >>> np.array([[]]).shape
  # (1, 0)

  name='numpy_scalar_array.zarr'
  
  mkdir -p $name
  cat > "$name/.zmetadata" <<-'EOF'
{
    "metadata": {
        ".zgroup": { "zarr_format": 2 },
        "empty_list/.zarray": {
            "chunks": [1], "compressor": null, "dtype": "<f8", "fill_value": 0.0, "filters": null, "order": "C",
            "shape": [0], "zarr_format": 2
        },
        "list_empty_list/.zarray": {
            "chunks": [1,1], "compressor": null, "dtype": "<f8", "fill_value": 0.0, "filters": null, "order": "C",
            "shape": [1,0], "zarr_format": 2
        },
        "scalar/.zarray": {
            "chunks": [], "compressor": null, "dtype": "<f8", "fill_value": 0.0, "filters": null, "order": "C",
            "shape": [], "zarr_format": 2
        },
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
netcdf numpy_scalar_array {
dimensions:
   _Anonymous_Dim_0 = UNLIMITED ; // (0 currently)
   _Anonymous_Dim_1 = 1 ;
variables:
   double empty_list(_Anonymous_Dim_0) ;
   double float_scalar ;
           float_scalar:_FillValue = 0. ;
   int64 int_scalar ;
           int_scalar:_FillValue = 0LL ;
   double list_empty_list(_Anonymous_Dim_1, _Anonymous_Dim_0) ;
   double scalar ;
           scalar:_FillValue = 0. ;
}
EOF

  # Ensure variable is shown
  ${NCDUMP} -h ${flags} "file://${name}#mode=zarr,file,consolidated" > result_${name}.out

  diff -w result_${name}.out expected_${name}.out
}

# run tests
test_default_numpy_scalars_and_empty
testnoshape1
if test "x$FEATURE_S3TESTS" = xyes && test "x$FEATURE_S3_INTERNAL" = xyes ; then
    # The aws-sdk-cpp driver does not support google storage
    testnoshape2
fi
