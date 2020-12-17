#!/bin/sh
# Tests for ncgen4 using list of test cdl files from the cdl4
# directory, and comparing output to expected results in the expected4
# directory. Note that these tests are run for classic files in
# tst_ncgen4_classic.sh
# Dennis Heimbigner

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

. "$srcdir/test_nczarr.sh"

set -e

#SHOWXFAILS=1


# To add a new test,
# 1. put the .cdl file in the 'ncdump/cdl' directory
# 2. put the result of running ncgen then ncdump
#    into the directory 'expected' as .dmp
# 3. Add the test to the end of the TESTS variable
# 4. Add the new files into ncdump/cdl/Makefile.am
#    and ncdump/expected/Makefile.am

TESTS="\
dimscope \
tst_group_data \
tst_solar_1 \
tst_nans \
tst_nul4 \
"       

HEADTESTS=""
SPECTESTS=""

XFAILTESTS=""

# Location constants
cdl="$srcdir/../ncdump/cdl"
expected="$srcdir/../ncdump/expected"
RESULTSDIR="./results"

# Functions

runtestset() {
extfor $1
echo "*** Testing nczarr X ncgen with zmap=${zext}"
rm -fr ${RESULTSDIR}.$zext
mkdir ${RESULTSDIR}.${zext}
cd ${RESULTSDIR}.${zext}
difftest
cd ..
echo "*** PASSED: zext=${zext}"
}

runtestset nzf
if test "x$FEATURE_HDF5" = xyes ; then runtestset nz4; fi
if test "x$FEATURE_S3TESTS" = xyes ; then runtestset s3; fi

rm -rf ${RESULTSDIR}.nzf ${RESULTSDIR}.nz4 ${RESULTSDIR}.s3

echo "*** PASSED ***"
