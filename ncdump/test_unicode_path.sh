#!/bin/sh
#
# Test to make sure ncdump works with a subdirectory which starts
# with a unicode character.
# See https://github.com/Unidata/netcdf-c/issues/1666 for more information.
# Ward Fisher

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

set -e

# Passing a utf8 name using either \x or actual characters
# to Visual Studio does not work well.
if test "x$FP_ISMSVC" = x ; then
#UNISTRING='\xe6\xb5\xb7'
UNISTRING='海'
else
UNISTRING='海'
fi

UNIFILE="tst_utf8_${UNISTRING}"

echo ""

echo "*** Generating netcdf-3 binary file ${UNIFILE}.nc ..."
${NCGEN} -3 -b -o "${UNIFILE}.nc" "${srcdir}/ref_tst_utf8.cdl"
echo "*** Accessing binary file ${UNIFILE}.nc ..."
${NCDUMP} -h "${UNIFILE}.nc"

if test "x$FEATURE_HDF5" = xyes ; then
echo "*** Generating netcdf-4 binary file ${UNIFILE}.nc ..."
rm -f "${UNIFILE}.nc"
${NCGEN} -4 -b -o "${UNIFILE}.nc" "${srcdir}/ref_tst_utf8.cdl"
echo "*** Accessing binary file ${UNIFILE}.nc ..."
${NCDUMP} -h "${UNIFILE}.nc"
fi

# This test was moved here from tst_nccopy4.sh
# to unify all the unicode path tests
echo "*** Test nccopy ${UNIFILe} copy_of_${UNIFILE} ..."
${NCCOPY} ${UNIFILE}.nc copy_of_${UNIFILE}.nc
${NCDUMP} -n copy_of_${UNIFILE} ${UNIFILE}.nc > tmp_${UNIFILE}.cdl
${NCDUMP} copy_of_${UNIFILE}.nc > copy_of_${UNIFILE}.cdl
echo "*** compare " with copy_of_${UNIFILE}.cdl
diff copy_of_${UNIFILE}.cdl tmp_${UNIFILE}.cdl
rm copy_of_${UNIFILE}.nc copy_of_${UNIFILE}.cdl tmp_${UNIFILE}.cdl

echo "Test Passed. Cleaning up."
rm ${UNIFILE}.nc
