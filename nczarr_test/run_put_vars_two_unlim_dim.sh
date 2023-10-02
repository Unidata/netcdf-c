#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

set -e

find ${execdir} -name 'test_put_vars_two_unlim_dim*'

if test -f ${execdir}/.libs/test_put_vars_two_unlim_dim${ext} ; then
nm ${execdir}/.libs/test_put_vars_two_unlim_dim${ext}
fi

#${execdir}/test_put_vars_two_unlim_dim${ext}

exit 0
