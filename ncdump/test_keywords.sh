#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi 
. ../test_common.sh

set -e

echo "*** Test use of keywords for formats where the keyword is not defined"
echo "*** classic: creating keyword1.nc from ref_keyword1.cdl..."
${NCGEN} -3 -lb -o keyword1.nc $srcdir/ref_keyword1.cdl
echo "*** creating tmp_keyword1.cdl from keyword1.nc..."
${NCDUMP} -h keyword1.nc > tmp_keyword1.cdl
echo "*** comparing tmp_keyword1.cdl to ref_keyword1.cdl..."
diff -b -w tmp_keyword1.cdl $srcdir/ref_keyword1.cdl

echo "*** cdf5: creating keyword2.nc from ref_keyword2.cdl..."
${NCGEN} -5 -lb -o keyword2.nc $srcdir/ref_keyword2.cdl
echo "*** creating tmp_keyword2.cdl from keyword2.nc..."
${NCDUMP} -h keyword2.nc > tmp_keyword2.cdl
echo "*** comparing tmp_keyword2.cdl to ref_keyword2.cdl..."
diff -b -w tmp_keyword2.cdl $srcdir/ref_keyword2.cdl

exit 0
