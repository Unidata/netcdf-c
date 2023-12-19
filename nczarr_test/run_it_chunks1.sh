#!/bin/sh

# Run (test_chunks,test_chunks2) X (file,zip,s3)


if test "x$srcdir" = x ; then srcdir=`pwd`; fi 
. ../test_common.sh

. "$srcdir/test_nczarr.sh"

set -e

# Functions

ittest() {
extfor $1
if test "x$2" != x ; then CLOUD="-c $2"; fi
${execdir}/test_chunks -e $1 $CLOUD
${execdir}/test_chunks2 -e $1 $CLOUD
}

ittest file
if test "x$FEATURE_NCZARR_ZIP" = xyes ; then ittest zip; fi
if test "x$FEATURE_S3TESTS" = xyes ; then ittest s3 "${NCZARR_S3_TEST_URL}/${S3TESTPATH}'; fi
}



