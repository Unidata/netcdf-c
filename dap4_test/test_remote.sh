#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

. ${srcdir}/d4test_common.sh

set -e

echo "test_remote.sh:"

#BIG=1
#NOCSUM=1

computetestablefiles

setresultdir results_test_remote

TESTSERVER=`${execdir}/findtestserver4 dap4 d4ts`
if test "x$TESTSERVER" = x ; then
echo "***XFAIL: Cannot find d4ts testserver"
exit 1
fi

if test "x${RESET}" = x1 ; then rm -fr ${BASELINEREM}/*.ncdump ; fi
for f in $F ; do
    URL="[log][show=fetch][dap4]${TESTSERVER}/testfiles/${f}"
    if test "x$BIG" = x1; then
	URL="[ucar.littleendian=0]${URL}"
    fi
    if test "x$NOCSUM" = x1; then
	URL="[ucar.checksummode=none]${URL}"
    fi
    ${NCDUMP} ${DUMPFLAGS} "${URL}" > ${builddir}/results_test_remote/${f}.ncdump
    if test "x${TEST}" = x1 ; then
	diff -wBb "${BASELINEREM}/${f}.ncdump" "${builddir}/results_test_remote/${f}.ncdump"
    elif test "x${RESET}" = x1 ; then
	echo "${f}:" 
	cp "${builddir}/results_test_remote/${f}.ncdump" "${BASELINEREM}/${f}.ncdump"
    fi
done

finish

