#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi 
. ../test_common.sh

# This shell script tests ncdump and ncgen on netCDF-4 variables with multiple 
# unlimited dimensions.

set -e

if test "x$TESTNCZARR" = x1 ; then
. "$srcdir/test_nczarr.sh"
s3isolate "testdir_mud4"
else
isolate "testdir_mud4"
ISOPATH=`pwd`
fi
THISDIR=`pwd`
cd $ISOPATH

echo ""
echo "*** Testing ncdump output for multiple unlimited dimensions"

# This is where the ref files are kept
refdir="${srcdir}/../ncdump"

testcase() {
zext=$1

if test "x$TESTNCZARR" = x1 ; then
fileargs "tmp_mud4_${zext}"
deletemap $zext $file
file="$fileurl"
else
file="tmp_mud4_${zext}.nc"
rm -f $file
fi

echo "*** creating netcdf file $file from ref_tst_mud4.cdl ..."
${NCGEN} -4 -b -o $file $refdir/ref_tst_mud4.cdl
echo "*** creating tmp_mud4.cdl from $file ..."
${NCDUMP} -n tst_mud4 $file > tmp_mud4.cdl
# echo "*** comparing tst_mud4.cdl with ref_tst_mud4.cdl..."
diff -b tmp_mud4.cdl $refdir/ref_tst_mud4.cdl
# echo "*** comparing annotation from ncdump -bc $file with expected output..."
${NCDUMP} -n tst_mud4 -bc $file > tmp_mud4-bc.cdl
diff -b tmp_mud4-bc.cdl $refdir/ref_tst_mud4-bc.cdl

# Now test with char arrays instead of ints
if test "x$TESTNCZARR" = x1 ; then
fileargs "tmp_mud4_chars${zext}"
deletemap $zext $file
file="$fileurl"
else
file="tmp_mud4_chars${zext}.nc"
rm -f $file
fi
echo "*** creating netcdf file $file from ref_tst_mud4_chars.cdl ..."
${NCGEN} -4 -b -o $file $refdir/ref_tst_mud4_chars.cdl
echo "*** creating ${file}.cdl from $file ..."
${NCDUMP} -n tst_mud4_chars $file > tmp_mud4_chars.cdl
# echo "*** comparing tmp_mud4_chars.cdl with ref_tst_mud4_chars.cdl..."
diff -b tmp_mud4_chars.cdl $refdir/ref_tst_mud4_chars.cdl
if test 1 = 0 ; then
  # unused
  echo "*** comparing annotation from ncdump -bc tst_mud4_chars.nc with expected output..."
  ${NCDUMP} -n tst_mud4_chars -bc $file > tmp_mud4_chars-bc.cdl
  diff -b tmp_mud4_chars-bc.cdl $refdir/ref_tst_mud4_chars-bc.cdl
  echo "*** All ncdump test output for multiple unlimited dimensions passed!"
fi
}

if test "x$TESTNCZARR" = x1 ; then
    testcase file
    if test "x$FEATURE_NCZARR_ZIP" = xyes ; then testcase zip ; fi
    if test "x$FEATURE_S3TESTS" = xyes ; then testcase s3 ; fi
else
    testcase nc
fi

exit 0
