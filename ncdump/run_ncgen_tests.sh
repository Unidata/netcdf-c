#!/bin/sh
# This shell script runs the ncgen tests.
# $Id: run_tests.sh,v 1.10 2010/04/04 22:06:03 dmh Exp $

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

echo "*** Testing ncgen."
set -e

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

#VALGRIND="valgrind -q --error-exitcode=2 --leak-check=full"

validateNC() {
    BASENAME=$1
    INFILE=$top_srcdir/ncgen/$1.cdl
    TMPFILE=tst_$2.cdl
    shift
    shift
    ARGS=$@

    echo "*** generating $BASENAME.nc ***"
    if test "x$VALGRIND" = x ; then
    ${NCGEN} $ARGS -o $BASENAME.nc $INFILE
    else
    ${VALGRIND} ${NCGEN} $ARGS -o $BASENAME.nc $INFILE
    fi
    ${NCDUMP} $BASENAME.nc | sed 's/e+0/e+/g' > $TMPFILE
    echo "*** comparing $BASENAME.nc against $INFILE *** "
    diff -b -w $INFILE $TMPFILE
}

echo "*** creating classic file c0.nc from c0.cdl..."

validateNC c0 c0 -b

echo "*** creating 64-bit offset file c0_64.nc from c0.cdl..."

validateNC c0 "c0_64" -k 64-bit-offset -b

if test "x$USE_CDF5" = x1 ; then

    echo "*** creating 64-bit data file c5.nc from c5.cdl..."
    ${NCGEN} -k 64-bit-data -b -o c5.nc $top_srcdir/ncgen/c5.cdl
    if [ ! -f c5.nc ]; then
        echo "Failure."
        exit 1
    fi

fi

echo "*** Test successful!"
exit 0
