#!/bin/sh

export SETX=1
#export NCPATHDEBUG=1

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

F="ber-2002-10-01.nc"
EXPECTED="${srcdir}/expected3"

URL='[log][cache]file://'
URL="${URL}${srcdir}/testdata3/$F"
rm -f ./tmp
${NCDUMP} "${URL}" >./tmp
diff -w ${EXPECTED}/$F.dmp ./tmp
#cleanup
rm -f ./tmp
exit
