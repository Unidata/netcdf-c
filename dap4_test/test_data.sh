#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

set -x
set -e

. ${top_srcdir}/dap4_test/d4test_common.sh

echo "test_data.sh:"

computetestablefiles

setresultdir results_test_data

if test "x${RESET}" = x1 ; then rm -fr ${BASELINE}/*.d4d ; fi
for f in $F ; do
    echo "testing: ${f}"
    if ! ${execdir}/test_data -s ${builddir}/results_test_data/${f} ${RAWTESTFILES}/${f}  ; then
        echo "failure"
    fi
    ${NCDUMP} ${DUMPFLAGS} ${builddir}/results_test_data/${f} > ${builddir}/results_test_data/${f}.d4d
    if test "x${TEST}" = x1 ; then
	diff -wBb ${BASELINE}/${f}.d4d ${builddir}/results_test_data/${f}.d4d
    elif test "x${RESET}" = x1 ; then
	echo "${f}:"
	cp ${builddir}/results_test_data/${f}.d4d ${BASELINE}/${f}.d4d
    fi
done

# Remove empty lines and trim lines in a cdl file
trim() {
  if test $# != 2 ; then
    echo "simplify: too few args"
  else
    rm -f $2
    while read -r iline; do
      oline=`echo $iline | sed -e 's/^[\t ]*\([^\t ]*\)[\t ]*$/\\1/'`
      if test "x$oline" = x ; then continue ; fi
      echo "$oline" >> $2
    done < $1
  fi
}

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

if test "x${CDLDIFF}" = x1 ; then
  for f in $F ; do
    STEM=`echo $f | cut -d. -f 1`
    if ! test -e ${CDLTESTFILES}/${STEM}.cdl ; then
      echo "Not found: ${CDLTESTFILES}/${STEM}.cdl"
      continue
    fi
    echo "diff -wBb ${CDLTESTFILES}/${STEM}.cdl ${builddir}/results_test_data/${f}.d4d"
    rm -f ${builddir}/b1 ${builddir}/b2 ${builddir}/r1 ${builddir}/r2
    trim ${CDLTESTFILES}/${STEM}.cdl ${builddir}/b1
    trim ${builddir}/results_test_data/${f}.d4d ${builddir}/r1
    baseclean b1 b2
    resultclean r1 r2
    diff -wBb ${builddir}/b2 ${builddir}/r2
  done
fi

