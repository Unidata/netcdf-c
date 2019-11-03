#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi 
. ../test_common.sh

verbose=1
set -e

# Setup
PASS=1

# Do some limited testing of the ncgen -lc generated code

NETCDF4=0
if test -f ${top_builddir}/libnetcdf.settings ; then
  if grep 'HDF5 Support:.*yes' ${top_builddir}/libnetcdf.settings >/dev/null ; then
    NETCDF4=1
  fi
fi

# Dump classic files two ways and compare
dotest() {
K=$1
SF="_${K}"
for f in $2 ; do
  echo "Testing ${f}"
  F="${f}${SF}"
  rm -f ./ctest_${F}.c 
  ${NCGEN} -$K -lc ${srcdir}/ref_tst_${f}.cdl > ./ctest_${F}.c
  diff -w ./ctest_${F}.c ${srcdir}/ref_ctest_${F}.c
done
}

#dotest 3 "small"
if test "x$NETCDF4" = x1 ; then
#  dotest 4 "small special_atts"
  dotest 4 "special_atts"
fi
