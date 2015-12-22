#!/bin/sh
# This shell script runs the ncgen tests.
# $Id: run_tests.sh,v 1.10 2010/04/04 22:06:03 dmh Exp $

if test "x$srcdir" = x ; then
srcdir="."
fi

echo "*** Testing ncgen."
set -e

echo "*** creating classic file c0.nc from c0.cdl..."
./ncgen -b -o c0.nc $srcdir/c0.cdl
if [ ! -f c0.nc ]; then
    echo "Failure."
    exit 1
fi

echo "*** creating 64-bit offset file c0_64.nc from c0.cdl..."
./ncgen -k 64-bit-offset -b -o c0_64.nc $srcdir/c0.cdl
if [ ! -f c0_64.nc ]; then
    echo "Failure."
    exit 1
fi

echo "*** creating 64-bit offset file c5.nc from c5.cdl..."
./ncgen -k 64-bit-data -b -o c5.nc $srcdir/c5.cdl
if [ ! -f c5.nc ]; then
    echo "Failure."
    exit 1
fi

echo "*** Test successful!"
exit 0
