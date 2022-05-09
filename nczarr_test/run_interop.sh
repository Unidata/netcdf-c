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
  ref=$1
  mode=$2
  metaonly=$3
  if test "x$metaonly" = xmetaonly ; then flags="-h"; fi
  fileargs ${execdir}/$ref "mode=$mode,$zext"
  rm -f tmp_${ref}_${zext}.cdl
  ${NCDUMP} $flags $fileurl > tmp_${ref}_${zext}.cdl
  diff -b ${srcdir}/${ref}.cdl tmp_${ref}_${zext}.cdl
}

testcasezip() {
  zext=zip
  ref=$1
  mode=$2
  fileargs $ref "mode=$mode,$zext"
  rm -f tmp_${ref}_${zext}.cdl
  ${NCDUMP} -h $flags $fileurl > tmp_${ref}_${zext}.cdl
  diff -b ${srcdir}/${ref}.cdl tmp_${ref}_${zext}.cdl
}

testcases3() {
  zext=s3
  zarr=$1
  ref=$2
  mode=$3
  rm -f tmp_${zarr}_${zext}.cdl
  url="https://${UH}/${UB}/${zarr}#mode=${mode},s3"
  ${NCDUMP} $url > tmp_${zarr}_${zext}.cdl
  diff -b ${srcdir}/${ref}.cdl tmp_${zarr}_${zext}.cdl
}

testallcases() {
zext=$1
case "$zext" in 
    file)
	# need to unpack
	rm -fr ref_power_901_constants ref_power_901_constants.file
	unzip ${srcdir}/ref_power_901_constants_orig.zip > /dev/null
	mv ref_power_901_constants ref_power_901_constants.file
	testcasefile ref_power_901_constants zarr metaonly; # test xarray as default
	;;
    zip)
	# Move into position
        cp ${srcdir}/ref_power_901_constants_orig.zip ${execdir}/ref_power_901_constants.zip
        cp ${srcdir}/ref_quotes_orig.zip ${execdir}/ref_quotes.zip
	testcasezip ref_power_901_constants xarray metaonly
	# Test large constant interoperability 
	testcasezip ref_quotes zarr metaonly
	;;
    s3)
	# Read a test case created by netcdf-java zarr.
	# Move into position
        rm -f ${execdir}/ref_zarr_test_data.cdl
	# Use gunzip because it always appears to be available
	if ! test -f ${srcdir}/ref_zarr_test_data.cdl ; then
            gunzip -c ${srcdir}/ref_zarr_test_data.cdl.gz > ${srcdir}/ref_zarr_test_data.cdl
	fi
        testcases3 zarr_test_data.zarr ref_zarr_test_data xarray
 	;;
    *) echo "unimplemented kind: $1" ; exit 1;;
esac
}

testallcases file
if test "x$FEATURE_NCZARR_ZIP" = xyes ; then testallcases zip; fi
if test "x$FEATURE_S3TESTS" = xyes ; then testallcases s3; fi
exit
# Cleanup
rm -fr ${execdir}/ref_power_901_constants.file
rm -f ${execdir}/ref_zarr_test_data.cdl

exit 0
