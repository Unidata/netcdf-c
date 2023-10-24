#!/bin/sh

if test "x$srcdir" = "x"; then srcdir=`dirname $0`; fi
export srcdir;

. ../test_common.sh

set -e

# Uncomment to get more verbose info
#VERBOSE=1

isolate testdir_hyrax
THISDIR=`pwd`
cd $ISOPATH

echo "test_hyrax.sh:"

WD=`pwd`
cd ${top_srcdir}/ncdap_test/expectedhyrax; BASELINEHY=`pwd` ; cd ${WD}
TESTSERVER="http://test.opendap.org"

F="\
opendap/netcdf/examples/ECMWF_ERA-40_subset.nc?tcw.tcw[1][1][1] \
opendap/netcdf/examples/cami_0000-09-01_64x128_L26_c030918.nc?hyai.hyai
"

failure() {
      echo "*** Fail: $1"
      exit 1
}

makehyraxurl() {
    URL="${TESTSERVER}/${PREFIX}/${FILE}${QUERY}"
    URL="$URL#dap2&log"
    if test "x$VERBOSE" != x ; then URL="$URL&show=fetch"; fi
}

hyraxsplit() {
    P="$1"
    QUERY=`echo $P | cut -d? -f2`
    if test "x$QUERY" = "x$P" ; then QUERY="" ; else QUERY="?${QUERY}" ; fi
    P=`echo $P | cut -d? -f1`
    FILE=`basename $P`
    PREFIX=`dirname $P`
}

if test "x${RESET}" = x1 ; then rm -fr ${BASELINEHY}/*.hyrax ; fi
for f in $F ; do
  hyraxsplit $f
  makehyraxurl
  echo "testing: $URL"
  if ! ${NCDUMP} ${DUMPFLAGS} "${URL}" > ./${FILE}.hyrax; then
    failure "${URL}"
  fi
  if test "x${TEST}" = x1 ; then
    if ! diff -wBb ${BASELINEHY}/${FILE}.hyrax ./${FILE}.hyrax ; then
      failure "diff ${FILE}.hyrax"
    fi
  elif test "x${RESET}" = x1 ; then
    echo "${FILE}:" 
    cp ./${FILE}.hyrax ${BASELINEHY}/${FILE}.hyrax
  fi
done

echo "*** Pass"
exit 0
