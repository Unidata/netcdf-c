#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

. ${srcdir}/d4test_common.sh

set -e

#BIG=1
#CSUM=1

computetestablefiles

setresultdir results_test_remote

TESTSERVER=`${execdir}/findtestserver4 dap4 d4ts`
if test "x$TESTSERVER" = x ; then
echo "***XFAIL: Cannot find d4ts testserver"
exit 1
fi

if test "x${RESET}" = x1 ; then rm -fr ${BASELINEREM}/*.ncdump ; fi
for f in $F ; do
    FRAG="#dap4&log&show=fetch"
    QUERY=""
    if test "x$BIG" = x1; then
	FRAG="${FRAG}&ucar.littleendian=0"
    fi
    if test "x$CSUM" = x1 ; then
	QUERY="dap4.checksum=true"
    else
	QUERY="dap4.checksum=false"
    fi
    # Fix up QUERY
    QUERY="?${QUERY}"
    # Built test URL
    URL="${TESTSERVER}/testfiles/${f}${QUERY}${FRAG}"
    ${NCDUMP} ${DUMPFLAGS} "${URL}" > ${builddir}/results_test_remote/${f}.ncdump
    if test "x${TEST}" = x1 ; then
	diff -wBb "${BASELINEREM}/${f}.ncdump" "${builddir}/results_test_remote/${f}.ncdump"
    elif test "x${RESET}" = x1 ; then
	echo "${f}:" 
	cp "${builddir}/results_test_remote/${f}.ncdump" "${BASELINEREM}/${f}.ncdump"
    fi
done

finish

