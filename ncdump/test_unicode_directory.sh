#!/bin/sh
#
# Test to make sure ncdump works with a subdirectory which starts
# with a unicode character.
# See https://github.com/Unidata/netcdf-c/issues/1666 for more information.
# Ward Fisher

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

ERR() {
    RES=$?
    if [ $RES -ne 0 ]; then
        echo "Error found: $RES"
        exit $RES
    fi
}

#UNISTRING=$(echo '\xe6\xb5\xb7')
UNISTRING='海'

echo ""
echo "Creating Unicode String Directory ${UNISTRING}"
mkdir -p "${UNISTRING}"; ERR
ls -ld "${UNISTRING}"

# Do test for netcdf-3 and (optionally) netcdf-4

echo "*** Generating netcdf-3 binary file ${UNISTRING}/tst_utf.nc..."
${NCGEN} -b -o "${UNISTRING}/tst_utf.nc" "${srcdir}/ref_tst_utf8.cdl"; ERR
echo "*** Accessing binary file ${UNISTRING}/tst_utf.nc..."
${NCDUMP} -h "${UNISTRING}/tst_utf.nc"; ERR

if test "x$FEATURE_HDF5" = xyes ; then
echo "*** Generating netcdf-4 binary file ${UNISTRING}/tst_utf.nc..."
rm -f "${UNISTRING}/tst_utf.nc"
${NCGEN} -4 -b -o "${UNISTRING}/tst_utf.nc" "${srcdir}/ref_tst_utf8.cdl"; ERR
echo "*** Accessing binary file ${UNISTRING}/tst_utf.nc..."
${NCDUMP} -h "${UNISTRING}/tst_utf.nc"; ERR
fi

echo "Test Passed. Cleaning up."
rm -fr "${UNISTRING}"; ERR
