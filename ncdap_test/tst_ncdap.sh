#!/bin/sh

if test "x$SETX" != x ; then set -x ; fi

quiet=0
leakcheck=0

PARAMS="[log]"
#PARAMS="${PARAMS}[fetch=memory]"
#PARAMS="${PARAMS}[show=fetch]"

OCLOGFILE=/dev/null

# Capture arguments
srcdir="$1"
builddir="$2"
mode="$3"

# Locate the testdata and expected directory
testdata3="${srcdir}/testdata3"
expected3="${srcdir}/expected3"

TITLE="DAP to netCDF-3 translation"
EXPECTED="$expected3"
PARAMS="${PARAMS}[cache]"

# get the list of test files
. ${srcdir}/tst_ncdap_shared.sh

case "$mode" in
file*)
    TESTURL="$FILEURL"
    TESTSET="$FILETESTS"
    ;;
dds*)
    TESTURL="[noprefetch]$FILEURL"
    TESTSET="$DDSTESTS"
    FLAGS="$FLAGS -h"
    ;;
remote*)
    TESTURL="$REMOTEURL"
    TITLE="Remote $TITLE"
    TESTSET="$REMOTETESTS"
    ;;
esac

RESULTSDIR="./results_tst_ncdap"
#
if test "x$leakcheck" = "x1" ; then
VALGRIND="valgrind -q --error-exitcode=2 --leak-check=full"
fi

rm -fr ${RESULTSDIR}
mkdir "${RESULTSDIR}"

rm -f ./.dodsrc ./.ocrc ./.daprc
passcount=0
xfailcount=0
failcount=0

echo "*** Testing $TITLE "
echo "        Base URL: ${TESTURL}"
echo "        Client Parameters: ${PARAMS}"

cd ${RESULTSDIR}

for x in ${TESTSET} ; do
  url="${PARAMS}${TESTURL}/$x"
  if test "x$quiet" = "x0" ; then echo "*** Testing: ${x} ; url=$url" ; fi
  # determine if this is an xfailtest
  isxfail=0
  for t in ${XFAILTESTS} ; do
    if test "x${t}" = "x${x}" ; then isxfail=1; fi
  done
  ok=1
  if ${VALGRIND} ${NCDUMP} ${FLAGS} "${url}" | sed 's/\\r//g' > ${x}.dmp ; then ok=$ok; else ok=0; fi
  # compare with expected
  if diff -w ${EXPECTED}/${x}.dmp ${x}.dmp  ; then ok=$ok; else ok=0; fi
  if test "$ok" = 1 ; then
    status=0  # succeed
  elif test "x$isxfail" = "x0" ; then
    status=1  # fail
  else
    status=2  # xfail
  fi

  case "$status" in
  0)
    passcount=`expr $passcount + 1`
    if test "x$quiet" = "x" ; then echo "*** SUCCEED: ${x}"; fi
    ;;
  1)
    failcount=`expr $failcount + 1`
    echo "*** FAIL:  ${x}"
    ;;
  2)
    xfailcount=`expr $xfailcount + 1`
    echo "*** XFAIL : ${x}"
    ;;
  esac

done

rm -f ./.dodsrc ./.ocrc ./.daprc

cd ..
echo "pwd=" `pwd`

totalcount=`expr $passcount + $failcount + $xfailcount`
okcount=`expr $passcount + $xfailcount`
rm -fr ${RESULTSDIR}
echo "*** PASSED: ${okcount}/${totalcount} ; ${xfailcount} expected failures ; ${failcount} unexpected failures"

#failcount=0
if test "$failcount" -gt 0
then
  exit 1
else
  exit 0
fi
