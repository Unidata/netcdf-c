#!/bin/sh

# Test the various plugin path defaults that can occur

if test "x$srcdir" = x ; then srcdir=`pwd`; fi 
. ../test_common.sh

#CMD="valgrind --leak-check=full"

if test "x$FP_MSVC" = x && test "x$FP_ISMINGW" = x ; then
FEATURE_PLUGIN_SEARCH_PATH=`echon "${FEATURE_PLUGIN_SEARCH_PATH}" | tr ':' ';'`
fi

TESTHPPWIN=`${execdir}/../ncdump/ncpathcvt -S';' -w "/tmp;${HOME}"`
TESTHPPNIX=`${execdir}/../ncdump/ncpathcvt -S';' -u "/tmp;${HOME}"`
#DFALTWIN=`${execdir}/../ncdump/ncpathcvt -S';' -w "C:\\ProgramData\hdf5\\lib\\plugin"`
#DFALTNIX=`${execdir}/../ncdump/ncpathcvt -S';' -u "/usr/local/hdf5/lib/plugin"`
DFALTWIN=`${execdir}/../ncdump/ncpathcvt -S';' -w "${FEATURE_PLUGIN_SEARCH_PATH}"`
DFALTNIX=`${execdir}/../ncdump/ncpathcvt -S';' -u "${FEATURE_PLUGIN_SEARCH_PATH}"`

FAIL=

# Test with no HDF5_PLUGIN_PATH
testnohpp() {
unset HDF5_PLUGIN_PATH
NOHPP1=`${execdir}/ncpluginpath -f global`
if test "x$ISMSVC" = x && test "x$ISMINGW" = x ; then
    NOHPP1=`${execdir}/../ncdump/ncpathcvt -S';' -u "$NOHPP1"`
    BASELINE="$DFALTNIX"
else
    NOHPP1=`${execdir}/../ncdump/ncpathcvt -S';' -w "$NOHPP1"`
    BASELINE="$DFALTWIN"
fi
if test "x$NOHPP1" = "x$BASELINE"; then TF=yes ; else TF=no; fi
echo "** compare: $NOHPP1 :: $BASELINE == $TF"
if test "$TF" = yes ; then
  echo "***PASS: default plugin path = |$NOHPP1|"
else
  FAIL=1
  echo "***FAIL: default plugin path = |$NOHPP1|"
fi
}

# Test with given HDF5_PLUGIN_PATH
testhpp() {
unset HDF5_PLUGIN_PATH
if test "x$ISMSVC" = x && test "x$ISMINGW" = x ; then
    export HDF5_PLUGIN_PATH="$TESTHPPNIX"
    HPP1=`${execdir}/ncpluginpath -f global`
    HPP1=`${execdir}/../ncdump/ncpathcvt -S';' -u "$HPP1"`
    BASELINE="$TESTHPPNIX"
else
    export HDF5_PLUGIN_PATH="$TESTHPPWIN"
    HPP1=`${execdir}/ncpluginpath -f global`
    HPP1=`${execdir}/../ncdump/ncpathcvt -S';' -w "$HPP1"`
    BASELINE="$TESTHPPWIN"
fi
unset TF
if test "x$HPP1" = "x$BASELINE"; then TF=yes ; else TF=no; fi
echo "** compare: $HPP1 :: $BASELINE == $TF"
if test "$TF" = yes ; then
  echo "***PASS: default plugin path: |$HPP1| HDF5_PLUGIN_PATH=|$HDF5_PLUGIN_PATH|"
else
  FAIL=1
  echo "***FAIL: default plugin path: |$HPP1| HDF5_PLUGIN_PATH=|$HDF5_PLUGIN_PATH|"
fi
}

testnohpp
testhpp

if test "x$FAIL" != x ; then exit 1; fi
exit 0
