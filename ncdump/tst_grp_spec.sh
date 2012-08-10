#!/bin/sh
# This shell script tests ncdump -g option for specifying groups for
# which data is to be output.
# $Id$
srcdir=`dirname $0`
cd $srcdir
srcdir=`pwd`

# compute the build directory
builddir=`pwd`/..
# Hack for CYGWIN
cd $srcdir
if [ `uname | cut -d "_" -f 1` = "MINGW32" ]; then
    srcdir=`pwd | sed 's/\/c\//c:\//g'`
    builddir="$srcdir"/..
fi

set -e
echo ""
echo "*** Testing ncdump -g output for specifying group subsets"
echo "*** creating netcdf file tst_grp_spec.nc from ref_tst_grp_spec0.cdl..."
../ncgen/ncgen -k3 -o tmp_all.nc -b $srcdir/ref_tst_grp_spec0.cdl
echo "*** creating tmp_subset.cdl from tmp_all.nc with ncdump -g ..."
./ncdump -g g1,g4 -n tmp_subset tmp_all.nc > tmp_subset.cdl
echo "*** comparing tmp_subset.cdl with ref_tst_grp_spec.cdl..."
diff -b tmp_subset.cdl $srcdir/ref_tst_grp_spec.cdl

echo "*** All ncdump test output for -g option passed!"
exit 0
