#!/bin/sh
# This shell script runs the ncgen3 tests.
# $Id: run_tests.sh,v 1.9 2009/09/24 18:19:11 dmh Exp $
srcdir=`dirname $0`
cd $srcdir
srcdir=`pwd`

# compute the build directory
builddir=`pwd`/..
# Hack for CYGWIN
cd $srcdir
if [ `uname | cut -d "_" -f 1` = "MINGW32" ]; then
    srcdir=`pwd | sed 's/\/c\//c:\//g'`
    builddir="$srcdir"/..
fi
echo "*** Testing ncgen3."
set -e
echo "*** creating classic file c0.nc from c0.cdl..."
./ncgen3 -b -o c0.nc $srcdir/c0.cdl
echo "*** creating 64-bit offset file c0_64.nc from c0.cdl..."
./ncgen3 -k 64-bit-offset -b -o c0_64.nc $srcdir/c0.cdl

echo "*** Test successful!"
exit 0
