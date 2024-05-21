#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

. ${srcdir}/d4test_common.sh

set -e

#BIG=1
CSUM=1

setresultdir results_test_constraints

TESTSERVER=`${execdir}/findtestserver4 dap4 d4ts`
if test "x$TESTSERVER" = x ; then
echo "***XFAIL: Cannot find d4ts testserver"
exit 1
fi

for triple in ${constrained_manifest} ; do
    # split the triple into its parts
    splitconstraint $triple
    if test "x$BIG" = x1; then
	FRAG="${FRAG}&ucar.littleendian=0"
    fi
    # Fix up QUERY
    QUERY="dap4.ce=${QUERY}"    
    if test "x$CSUM" = x1 ; then
	QUERY="${QUERY}&dap4.checksum=true"
    else
	QUERY="${QUERY}&dap4.checksum=false"
    fi
    QUERY="?${QUERY}"

    # Build complete test url
    URL="${TESTSERVER}/testfiles/${FILE}.nc${QUERY}${FRAG}"
    # and outputfile stem
    OFILE="${FILE}.${INDEX}.nc"
    ${NCDUMP} ${DUMPFLAGS} "${URL}" > ${builddir}/results_test_constraints/${OFILE}.ncdump
    if test "x${TEST}" = x1 ; then
	diff -wBb "${BASELINEREM}/${OFILE}.ncdump" "${builddir}/results_test_constraints/${OFILE}.ncdump"
    elif test "x${RESET}" = x1 ; then
	echo "${OFILE}:" 
        rm -f "${BASELINEREM}/${OFILE}.ncdump"
	cp "${builddir}/results_test_constraints/${OFILE}.ncdump" "${BASELINEREM}/${OFILE}.ncdump"
    fi
done

finish

