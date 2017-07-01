#!/bin/sh
# This shell script runs the ncdump tests.
# $Id: run_tests.sh,v 1.18 2010/05/19 13:43:39 ed Exp $

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

set -e
echo ""
echo "*** Testing ncgen and ncdump using some test CDL files."
echo "*** creating tst_small.nc from ref_tst_small.cdl..."
cat $srcdir/ref_tst_small.cdl
${NCGEN} -b -o tst_small.nc $srcdir/ref_tst_small.cdl
echo "*** creating tst_small.cdl from tst_small.nc..."
${NCDUMP} tst_small.nc > tst_small.cdl
diff -b -w tst_small.cdl $srcdir/ref_tst_small.cdl

echo "*** creating test0.nc from test0.cdl..."
${NCGEN} -b $srcdir/test0.cdl
echo "*** creating test1.cdl from test0.nc..."
${NCDUMP} -n test1 test0.nc > test1.cdl
echo "*** creating test1.nc from test1.cdl..."
${NCGEN} -b test1.cdl
echo "*** creating test2.cdl from test1.nc..."
${NCDUMP} test1.nc > test2.cdl
echo "*** checking that test1.cdl and test2.cdl are the same..."
diff -b -w test1.cdl test2.cdl

echo "*** All tests of ncgen and ncdump using test0.cdl passed!"
exit 0
