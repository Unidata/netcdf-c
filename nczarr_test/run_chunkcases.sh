#!/bin/sh

# Note that this test builds a special results.<pid> directory in
# which to run the tests, where <pid> is the process id number of
# the bash shell instance.  The reason for doing this is so that
# it works correctly under Github actions. By empirical
# observation, it appears that the various workflow matrix
# elements are not running in isolation when using cmake.  Rather
# they appear to be running simultaneously while sharing the build
# directory for cmake.  By running the tests in a separate
# results.<pid> I can guarantee that isolation is preserved.

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

. ${srcdir}/test_nczarr.sh

set -e

TC="${execdir}/tst_chunkcases -4"
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

testcases() {

zext=$1
echo ""; echo "*** Test format $1"
    
# Test whole chunk write and read
echo "Test whole chunk write then read"
makefile tmp_whole
rm -f tmp_whole_${zext}.txt tmp_whole_${zext}.cdl tmp_err_${zext}.txt
# This should fail 
if ! $TC -d 8,8 -c 4,4 -f 4,3 -e 4,4 -X w -OWw $F >> tmp_err_${zext}.txt ; then
echo "XFAIL: wholechunk with bad -f"
fi
remfile $file
if ! $TC -d 8,8 -c 4,4 -f 4,4 -e 1,4 -X w -OWw $F  >> tmp_err_${zext}.txt ; then

echo "XFAIL: wholechunk with bad -e"
fi
remfile $file
# This should succeed
$TC -d 8,8 -c 4,4 -f 4,4 -e 4,4 -X w -OWw $F
$TC -d 8,8 -c 4,4 -f 4,4 -e 4,4 -X w -OWr $F > tmp_whole_${zext}.txt
diff -b ${srcdir}/ref_whole.txt tmp_whole_${zext}.txt
${NCDUMP} $F > tmp_whole_${zext}.cdl
diff -b ${srcdir}/ref_whole.cdl tmp_whole_${zext}.cdl

# Test skipping whole chunks
echo "Test chunk skipping during read"
makefile tmp_skip
rm -f tmp_skip_${zext}.txt tmp_skip_${zext}.cdl
$TC -d 6,6 -c 2,2 -Ow $F
$TC -s 5,5 -p 6,6 -Or $F > tmp_skip_${zext}.txt
${NCDUMP} $F > tmp_skip_${zext}.cdl
diff -b ${srcdir}/ref_skip.txt tmp_skip_${zext}.txt
diff -b ${srcdir}/ref_skip.cdl tmp_skip_${zext}.cdl

echo "Test chunk skipping during write"
makefile tmp_skipw
rm -f tmp_skipw_${zext}.cdl

$TC -d 6,6 -s 5,5 -p 6,6 -Ow $F
${NCDUMP} $F > tmp_skipw_${zext}.cdl
diff -b ${srcdir}/ref_skipw.cdl tmp_skipw_${zext}.cdl

echo "Test dimlen % chunklen != 0"
makefile tmp_rem

rm -f tmp_rem_${zext}.txt tmp_rem_${zext}.cdl
$TC -d 8,8 -c 3,3 -Ow $F
${NCDUMP} $F > tmp_rem_${zext}.cdl
diff -b ${srcdir}/ref_rem.cdl tmp_rem_${zext}.cdl
${execdir}/ncdumpchunks -v v $F > tmp_rem_${zext}.txt
diff -b ${srcdir}/ref_rem.dmp tmp_rem_${zext}.txt

echo "Test rank > 2"
makefile tmp_ndims
rm -f tmp_ndims_${zext}.txt tmp_ndims_${zext}.cdl
$TC -d 8,8,8,8 -c 3,3,4,4 -Ow $F
${NCDUMP} $F > tmp_ndims_${zext}.cdl
diff -b ${srcdir}/ref_ndims.cdl tmp_ndims_${zext}.cdl
${execdir}/ncdumpchunks -v v $F > tmp_ndims_${zext}.dmp
diff -b ${srcdir}/ref_ndims.dmp tmp_ndims_${zext}.dmp

echo "Test miscellaneous 1"
makefile tmp_misc1
rm -f tmp_misc1_${zext}.txt tmp_misc1_${zext}.cdl
$TC -d 6,12,4 -c 2,3,1 -f 0,0,0 -e 6,1,4 -Ow $F
${NCDUMP} $F > tmp_misc1_${zext}.cdl
diff -b ${srcdir}/ref_misc1.cdl tmp_misc1_${zext}.cdl
${execdir}/ncdumpchunks -v v $F > tmp_misc1_${zext}.dmp
diff -b ${srcdir}/ref_misc1.dmp tmp_misc1_${zext}.dmp

} # testcases()

testcases file
if test "x$FEATURE_NCZARR_ZIP" = xyes ; then testcases zip; fi
if test "x$FEATURE_S3TESTS" = xyes ; then testcases s3; fi

