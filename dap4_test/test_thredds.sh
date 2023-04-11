#!/bin/sh

if test "x$srcdir" = "x"; then srcdir=`dirname $0`; fi
export srcdir;

. ../test_common.sh

. ${srcdir}/d4test_common.sh

set -e

echo "test_thredds.sh:"

F="\
dap4/testdata/H.1.1.nc \
dap4/testdata/2004050300_eta_211.nc?/reftime;/valtime;/Z_sfc[0][10:3:20][100:114] \
"

failure() {
      echo "*** Fail: $1"
      exit 1
}

setresultdir results_test_thredds
TESTSERVER=`${execdir}/findtestserver4 dap4 thredds`
#TESTSERVER="http://192.168.56.1:8081/thredds"
if test "x$TESTSERVER" = x ; then
echo "***XFAIL: Cannot find thredds server; test skipped"
exit 0
fi

makethreddsurl() {
    if test "x$QUERY" != x ; then QUERY="&dap4.ce=$QUERY"; fi
    QUERY="?dap4.checksum=true${QUERY}"
    URL="${TESTSERVER}/${PREFIX}/${FILE}${QUERY}"
    URL="$URL#dap4"
    URL="$URL&log&show=fetch"
}

threddssplit() {
    P="$1"
    QUERY=`echo $P | cut -d? -f2`
    if test "x$QUERY" = "x$P" ; then QUERY="" ; fi
    P=`echo $P | cut -d? -f1`
    FILE=`basename $P`
    PREFIX=`dirname $P`
}

if test "x${RESET}" = x1 ; then rm -fr ${BASELINETH}/*.thredds ; fi
for f in $F ; do
  threddssplit $f
  makethreddsurl
  echo "testing: $URL"
  if ! ${NCDUMP} ${DUMPFLAGS} "${URL}" > ./results_test_thredds/${FILE}.thredds; then
    failure "${URL}"
  fi
  if test "x${TEST}" = x1 ; then
    if ! diff -wBb ${BASELINETH}/${FILE}.thredds ./results_test_thredds/${FILE}.thredds ; then
      failure "diff ${FILE}.thredds"
    fi
  elif test "x${RESET}" = x1 ; then
    echo "${FILE}:" 
    cp ./results_test_thredds/${FILE}.thredds ${BASELINETH}/${FILE}.thredds
  fi
done

echo "*** Pass"
exit 0
