#!/bin/bash
# This shell script tests that an hdf4 file can be read a
# variable at a time.
#
# this was added in support of https://github.com/Unidata/netcdf-c/issues/264

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh


FILE=tst_interops2.h4

set -e

echo ""
echo "*** Testing reading an individual variable from an HDF4 file."

${NCDUMP} -v hdf4_dataset_type_0 $FILE
${NCDUMP} -v hdf4_dataset_type_1 $FILE
${NCDUMP} -v hdf4_dataset_type_2 $FILE
${NCDUMP} -v hdf4_dataset_type_3 $FILE
${NCDUMP} -v hdf4_dataset_type_4 $FILE
${NCDUMP} -v hdf4_dataset_type_5 $FILE
${NCDUMP} -v hdf4_dataset_type_6 $FILE
${NCDUMP} -v hdf4_dataset_type_7 $FILE

echo "*** Success."
