#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi 
. ../test_common.sh

. "$srcdir/test_nczarr.sh"

# This shell script runs tst_nczfilter.c

set -e

pwd

s3isolate "testdir_nczfilter"
cd $ISOPATH

${execdir}/tst_nczfilter 

if test "x$FEATURE_S3TESTS" = xyes ; then s3sdkdelete "/${S3ISOPATH}" ; fi # Cleanup
