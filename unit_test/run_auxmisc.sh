#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi 
. ../test_common.sh

set -e

isolate "testdir_auxmisc"
THISDIR=`pwd`
cd $ISOPATH

# List of provenance strings to parse
TESTS=
TESTS="$TESTS abc=2\|z\=17,yyy=|zzz"
TESTS="$TESTS version=2,netcdf=4.7.4-development,hdf5=1.10.4"
TESTS="$TESTS version=2,netcdf=4.6.2-development,hdf5=1.10.1"
TESTS="$TESTS version=1|netcdf=4.6.2-development|hdf5=1.8.1"

# Test provenance parsing
testprov() {
rm -f ${ISOPATH}/tmp_provparse.txt
for t in $TESTS ; do
${execdir}/test_auxmisc -P ${t} >> ${ISOPATH}/tmp_provparse.txt
done
# Verify
diff ${srcdir}/ref_provparse.txt ${ISOPATH}/tmp_provparse.txt
}

testprov
