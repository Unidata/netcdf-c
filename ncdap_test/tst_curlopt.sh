#!/bin/sh

# These tests are intended to be run only manually.
# The idea is to gdb ncdump and check that the CURLOPT flags
# is being processed correctly.
# As a rule, you will need to set the breakpoint in
# ocget_rcproperties

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

# Figure our server; if none, then just stop
SVC=`${execdir}/findtestserver dap2 dts`
TESTCASE=test.01

if test "x$SVC" = "x" ; then
echo "WARNING: Cannot locate test server"
exit
fi

URL="[log]${SVC}/${TESTCASE}"

# Create the .daprc file
rm -f ./.daprc
echo '# tst_curlopt.sh' > ./.daprc
echo 'HTTP.READ.BUFFERSIZE=max' >> ./.daprc
echo 'HTTP.KEEPALIVE=60/60' >> ./.daprc
gdb --args ${NCDUMP} "${URL}"

# cleanup
rm -f ./.daprc

exit
