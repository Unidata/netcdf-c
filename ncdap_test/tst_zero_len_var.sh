#!/bin/sh

if test "x$SETX" != x ; then set -x; fi
set -e

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

##
# If the bug referenced in https://github.com/Unidata/netcdf-c/issues/1300
# reoccurs, then the following command would fail.

${NCDUMP} http://test.opendap.org/opendap/data/nc/zero_length_array.nc > tst_zero_len_var.cdl

RES=$?

if [ $RES -ne 0 ]; then
    echo "Error $RES"
    exit $RES
fi

rm -f tst_zero_len_var.cdl

exit 0
