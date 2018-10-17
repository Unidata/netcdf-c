#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

# Figure our server; if none, then just stop
SVC=`${execdir}/findtestserver4 dap4 d4ts`

if test "x$SVC" = "x" ; then
echo "WARNING: Cannot locate test server"
exit
fi

URL="${SVC}/d4ts/testfiles/test_one_var.nc"
URL="[HTTP.VERBOSE=1]$URL"
#URL="[log][show=fetch]$URL"

rm -f ./tmp_test_rcparam
${NCDUMP} "${URL}" > ./tmp_test_rcparam 2>&1

# Verify result by looking for VERBOSE output markers
TAGS=`sed -e '/STATE: INIT/p' -e 'd' tmp_test_rcparam`
if test "x$TAGS" != x ; then
echo '*** PASS'
code=0
else
echo '*** FAIL'
code=1
fi

#cleanup
rm -f ./tmp_test_rcparam
exit $code

