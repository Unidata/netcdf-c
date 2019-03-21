#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi 
. ../test_common.sh

set -e

echo "*** Test nccopy -w on netcdf enhanced file"
echo "*** create nccopy_w4.nc from ref_nccopy_w.cdl..."
${NCGEN} -lb -o nccopy_w4.nc -N nccopy_w $srcdir/ref_nccopy_w.cdl
echo "*** diskless copy nccopy_w4.nc to nccopy_w4c.nc..."
${NCCOPY} -w nccopy_w4.nc nccopy_w4c.nc
echo "*** Convert nccopy_w4c.nc to nccopy_w4c.cdl..."
${NCDUMP} -n nccopy_w nccopy_w4c.nc > nccopy_w4c.cdl
echo "*** comparing ref_nccopy_w.cdl nccopy_w4c.cdl..."
diff -b -w $srcdir/ref_nccopy_w.cdl nccopy_w4c.cdl

rm -f nccopy_w4c.cdl
rm -f nccopy_w4.nc nccopy_w4c.nc

exit 0
