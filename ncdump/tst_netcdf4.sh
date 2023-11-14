#!/bin/sh
# This shell script tests ncdump for netcdf-4
# Ed Hartnett, Dennis Heimbigner, Ward Fisher

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

. $srcdir/test_ncdump.sh

isolate "testdir_nccopy4"
THISDIR=`pwd`
cd $ISOPATH

set -e

# Create common test inputs
createtestinputs

echo ""
echo "*** Testing ncgen and ncdump for netCDF-4 format."

${NCGEN} -k nc4 -b -o tst_netcdf4_c0_4.nc ${ncgenc04} ;ERR
${NCDUMP} -n c1 tst_netcdf4_c0_4.nc | sed 's/e+0/e+/g' > tst_netcdf4_c1_4.cdl ; ERR
diff -b tst_netcdf4_c1_4.cdl $srcdir/ref_ctest1_nc4.cdl ; ERR

echo "*** Creating test output tst_netcdf4_c0.nc."
${NCGEN} -k nc7 -b -o tst_netcdf4_c0.nc ${ncgenc0} ; ERR

echo "*** Testing that program tst_h_rdc0 can read tst_netcdf4_c0.nc."
${execdir}/tst_h_rdc0 ; ERR

if test -f tst_roman_szip_simple.nc; then
  echo "*** Testing szip compression."    
  ${NCDUMP} tst_roman_szip_simple.nc | sed 's/e+0/e+/g' > tst_roman_szip_simple.cdl ; ERR
  diff -b tst_roman_szip_simple.cdl $srcdir/ref_roman_szip_simple.cdl ; ERR
  ${NCDUMP} tst_roman_szip_unlim.nc | sed 's/e+0/e+/g' > tst_roman_szip_unlim.cdl ; ERR
  diff -b tst_roman_szip_unlim.cdl $srcdir/ref_roman_szip_unlim.cdl ; ERR
fi

echo "*** Testing -v option with absolute name and groups..."
${NCDUMP} -v g2/g3/var tst_group_data.nc | sed 's/e+0/e+/g' > tst_group_data.cdl ; ERR
diff -b tst_group_data.cdl $srcdir/ref_tst_group_data_v23.cdl ; ERR

echo "*** Running tst_enum_undef.c to create test files."
${execdir}/tst_enum_undef ; ERR
${NCDUMP} tst_enum_undef.nc | sed 's/e+0/e+/g' > tst_enum_undef.cdl ; ERR
diff -b tst_enum_undef.cdl $srcdir/ref_tst_enum_undef.cdl ; ERR

# This creates a memory leak 
if test 0 = 1 ; then
echo "*** Running tst_vlen_data.c to create test files."
if ! ${execdir}/tst_vlen_data ; then if test $? != 027 ; then ERR; fi; fi
${NCDUMP} tst_vlen_data.nc | sed 's/e+0/e+/g' > tst_vlen_data.cdl ; ERR
diff -b tst_vlen_data.cdl $srcdir/ref_tst_vlen_data.cdl ; ERR
fi

#echo ""
#echo "*** Testing ncdump on file with corrupted header "
#rm -f ./ignore_tst_netcdf4
#if ${NCDUMP} ${srcdir}/ref_test_corrupt_magic.nc &> ./ignore_tst_netcdf4 ; then
#echo "***Fail: ncdump should have failed on ref_test_corrupt_magic.nc"
#else
#echo "***XFail: ncdump properly failed on ref_test_corrupt_magic.nc"
#fi
#rm -fr ./ignore_tst_netcdf4

# This should work, but does not. See github issue 982.
#echo "*** creating tst_output_irish_rover.cdl from ref_tst_irish_rover.nc..."
#${NCDUMP} ref_tst_irish_rover.nc > tst_output_irish_rover.cdl

rm -f *.tmp
echo "*** All ncgen and ncdump test output for netCDF-4 format passed!"
exit 0
