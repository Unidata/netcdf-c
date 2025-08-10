#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi 
. ../test_common.sh

set -x
set -e

echo "@@@ run_pathcvt.sh: MSYS_NO_PATHCONV=${MSYS_NO_PATHCONV}"
${execdir}/test_pathcvt
