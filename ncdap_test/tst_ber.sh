#!/bin/sh

export SETX=1

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

export NCPATHDEBUG=1
#export NCPATHFORMAT=1

F="ber-2002-10-01.nc"
PWD=`pwd`
EXPECTED="${PWD}/expected3"

URL='[log][cache]file://'
URL="${URL}${PWD}/testdata3/$F"
rm -f ./tmp
${NCDUMP} "${URL}" >./tmp
diff -w ${EXPECTED}/$F.dmp ./tmp
exit
