#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

. ${srcdir}/test_nczarr.sh

set -e

s3isolate "testdir_unlim_io"
THISDIR=`pwd`
cd $ISOPATH

TC="${execdir}/test_unlim_io -4"

remfile() {
  case "$zext" in
  nc4) rm -fr $1 ;;
  file) rm -fr $1 ;;
  zip) rm -fr $1 ;;
  s3) ;;
  *) echo "no such extension: $zext" ; exit 1;;
  esac
}

remfile() {
  case "$zext" in
  nc4) rm -fr $1 ;;
  file) rm -fr $1 ;;
  zip) rm -fr $1 ;;
  s3) ;;
  *) echo "no such extension: $zext" ; exit 1;;
  esac
}

buildfile() {
zext=$1
base=$2
if test "x$TESTNCZARR" = x1 ; then
fileargs "${base}_${zext}"
deletemap $zext $file
file="$fileurl"
else
file="${base}_${zext}.nc"
rm -f $file
fi
}

testcase1() {
zext=$1
buildfile $zext tmp_unlim_io1
echo ""; echo "*** Test simple use of unlimited"
rm -fr tmp_unlim_io1.nc tmp_unlim_io1.$zext tmp_unlim_io1.nc.txt tmp_unlim_io1.${zext}.txt
$TC -d 0 -c 2 -s 0 -e 1 -Ocw tmp_unlim_io1.nc
$TC -d 0 -c 2 -s 0 -e 1 -Ocw "$file"
${NCDUMP} -n tmp_unlim_io1 tmp_unlim_io1.nc >tmp_unlim_io1.nc.txt
${NCDUMP} -n tmp_unlim_io1 "$file" >tmp_unlim_io1.${zext}.txt
diff -b tmp_unlim_io1.nc.txt tmp_unlim_io1.${zext}.txt
}

testcase2() {
zext=$1
buildfile $zext tmp_unlim_io2
echo ""; echo "*** Test 2-d chunking"
rm -fr tmp_unlim_io2.nc tmp_unlim_io2.$zext tmp_unlim_io2.nc.txt tmp_unlim_io2.${zext}.txt
$TC -d 0,0 -c 2,2 -s 0,0 -e 2,2 -v 17 -Ocw tmp_unlim_io2.nc
$TC -d 0,0 -c 2,2 -s 0,0 -e 2,2 -v 17 -Ocw "$file"
${NCDUMP} -n tmp_unlim_io2 tmp_unlim_io2.nc >tmp_unlim_io2.nc.txt
${NCDUMP} -n tmp_unlim_io2 "$file" >tmp_unlim_io2.${zext}.txt
diff -b tmp_unlim_io2.nc.txt tmp_unlim_io2.${zext}.txt
${NCDUMPCHUNKS} -v v "$file"
}

testcase3() {
zext=$1
buildfile $zext tmp_unlim_io3
echo ""; echo "*** Test multi-chunk extension"
rm -fr tmp_unlim_io3.nc tmp_unlim_io3.$zext tmp_unlim_io3.nc.txt tmp_unlim_io3.${zext}.txt
$TC -d 0,0 -c 2,2 -s 6,6 -e 2,2 -v 17 -Ocw tmp_unlim_io3.nc
$TC -d 0,0 -c 2,2 -s 6,6 -e 2,2 -v 17 -Ocw "$file"
${NCDUMP} -n tmp_unlim_io3 tmp_unlim_io3.nc >tmp_unlim_io3.nc.txt
${NCDUMP} -n tmp_unlim_io3 "$file" >tmp_unlim_io3.${zext}.txt
diff -b tmp_unlim_io3.nc.txt tmp_unlim_io3.${zext}.txt
}

testcase4() {
zext=$1
buildfile $zext tmp_unlim_io4
echo ""; echo "*** Test unlimited as second dimension"
rm -fr tmp_unlim_io4.nc tmp_unlim_io4.$zext tmp_unlim_io4.nc.txt tmp_unlim_io4.${zext}.txt
$TC -d 8,0 -c 2,2 -s 6,6 -e 2,2 -v 17 -Ocw tmp_unlim_io4.nc
$TC -d 8,0 -c 2,2 -s 6,6 -e 2,2 -v 17 -Ocw "$file"
${NCDUMP} -n tmp_unlim_io3 tmp_unlim_io4.nc >tmp_unlim_io4.nc.txt
${NCDUMP} -n tmp_unlim_io3 "$file" >tmp_unlim_io4.${zext}.txt
diff -b tmp_unlim_io4.nc.txt tmp_unlim_io4.${zext}.txt
}

testcases() {
    testcase1 $1
    testcase2 $1
    testcase3 $1
    testcase4 $1
}

testcases file
if test "x$FEATURE_NCZARR_ZIP" = xyes ; then testcases zip; fi
if test "x$FEATURE_S3TESTS" = xyes ; then testcases s3; fi
