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

LC_ALL="C.UTF-8"
export LC_ALL

#UNISTRING='海'
UNISTRING=$(echo '\xe6\xb5\xb7')

echo ""
echo "Creating Unicode String Directory ${UNISTRING}"
mkdir -p ${UNISTRING}; ERR

echo "*** Generating binary file ${UNISTRING}/tst_utf.nc..."
${NCGEN} -b -o "${UNISTRING}/tst_utf.nc" "${srcdir}/ref_tst_utf8.cdl"; ERR
echo "*** Accessing binary file ${UNISTRING}/tst_utf.nc..."
${NCDUMP} -h "${UNISTRING}/tst_utf.nc"; ERR

echo "Test Passed. Cleaning up."
rm "${UNISTRING}/tst_utf.nc"; ERR
rmdir "${UNISTRING}"; ERR
