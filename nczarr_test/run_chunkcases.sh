#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

set -e

alias zc='../ncdump/.libs/ncdumpchunks -v v'

#TC="${execdir}/tst_chunkcases -4"
TC="${execdir}/tst_chunkcases"

#F="file://tmp_xxx.nzf#mode=nczarr,nzf"
F=tmp_xxx.nc
echo "Test BM case"
$TC -d 32,90,91 -c 8,10,13 -f 0,0,0 -p 32,1,91 -X wd4 -Ow $F
exit

# Test whole variable write and read
F="file://tmp_whole.nzf#mode=nczarr,nzf"
#F=tmp_whole.nc
echo "Test whole variable write then read"
rm -f tmp_whole.txt tmp_whole.cdl
$TC -d 6,6 -c 6,6 -X w -OWw $F
$TC -d 6,6 -c 6,6 -X w -OWr $F > tmp_whole.txt
diff -b ${srcdir}/ref_whole.txt tmp_whole.txt
${NCDUMP} $F > tmp_whole.cdl
diff -b ${srcdir}/ref_whole.cdl tmp_whole.cdl

# Test skipping whole chunks
F="file://tmp_skip.nzf#mode=nczarr,nzf"
#F=tmp_skip.nc
echo "Test chunk skipping during read"
rm -f tmp_skip.txt tmp_skip.cdl tmp_skipw.cdl
$TC -d 6,6 -c 2,2 -Ow $F
$TC -s 5,5 -p 6,6 -Or $F > tmp_skip.txt
${NCDUMP} $F > tmp_skip.cdl
diff -b ${srcdir}/ref_skip.txt tmp_skip.txt
diff -b ${srcdir}/ref_skip.cdl tmp_skip.cdl

echo "Test chunk skipping during write"
rm -f tmp_skipw.cdl
$TC -d 6,6 -s 5,5 -p 6,6 -Ow $F
${NCDUMP} $F > tmp_skipw.cdl
diff -b ${srcdir}/ref_skipw.cdl tmp_skipw.cdl

F="file://tmp_skip.nzf#mode=nczarr,nzf"
#F=tmp_skip.nc
echo "Test dimlen % chunklen != 0"
rm -f tmp_rem.txt tmp_rem.cdl
$TC -d 8,8 -c 3,3 -Ow $F
${NCDUMP} $F > tmp_rem.cdl
diff -b ${srcdir}/ref_rem.cdl tmp_rem.cdl
${execdir}/ncdumpchunks -v v $F > tmp_rem.txt
diff -b ${srcdir}/ref_rem.txt tmp_rem.txt

F="file://tmp_skip.nzf#mode=nczarr,nzf"
#F=tmp_skip.nc
echo "Test rank > 2"
rm -f tmp_ndims.txt tmp_ndims.cdl
$TC -d 8,8,8,8 -c 3,3,4,4 -Ow $F
${NCDUMP} $F > tmp_ndims.cdl
diff -b ${srcdir}/ref_ndims.cdl tmp_ndims.cdl
${execdir}/ncdumpchunks -v v $F > tmp_ndims.txt
diff -b ${srcdir}/ref_ndims.txt tmp_ndims.txt
