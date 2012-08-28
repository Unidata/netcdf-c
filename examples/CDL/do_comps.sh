#!/bin/sh
# This shell script runs the cmp test on the example programs.
# $Id: do_comps.sh,v 1.1 2006/06/27 17:44:54 ed Exp $
srcdir=`dirname $0`

# compute the build directory
# Do a hack to remove e.g. c: for CYGWIN
cd `pwd`
builddir=`pwd`/..

# Hack for CYGWIN
cd $srcdir
srcdir=`pwd`
if [ `uname | cut -d "_" -f 1` = "MINGW32" ]; then
    srcdir=`pwd | sed 's/\/c\//c:\//g'`
    builddir=`echo $builddir | sed 's/\/c\//c:\//g'`
fi

set -e
echo ""
echo "*** Testing that the CDL examples produced same files as C examples."
echo "*** checking simple_xy.nc..."
cmp simple_xy.nc ../C/simple_xy.nc

echo "*** checking sfc_pres_temp.nc..."
cmp sfc_pres_temp.nc ../C/sfc_pres_temp.nc

echo "*** checking pres_temp_4D.nc..."
cmp pres_temp_4D.nc ../C/pres_temp_4D.nc

echo "*** All CDL example comparisons worked!"
exit 0
