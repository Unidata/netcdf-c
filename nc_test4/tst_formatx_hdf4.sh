#!/bin/sh
# This shell script tests the output several previous tests.
# $Id: tst_output.sh,v 1.17 2010/05/14 16:21:15 ed Exp $

PATH=tst_interops2.h4

echo ""
echo "*** Testing extended file format output."
set -e

echo "Test extended format output for a HDF4 file"
rm -f tmp
../ncdump/ncdump -K $PATH >tmp
if ! fgrep 'HDF4 mode=00000000' <tmp ; then
echo "*** Fail: extended format for an HDF4 file"
fi
