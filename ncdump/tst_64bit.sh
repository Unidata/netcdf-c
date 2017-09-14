#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

# This shell script runs the ncdump tests.
# get some config.h parameters
if test -f ${top_builddir}/config.h ; then
  if fgrep -e '#define USE_CDF5 1' ${top_builddir}/config.h >/dev/null ; then
    CDF5=1
  else
    CDF5=0
  fi
else
  echo "Cannot locate config.h"
  exit 1
fi

echo ""
echo "*** Testing ncgen and ncdump with 64-bit offset format."
set -e
echo "*** creating test0.nc from test0.cdl..."
${NCGEN} -b -k2 $srcdir/test0.cdl
echo "*** creating test1.cdl from test0.nc..."
${NCDUMP} -n test1 test0.nc > test1.cdl
echo "*** creating test1.nc from test1.cdl..."
${NCGEN} -b -k2 test1.cdl
echo "*** creating test2.cdl from test1.nc..."
${NCDUMP} test1.nc > test2.cdl
cmp test1.cdl test2.cdl
echo "*** All ncgen and ncdump with 64-bit offset format tests passed!"


if [ "x$CDF5" == "x1" ]; then
echo ""
echo "*** Testing ncgen and ncdump with CDF5 format."
set -e
echo "*** creating test0.nc from test0.cdl..."
${NCGEN} -b -k5 $srcdir/test0.cdl
echo "*** creating test1.cdl from test0.nc..."
${NCDUMP} -n test1 test0.nc > test1.cdl
echo "*** creating test1.nc from test1.cdl..."
${NCGEN} -b -k5 test1.cdl
echo "*** creating test2.cdl from test1.nc..."
${NCDUMP} test1.nc > test2.cdl
cmp test1.cdl test2.cdl
echo "*** All ncgen and ncdump with CDF5 format tests passed!"
fi

exit 0
