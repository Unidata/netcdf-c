#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi 
. ../test_common.sh

set -x
set -e

${execdir}/test_pathcvt
