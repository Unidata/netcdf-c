#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi

. ../test_common.sh

#
# Moving some netcdf-4 only tests here, out of tst_nccopy and run_utf8_tests.
# Without this, the tests fail when netcdf-4 is disabled.

set -e

echo ""

rm -f nc4_utf8.nc nc4_utf8.cdl
echo "*** creating enhanced file with utf8 characters..."
${NCGEN} -4 -b -o nc4_utf8.nc ${srcdir}/ref_tst_nc4_utf8_4.cdl
echo "*** dump and compare utf8 output..."
${NCDUMP} nc4_utf8.nc > nc4_utf8.cdl
diff -b -w nc4_utf8.cdl ${srcdir}/ref_tst_nc4_utf8_4.cdl
