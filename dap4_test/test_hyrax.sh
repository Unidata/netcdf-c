#!/bin/sh

if test "x$srcdir" = "x"; then srcdir=`dirname $0`; fi
export srcdir;


. ../test_common.sh

. ${srcdir}/d4test_common.sh

set -e

echo "test_hyrax.sh:"

# Some test baseline paths are too long for tar.
TOOLONG="AIRS/AIRH3STM.003/2002.12.01/AIRS.2002.12.01.L3.RetStd_H031.v4.0.21.0.G06101132853.hdf?/TotalCounts_A"

F="\
RSS/amsre/bmaps_v05/y2006/m01/amsre_20060131v5.dat?/time_a[0:2][0:5] \
nc4_test_files/nc4_nc_classic_no_comp.nc \
nc4_test_files/nc4_nc_classic_comp.nc \
nc4_test_files/nc4_unsigned_types.nc \
nc4_test_files/nc4_unsigned_types_comp.nc \
nc4_test_files/nc4_strings.nc \
nc4_test_files/nc4_strings_comp.nc \
nc4_test_files/ref_tst_compounds.nc \
"

failure() {
      echo "*** Fail: $1"
      exit 1
}

setresultdir results_test_hyrax
TESTSERVER=`${execdir}/findtestserver4 dap4 opendap test.opendap.org`
if test "x$TESTSERVER" = x ; then
echo "***XFAIL: Cannot find test.opendap.org testserver; test skipped"
exit 0
fi

gethyraxbaseline() {
    ARG1="$1"
    BASELINEPART=`echo $ARG1 | cut -d':' -f1`
    URLPART=`echo $ARG1 | cut -d':' -f2`    
    if test "x$BASELINEPART" = "x$URLPART" ; then BASELINEPART=""; fi
}

makehyraxurl() {
    if test "x$QUERY" != x ; then QUERY="&dap4.ce=$QUERY"; fi
    QUERY="?dap4.checksum=true${QUERY}"
    URL="${TESTSERVER}/${PREFIX}/${FILE}${QUERY}"
    URL="$URL#dap4&hyrax"
    URL="$URL&log&show=fetch"
}

hyraxsplit() {
    P="$1"
    QUERY=`echo $P | cut -d? -f2`
    if test "x$QUERY" = "x$P" ; then QUERY="" ; fi
    P=`echo $P | cut -d? -f1`
    FILE=`basename $P`
    PREFIX=`dirname $P`
}

# Turn on only if regenerating test baseline
#if test "x${RESET}" = x1 ; then rm -fr ${BASELINEHY}/*.hyrax ; fi
for f in $F ; do
  hyraxsplit $f
  makehyraxurl
  echo "testing: $URL"
  if ! ${NCDUMP} ${DUMPFLAGS} "${URL}" > ./results_test_hyrax/${FILE}.hyrax; then
    failure "${URL}"
  fi
  if test "x${TEST}" = x1 ; then
    if ! diff -wBb ${BASELINEHY}/${FILE}.hyrax ./results_test_hyrax/${FILE}.hyrax ; then
      failure "diff ${FILE}.hyrax"
    fi
  elif test "x${RESET}" = x1 ; then
    echo "${FILE}:" 
    cp ./results_test_hyrax/${FILE}.hyrax ${BASELINEHY}/${FILE}.hyrax
  fi
done

echo "*** Pass"
exit 0

