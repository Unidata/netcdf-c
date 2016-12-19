#!/bin/sh

if test "x$srcdir" = "x"; then srcdir=`dirname $0`; fi; export srcdir

. ${srcdir}/test_common.sh

cd ${DAPTESTFILES}
F=`ls -1 *.dap | sed -e 's/[.]dap//g' | tr '\r\n' '  '`
cd $WD

if test "x${RESET}" = x1 ; then rm -fr ${BASELINE}/*.dmp ; fi
for f in $F ; do
    if ! ./t_dmrdata ${DAPTESTFILES}/${f}.dap ./results/${f}.nc ; then
        failure "./t_dmrdata ${DAPTESTFILES}/${f}.nc.dap ./results/${f}.nc"
    fi
    ncdump ./results/${f}.nc > ./results/${f}.dmp
    if test "x${TEST}" = x1 ; then
	echo diff -wBb ${BASELINE}/${f}.dmp ./results/${f}.dmp 
	if ! diff -wBb ${BASELINE}/${f}.dmp ./results/${f}.dmp ; then
	    failure "diff -wBb ${BASELINE}/${f}.dmp ./results/${f}.dmp"
	fi
    elif test "x${RESET}" = x1 ; then
	echo "${f}:" 
	cp ./results/${f}.dmp ${BASELINE}/${f}.dmp
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
    if ! test -f ${CDLTESTFILES}/${STEM}.cdl ; then
      echo "Not found: ${CDLTESTFILES}/${STEM}.cdl"
      continue
    fi
    echo "diff -wBb ${CDLTESTFILES}/${STEM}.cdl ./results/${f}.dmp"
    rm -f ./b1 ./b2 ./r1 ./r2
    trim ${CDLTESTFILES}/${STEM}.cdl ./b1
    trim ./results/${f}.dmp ./r1
    baseclean b1 b2
    resultclean r1 r2  
    if ! diff -wBb ./b2 ./r2 ; then
	failure "${f}" 
    fi
  done
fi

if test "x$FAILURES" = x1 ; then
echo "*** Fail"
exit 1
else
echo "*** Pass"
exit 0
fi
