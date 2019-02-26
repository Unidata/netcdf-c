#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi 
. ../test_common.sh

set -e

echo "*** Test integer constant radix specifications"
echo "*** creating tst_radix.nc from tst_radix.cdl..."
${NCGEN} -lb -o tst_radix.nc $srcdir/tst_radix.cdl
echo "*** creating tmp_radix.cdl from radix.nc..."
${NCDUMP} tst_radix.nc > tmp_radix.cdl
echo "*** comparing tmp_radix.cdl to ref_tst_radix.cdl..."
diff -b -w tmp_radix.cdl $srcdir/ref_tst_radix.cdl

exit 0
