#!/bin/sh
# This shell script runs the PIO examples.
# Ed Hartnett 12/18/17

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../../test_common.sh

echo "*** Running PIO examples."
set -e

echo "*** running pio_simple_async..."
mpiexec -n 4 ./pio_simple_async

echo "*** Examples for PIO successful!"
exit 0
