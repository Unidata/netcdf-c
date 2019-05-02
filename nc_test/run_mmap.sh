#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

set -e

# Get the target OS and CPU
CPU=`uname -p`
OS=`uname`

#Constants
FILE1=tst_mmap1.nc
FILE3=tst_mmap3.nc

echo ""
echo "*** Testing create files with mmap"

echo "**** Test create mmap netCDF classic file without persistence"
${execdir}/tst_diskless mmap
echo "PASS: create mmap netCDF classic file without persistence"

echo ""
echo "**** Test create mmap netCDF classic file with persistence"
rm -f $FILE1
${execdir}/tst_diskless mmap persist file:tst_mmap1.nc
if test -f $FILE1 ; then
echo "**** $FILE1 created"
# ${NCDUMP} $FILE1
echo "PASS: create mmap netCDF classic file with persistence"
else
echo "#### $FILE1 not created"
echo "FAIL: create mmap netCDF classic file with persistence"
fi

rm -f tmp1.cdl tmp2.cdl tmp1.nc tmp2.nc

echo ""
echo "**** Testing open files with mmap"

# clear old files
rm -f tst_diskless3_mmap_create.cdl
rm -f tst_diskless3_mmap_open.cdl

echo ""
echo "**** create and modify file using mmap"
rm -f $FILE3

${execdir}/tst_diskless3 mmap persist create
${NCDUMP} $FILE3 >tst_diskless3_mmap_create.cdl
# compare
diff ${srcdir}/ref_tst_diskless3_create.cdl tst_diskless3_mmap_create.cdl

echo ""
echo "**** open and modify file using mmap"
${execdir}/tst_diskless3 mmap persist open
${NCDUMP} $FILE3 >tst_diskless3_mmap_open.cdl
# compare
diff ${srcdir}/ref_tst_diskless3_open.cdl tst_diskless3_mmap_open.cdl

# cleanup
rm -f $FILE1 $FILE3 tst_diskless3_mmap_create.cdl tst_diskless3_mmap_open.cdl

exit
