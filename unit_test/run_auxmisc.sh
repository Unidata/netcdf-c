#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi 
. ../test_common.sh

set -x
set -e

# List of provenance strings to parse
# Start with some edge cases
TESTS=
TESTS="$TESTS abc=2\|z\=17,yyy=|zzz"
TESTS="$TESTS version=2,netcdf=4.7.4-development,hdf5=1.10.4"
TESTS="$TESTS version=2,netcdf=4.6.2-development,hdf5=1.10.1"
TESTS="$TESTS version=1|netcdf=4.6.2-development|hdf5=1.8.1"

# Test provenance parsing
testprov() {
rm -f tmp_provparse.txt
for t in $TESTS ; do
${execdir}/test_auxmisc -P ${t} >> tmp_provparse.txt
done
echo "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"
cat ${srcdir}/ref_provparse.txt
echo "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"
cat tmp_provparse.txt
echo "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"
# Verify
#diff ref_provparse.txt tmp_provparse.txt
}

testprov
