#!/bin/sh
# This shell script tests the output several previous tests.
# Ed Hartnett

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

FILE=tst_interops2.h4

ECODE=0

echo ""
echo "*** Testing extended file format output."
set -e

echo "Creating HDF4 file"
${execdir}/tst_interops2

echo "Test extended format output for a HDF4 file"
rm -f tmp_tst_formatx_hdf4
${NCDUMP} -K $FILE >tmp_tst_formatx_hdf4
if ! fgrep 'HDF4 mode=00001000' <tmp_tst_formatx_hdf4 ; then
TMP=`cat tmp_tst_formatx_hdf4`
echo "*** Fail: extended format for an HDF4 file: result=" $TMP
ECODE=1
fi

rm -f tmp_tst_formatx_hdf4

exit $ECODE


