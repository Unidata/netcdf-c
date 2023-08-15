#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

. ${srcdir}/test_nczarr.sh

set -x
set -e

s3isolate "testdir_unlim_io"
THISDIR=`pwd`
cd $ISOPATH

TC="${execdir}/test_unlim_io -4"
ZM="${execdir}/zmapio -t int"

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

makefile() {
  fileargs $1
  remfile $file
  case "$zext" in
  nc4) F=$file;;
  file) F=$fileurl;;
  zip) F=$fileurl;;
  s3) F=$fileurl;;
  *) echo "no such extension: $zext" ; exit 1;;
  esac
}

testcase1() {
zext=$1
echo ""; echo "*** Test simple use of unlimited"
rm -fr tmp_unlim_io1.nc tmp_unlim_io1.$zext tmp_unlim_io1.nc.txt tmp_unlim_io1.${zext}.txt
$TC -d 0 -c 2 -s 0 -e 1 -Ocw tmp_unlim_io1.nc
$TC -d 0 -c 2 -s 0 -e 1 -Ocw "file://tmp_unlim_io1.$zext\#mode=nczarr,$zext"
${NCDUMP} tmp_unlim_io1.nc >tmp_unlim_io1.nc.txt
${NCDUMP} "file://tmp_unlim_io1.$zext#mode=nczarr,$zext" >tmp_unlim_io1.${zext}.txt
diff -b tmp_unlim_io1.nc.txt tmp_unlim_io1.${zext}.txt
}

testcase2() {
zext=$1
echo ""; echo "*** Test 2-d chunking"
rm -fr tmp_unlim_io2.nc tmp_unlim_io2.$zext tmp_unlim_io2.nc.txt tmp_unlim_io2.${zext}.txt
$TC -d 0,0 -c 2,2 -s 0,0 -e 2,2 -v 17 -Ocw tmp_unlim_io2.nc
$TC -d 0,0 -c 2,2 -s 0,0 -e 2,2 -v 17 -Ocw "file://tmp_unlim_io2.$zext\#mode=nczarr,$zext"
${NCDUMP} tmp_unlim_io2.nc >tmp_unlim_io2.nc.txt
${NCDUMP} "file://tmp_unlim_io2.$zext#mode=nczarr,$zext" >tmp_unlim_io2.${zext}.txt
diff -b tmp_unlim_io2.nc.txt tmp_unlim_io2.${zext}.txt
${execdir}/ncdumpchunks -v v "file://tmp_unlim_io2.$zext#mode=nczarr,$zext"
}

testcase3() {
zext=$1
echo ""; echo "*** Test multi-chunk extension"
rm -fr tmp_unlim_io3.nc tmp_unlim_io3.$zext tmp_unlim_io3.nc.txt tmp_unlim_io3.${zext}.txt
$TC -d 0,0 -c 2,2 -s 6,6 -e 2,2 -v 17 -Ocw tmp_unlim_io3.nc
$TC -d 0,0 -c 2,2 -s 6,6 -e 2,2 -v 17 -Ocw "file://tmp_unlim_io3.$zext\#mode=nczarr,$zext"
${NCDUMP} tmp_unlim_io3.nc >tmp_unlim_io3.nc.txt
${NCDUMP} "file://tmp_unlim_io3.$zext#mode=nczarr,$zext" >tmp_unlim_io3.${zext}.txt
diff -b tmp_unlim_io3.nc.txt tmp_unlim_io3.${zext}.txt
}

testcase4() {
zext=$1
echo ""; echo "*** Test unlimited as second dimension"
rm -fr tmp_unlim_io4.nc tmp_unlim_io4.$zext tmp_unlim_io4.nc.txt tmp_unlim_io4.${zext}.txt
$TC -d 8,0 -c 2,2 -s 6,6 -e 2,2 -v 17 -Ocw tmp_unlim_io4.nc
$TC -d 8,0 -c 2,2 -s 6,6 -e 2,2 -v 17 -Ocw "file://tmp_unlim_io4.$zext\#mode=nczarr,$zext"
${NCDUMP} tmp_unlim_io4.nc >tmp_unlim_io4.nc.txt
${NCDUMP} "file://tmp_unlim_io4.$zext#mode=nczarr,$zext" >tmp_unlim_io4.${zext}.txt
diff -b tmp_unlim_io4.nc.txt tmp_unlim_io4.${zext}.txt
}

testcases() {
  testcase4 $1
}

testcases file
if test "x$FEATURE_NCZARR_ZIP" = xyes ; then testcases zip; fi
if test "x$FEATURE_S3TESTS" = xyes ; then testcases s3; fi
if test "x$FEATURE_S3TESTS" = xyes ; then s3sdkdelete "/${S3ISOPATH}" ; fi # Cleanup
