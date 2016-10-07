#!/bin/sh

if test $# = 0 ; then
TEST=1
else
if test "x$1" = xtest ; then
TEST=1
elif test "x$1" = xdiff ; then
DIFF=1
elif test "x$1" = xreset ; then
RESET=1
else
echo unknown argument $1 
fi
fi

if test "x$SETX" = x1 ; then echo "file=$0"; set -x ; fi
# This shell script tests ncdump for netcdf-4
# $Id: tst_netcdf4.sh,v 1.34 2009/09/25 18:22:10 dmh Exp $

set -e

if test "x$srcdir" = "x"; then
    srcdir=`dirname $0`; 
fi
# add hack for sunos
export srcdir;

BASELINE=${srcdir}/baseline
TESTFILES=${srcdir}/testfiles

F="\
test_atomic_array.nc \
test_atomic_types.nc \
test_enum.nc \
test_enum_2.nc \
test_enum_array.nc \
test_enum1.nc \
test_enum2.nc \
test_fill.nc \
test_groups1.nc \
test_numeric_types.nc \
test_one_var.nc \
test_one_vararray.nc \
test_opaque.nc \
test_opaque_array.nc \
test_opaque_type.nc \
test_sequence_1.syn \
test_sequence_2.syn \
test_string_type.nc \
test_struct_array.nc \
test_struct_nested.nc \
test_struct_nested3.hdf5 \
test_struct_type.nc \
test_struct1.nc \
test_utf8.nc \
tst_fills.nc \
tst_h5fill.h5"

failure() {
      echo "*** Fail: $1"
      exit 1
}

rm -fr ./results
mkdir -p ./results
if test "x${RESET}" = x1 ; then rm -fr ${BASELINE}/*.d4p ; fi
for f in $F ; do
    if ! ./t_dmrparse ${TESTFILES}/${f}.dmr > ./results/${f}.d4p ; then
	failure "${f}"
    fi
    if test "x${TEST}" = x1 ; then
	if ! diff -wBb ${BASELINE}/${f}.d4p ./results/${f}.d4p ; then
	    failure "${f}"
	fi
    elif test "x${DIFF}" = x1 ; then
	echo "diff -wBb ${TESTFILES}/${f}.dmr ./results/${f}.d4p"
	rm -f ./tmp
	cat ./results/${f}.d4p \
	| sed -e '/<Dimensions>/d' -e '/<Types>'/d -e '/<Variables>'/d -e '/<Groups>'/d \
	| sed -e '/<\/Dimensions>/d' -e '/<\/Types>'/d -e '/<\/Variables>'/d -e '/<\/Groups>'/d  \
	| cat > ./tmp
	if ! diff -wBb ${TESTFILES}/${f}.dmr ./tmp ; then
	    failure "${f}" 
	fi
    elif test "x${RESET}" = x1 ; then
	echo "${f}:" 
	cp ./results/${f}.d4p ${BASELINE}/${f}.d4p	
    fi
done

echo "*** Pass"
exit 0

