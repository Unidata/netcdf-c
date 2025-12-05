#!/bin/sh

# This test is intended to test various XARRAY support features.

# Test Dim Location: XARRAY group creation:
# We have changed where XARRAY dimensions are created, so test this out.
# As before, unnamed dimensions are created as anonymous dimensions in root group.
# Now, named dimensions from XARRAY are created in the group immediately containing
# a variable instead of in the root group. This still leaves room for failure if multiple
# variables in the group map dimname -> shape inconsistently.
# Note we test only for file type because this feature is supported outside of zmaps.

if test "x$srcdir" = x ; then srcdir=`pwd`; fi 
. ../test_common.sh

. "$srcdir/test_nczarr.sh"

set -e

s3isolate "testdir_xarray"
THISDIR=`pwd`
cd $ISOPATH


testdimlocation() {
  zext=file
  fileargs "tmp_xarraygroup" "mode=xarray,${zext}"
  ${NCGEN} -4 -lb -o ${fileurl} ${srcdir}/ref_xarraygroup.cdl
  ${NCDUMP} $flags $fileurl > tmp_xarraygroup_${zext}.cdl
  diff -b ${srcdir}/ref_xarraygroup.cdl tmp_xarraygroup.cdl
}

# Test Anon Dim Location: Anonymous dimension creation location:
# Anonymous dimensions are created in the root group; verify/
testanonlocation() {
  zext=file
  fileargs "tmp_xarrayanon" "mode=zarr,noxarray"
  # Create a simple zarr file
  ${NCGEN} -4 -lb -o ${fileurl} ${srcdir}/ref_xarrayanon.cdl
  ${NCDUMP} $flags $fileurl > tmp_xarrayanon_${zext}.cdl
  diff -b ${srcdir}/ref_xarrayanon.cdl tmp_xarrayanon_${zext}.cdl
}

#testdimlocation
testanonlocation

exit

