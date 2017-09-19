#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

# For a netCDF-3 build, test nccopy on netCDF files in this directory

set -e
echo ""

# get some config.h parameters
if test -f ${top_builddir}/config.h ; then
  if fgrep -e '#define USE_CDF5 1' ${top_builddir}/config.h >/dev/null ; then
    HAVE_CDF5=1
  else
    HAVE_CDF5=0
  fi
else
  echo "Cannot locate config.h"
  exit 1
fi

TESTFILES='c0 c0tmp ctest0 ctest0_64 test0 test1
 tst_calendars tst_mslp tst_mslp_64 tst_ncml tst_small tst_utf8 utf8'

if test "x$HAVE_CDF5" = x1 ; then 
    TESTFILES="$TESTFILES small small2"
fi


echo "*** Testing netCDF-3 features of nccopy on ncdump/*.nc files"
for i in $TESTFILES ; do
    echo "*** Testing nccopy $i.nc copy_of_$i.nc ..."
    ${NCCOPY} $i.nc copy_of_$i.nc
    ${NCDUMP} -n copy_of_$i $i.nc > tmp.cdl
    ${NCDUMP} copy_of_$i.nc > copy_of_$i.cdl
    diff copy_of_$i.cdl tmp.cdl
    rm copy_of_$i.nc copy_of_$i.cdl tmp.cdl
done
echo "*** Testing nccopy -u"
${NCGEN} -b $srcdir/tst_brecs.cdl
# convert record dimension to fixed-size dimension
$NCCOPY -u tst_brecs.nc copy_of_tst_brecs.nc
${NCDUMP} -n copy_of_tst_brecs tst_brecs.nc | sed '/ = UNLIMITED ;/s/\(.*\) = UNLIMITED ; \/\/ (\(.*\) currently)/\1 = \2 ;/' > tmp.cdl
${NCDUMP} copy_of_tst_brecs.nc >  copy_of_tst_brecs.cdl
diff -b copy_of_tst_brecs.cdl tmp.cdl
rm copy_of_tst_brecs.cdl tmp.cdl tst_brecs.nc copy_of_tst_brecs.nc

echo "*** All nccopy tests passed!"
exit 0
