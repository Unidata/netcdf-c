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
  nz4) rm -fr $1 ;;
  nzf) rm -fr $1 ;;
  s3) ;;
  *) echo "no such extension: $zext" ; exit 1;;
  esac
}

makefile() {
  fileargs $1
  remfile $file
  case "$zext" in
  nc4) F=$file;;
  nz4) F=$fileurl;;
  nzf) F=$fileurl;;
  s3) F=$fileurl;;
  *) echo "no such extension: $zext" ; exit 1;;
  esac
}

mkdirresults() {
RESDIR=results.`echo $$`
echo "RESDIR=|$RESDIR|"
rm -fr $RESDIR
mkdir $RESDIR
}

delresults() {
rm -fr $RESDIR
}

runtests() {

echo ""; echo "*** Test format $1"

# Test whole chunk write and read
echo "Test whole chunk write then read"
makefile tmp_whole
# These two should fail 
if ! $TC -d 8,8 -c 4,4 -f 4,3 -e 4,4 -OWw $F >> tmp_err_${zext}.txt ; then
echo "XFAIL: wholechunk with bad -f"
fi
remfile $file
if ! $TC -d 8,8 -c 4,4 -f 4,4 -e 1,4 -OWw $F  >> tmp_err_${zext}.txt ; then
echo "XFAIL: wholechunk with bad -e"
fi
remfile $file
# This should succeed
$TC -d 8,8 -c 4,4 -f 4,4 -e 4,4 -OWw $F
$TC -d 8,8 -c 4,4 -f 4,4 -e 4,4 -OWr $F > tmp_whole_${zext}.txt
diff -b ${srcdir}/ref_whole.txt tmp_whole_${zext}.txt
${NCDUMP} $F > tmp_whole_${zext}.cdl
diff -b ${srcdir}/ref_whole.cdl tmp_whole_${zext}.cdl

# Test skipping whole chunks
echo "Test chunk skipping during read"
makefile tmp_skip
$TC -d 6,6 -c 2,2 -Ow $F
$TC -s 5,5 -p 6,6 -Or $F > tmp_skip_${zext}.txt
diff -b ${srcdir}/ref_skip.txt tmp_skip_${zext}.txt
${NCDUMP} $F > tmp_skip_${zext}.cdl
diff -b ${srcdir}/ref_skip.cdl tmp_skip_${zext}.cdl

echo "Test chunk skipping during write"
makefile tmp_skipw
$TC -d 6,6 -s 5,5 -p 6,6 -Ow $F
${NCDUMP} $F > tmp_skipw_${zext}.cdl
diff -b ${srcdir}/ref_skipw.cdl tmp_skipw_${zext}.cdl

echo "Test dimlen % chunklen != 0"
makefile tmp_rem
$TC -d 8,8 -c 3,3 -Ow $F
${NCDUMP} $F > tmp_rem_${zext}.cdl
diff -b ${srcdir}/ref_rem.cdl tmp_rem_${zext}.cdl
${execdir}/ncdumpchunks -v v $F > tmp_rem_${zext}.dmp
diff -b ${srcdir}/ref_rem.dmp tmp_rem_${zext}.dmp

echo "Test rank > 2"
makefile tmp_ndims
$TC -d 8,8,8,8 -c 3,3,4,4 -Ow $F
${execdir}/ncdumpchunks -v v $F > tmp_ndims_${zext}.dmp
diff -b ${srcdir}/ref_ndims.dmp tmp_ndims_${zext}.dmp
${NCDUMP} $F > tmp_ndims_${zext}.cdl
diff -b ${srcdir}/ref_ndims.cdl tmp_ndims_${zext}.cdl

echo "Test miscellaneous 1"
makefile tmp_misc1
$TC -d 6,12,4 -c 2,3,1 -f 0,0,0 -e 6,1,4 -Ow $F
${execdir}/ncdumpchunks -v v $F > tmp_misc1_${zext}.dmp
diff -b ${srcdir}/ref_misc1.dmp tmp_misc1_${zext}.dmp
${NCDUMP} $F > tmp_misc1_${zext}.cdl
diff -b ${srcdir}/ref_misc1.cdl tmp_misc1_${zext}.cdl

echo "Test writing avail > 0"
makefile tmp_avail1
$TC -d 6,12,100 -c 2,3,50 -f 0,0,0 -p 6,12,100 -Ow $F
$TC -f 0,0,0 -e 6,3,75 -Or $F > tmp_avail1_${zext}.txt
diff -b ${srcdir}/ref_avail1.txt tmp_avail1_${zext}.txt
${NCDUMP} $F > tmp_avail1_${zext}.cdl
diff -b ${srcdir}/ref_avail1.cdl tmp_avail1_${zext}.cdl
}

testcase() {
zext=$1
runtests
}

mkdirresults
cd $RESDIR
testcase nzf
if test "x$FEATURE_HDF5" = xyes ; then testcase nz4; fi
if test "x$FEATURE_S3TESTS" = xyes ; then testcase s3; fi
cd ..
delresults
