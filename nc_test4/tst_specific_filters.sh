#!/bin/bash 

# Test the implementations of specific filters
# Also test nc_inq_filter_avail

# WARNING: This file is also used to build nczarr_test/run_specific_filters.sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

set -e

if test "x$TESTNCZARR" = x1 ; then
. "$srcdir/test_nczarr.sh"
BLOSCARGS="32001,0,0,0,256,5,1,1"
BLOSCCODEC='[{\"id\": \"blosc\",\"clevel\": 5,\"blocksize\": 256,\"cname\": \"lz4\",\"shuffle\": 1}]'
else
BLOSCARGS="32001,0,0,4,256,5,1,1"
BLOSCCODEC='[{\"id\": \"blosc\",\"clevel\": 5,\"blocksize\": 256,\"cname\": \"lz4\",\"shuffle\": 1}]'
fi

# Load the findplugins function
. ${builddir}/findplugin.sh
echo "findplugin.sh loaded"

# Locate the plugin path and the library names; argument order is critical
# Find bzip2 and capture
# Assume all test filters are in same plugin dir
findplugin h5bzip2

# Function to remove selected -s attributes from file;
# These attributes might be platform dependent
sclean() {
    cat $1 \
 	| sed -e '/:_IsNetcdf4/d' \
	| sed -e '/:_Endianness/d' \
	| sed -e '/_NCProperties/d' \
	| sed -e '/_SuperblockVersion/d' \
      	| sed -e '/_Format/d' \
        | sed -e '/global attributes:/d' \
	| cat > $2
}

# Function to extract _Filter attribute from a file
# These attributes might be platform dependent
getfilterattr() {
V="$1"
sed -e '/${V}.*:_Filter/p' -ed <$2 >$3
}

# Function to extract _Codecs attribute from a file
# These attributes might be platform dependent
getcodecsattr() {
V="$1"
sed -e '/${V}.*:_Codecs/p' -ed <$2 >$3
}

trimleft() {
sed -e 's/[ 	]*\([^ 	].*\)/\1/' <$1 >$2
}


setfilter() {
    FF="$1"
    FSRC="$2"
    FDST="$3"
    FIH5="$4"
    FICX="$5"
    FFH5="$6"
    FFCX="$7"
    if test "x$FFH5" = x ; then FFH5="$FIH5" ; fi
    if test "x$FFCX" = x ; then FFCX="$FICX" ; fi
    rm -f $FDST
    cat ${srcdir}/$FSRC \
	| sed -e "s/ref_any/${FF}/" \
	| sed -e "s/IH5/${FIH5}/" -e "s/FH5/${FFH5}/" \
	| sed -e "s/ICX/${FICX}/" -e "s/FCX/${FFCX}/" \
	| sed -e 's/"/\\"/g' -e 's/@/"/g' \
	| cat > $FDST
}

# Execute the specified tests

runfilter() {
zext=$1
zfilt="$2"
zparams="$3"
zcodec="$4"
echo "*** Testing processing of filter $zfilt for map $zext"
if test "x$TESTNCZARR" = x1 ; then
fileargs "tmp_filt_${zfilt}"
deletemap $zext $file
else
file="tmp_filt_${zfilt}.nc"
rm -f $file
fi
setfilter $zfilt ref_any.cdl "tmp_filt_${zfilt}.cdl" "$zparams" "$zcodec"
if test "x$TESTNCZARR" = x1 ; then
${NCGEN} -4 -lb -o $fileurl "tmp_filt_${zfilt}.cdl"
${NCDUMP} -n $zfilt -sF $fileurl > "tmp_filt_${zfilt}.tmp"
else
${NCGEN} -4 -lb -o $file "tmp_filt_${zfilt}.cdl"
${NCDUMP} -n $zfilt -sF $file > "tmp_filt_${zfilt}.tmp"
fi
sclean "tmp_filt_${zfilt}.tmp" "tmp_filt_${zfilt}.dump"
}

testfletcher32() {
  zext=$1
  runfilter $zext fletcher32 '3' '[{\"id\": \"fletcher32\"}]'
  # need to do fixup
  sed -e '/_Fletcher32 = "true"/d' -e '/_Filter = 3'/d -e '/_Codecs = \"[{\"id\": \"fletcher32\"}]\"/d' \
	< tmp_filt_fletcher32.cdl > tmp_filt_fletcher32x.dump
  diff -b -w "tmp_filt_fletcher32.cdl" "tmp_filt_fletcher32x.dump"
}

testshuffle() {
  zext=$1
  runfilter $zext shuffle '2' '[{\"id\": \"shuffle\",\"elementsize\": \"0\"}]'
  # need to replace _Filter
  sed -e '/_Shuffle = "true"/d' -e '/_Filter = 2'/d -e '/_Codecs = \"[{\"id\": \"shuffle\"}]\"/d' \
	< tmp_filt_shuffle.cdl > tmp_filt_shufflex.dump
  diff -b -w "tmp_filt_shuffle.cdl" "tmp_filt_shufflex.dump"
}

testdeflate() {
  zext=$1
  if ! avail deflate; then return 0; fi
  runfilter $zext deflate '1,9' '[{\"id\": \"zlib\",\"level\": \"9\"}]'
  # need to replace _DeflateLevel
#  sed -e 's/_DeflateLevel = 9/_Filter = "1,9"/' < tmp_filt_deflate.dump > tmp_filt_deflatex.dump
  diff -b -w "tmp_filt_deflate.cdl" "tmp_filt_deflate.dump"
}

testbzip2() {
  zext=$1
  if ! avail bzip2; then return 0; fi
  runfilter $zext bzip2 '307,9' '[{\"id\": \"bz2\",\"level\": \"9\"}]'
  diff -b -w "tmp_filt_bzip2.cdl" "tmp_filt_bzip2.dump"
}

testszip() {
  zext=$1
  if ! avail szip; then return 0; fi
#  H5_SZIP_NN_OPTION_MASK=32;  H5_SZIP_MAX_PIXELS_PER_BLOCK_IN=32
  runfilter $zext szip '4,32,32' '[{\"id\": \"szip\",\"mask\": 32,\"pixels-per-block\": 32}]'
  diff -b -w "tmp_filt_szip.cdl" "tmp_filt_szip.dump"
}

testblosc() {
  zext=$1
  if ! avail blosc; then return 0; fi
  runfilter $zext blosc $BLOSCARGS "$BLOSCCODEC"
  diff -b -w "tmp_filt_blosc.cdl" "tmp_filt_blosc.dump"
}

testzstd() {
  zext=$1
  if ! avail zstd; then return 0; fi
  runfilter $zext zstd '32015,1' '[{\"id\": \"zstd\",\"level\": \"1\"}]'
  diff -b -w "tmp_filt_zstd.cdl" "tmp_filt_zstd.dump"
}

testset() {
# Which test cases to exercise
if test "x$TESTNCZARR" = x1 ; then
    testfletcher32 $1
    testshuffle $1    
fi
    testdeflate $1
    testszip $1
    testbzip2 $1
    testblosc $1
    testzstd $1
}

if test "x$TESTNCZARR" = x1 ; then
    testset file
    if test "x$FEATURE_NCZARR_ZIP" = xyes ; then testset zip ; fi
    if test "x$FEATURE_S3TESTS" = xyes ; then testset s3 ; fi
else
    testset nc
fi
