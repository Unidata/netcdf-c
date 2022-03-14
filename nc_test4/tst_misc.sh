#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

set -e

ECODE=0

echo "*** Testing phony dimension creation on pure h5 file"
rm -f ./tmp
if $NCDUMP -L0 -K ${srcdir}/tdset.h5 >./tmp ; then
echo "*** Pass: phony dimension creation"
else
echo "*** Fail: phony dimension creation"
ECODE=1
fi

echo "*** Testing char(*) type printout error in ncdump"
rm -f ./tst_charvlenbug.nc ./tmp
${execdir}/tst_charvlenbug
if $NCDUMP ./tst_charvlenbug.nc 2>1 >./tmp ; then
    echo "*** Pass: char(*) ncdump printout"
else
    echo "*** Fail: char(*) ncdump printout"
    ECODE=1
fi

echo "*** Testing char(*) type "
rm -f ./tst_vlenstr.nc ./tmp
${execdir}/tst_vlenstr

rm -f tmp

exit $ECODE


