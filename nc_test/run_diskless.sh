#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

set -e

# Get the target OS and CPU
CPU=`uname -p`
OS=`uname`

#Constants
FILE1=tst_diskless.nc
FILE2=tst_diskless2.nc
FILE3=tst_diskless3.nc

echo ""
echo "*** Testing in-memory (diskless) files with and without persistence"

echo "**** Test diskless netCDF classic file without persistence"
${execdir}/tst_diskless
echo "PASS: diskless netCDF classic file without persistence"

if test "x$FEATURE_HDF5" = "xyes" ; then
echo ""
echo "**** Test diskless netCDF enhanced file without persistence"
${execdir}/tst_diskless netcdf4
echo "PASS: diskless netCDF enhanced file without persistence"
fi #FEATURE_HDF5

echo ""
echo "**** Test diskless netCDF classic file with persistence"
rm -f $FILE1
${execdir}/tst_diskless persist
if test -f $FILE1 ; then
echo "**** $FILE1 created"
# ${NCDUMP} $FILE1
echo "PASS: diskless netCDF classic file with persistence"
else
echo "#### $FILE1 not created"
echo "FAIL: diskless netCDF classic file with persistence"
fi

if test "x$FEATURE_HDF5" = "xyes" ; then
echo ""
echo "**** Test diskless netCDF enhanced file with persistence"
rm -f $FILE1
${execdir}/tst_diskless netcdf4 persist
if test -f $FILE1 ; then
echo "**** $FILE1 created"
# ${NCDUMP} $FILE1
echo "PASS: diskless netCDF enhanced file with persistence"
else
echo "$FILE1 not created"
echo "FAIL: diskless netCDF enhanced file with persistence"
fi

fi #FEATURE_HDF5

# Do extended netcdf enhanced test
if test "x$FEATURE_HDF5" = "xyes" ; then

ok=""
echo ""
echo "**** Test extended enhanced diskless netCDF with persistence"
rm -f $FILE2 tst_diskless2.cdl
${execdir}/tst_diskless2
if test -f $FILE2 ; then
  echo "**** $FILE2 created"
  # Do a cyle test
  if ${NCDUMP} $FILE2 |sed -e s/tst_diskless2/tmp1/ > tmp1.cdl ; then
    if ${NCGEN} -k nc4 -o tmp1.nc tmp1.cdl ;then
      if ${NCDUMP} tmp1.nc >tmp2.cdl ; then
        if diff -wb tmp1.cdl tmp2.cdl ; then
          ok=yes
        fi
      fi
    fi
  fi
else
  echo "#### $FILE2 not created"
fi
exit
rm -f tmp1.cdl tmp2.cdl tmp1.nc tmp2.nc

if test "x$ok" = xyes ; then
  echo "PASS: extended enhanced diskless netCDF with persistence"
else
  echo "FAIL: extended enhanced diskless netCDF with persistence"
fi

fi #FEATURE_HDF5

echo ""
echo "**** Testing nc_open in-memory (diskless) files"

# clear old files
rm -f tst_diskless3_file.cdl tst_diskless3_memory.cdl

echo ""
echo "**** Create baseline cdl"
rm -f $FILE3
${execdir}/tst_diskless3 file file:$FILE3
${NCDUMP} $FILE3 >tst_diskless3_file.cdl

echo ""
echo "**** Create and modify file using diskless"
rm -f $FILE3
${execdir}/tst_diskless3 diskless persist file:$FILE3
${NCDUMP} $FILE3 >tst_diskless3_memory.cdl

# compare
diff tst_diskless3_file.cdl tst_diskless3_memory.cdl

# cleanup
rm -f $FILE1 $FILE2 $FILE3 tst_diskless3_file.cdl tst_diskless3_memory.cdl

exit 0
