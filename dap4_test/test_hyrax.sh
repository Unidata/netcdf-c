#!/bin/sh

if test "x$srcdir" = "x"; then srcdir=`dirname $0`; fi
export srcdir;

if test "x$srcdir" = x ; then srcdir=`pwd`; fi 
. ../test_common.sh

. ${srcdir}/d4test_common.sh

set -e
echo "test_hyrax.sh:"

FRAG="#checksummode=ignore"

F="\
nc4_test_files/nc4_nc_classic_comp.nc \
nc4_test_files/nc4_unsigned_types_comp.nc \
nc4_test_files/nc4_strings_comp.nc \
nc4_test_files/ref_tst_compounds.nc \
hyrax/RSS/amsre/bmaps_v05/y2006/m01/amsre_20060131v5.dat?dap4.ce=time_a \
"

failure() {
      echo "*** Fail: $1"
      exit 1
}

setresultdir results_test_hyrax

if test "x${RESET}" = x1 ; then rm -fr ${BASELINEH}/*.hyrax ; fi
for f in $F ; do
    constraint=`echo "$f" | cut -d '?' -f2`
    unconstrained=`echo "$f" | cut -d '?' -f1`
    base=`basename $unconstrained`
    prefix=`dirname $unconstrained`
    if test "x$constraint" = "x$unconstrained" ; then
        URL="dap4://test.opendap.org:8080/opendap/${prefix}/${base}${FRAG}"
    else
	URL="dap4://test.opendap.org:8080/opendap/${prefix}/${base}?$constraint${FRAG}"
    fi
    echo "testing: $URL"
    if ! ${NCDUMP} "${URL}" > ./results_test_hyrax/${base}.hyrax; then
        failure "${URL}"
    fi
    if test "x${TEST}" = x1 ; then
	if ! diff -wBb ${BASELINEH}/${base}.hyrax ./results_test_hyrax/${base}.hyrax ; then
	    failure "diff ${base}.hyrax"
	fi
    elif test "x${RESET}" = x1 ; then
	echo "${f}:" 
	cp ./results_test_hyrax/${base}.hyrax ${BASELINEH}/${base}.hyrax
    fi
done

echo "*** Pass"
exit 0

