#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

. ${srcdir}/d4test_common.sh

echo "test_meta.sh:"

set -e

computetestablefiles

CDL=
for f in ${F} ; do
STEM=`echo $f | cut -d. -f 1`
if test -e ${CDLTESTFILES}/${STEM}.cdl ; then
  CDL="${CDL} ${STEM}"
else
  echo "Not found: ${CDLTESTFILES}/${STEM}.cdl; ignored"
fi
done

if test "x${RESET}" = x1 ; then rm -fr ${BASELINE}/*.d4m ; fi

setresultdir results_test_meta

for f in ${F} ; do
    echo "checking: $f"
    if ! ${VG} ${execdir}/test_meta ${RAWTESTFILES}/${f}.dmr ${builddir}/results_test_meta/${f} ; then
        failure "${execdir}/test_meta ${RAWTESTFILES}/${f}.dmr ${builddir}/results_test_meta/${f}"
    fi
    ${NCDUMP} ${DUMPFLAGS} -h ${builddir}/results_test_meta/${f} > ${builddir}/results_test_meta/${f}.d4m
    if test "x${TEST}" = x1 ; then
	if ! diff -wBb ${BASELINE}/${f}.d4m ${builddir}/results_test_meta/${f}.d4m ; then
	    failure "diff -wBb ${BASELINE}/${f}.ncdump ${builddir}/results_test_meta/${f}.d4m"
	fi
    elif test "x${RESET}" = x1 ; then
	echo "${f}:" 
	cp ${builddir}/results_test_meta/${f}.d4m ${BASELINE}/${f}.d4m
    fi
done

if test "x${CDLDIFF}" = x1 ; then
  for f in $CDL ; do
    echo "diff -wBb ${CDLTESTFILES}/${f}.cdl ${builddir}/results_test_meta/${f}.d4m"
    rm -f ${builddir}/tmp
    cat ${CDLTESTFILES}/${f}.cdl \
    cat >${builddir}/tmp
    echo diff -wBbu ${builddir}/tmp ${builddir}/results_test_meta/${f}.d4m
    if ! diff -wBbu ${builddir}/tmp ${builddir}/results_test_meta/${f}.d4m ; then
	failure "${f}" 
    fi
  done
fi

finish


