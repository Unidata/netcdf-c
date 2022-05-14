#!/bin/sh
# Copyright 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002,
# 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014,
# 2015, 2016, 2017, 2018
# University Corporation for Atmospheric Research/Unidata.

# See netcdf-c/COPYRIGHT file for more info.

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../../test_common.sh

echo "*** Running filter example for netCDF-4."

set -e

echo "*** running test_filter example..."
. ${builddir}/findplugin.sh
echo "findplugin.sh loaded"

# Locate the plugin path and the library names; argument order is critical
# Find bzip2 and capture
findplugin h5bzip2
BZIP2LIB="${HDF5_PLUGIN_LIB}"
BZIP2DIR="${HDF5_PLUGIN_DIR}"
BZIP2PATH="${BZIP2DIR}/${BZIP2LIB}"

# Verify
if ! test -f ${BZIP2PATH} ; then echo "Unable to locate ${BZIP2PATH}"; exit 1; fi

rm -f ./bzip2.nc
export HDF5_PLUGIN_PATH="${HDF5_PLUGIN_DIR}"
echo "*** running filter_example..."
${execdir}/filter_example
rm -f ./bzip2.nc

echo "*** Filter example successful!"
exit 0
