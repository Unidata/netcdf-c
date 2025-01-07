#!/bin/sh
# Tests for ncgen4 using list of test cdl files from the cdl4
# directory, and comparing output to expected results in the expected4
# directory. Note that these tests are run for classic files in
# test_ncgen4_classic.sh
# Dennis Heimbigner

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

. "${srcdir}/test_nczarr.sh"

# Isolate both test and S3
s3isolate "testdir_nan"
THISDIR=`pwd`
cd $ISOPATH

set -x
export SETX=1
set -e

# Location constants
cdl="$srcdir/../ncdump/cdl"
#expected="$srcdir/../ncdump/expected"
expected="$srcdir"

# Functions

# Remove fillvalue attribute since zarr generates it when hdf5 does not
fvclean() {
    cat $1 \
 	| sed -e '/:_FillValue/d' \
	| cat > $2
}

difftest() {
echo ""; echo "*** Test zext=$zext"
for t in tst_nans ; do
   echo "*** Testing: ${t}"
   # determine if we need the specflag set
   # determine properties
   checkprops ${t}
   ref="ref_${t}"
   rm -fr ${t}.$zext
   rm -f tmp_${t}.dmp
   fileargs $t
   ${NCGEN} -4 -lb -o ${fileurl} ${cdl}/${ref}.cdl
   ${NCDUMP} ${headflag} ${specflag} -n ${ref} ${fileurl} > tmp_${t}.dmp
   # compare against expected
   diff -b -w ${expected}/${ref}.dmp ./tmp_${t}.dmp
   echo "*** SUCCEED: ${t}"
done
}

runtestset() {
extfor $1
echo "*** Testing nans for Zarr Format 3 for nczarr X ncgen with zmap=${zext}"
difftest
echo "*** PASSED: zext=${zext}"
}

runtestset file
if test "x$FEATURE_NCZARR_ZIP" = xyes ; then runtestset zip; fi
if test "x$FEATURE_S3TESTS" = xyes ; then runtestset s3; fi

echo "*** PASSED ***"
