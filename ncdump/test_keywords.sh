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

echo "*** Test use of type aliases such as 'long' or 'real'..."
echo "*** classic: creating keyword3.nc from ref_keyword3.cdl..."
${NCGEN} -3 -lb -o keyword3.nc $srcdir/ref_keyword3.cdl
echo "*** creating tmp_keyword3.cdl from keyword3.nc..."
# We need to convert float back to real and int back to long
${NCDUMP} -h keyword3.nc | sed -e 's/float/real/g' -e 's/int/long/g' >tmp_keyword3.cdl
echo "*** comparing tmp_keyword3.cdl to ref_keyword3.cdl..."
diff -b -w tmp_keyword3.cdl $srcdir/ref_keyword3.cdl

echo "*** Test use of keywords both pass and fail"
# This should succeed
${NCGEN} -3 -lb -o keyword4.nc $srcdir/ref_keyword4.cdl
echo "***pass: ncgen -3 X ref_keyword4"
# This should (x)fail
if ${NCGEN} -4 -lb -o keyword4.nc $srcdir/ref_keyword4.cdl ; then
echo "***erroneous pass: ncgen -4 X ref_keyword4"
exit 1
else
echo "***xfail: ncgen -4 X ref_keyword4"
fi

echo "*** Test use of selected keywords as variable names..."
${NCGEN} -3 -lb -o keyword5.nc $srcdir/cdl/ref_keyword.cdl
${NCDUMP} -n file keyword5.nc >tmp_keyword5.cdl
echo "*** comparing tmp_keyword5.cdl to ref_keyword.cdl..."
diff -b -w tmp_keyword5.cdl $srcdir/cdl/ref_keyword.cdl

exit 0
