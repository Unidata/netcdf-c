#!/bin/sh
# Run test_chunk_hdf4 passing ${srcdir}



if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

set -e

echo ""
echo "*** Testing hdf4 chunking..."

if test "x${srcdir}" = "x" ; then
srcdir="."
fi

if ./tst_chunk_hdf4 ; then
  echo "***SUCCESS!! tst_chunk_hdf4"
else
  RES=$?
  echo "***FAIL: tst_chunk_hdf4"
  exit $RES
fi

exit 0
