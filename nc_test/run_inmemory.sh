#!/bin/sh


if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

set -e

# Get the target OS and CPU
CPU=`uname -p`
OS=`uname`

#Constants
FILE3=tst_inmemory3
CREATE3=tst_inmemory3_create
FILE4=tst_inmemory4
CREATE4=tst_inmemory4_create

# For tst_open_mem NETCDF4 only
OMEMFILE=f03tst_open_mem.nc

echo ""
echo "*** Testing in-memory operations"

HASHDF5=`${top_builddir}/nc-config --has-hdf5`

# Execute the core of the inmemory tests
${execdir}/tst_inmemory
if test "x$HASNC4" = xyes ; then
${execdir}/tst_open_mem ${srcdir}/${OMEMFILE}
fi

echo "**** Test ncdump of the resulting inmemory data"
${NCDUMP} -n "${FILE3}" ${FILE3}.nc > ${FILE3}.cdl
${NCDUMP} -n "${FILE3}" ${CREATE3}.nc > ${CREATE3}.cdl
diff -wb ${FILE3}.cdl ${CREATE3}.cdl

if test "x$HASHDF5" = "xyes" ; then
${NCDUMP} ${FILE4}.nc > ${FILE4}.cdl
${NCDUMP} -n ${FILE4} ${CREATE4}.nc > ${CREATE4}.cdl
diff -wb ${FILE4}.cdl ${CREATE4}.cdl
fi

# cleanup
rm -f ${FILE3}.nc ${FILE4}.nc ${CREATE3}.nc ${CREATE4}.nc
rm -f ${FILE3}.cdl ${FILE4}.cdl ${CREATE3}.cdl ${CREATE4}.cdl

echo "PASS: all inmemory tests"

exit 0
