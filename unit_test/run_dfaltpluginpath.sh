#!/bin/sh

# Test the various plugin path defaults that can occur

if test "x$srcdir" = x ; then srcdir=`pwd`; fi 
. ../test_common.sh

set -x

#CMD="valgrind --leak-check=full"

TESTHPP=`${execdir}/../ncdump/ncpathcvt -C "/tmp;${HOME}"`
DFALTWIN=`${execdir}/../ncdump/ncpathcvt -C "C:\\ProgramData\hdf5\\lib\\plugin"`
DFALTNIX=`${execdir}/../ncdump/ncpathcvt -C "/usr/local/hdf5/lib/plugin"`

FAIL=

# Test with no HDF5_PLUGIN_PATH
unset HDF5_PLUGIN_PATH
NOHPP1=`${execdir}/ncpluginpath -f global`
NOHPP1=`${execdir}/../ncdump/ncpathcvt -C "$NOHPP1"`
if test "x$NOHPP1" = "x$DFALTNIX" || test "x$NOHPP1" = "x$DFALTWIN" ; then
echo "***PASS: default plugin path = |$NOHPP1|"
else
FAIL=1
echo "***FAIL: default plugin path = |$NOHPP1|"
fi

# Test with given HDF5_PLUGIN_PATH
unset HDF5_PLUGIN_PATH
export HDF5_PLUGIN_PATH="$TESTHPP"
HPP1=`${execdir}/ncpluginpath -f global`
HPP1=`${execdir}/../ncdump/ncpathcvt -C "$HPP1"`
if test "x$HPP1" = "x$TESTHPP" ; then
echo "***PASS: default plugin path: |$HPP1| HDF5_PLUGIN_PATH=|$HDF5_PLUGIN_PATH|"
else
FAIL=1
echo "***FAIL: default plugin path: |$HPP1| HDF5_PLUGIN_PATH=|$HDF5_PLUGIN_PATH|"
fi

if test "x$FAIL" != x ; then exit 1; fi
exit 0
