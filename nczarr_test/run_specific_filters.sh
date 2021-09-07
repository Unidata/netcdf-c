#!/bin/bash 

# Test the implementations of specific filters

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

. "$srcdir/test_nczarr.sh"

set -e

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
zext="$1"
zfilt="$2"
zparams="$3"
zcodec="$4"
echo "*** Testing processing of filter $zfilt for map $zext"
deletemap $zext "tmp_${zfilt}"
fileargs "tmp_${zfilt}"
setfilter $zfilt ref_any.cdl "tmp_${zfilt}.cdl" "$zparams" "$zcodec"
if ${NCGEN} -4 -lb -o $fileurl "tmp_${zfilt}.cdl" ; then
  ${NCDUMP} -n $zfilt -s $fileurl > "tmp_${zfilt}.tmp"
  sclean "tmp_${zfilt}.tmp" "tmp_${zfilt}.dump"
fi
}

testfletcher32() {
  zext=$1
  runfilter $zext fletcher32 '3' '[{\"id\": \"fletcher32\"}]'
  if test -f "tmp_fletcher32.dump" ; then
      # need to remove _Filter
      sed -e '/_Fletcher32 = "true"/d' < tmp_fletcher32.dump > tmp_fletcher32x.dump
      diff -b -w "tmp_fletcher32.cdl" "tmp_fletcher32x.dump"
  else
      echo "XFAIL: filter=fletcher32 zext=$zext"
  fi
}

testshuffle() {
  zext=$1
  runfilter $zext shuffle '2' '[{\"id\": \"shuffle\",\"elementsize\": \"0\"}]'
  if test -f "tmp_shuffle.dump" ; then
      # need to replace _Filter
      sed -e 's/_Filter = "2,4"/_Filter = "2"/' -e '/_Shuffle = "true"/d' < tmp_shuffle.dump > tmp_shufflex.dump
      diff -b -w "tmp_shuffle.cdl" "tmp_shufflex.dump"
  else
      echo "XFAIL: filter=shuffle zext=$zext"
  fi
}

testdeflate() {
  zext=$1
  runfilter $zext deflate '1,9' '[{\"id\": \"zlib\",\"level\": \"9\"}]'
  if test -f "tmp_deflate.dump" ; then
      # need to replace _DeflateLevel
      sed -e 's/_DeflateLevel = 9/_Filter = "1,9"/' < tmp_deflate.dump > tmp_deflatex.dump
      diff -b -w "tmp_deflate.cdl" "tmp_deflatex.dump"
  else
      echo "XFAIL: filter=deflate zext=$zext"
  fi
}

testbzip2() {
  zext=$1
  runfilter $zext bzip2 '307,9' '[{\"id\": \"bz2\",\"level\": \"9\"}]'
  if test -f "tmp_bzip2.dump" ; then
      diff -b -w "tmp_bzip2.cdl" "tmp_bzip2.dump"
  else
      echo "XFAIL: filter=bzip2 zext=$zext"
  fi
}

testszip() {
  zext=$1
#  H5_SZIP_NN_OPTION_MASK=32;  H5_SZIP_MAX_PIXELS_PER_BLOCK_IN=32
  runfilter $zext szip '4,32,32' '[{\"id\": \"szip\",\"mask\": 32,\"pixels-per-block\": 32}]'
  if test -f "tmp_szip.dump" ; then
      diff -b -w "tmp_szip.cdl" "tmp_szip.dump"
  else
      echo "XFAIL: filter=szip zext=$zext"
  fi
}

testblosc() {
  zext=$1
  runfilter $zext blosc '32001,0,0,0,0,5,1,1' '[{\"id\": \"blosc\",\"clevel\": 5,\"blocksize\": 0,\"cname\": \"lz4\",\"shuffle\": 1}]'
  if test -f "tmp_blosc.dump" ; then
      diff -b -w "tmp_blosc.cdl" "tmp_blosc.dump"
  else
      echo "XFAIL: filter=blosc zext=$zext"
  fi
}

testset() {
# Which test cases to exercise
    testfletcher32 $1
    testshuffle $1    
    testdeflate $1
    testszip $1
    testbzip2 $1
    testblosc $1
}

testset file
#if test "x$FEATURE_NCZARR_ZIP" = xyes ; then testset zip ; fi
#if test "x$FEATURE_S3TESTS" = xyes ; then testset s3 ; fi

exit 0
