#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

. ${srcdir}/test_nczarr.sh

set -e

#zext=nc4
zext=nzf

alias zc='../ncdump/.libs/ncdumpchunks -v v'

TC="${execdir}/tst_chunkcases -4"

makefile() {
  fileargs $1
  case "$zext" in
  nc4) F=$file;;
  nzf) F=$fileurl;;
  *) echo "no such extension: $zext" ; exit 1;;
  esac
}

# Test whole variable write and read

if test "x$zext" != xnc4 ; then
echo "Test whole variable write then read"
makefile tmp_whole
rm -f tmp_whole.txt tmp_whole.cdl
$TC -d 6,6 -c 6,6 -X w -OWw $F
$TC -d 6,6 -c 6,6 -X w -OWr $F > tmp_whole.txt
diff -b ${srcdir}/ref_whole.txt tmp_whole.txt
${NCDUMP} $F > tmp_whole.cdl
diff -b ${srcdir}/ref_whole.cdl tmp_whole.cdl
fi

# Test skipping whole chunks
echo "Test chunk skipping during read"
makefile tmp_skip
rm -f tmp_skip.txt tmp_skip.cdl tmp_skipw.cdl
$TC -d 6,6 -c 2,2 -Ow $F
$TC -s 5,5 -p 6,6 -Or $F > tmp_skip.txt
${NCDUMP} $F > tmp_skip.cdl
diff -b ${srcdir}/ref_skip.txt tmp_skip.txt
diff -b ${srcdir}/ref_skip.cdl tmp_skip.cdl

echo "Test chunk skipping during write"
makefile tmp_skipw
rm -f tmp_skipw.cdl
$TC -d 6,6 -s 5,5 -p 6,6 -Ow $F
${NCDUMP} $F > tmp_skipw.cdl
diff -b ${srcdir}/ref_skipw.cdl tmp_skipw.cdl

echo "Test dimlen % chunklen != 0"
makefile tmp_rem
rm -f tmp_rem.txt tmp_rem.cdl
$TC -d 8,8 -c 3,3 -Ow $F
${NCDUMP} $F > tmp_rem.cdl
diff -b ${srcdir}/ref_rem.cdl tmp_rem.cdl
${execdir}/ncdumpchunks -v v $F > tmp_rem.txt
diff -b ${srcdir}/ref_rem.txt tmp_rem.txt

echo "Test rank > 2"
makefile tmp_ndims
rm -f tmp_ndims.txt tmp_ndims.cdl
$TC -d 8,8,8,8 -c 3,3,4,4 -Ow $F
${NCDUMP} $F > tmp_ndims.cdl
diff -b ${srcdir}/ref_ndims.cdl tmp_ndims.cdl
${execdir}/ncdumpchunks -v v $F > tmp_ndims.txt
diff -b ${srcdir}/ref_ndims.txt tmp_ndims.txt

echo "Test miscellaneous 1"
makefile tmp_misc1
rm -f tmp_misc1.txt tmp_misc1.cdl
$TC -d 6,12,4 -c 2,3,1 -f 0,0,0 -e 6,1,4 -Ow $F
${NCDUMP} $F > tmp_misc1.cdl
diff -b ${srcdir}/ref_misc1.cdl tmp_misc1.cdl
${execdir}/ncdumpchunks -v v $F > tmp_misc1.txt
diff -b ${srcdir}/ref_misc1.txt tmp_misc1.txt
