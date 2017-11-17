#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi 
. ../test_common.sh

set -e
echo ""
verbose=0

export verbose

KFLAG=1 ; export KFLAG
bash -x  ${srcdir}/tst_ncgen4_diff.sh
bash -x  ${srcdir}/tst_ncgen4_cycle.sh
exit 0


