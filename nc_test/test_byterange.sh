#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

set -e

#Constants
URL3="http://149.165.169.123:8080/thredds/fileServer/testdata/2004050300_eta_211.nc#bytes"
URL4="http://noaa-goes16.s3.amazonaws.com/ABI-L1b-RadC/2017/059/03/OR_ABI-L1b-RadC-M3C13_G16_s20170590337505_e20170590340289_c20170590340316.nc#mode=bytes"

# See if netcdf-4 support is enabled
HAVENC4=`cat ${TOPBUILDDIR}/libnetcdf.settings | sed -e '/NetCDF-4[ ]*API:[ 	]*yes/p' -e d`
if test "x$HAVENC4" = x ; then HAVENC4=no; else HAVENC4=yes; fi

rm -f tst_http_nc3.cdl tst_http_nc4.cdl 

echo ""

echo "*** Testing reading NetCDF-3 file with http"
# Test using -k flag
K=`${NCDUMP} -k "$URL3"`
EXPECTED="classic"
if test "x$K" != "x$EXPECTED" ; then
   echo "test_http: -k flag mismatch: expected=$EXPECTED have=$K"
   exit 1
fi
# Now test the reading of at least the metadata
${NCDUMP} -h "$URL3" >tst_http_nc3.cdl
# compare
diff tst_http_nc3.cdl ${srcdir}/ref_tst_http_nc3.cdl 

if test "x$HAVENC4" = xyes ; then
echo "*** Testing reading NetCDF-4 file with http"
# Test using -k flag
K=`${NCDUMP} -k "$URL4"`
EXPECTED="netCDF-4"
if test "x$K" != "x$EXPECTED" ; then
   echo "test_http: -k flag mismatch: expected=$EXPECTED have=$K"
   exit 1
fi
# Now test the reading of at least the metadata
${NCDUMP} -h "$URL4" >tst_http_nc4.cdl
# compare
diff tst_http_nc4.cdl ${srcdir}/ref_tst_http_nc4.cdl 
fi

exit
