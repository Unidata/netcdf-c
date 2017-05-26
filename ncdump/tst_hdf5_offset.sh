#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi 
. ../test_common.sh

set -e

# This tests that we can detect an HDF5 file with an offset
L64="                                                                "
L128="${L64}${L64}"

rm -f ./offset.cdl ./offset.nc
cp ${srcdir}/small.cdl ./offset.cdl
${NCGEN} -4 ./offset.cdl

# Test a 512 byte offset
rm -f L512.txt L512.hdf5
L512="${L128}${L128}${L128}${L128}"
echo -n "$L512" > L512.txt
cat L512.txt offset.nc > L512.hdf5
K=`${NCDUMP} -k L512.hdf5`
if test "x$K" = "xnetCDF-4" ; then
echo "***Pass: 512 offset"
else
echo "***FAIL: 512 offset"
FAILURES=1
fi

# Test a 1024 byte offset
rm -f L1024.txt L1024.hdf5
L1024="${L512}${L512}"
echo -n "$L1024" > L1024.txt
cat L1024.txt offset.nc > L1024.hdf5
K=`${NCDUMP} -k L1024.hdf5`
if test "x$K" = "xnetCDF-4" ; then
echo "***Pass: 1024 offset"
else
echo "***FAIL: 1024 offset"
FAILURES=1
fi

#cleanup
rm -f L512.txt L512.hdf5
rm -f L1024.txt L1024.hdf5
rm -f ./offset.cdl ./offset.nc

if test "x$FAILURES" = x1 ; then
exit 1
fi
exit 0
