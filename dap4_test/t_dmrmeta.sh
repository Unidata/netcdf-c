#!/bin/sh

if test $# = 0 ; then
TEST=1
else
if test "x$1" = xtest ; then
TEST=1
elif test "x$1" = xreset ; then
RESET=1
elif test "x$1" = xcdl ; then
CDLDIFF=1
else
echo unknown argument $1 
fi
fi

if test "x$SETX" = x1 ; then echo "file=$0"; set -x ; fi

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
tst_h5fill.h5\
"

CDL="\
test_atomic_array \
test_atomic_types \
test_enum \
test_enum_2 \
test_enum_array \
test_fill \
test_groups1 \
test_one_var \
test_one_vararray \
test_opaque \
test_opaque_array \
test_struct_array \
test_struct_nested \
test_struct_type \
test_utf8 \
"

failure() {
      echo "*** Fail: $1"
#      exit 1
}

rm -fr ./results
mkdir -p ./results
if test "x${RESET}" = x1 ; then rm -fr ${BASELINE}/*.ncdump ; fi
for f in $F ; do
    if ! ./t_dmrmeta ${TESTFILES}/${f}.dmr ./results/${f} ; then
        failure "./t_dmrmeta ${TESTFILES}/${f}.dmr ./results/${f}"
    fi
    ncdump -h ./results/${f} > ./results/${f}.ncdump
    if test "x${TEST}" = x1 ; then
	echo diff -wBb ${BASELINE}/${f}.ncdump ./results/${f}.ncdump
	if ! diff -wBb ${BASELINE}/${f}.ncdump ./results/${f}.ncdump ; then
	    failure "diff -wBb ${BASELINE}/${f}.ncdump ./results/${f}.ncdump"
	fi
    elif test "x${RESET}" = x1 ; then
	echo "${f}:" 
	cp ./results/${f}.ncdump ${BASELINE}/${f}.ncdump
    fi
done

if test "x${CDLDIFF}" = x1 ; then
  for f in $CDL ; do
    echo "diff -wBb ${TESTFILES}/${f}.cdl ./results/${f}.nc.ncdump"
    rm -f ./tmp
	cat ./testfiles/${f}.cdl \
	| sed -e 's|^netcdf \(test[^	 ]*\).*$|netcdf \1.nc {|' \
	| cat > ./tmp
    if ! diff -wBbu ./tmp ./results/${f}.nc.ncdump ; then
	failure "${f}" 
    fi
  done
fi

echo "*** Pass"
exit 0

