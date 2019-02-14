#!/bin/sh
# Copyright 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002,
# 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014,
# 2015, 2016, 2017, 2018
# University Corporation for Atmospheric Research/Unidata.

# See netcdf-c/COPYRIGHT file for more info.
# This shell script runs the examples for netCDF4.
# Ed Hartnett

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../../test_common.sh

echo "*** Running examples for netCDF-4."
set -e

echo "*** running simple_nc4 examples..."
${execdir}/simple_nc4_wr
${execdir}/simple_nc4_rd

echo "*** running simple_xy_nc4 examples..."
${execdir}/simple_xy_nc4_wr
${execdir}/simple_xy_nc4_rd

echo "*** Examples successful!"
exit 0
