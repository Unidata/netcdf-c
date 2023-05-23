#!/bin/sh

#export NCPATHDEBUG=1

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

set -e

F="fillmismatch.nc"
EXPECTED="${srcdir}/expected3"

URL='file://'
URL="${URL}${srcdir}/testdata3/$F"

echo "Check that with [nofillmismatch], we get a failure"
NOURL="$URL#nofillmismatch"
if ${NCDUMP} "${NOURL}" > /dev/null ; then
echo "*** Fail: ${NCDUMP} ${NOURL} passed"
exit 1
else
echo "*** XFail: ${NCDUMP} ${NOURL} failed"
fi

echo "Check that with [fillmismatch] (default), we get success"
rm -f tmp_tst_mismatch
if ${NCDUMP} "${URL}" > tmp_tst_mismatch ; then
echo "*** Pass: ${NCDUMP} ${URL} passed"
else
echo "*** Fail: ${NCDUMP} ${URL} failed"
exit 1
fi

# Verify result
diff -w ${EXPECTED}/$F.dmp ./tmp_tst_mismatch
#cleanup
rm -f ./tmp_tst_mismatch

