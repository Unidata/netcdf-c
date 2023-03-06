#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

set -e

# For a netCDF-4 build, test nccopy on netCDF files in this directory

#if 0
if test -f tst_group_data${ext} ; then ${execdir}/tst_group_data ; fi
if test -f tst_enum_data${ext} ; then ${execdir}/tst_enum_data ; fi
if test -f tst_comp${ext} ; then ${execdir}/tst_comp ; fi
if test -f tst_comp2${ext} ; then ${execdir}/tst_comp2 ; fi
#endif

echo ""

# These files are actually in $srcdir in distcheck builds, so they
# need to be handled differently.
# ref_tst_compounds2 ref_tst_compounds3 ref_tst_compounds4
TESTFILES0='tst_comp tst_comp2 tst_enum_data tst_fillbug
 tst_group_data tst_nans tst_opaque_data tst_solar_1 tst_solar_2
 tst_solar_cmp tst_special_atts'

TESTFILES="$TESTFILES0 tst_string_data"

# Causes memory leak; source unknown
MEMLEAK="tst_vlen_data"

echo "*** Testing netCDF-4 features of nccopy on ncdump/*.nc files"
for i in $TESTFILES ; do
    echo "*** Test nccopy $i.nc copy_of_$i.nc ..."
#    if test "x$i" = xtst_vlen_data ; then
#	ls -l tst_vlen_data*
#	ls -l *.nc
#    fi
    ${NCCOPY} $i.nc copy_of_$i.nc
    ${NCDUMP} -n copy_of_$i $i.nc > tmp_$i.cdl
    ${NCDUMP} copy_of_$i.nc > copy_of_$i.cdl
    echo "*** compare " with copy_of_$i.cdl
    diff copy_of_$i.cdl tmp_$i.cdl
    rm copy_of_$i.nc copy_of_$i.cdl tmp_$i.cdl
done

# echo "*** Testing compression of deflatable files ..."
./tst_compress
echo "*** Test nccopy -d1 can compress a classic format file ..."
${NCCOPY} -d1 tst_inflated.nc tst_deflated.nc
if test `wc -c < tst_deflated.nc` -ge  `wc -c < tst_inflated.nc`; then
    exit 1
fi
echo "*** Test nccopy -d1 can compress a netCDF-4 format file ..."
${NCCOPY} -d1 tst_inflated4.nc tst_deflated.nc
if test `wc -c < tst_deflated.nc` -ge  `wc -c < tst_inflated4.nc`; then
    exit 1
fi
echo "*** Test nccopy -d1 -s can compress a classic model netCDF-4 file even more ..."
${NCCOPY} -d1 -s tst_inflated.nc tmp_ncc4.nc
if test `wc -c < tmp_ncc4.nc` -ge  `wc -c < tst_inflated.nc`; then
    exit 1
fi
echo "*** Test nccopy -d1 -s can compress a netCDF-4 file even more ..."
${NCCOPY} -d1 -s tst_inflated4.nc tmp_ncc4.nc
if test `wc -c < tmp_ncc4.nc` -ge  `wc -c < tst_inflated4.nc`; then
    exit 1
fi
echo "*** Test nccopy -d0 turns off compression, shuffling of compressed, shuffled file ..."
${NCCOPY} -d0 tst_inflated4.nc tmp_ncc4.nc
${NCDUMP} -sh tmp_ncc4.nc > tmp_ncc4.cdl
if fgrep '_DeflateLevel' < tmp_ncc4.cdl ; then
    exit 1
fi
if fgrep '_Shuffle' < tmp_ncc4.cdl ; then
    exit 1
fi
rm tst_deflated.nc tst_inflated.nc tst_inflated4.nc tmp_ncc4.nc tmp_ncc4.cdl

echo "*** Testing nccopy -d1 -s on ncdump/*.nc files"
for i in $TESTFILES0 ; do
    echo "*** Test nccopy -d1 -s $i.nc copy_of_$i.nc ..."
    ${NCCOPY} -d1 -s $i.nc copy_of_$i.nc
    ${NCDUMP} -n copy_of_$i $i.nc > tmp_ncc4.cdl
    ${NCDUMP} copy_of_$i.nc > copy_of_$i.cdl
    #    echo "*** compare " with copy_of_$i.cdl
    diff copy_of_$i.cdl tmp_ncc4.cdl
    rm copy_of_$i.nc copy_of_$i.cdl tmp_ncc4.cdl
done
./tst_chunking
echo "*** Test that nccopy -c can chunk and unchunk files"
${NCCOPY} -M0 tst_chunking.nc tmp_ncc4.nc
${NCDUMP} tmp_ncc4.nc > tmp_ncc4.cdl
${NCCOPY} -c dim0/,dim1/1,dim2/,dim3/1,dim4/,dim5/1,dim6/ tst_chunking.nc tmp-chunked.nc
${NCDUMP} -n tmp_ncc4 tmp-chunked.nc > tmp-chunked.cdl
diff tmp_ncc4.cdl tmp-chunked.cdl
${NCCOPY} -c dim0/,dim1/,dim2/,dim3/,dim4/,dim5/,dim6/ tmp-chunked.nc tmp-unchunked.nc
${NCDUMP} -n tmp_ncc4 tmp-unchunked.nc > tmp-unchunked.cdl
diff tmp_ncc4.cdl tmp-unchunked.cdl
${NCCOPY} -c // tmp-chunked.nc tmp-unchunked2.nc
${NCDUMP} -n tmp_ncc4 tmp-unchunked.nc > tmp-unchunked2.cdl
diff tmp_ncc4.cdl tmp-unchunked2.cdl
echo "*** Test that nccopy -c works as intended for record dimension default (1)"
${NCGEN} -b -o tst_bug321.nc $srcdir/tst_bug321.cdl
${NCCOPY} -k nc7 -c"lat/2,lon/2" tst_bug321.nc tmp_ncc4.nc
${NCDUMP} -n tst_bug321 tmp_ncc4.nc > tmp_ncc4.cdl
diff -b $srcdir/tst_bug321.cdl tmp_ncc4.cdl

rm tst_chunking.nc tmp_ncc4.nc tmp_ncc4.cdl tmp-chunked.nc tmp-chunked.cdl tmp-unchunked.nc tmp-unchunked.cdl

echo "*** Test that nccopy -c dim/n works as intended "
${NCGEN} -4 -b -o tst_perdimspecs.nc $srcdir/ref_tst_perdimspecs.cdl
${NCCOPY} -M0 -4 -c "time/10,lat/15,lon/20" tst_perdimspecs.nc tmppds.nc
${NCDUMP} -hs tmppds.nc > tmppds.cdl
STORAGE=`cat tmppds.cdl | sed -e '/tas:_Storage/p' -ed | tr -d '\t \r'`
test "x$STORAGE" = 'xtas:_Storage="chunked";'
CHUNKSIZES=`cat tmppds.cdl | sed -e '/tas:_ChunkSizes/p' -ed | tr -d '\t \r'`
test "x$CHUNKSIZES" = 'xtas:_ChunkSizes=10,15,20;'

echo "*** Test that nccopy -F var1,none works as intended "
${NCGEN} -4 -b -o tst_nofilters.nc $srcdir/ref_tst_nofilters.cdl
${NCCOPY} -M0 -4 -F var1,none -c // tst_nofilters.nc tmp_nofilters.nc
${NCDUMP} -hs tmp_nofilters.nc > tmp_nofilters.cdl
STORAGE=`cat tmp_nofilters.cdl | sed -e '/var1:_Storage/p' -ed | tr -d '\t \r'`
test "x$STORAGE" = 'xvar1:_Storage="contiguous";'
FILTERS=`cat tmp_nofilters.cdl | sed -e '/var1:_Filters/p' -ed | tr -d '\t \r'`
test "x$FILTERS" = 'x'

echo "*** All nccopy tests passed!"

exit 0
