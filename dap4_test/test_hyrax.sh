#!/bin/sh

if test "x$srcdir" = "x"; then srcdir=`dirname $0`; fi
export srcdir;

. ../test_common.sh

. ${srcdir}/d4test_common.sh

set -e
echo "test_hyrax.sh:"

FRAG=""

F="\
nc4_test_files/nc4_nc_classic_no_comp.nc \
nc4_test_files/nc4_nc_classic_comp.nc \
nc4_test_files/nc4_unsigned_types.nc \
nc4_test_files/nc4_unsigned_types_comp.nc \
nc4_test_files/nc4_strings.nc \
nc4_test_files/nc4_strings_comp.nc \
nc4_test_files/ref_tst_compounds.nc \
hyrax/RSS/amsre/bmaps_v05/y2006/m01/amsre_20060131v5.dat?dap4.ce=time_a \
"
F=nc4_test_files/nc4_nc_classic_no_comp.nc

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

if test "x${RESET}" = x1 ; then rm -fr ${BASELINEHY}/*.hyrax ; fi
for f in $F ; do

    makeurl "dap4://test.opendap.org/opendap" "$f?dap4.checksum=true"

URL="$URL#log&show=fetch"
    echo "testing: $URL"
    if ! ${NCDUMP} ${DUMPFLAGS} "${URL}" > ./results_test_hyrax/${base}.hyrax; then
        failure "${URL}"
    fi
    if test "x${TEST}" = x1 ; then
	if ! diff -wBb ${BASELINEHY}/${base}.hyrax ./results_test_hyrax/${base}.hyrax ; then
	    failure "diff ${base}.hyrax"
	fi
    elif test "x${RESET}" = x1 ; then
	echo "${f}:" 
	cp ./results_test_hyrax/${base}.hyrax ${BASELINEHY}/${base}.hyrax
    fi
done

echo "*** Pass"
exit 0

