#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi 
. ../test_common.sh

. "$srcdir/test_nczarr.sh"

# This shell script tests compatibility between
# this implementation and other implementations
# by means of files constructed by that other implementation

set -e

UH="${NCZARR_S3_TEST_HOST}"
UB="${NCZARR_S3_TEST_BUCKET}"


testcasefile() {
  zext=file
  base=$1
  mode=$2
  metaonly=$3
  if test "x$metaonly" = xmetaonly ; then flags="-h"; fi
  fileargs ${builddir}/ref_$base "mode=$mode,$zext"
  rm -f tmp_${base}_${zext}.cdl
  ${NCDUMP} $flags $fileurl > tmp_${base}_${zext}.cdl
  diff -b ${srcdir}/ref_${base}.cdl tmp_${base}_${zext}.cdl
}

testcasezip() {
  zext=zip
  base=$1
  mode=$2
  fileargs ${builddir}/ref_$base "mode=$mode,$zext"
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
  diff -b ${builddir}/ref_${base}.cdl tmp_${base}_${zext}.cdl
}

testallcases() {
zext=$1
case "$zext" in 
    file)
	# need to unpack
	rm -fr ref_power_901_constants ref_power_901_constants.file
        if ! test -f ${builddir}/ref_power_901_constants_orig.zip ; then
          cp -f ${srcdir}/ref_power_901_constants_orig.zip ${builddir}/ref_power_901_constants_orig.zip
	fi
	unzip ${builddir}/ref_power_901_constants_orig.zip > /dev/null
	mv ${builddir}/ref_power_901_constants ${builddir}/ref_power_901_constants.file
	testcasefile power_901_constants zarr metaonly; # test xarray as default
	;;
    zip)
	# Move into position
        if ! test -f ${builddir}/ref_power_901_constants.zip ; then
          cp -f ${srcdir}/ref_power_901_constants_orig.zip ${builddir}/ref_power_901_constants.zip
        fi
        if ! test -f ${builddir}/ref_quotes.zip ; then
          cp -f ${srcdir}/ref_quotes_orig.zip ${builddir}/ref_quotes.zip
	fi
	testcasezip power_901_constants xarray metaonly
	# Test large constant interoperability 
	testcasezip quotes zarr metaonly
	;;
    s3)
	# Read a test case created by netcdf-java zarr.
	# Move into position
        rm -f ${builddir}/ref_zarr_test_data.cdl
        if ! test -f ${builddir}/ref_zarr_test_data.cdl.gz ; then
	  cp -f ${srcdir}/ref_zarr_test_data.cdl.gz ${builddir}/ref_zarr_test_data.cdl.gz 
	fi
	# Use gunzip because it always appears to be available
        gunzip -c ${builddir}/ref_zarr_test_data.cdl.gz > ${builddir}/ref_zarr_test_data.cdl
        testcases3 zarr_test_data xarray
 	;;
    *) echo "unimplemented kind: $1" ; exit 1;;
esac
}

testallcases file
if test "x$FEATURE_NCZARR_ZIP" = xyes ; then testallcases zip; fi
if test "x$FEATURE_S3TESTS" = xyes ; then testallcases s3; fi

# Cleanup
rm -fr ${builddir}/ref_power_901_constants_orig.zip
rm -fr ${builddir}/ref_zarr_test_data.cdl.gz

exit 0
