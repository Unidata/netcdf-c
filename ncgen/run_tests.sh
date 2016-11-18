#!/bin/sh
# This shell script runs the ncgen tests.
# $Id: run_tests.sh,v 1.10 2010/04/04 22:06:03 dmh Exp $

if test "x$srcdir" = x ; then
srcdir=`pwd`
fi

echo "*** Testing ncgen."
set -e

##
# Function to test a netCDF CDL file.
# 1. Generate binary nc.
# Use ncdump to compare against original CDL file.
# Input: CDL file name, minus the suffix.
# Other input: arguments.
#
# Example:
#     $ validateNC compound_datasize_test -k nc4
##
validateNC() {
    BASENAME=$1
    INFILE=$srcdir/$1.cdl
    TMPFILE="tst_$1".cdl
    shift
    ARGS=$@

    echo "*** generating $BASENAME.nc ***"
    ./ncgen $ARGS $INFILE
    ../ncdump/ncdump $BASENAME.nc > $TMPFILE
    echo "*** comparing binary against source CDL file *** "
    diff -b -w $INFILE $TMPFILE

}


echo "*** creating classic file c0.nc from c0.cdl..."

validateNC c0

echo "*** creating 64-bit offset file c0_64.nc from c0.cdl..."

validate c0 -k 64-bit-offset -b

echo "*** creating 64-bit offset file c5.nc from c5.cdl..."
validate c5 -k 64-bit-data -b

echo "*** Test successful!"
exit 0
