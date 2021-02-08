#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi 
. ../test_common.sh

. "$srcdir/test_nczarr.sh"

set -e

echo ""
echo "*** Remove /netcdf-c from S3 repository"

fileargs netcdf-c

if test "x$FEATURE_S3TESTS" = xyes ; then
  ${execdir}/s3util -u "${NCZARR_S3_TEST_URL}" -k "/netcdf-c" clear
fi

exit 0
