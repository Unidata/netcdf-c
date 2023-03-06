#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

. "$srcdir/test_nczarr.sh"

set -e
echo ""

#chunkclean src dst
chunkclean() {
    rm -f ./$2
    cat ./$1 | sed -e "/:_Storage/d" | sed -e "/:_ChunkSizes/d" > ./$2
}

#verifychunking cdl-file  <chunkspec>...
verifychunking() {
  f=$1
  shift
  for t in "$@" ; do
    x=`cat $f | tr -d "\t \r" | sed -e "/$t/p" -ed`
    if test "x$x" = x ; then echo "$f: $t not found"; exit 1; fi
  done
}

testcase() {
zext=$1
fileargs tmp
./tst_zchunks3 -e ${zext}
echo "*** Test that nccopy -c can chunk files"
${NCCOPY} -M0 tmp_chunks3.nc "$fileurl"
${NCDUMP} -n tmp -sh "$fileurl" > tmp_nccz.cdl
verifychunking tmp_nccz.cdl "ivar:_ChunkSizes=7,4,2,3,5,6,9;" "fvar:_ChunkSizes=9,6,5,3,2,4,7;"

fileargs tmp_chunked
./tst_zchunks3 -e ${zext}
${NCCOPY} -M0 -c dim0/,dim1/1,dim2/,dim3/1,dim4/,dim5/1,dim6/ tmp_chunks3.nc "$fileurl"
${NCDUMP} -sh -n tmp "$fileurl" > tmp_chunked.cdl
verifychunking tmp_chunked.cdl "ivar:_ChunkSizes=7,1,2,1,5,1,9;" "fvar:_ChunkSizes=9,1,5,1,2,1,7;"
chunkclean tmp_nccz.cdl tmpx.cdl
chunkclean tmp_chunked.cdl tmp_chunkedx.cdl
diff tmpx.cdl tmp_chunkedx.cdl

# Note that unchunked means that there is only one chunk
SRC="$fileurl"
fileargs tmp_unchunked
${NCCOPY} -M0 -c dim0/,dim1/,dim2/,dim3/,dim4/,dim5/,dim6/ "$SRC" "$fileurl"
${NCDUMP} -sh -n tmp "$fileurl" > tmp_unchunked.cdl
verifychunking tmp_unchunked.cdl "ivar:_ChunkSizes=7,4,2,3,5,6,9;" "fvar:_ChunkSizes=9,6,5,3,2,4,7;"
chunkclean tmp_unchunked.cdl tmp_unchunkedx.cdl
diff tmpx.cdl tmp_unchunkedx.cdl

# Test -c /
fileargs tmp_unchunked2
${NCCOPY} -M0 -c "//" "$SRC" "$fileurl"
${NCDUMP} -sh -n tmp "$fileurl" > tmp_unchunked2.cdl
verifychunking tmp_unchunked.cdl "ivar:_ChunkSizes=7,4,2,3,5,6,9;" "fvar:_ChunkSizes=9,6,5,3,2,4,7;"
chunkclean tmp_unchunked.cdl tmp_unchunkedx.cdl
diff tmpx.cdl tmp_unchunkedx.cdl

echo "*** Test that nccopy -c dim/n is used "
fileargs tmp_perdimspecs
${NCGEN} -4 -b -o "$fileurl" $srcdir/ref_perdimspecs.cdl
${NCDUMP} -n tmp_perdimspecs -hs "$fileurl" > tmp_perdimspecs.cdl
SRC=$fileurl
fileargs tmp_pds

${NCCOPY} -M0 -4 -c "time/10,lat/15,lon/20" "$SRC" "$fileurl"
${NCDUMP} -n tmp_pds -hs "$fileurl" > tmp_pds.cdl
STORAGE=`cat tmp_pds.cdl | sed -e "/tas:_Storage/p" -ed | tr '"' "'" | tr -d "\t \r"`
test "x$STORAGE" = "xtas:_Storage='chunked';"
CHUNKSIZES=`cat tmp_pds.cdl | sed -e "/tas:_ChunkSizes/p" -ed | tr -d "\t \r"`
test "x$CHUNKSIZES" = "xtas:_ChunkSizes=10,15,20;"
}

testcase file
if test "x$FEATURE_NCZARR_ZIP" = xyes ; then testcase zip; fi
if test "x$FEATURE_S3TESTS" = xyes ; then testcase s3; fi

echo "*** All nccopy nczarr tests passed!"
exit 0
