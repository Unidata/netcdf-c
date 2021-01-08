#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

. ${srcdir}/d4test_common.sh

set -e

echo "test_fillmismatch.sh:"

F="test_fillmismatch.nc"

URL='[dap4]file://'
URL="${URL}${srcdir}/misctestfiles/$F"

# First check that with [nofillmismatch], we get a failure
rm -f ./tmp_dap4_mismatch
URLNO="[nofillmismatch]$URL"
if ${NCDUMP} -h "${NOURL}" > ./tmp_dap4_mismatch 2>&1 ; then
echo "*** Fail: ${NCDUMP} ${NOURL} passed"
exit 1
else
echo "*** XFail: ${NCDUMP} ${NOURL} failed"
fi

# Now check that with [fillmismatch] (default), we get success
rm -f ./tmp_dap4_mismatch
if ${NCDUMP} -h "${URL}" > ./tmp_dap4_mismatch ; then
echo "*** Pass: ${NCDUMP} ${URL} passed"
else
echo "*** Fail: ${NCDUMP} ${URL} failed"
exit 1
fi

# Verify result
diff -w ${srcdir}/baselineraw/$F.dmp ./tmp_dap4_mismatch
#cleanup
rm -f ./tmp_dap4_mismatch
