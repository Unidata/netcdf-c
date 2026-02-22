#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

set -e

. "$srcdir/test_nczarr.sh"

s3isolate "testdir_s3_credentials"
THISDIR=`pwd`
cd $ISOPATH

testcase_public_data_invalid_env() {
  # ncdump is expected to fail because we are authenticatin
  local cmd="${NCDUMP} -h gs3://weatherbench2/datasets/era5/1959-2023_01_10-wb13-6h-1440x721_with_derived_variables.zarr#mode=zarr,s3,consolidated"
  ${cmd} > /dev/null
  if env -i AWS_ACCESS_KEY_ID=INVALID AWS_SECRET_ACCESS_KEY=INVALID "${cmd}" 2>&1 | grep -q 'NetCDF: Authorization failure'; then
    echo "The operation should NOT have succeeded due to authentication: ${cmd}"
    return 1
  fi
}

if test "x$FEATURE_S3" = xyes ; then
 testcase_public_data_invalid_env
fi
