#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi 
. ../test_common.sh

. "$srcdir/test_nczarr.sh"

# This shell script tests compatibility between
# this implementation and other implementations
# by means of files constructed by that other implementation

set -e

testcasefile() {
  zext=file
  ref=$1
  mode=$2
  if test "x$3" = xmetaonly ; then flags="-h"; fi
  fileargs ${execdir}/$ref "mode=$mode,$zext"
  rm -f tmp_${ref}_${zext}.cdl
  ${NCDUMP} $flags $fileurl > tmp_${ref}_${zext}.cdl
  diff -b ${srcdir}/${ref}.cdl tmp_${ref}_${zext}.cdl
}

testcasezip() {
  zext=zip
  ref=$1
  mode=$2
  if test "x$3" = xmetaonly ; then flags="-h"; fi
  fileargs ${execdir}/$ref "mode=$mode,$zext"
  rm -f tmp_${ref}_${zext}.cdl
  ${NCDUMP} $flags $fileurl > tmp_${ref}_${zext}.cdl
  diff -b ${srcdir}/${ref}.cdl tmp_${ref}_${zext}.cdl
}

testallcases() {
zext=$1
case "$zext" in 
    file)
	# need to unpack
	rm -fr ref_power_901_constants ref_power_901_constants.file
	unzip ${srcdir}/ref_power_901_constants.zip > /dev/null
	mv ref_power_901_constants ref_power_901_constants.file
	testcasefile ref_power_901_constants zarr metaonly; # test xarray as default
	;;
    zip)
	# Move into position
	if test "x$srcdir" != "x$execdir" ; then
	    cp ${srcdir}/ref_power_901_constants.zip ${execdir}
	    cp ${srcdir}/ref_quotes.zip ${execdir}
	fi
	testcasezip ref_power_901_constants xarray metaonly
	# Test large constant interoperability 
	testcasezip ref_quotes zarr metaonly
	;;
    *) echo "unimplemented kind: $1" ; exit 1;;
esac
}

testallcases file
if test "x$FEATURE_NCZARR_ZIP" = xyes ; then testallcases zip; fi
#No examples yet: if test "x$FEATURE_S3TESTS" = xyes ; then testallcases s3; fi

exit 0
