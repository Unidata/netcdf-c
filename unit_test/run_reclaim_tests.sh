#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi 
. ../test_common.sh

set -e

rm -f reclaim_tests.nc reclaim_tests*.txt
${NCGEN} -4 ${srcdir}/reclaim_tests.cdl
${execdir}/tst_reclaim > reclaim_tests.txt
sed -e '/^(o)/p' -ed reclaim_tests.txt | sed -e 's/^(o) //' > reclaim_tests_o.txt
sed -e '/^(c)/p' -ed reclaim_tests.txt | sed -e 's/^(c) //' > reclaim_tests_c.txt
diff reclaim_tests_o.txt reclaim_tests_c.txt

    

