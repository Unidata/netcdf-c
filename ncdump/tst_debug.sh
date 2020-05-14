#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

# This shell script tests the output from several previous tests.
set -e
set -x

find . -type f

exit 1


