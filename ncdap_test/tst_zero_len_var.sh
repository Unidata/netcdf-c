#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

set -e

##
# If the bug referenced in https://github.com/Unidata/netcdf-c/issues/1300
# reoccurs, then the following command would fail.

# Figure our server; if none, then just stop
SVCP=`${execdir}/pingurl test.opendap.org/opendap|tr -d '\r'`
if test "x$SVCP" = xno ; then
    echo "WARNING: test.opendap.org is not accessible";
    exit 0;
fi

if test "x$SVCP" = xhttp ; then
URL="http://test.opendap.org/opendap"
else
URL="https://test.opendap.org/opendap"
fi

URL="${URL}/data/nc/zero_length_array.nc"

${NCDUMP} "$URL" > tst_zero_len_var.cdl

RES=$?

if [ $RES -ne 0 ]; then
    echo "Error $RES"
    exit $RES
fi

rm -f tst_zero_len_var.cdl

exit 0
