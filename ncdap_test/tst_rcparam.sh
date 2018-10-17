#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

# Figure our server; if none, then just stop
SVC=`${execdir}/findtestserver dap2 dts`

if test "x$SVC" = "x" ; then
echo "WARNING: Cannot locate test server"
exit
fi

URL="${SVC}/dts/group.test1"
URL="[HTTP.VERBOSE=1]$URL"
#URL="[log][show=fetch]$URL"

rm -f ./tmp_tst_rcparam
${NCDUMP} "${URL}" > ./tmp_tst_rcparam 2>&1

# Verify result by looking for VERBOSE output markers
TAGS=`sed -e '/STATE: INIT/p' -e 'd' tmp_tst_rcparam`
if test "x$TAGS" != x ; then
echo '*** PASS'
code=0
else
echo '*** FAIL'
code=1
fi

#cleanup
rm -f ./tmp_tst_rcparam
exit $code

