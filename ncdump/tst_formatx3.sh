#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

# This shell script runs the ncdump tests.
# get some config.h parameters
if test -f ${top_builddir}/config.h ; then
  if fgrep -e '#define USE_CDF5 1' ${top_builddir}/config.h >/dev/null ; then
    USE_CDF5=1
  else
    USE_CDF5=0
  fi
else
  echo "Cannot locate config.h"
  exit 1
fi

# This shell script tests the output several previous tests.

ECODE=0

echo ""
echo "*** Testing extended file format output."
set -e
echo "Test extended format output for a netcdf-3 file"
rm -f tmp
${NCGEN} -k nc3 -b -o ./test.nc $srcdir/ref_tst_small.cdl
${NCDUMP} -K test.nc >tmp
if ! grep 'classic mode=00000000' <tmp ; then
echo "*** Fail: extended format for a classic file"
ECODE=1
fi

echo "Test extended format output for a 64-bit offset netcdf-3 file"
rm -f tmp
${NCGEN} -k nc6 -b -o ./test.nc $srcdir/ref_tst_small.cdl
${NCDUMP} -K test.nc >tmp
if ! grep '64-bit offset mode=00000200' <tmp ; then
echo "*** Fail: extended format for a 64-bit classic file"
ECODE=1
fi


# Only do following test if USE_CDF5 is true.

if test "x$USE_CDF5" = x1 ; then
    echo "Test extended format output for a 64-bit CDF-5 classic file"
    rm -f tmp
    ${NCGEN} -k5 -b -o ./test.nc $srcdir/ref_tst_small.cdl
    ${NCDUMP} -K test.nc >tmp
    if ! grep -F '64-bit data mode=00000020' <tmp ; then
        echo "*** Fail: extended format for a 64-bit CDF-5 classic file"
        ECODE=1
    fi
fi

rm -f tmp test.nc

exit $ECODE
