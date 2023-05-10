#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi 
. ../test_common.sh

. "$srcdir/test_nczarr.sh"

set -e

s3isolate "testdir_interop"
THISDIR=`pwd`
cd $ISOPATH

# This shell script tests compatibility between
# this implementation and other implementations
# by means of files constructed by that other implementation

UH="${NCZARR_S3_TEST_HOST}"
UB="${NCZARR_S3_TEST_BUCKET}"

testcasefile() {
  zext=file
  base=$1
  mode=$2
  metaonly=$3
  if test "x$metaonly" = xmetaonly ; then flags="-h"; fi
  fileargs ${ISOPATH}/ref_$base "mode=$mode,$zext"
  rm -f tmp_${base}_${zext}.cdl
  ${NCDUMP} $flags $fileurl > tmp_${base}_${zext}.cdl
  diff -b ${srcdir}/ref_${base}.cdl tmp_${base}_${zext}.cdl
}

testcasezip() {
  zext=zip
  base=$1
  mode=$2
  fileargs ${ISOPATH}/ref_$base "mode=$mode,$zext"
  rm -f tmp_${base}_${zext}.cdl
  ${NCDUMP} -h $flags $fileurl > tmp_${base}_${zext}.cdl
  diff -b ${srcdir}/ref_${base}.cdl tmp_${base}_${zext}.cdl
}

testcases3() {
  zext=s3
  base=$1
  mode=$2
  rm -f tmp_${base}_${zext}.cdl
  url="https://${UH}/${UB}/${base}.zarr#mode=${mode},s3"
  ${NCDUMP} $url > tmp_${base}_${zext}.cdl
  # Find the proper ref file
  diff -b ${ISOPATH}/ref_${base}.cdl tmp_${base}_${zext}.cdl
}

testallcases() {
zext=$1
case "$zext" in 
    file)
	# need to unpack
	unzip ref_power_901_constants.zip >> tmp_ignore.txt
	mv ${ISOPATH}/ref_power_901_constants ${ISOPATH}/ref_power_901_constants.file
	testcasefile power_901_constants zarr metaonly; # test xarray as default
	;;
    zip)
	# Move into position
	testcasezip power_901_constants xarray metaonly
	# Test large constant interoperability 
	testcasezip quotes zarr metaonly
	;;
    s3)
	# Read a test case created by netcdf-java zarr.
	# unpack
	# Use gunzip because it always appears to be available
        gunzip -c ${srcdir}/ref_zarr_test_data.cdl.gz > ${ISOPATH}/ref_zarr_test_data.cdl
        testcases3 zarr_test_data xarray
 	;;
    *) echo "unimplemented kind: $1" ; exit 1;;
esac
}

# common setup
if ! test -f ${ISOPATH}/ref_power_901_constants.zip ; then
  cp -f ${srcdir}/ref_power_901_constants_orig.zip ${ISOPATH}/ref_power_901_constants.zip
fi
if ! test -f ${ISOPATH}/ref_quotes.zip ; then
  cp -f ${srcdir}/ref_quotes_orig.zip ${ISOPATH}/ref_quotes.zip
fi

testallcases file
if test "x$FEATURE_NCZARR_ZIP" = xyes ; then testallcases zip; fi
if test "x$FEATURE_S3TESTS" = xyes ; then testallcases s3; fi

if test "x$FEATURE_S3TESTS" = xyes ; then s3sdkdelete "/${S3ISOPATH}" ; fi # Cleanup
