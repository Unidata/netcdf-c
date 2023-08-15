#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi 
. ../test_common.sh

. "$srcdir/test_nczarr.sh"

# This shell script runs test_nczfilter.c

set -e

s3isolate "testdir_nczfilter"
THISDIR=`pwd`
cd $ISOPATH

${execdir}/test_nczfilter

if test "x$FEATURE_S3TESTS" = xyes ; then s3sdkdelete "/${S3ISOPATH}" ; fi # Cleanup
