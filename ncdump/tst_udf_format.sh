#!/bin/sh
#
# This is part of the netCDF package. Copyright 2026 University
# Corporation for Atmospheric Research/Unidata. See COPYRIGHT file
# for conditions of use.
#
# Test for netCDF-C issue #3411. ncdump could open a UDF file and
# print its metadata, but then rejected the UDF extended format code
# while generating the virtual _Format attribute or processing -K.
#
# The test autoloads tst_udf_format_plugin through an RC file and opens
# a fixture recognized by its UDF magic number. It verifies that -hs
# prints the generic user-defined _Format value and -K prints the
# generic extended format name with its mode.
#
# Author: Edward Hartnett, 2026-07-21

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

if test -f ./tst_udf_format_plugin.so; then
    plugin=`pwd`/tst_udf_format_plugin.so
else
    plugin=`find .libs -maxdepth 1 -type f -name 'tst_udf_format_plugin.*' | head -1`
fi

if test -z "$plugin" || test ! -f "$plugin"; then
    echo "*** Fail: UDF format test plugin was not built"
    exit 1
fi

cat > tst_udf_format.ncrc <<EOF
NETCDF.UDF0.LIBRARY=$plugin
NETCDF.UDF0.INIT=tst_udf_format_init
NETCDF.UDF0.MAGIC=TUSF
EOF
printf TUSF00000 > tst_udf_format.udf

if ! NCRCENV_RC=`pwd`/tst_udf_format.ncrc ${NCDUMP} -hs tst_udf_format.udf > tst_udf_format.out 2> tst_udf_format.err; then
    cat tst_udf_format.err
    rm -f tst_udf_format.ncrc tst_udf_format.udf tst_udf_format.out tst_udf_format.err
    exit 1
fi

if ! grep ':_Format = "user-defined format" ;' tst_udf_format.out; then
    cat tst_udf_format.out
    rm -f tst_udf_format.ncrc tst_udf_format.udf tst_udf_format.out tst_udf_format.err
    exit 1
fi

if ! NCRCENV_RC=`pwd`/tst_udf_format.ncrc ${NCDUMP} -K tst_udf_format.udf > tst_udf_format.out 2> tst_udf_format.err; then
    cat tst_udf_format.err
    rm -f tst_udf_format.ncrc tst_udf_format.udf tst_udf_format.out tst_udf_format.err
    exit 1
fi

if ! grep 'user-defined format mode=' tst_udf_format.out; then
    cat tst_udf_format.out
    rm -f tst_udf_format.ncrc tst_udf_format.udf tst_udf_format.out tst_udf_format.err
    exit 1
fi

rm -f tst_udf_format.ncrc tst_udf_format.udf tst_udf_format.out tst_udf_format.err
