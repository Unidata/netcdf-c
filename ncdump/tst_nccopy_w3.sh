#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi 
. ../test_common.sh

set -e

echo "*** Test nccopy -w on netcdf classic file"
echo "*** create nccopy_w3.nc from ref_nccopy_w.cdl..."
${NCGEN} -lb -o nccopy_w3.nc -N nccopy_w $srcdir/ref_nccopy_w.cdl
echo "*** diskless copy nccopy_w3.nc to nccopy_w3c.nc..."
${NCCOPY} -w nccopy_w3.nc nccopy_w3c.nc
echo "*** Convert nccopy_w3c.nc to nccopy_w3c.cdl..."
${NCDUMP} -n nccopy_w nccopy_w3c.nc > nccopy_w3c.cdl
echo "*** comparing ref_nccopy_w.cdl nccopy_w3c.cdl..."
diff -b -w $srcdir/ref_nccopy_w.cdl nccopy_w3c.cdl

rm -f nccopy_w3c.cdl nccopy_w3.nc nccopy_w3c.nc

exit 0
