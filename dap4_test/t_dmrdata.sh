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
test_struct_type \
test_utf8 \
test_sequence_1 \
test_sequence_2 \
"

CDL="${F}"

failure() {
      echo "*** Fail: $1"
      exit 1
}

filesexist() {
    for x in "$@" ; do
	if ! test -f $x ; then
	  failure "missing file: $x"
	fi
    done
}

rm -fr ./results
mkdir -p ./results
if test "x${RESET}" = x1 ; then rm -fr ${BASELINE}/*.dmp ; fi
for f in $F ; do
    filesexist ${TESTFILES}/${f}.nc.raw
    if ! ./t_dmrdata ${TESTFILES}/${f}.nc.raw ./results/${f}.nc ; then
        failure "./t_dmrdata ${TESTFILES}/${f}.nc.raw ./results/${f}.nc"
    fi
    ncdump ./results/${f}.nc > ./results/${f}.dmp
    if test "x${TEST}" = x1 ; then
	filesexist ${BASELINE}/${f}.dmp ./results/${f}.dmp
	echo diff -wBb ${BASELINE}/${f}.dmp ./results/${f}.dmp 
	if ! diff -wBb ${BASELINE}/${f}.dmp ./results/${f}.dmp ; then
	    failure "diff -wBb ${BASELINE}/${f}.dmp ./results/${f}.dmp"
	fi
    elif test "x${RESET}" = x1 ; then
	echo "${f}:" 
	cp ./results/${f}.dmp ${BASELINE}/${f}.dmp
    fi
done

if test "x${CDLDIFF}" = x1 ; then
  for f in $CDL ; do
    filesexist ${TESTFILES}/${f}.cdl ./results/${f}.dmp
    echo "diff -wBb ${TESTFILES}/${f}.cdl ./results/${f}.dmp"
    rm -f ./tmp
	cat ./testfiles/${f}.cdl \
	| cat > ./tmp
    if ! diff -wBbu ./tmp ./results/${f}.dmp ; then
	failure "${f}" 
    fi
  done
fi

echo "*** Pass"
exit 0

#	| sed -e 's|^netcdf \(test[^	 ]*\).*$|netcdf \1 {|' 
