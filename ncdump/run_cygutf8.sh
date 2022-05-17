#!/bin/bash -x

if test "x$srcdir" = x ; then srcdir=`pwd`; fi 
. ../test_common.sh

echo "Test unicode paths"
${execdir}/acpget
${execdir}/tst_cygutf8
ls xutf8*
rm xutf8*
