#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

set -e

# Uncomment to run big test
#BIGTEST=1

# Test Urls (S3 URLS must be in path format)
URL3="https://thredds-test.unidata.ucar.edu/thredds/fileServer/pointData/cf_dsg/example/point.nc#mode=bytes"
#URL3="https://remotetest.unidata.ucar.edu/thredds/fileServer/testdata/2004050300_eta_211.nc#bytes"
URL4a="https://s3.us-east-1.amazonaws.com/noaa-goes16/ABI-L1b-RadC/2017/059/03/OR_ABI-L1b-RadC-M3C13_G16_s20170590337505_e20170590340289_c20170590340316.nc#mode=bytes"
URL4b="https://thredds-test.unidata.ucar.edu/thredds/fileServer/irma/metar/files/METAR_20170910_0000.nc#bytes"
# Do not use unless we know it has some permanence (note the segment 'testing' in the URL);
if test "x$BIGTEST" = x1 ; then
URL4c="https://s3.us-west-2.amazonaws.com/coawst-public/testing/HadCRUT.4.6.0.0.median.nc#mode=bytes"
fi

echo ""

rm -f tst_http_nc3.cdl tst_http_nc4?.cdl 

testbytes() {
TAG="$1"
EXPECTED="$2"
U="$3"
K=`${NCDUMP} -k "$U" | tr -d '\r'`
if test "x$K" != "x$EXPECTED" ; then
   echo "test_http: -k flag mismatch: expected=$EXPECTED have=$K"
   exit 1
fi
rm -f tst_http_$TAG.cdl
# Now test the reading of at least the metadata
${NCDUMP} -h "$U" >tst_http_$TAG.cdl
# compare
diff -wb tst_http_$TAG.cdl ${srcdir}/ref_tst_http_$TAG.cdl 
}

echo "*** Testing reading NetCDF-3 file with http"

echo "***Test remote classic file"
testbytes nc3 classic "$URL3"
if test "x$FEATURE_HDF5" = xyes ; then
echo "***Test remote netdf-4 file: s3"
testbytes nc4a netCDF-4 "$URL4a"
echo "***Test remote netcdf-4 file: non-s3"
testbytes nc4b netCDF-4 "$URL4b"
if test "x$BIGTEST" = x1 ; then
# Following is a non-permanent dataset
echo "***Test remote netdf-4 file: big s3"
testbytes nc4c netCDF-4 "$URL4c"
fi #BIGTEST

fi #HDF5

exit
