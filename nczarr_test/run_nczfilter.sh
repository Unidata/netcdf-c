#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi 
. ../test_common.sh

. "$srcdir/test_nczarr.sh"

# This shell script runs tst_nczfilter.c

set -e

pwd

${execdir}/tst_nczfilter 
