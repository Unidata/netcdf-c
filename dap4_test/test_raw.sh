#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

. ${srcdir}/d4test_common.sh

set -e

echo "test_raw.sh:"

computetestablefiles

# Do cleanup on the baseline file
baseclean() {
  if test $# != 2 ; then
    echo "simplify: too few args"
  else
    rm -f $2
    while read -r iline; do
      oline=`echo $iline | tr "'" '"'`
      echo "$oline" >> $2
    done < $1
  fi
}

# Do cleanup on the result file
resultclean() {
  if test $# != 2 ; then
    echo "simplify: too few args"
  else
    rm -f $2
    while read -r iline; do
      oline=`echo $iline | sed -e 's|^\(netcdf.*\)[.]nc\(.*\)$|\\1\\2|'`
      echo "$oline" >> $2
    done < $1
  fi
}

setresultdir results_test_raw

if test "x${RESET}" = x1 ; then rm -fr ${BASELINERAW}/*.ncdump ; fi
for f in $F ; do
    echo "testing: $f"
    URL="file://${RAWTESTFILES}/${f}?dap4.checksum=false#log&dap4"
    if ! ${NCDUMP} ${DUMPFLAGS} "${URL}" > ${builddir}/results_test_raw/${f}.ncdump; then
        failure "${URL}"
    fi
    if test "x${TEST}" = x1 ; then
	if ! diff -wBb ${BASELINERAW}/${f}.ncdump ${builddir}/results_test_raw/${f}.ncdump ; then
	    failure "diff ${f}.ncdump"
	fi
    elif test "x${RESET}" = x1 ; then
	echo "${f}:"
	cp ${builddir}/results_test_raw/${f}.ncdump ${BASELINERAW}/${f}.ncdump
    elif test "x${DIFF}" = x1 ; then
	echo "hdrtest: ${f}"
	baseclean
        if ! diff -wBb ${BASELINERAW}/${f}.ncdump ${BASELINE}/${f}.ncdump ; then
          failure diff -wBb ${BASELINERAW}/${f}.ncdump ${BASELINE}/${f}.ncdump
	fi
    fi
done

finish
