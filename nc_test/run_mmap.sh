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
echo "*** Testing in-memory (diskless) files with mmap"

echo "**** Test diskless+mmap netCDF classic file without persistence"
${execdir}/tst_diskless mmap
echo "PASS: diskless+mmap netCDF classic file without persistence"

echo ""
echo "**** Test diskless+mmap netCDF classic file with persistence"
rm -f $FILE1
${execdir}/tst_diskless mmap persist
if test -f $FILE1 ; then
echo "**** $FILE1 created"
# ${NCDUMP} $FILE1
echo "PASS: diskless+mmap netCDF classic file with persistence"
else
echo "#### $FILE1 not created"
echo "FAIL: diskless+mmap netCDF classic file with persistence"
fi

rm -f tmp1.cdl tmp2.cdl tmp1.nc tmp2.nc

echo ""
echo "**** Testing nc_open in-memory (diskless+mmap) files"

# clear old files
rm -f tst_diskless3_file.cdl tst_diskless3_memory.cdl

echo ""
echo "**** Create and modify file without using diskless+mmap"
rm -f $FILE3
${execdir}/tst_diskless3
${NCDUMP} $FILE3 >tst_diskless3_file.cdl

echo ""
echo "**** Create and modify file using diskless+mmap"
rm -f $FILE3
${execdir}/tst_diskless3 diskless mmap
${NCDUMP} $FILE3 >tst_diskless3_memory.cdl

# compare
diff tst_diskless3_file.cdl tst_diskless3_memory.cdl

# cleanup
rm -f $FILE3 tst_diskless3_file.cdl tst_diskless3_memory.cdl

exit
