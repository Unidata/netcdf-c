#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

set -e
echo ""

#chunkclean src dst
chunkclean() {
    rm -f ./$2
    cat ./$1 | sed -e '/:_Storage/d' | sed -e '/:_ChunkSizes/d' > ./$2
}

#verifychunking cdl-file  <chunkspec>...
verifychunking() {
  f=$1
  shift
  for t in "$@" ; do
    x=`cat $f | tr -d '\t \r' | sed -e "/$t/p" -ed`
    if test "x$x" = x ; then echo "$f: $t not found"; exit 1; fi
  done
}

./tst_chunks3 -e nzf
echo "*** Test that nccopy -c can chunk files"
${NCCOPY} -M0 tst_chunks3.nc 'file://tmp.nzf#mode=nczarr,nzf'
${NCDUMP} -n tmp -sh 'file://tmp.nzf#mode=nczarr,nzf' > tmp.cdl
verifychunking tmp.cdl 'ivar:_ChunkSizes=7,4,2,3,5,6,9;' 'fvar:_ChunkSizes=9,6,5,3,2,4,7;'

${NCCOPY} -M0 -c dim0/,dim1/1,dim2/,dim3/1,dim4/,dim5/1,dim6/ tst_chunks3.nc 'file://tmp_chunked.nzf#mode=nczarr,nzf'
${NCDUMP} -sh -n tmp 'file://tmp_chunked.nzf#mode=nczarr,nzf' > tmp_chunked.cdl
verifychunking tmp_chunked.cdl 'ivar:_ChunkSizes=7,1,2,1,5,1,9;' 'fvar:_ChunkSizes=9,1,5,1,2,1,7;'
chunkclean tmp.cdl tmpx.cdl
chunkclean tmp_chunked.cdl tmp_chunkedx.cdl 
diff tmpx.cdl tmp_chunkedx.cdl

# Note that unchunked means that there is only one chunk
${NCCOPY} -M0 -c dim0/,dim1/,dim2/,dim3/,dim4/,dim5/,dim6/ 'file://tmp_chunked.nzf#mode=nczarr,nzf' 'file://tmp_unchunked.nzf#mode=nczarr,nzf'
${NCDUMP} -sh -n tmp 'file://tmp_unchunked.nzf#mode=nczarr,nzf' > tmp_unchunked.cdl
verifychunking tmp_unchunked.cdl 'ivar:_ChunkSizes=7,4,2,3,5,6,9;' 'fvar:_ChunkSizes=9,6,5,3,2,4,7;'
chunkclean tmp_unchunked.cdl tmp_unchunkedx.cdl 
diff tmpx.cdl tmp_unchunkedx.cdl

# Test -c /
${NCCOPY} -M0 -c '/' 'file://tmp_chunked.nzf#mode=nczarr,nzf' 'file://tmp_unchunked2.nzf#mode=nczarr,nzf'
${NCDUMP} -sh -n tmp 'file://tmp_unchunked2.nzf#mode=nczarr,nzf' > tmp_unchunked2.cdl
verifychunking tmp_unchunked.cdl 'ivar:_ChunkSizes=7,4,2,3,5,6,9;' 'fvar:_ChunkSizes=9,6,5,3,2,4,7;'
chunkclean tmp_unchunked.cdl tmp_unchunkedx.cdl 
diff tmpx.cdl tmp_unchunkedx.cdl

echo "*** Test that nccopy -c dim/n is used "
${NCGEN} -4 -b -o 'file://tst_perdimspecs.nzf#mode=nczarr,nzf' $srcdir/ref_perdimspecs.cdl
${NCDUMP} -n tmp_perdimspecs -hs 'file://tst_perdimspecs.nzf#mode=nczarr,nzf' > tmp_perdimspecs.cdl
${NCCOPY} -M0 -4 -c "time/10,lat/15,lon/20" 'file://tst_perdimspecs.nzf#mode=nczarr,nzf' 'file://tmppds.nzf#mode=nczarr,nzf'
${NCDUMP} -n tmppds -hs 'file://tmppds.nzf#mode=nczarr,nzf' > tmppds.cdl
STORAGE=`cat tmppds.cdl | sed -e '/tas:_Storage/p' -ed | tr -d '\t \r'`
test "x$STORAGE" = 'xtas:_Storage="chunked";'
CHUNKSIZES=`cat tmppds.cdl | sed -e '/tas:_ChunkSizes/p' -ed | tr -d '\t \r'`
test "x$CHUNKSIZES" = 'xtas:_ChunkSizes=10,15,20;'

# Cleanup
rm -fr tmp*.nc tmp*.cdl
rm -fr tmp*.nzf

echo "*** All nccopy nczarr tests passed!"
exit 0

