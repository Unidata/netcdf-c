#!/bin/bash 

# Test the implementations of specific filters

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

# Load the findplugins function
. ${builddir}/findplugin.sh
echo "findplugin.sh loaded"

# Locate the plugin path and the library names; argument order is critical
# Find bzip2 and capture
# Assume all test filters are in same plugin dir
findplugin h5bzip2

echo "final HDF5_PLUGIN_PATH=${HDF5_PLUGIN_PATH}"
export HDF5_PLUGIN_PATH

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
zext=$1
zfilt="$2"
zparams="$3"
zcodec="$4"
echo "*** Testing processing of filter $zfilt"
file="tmp_${zfilt}.nc"
rm -f "tmp_${zfilt},nc"
setfilter $zfilt ref_any.cdl "tmp_${zfilt}.cdl" "$zparams" "$zcodec"
if ${NCGEN} -4 -lb -o $file "tmp_${zfilt}.cdl" ; then
  ${NCDUMP} -n $zfilt -s $file > "tmp_${zfilt}.tmp"
  sclean "tmp_${zfilt}.tmp" "tmp_${zfilt}.dump"
fi
unset NCTRACING
}

testdeflate() {
  zext=$1
  runfilter $zext deflate '1,9' '[{\"id\": \"zlib\",\"level\": \"9\"}]'
  if test -f "tmp_deflate.dump" ; then
      # need to replace _DeflateLevel
      sed -e 's/_DeflateLevel = 9/_Filter = "1,9"/' < tmp_deflate.dump > tmp_deflatex.dump
      diff -b -w "tmp_deflate.cdl" "tmp_deflatex.dump"
  else
      echo "XFAIL: filter=deflate"
  fi
}

testbzip2() {
  zext=$1
  runfilter $zext bzip2 '307,9' '[{\"id\": \"bz2\",\"level\": \"9\"}]'
  if test -f "tmp_bzip2.dump" ; then
      diff -b -w "tmp_bzip2.cdl" "tmp_bzip2.dump"
  else
      echo "XFAIL: filter=bzip2"
  fi
}

testblosc() {
  zext=$1
  runfilter $zext blosc '32001,2,2,4,256,5,1,1' '[{\"id\": \"blosc\",\"clevel\": 5,\"blocksize\": 0,\"cname\": \"lz4\",\"shuffle\": 1}]'
  if test -f "tmp_blosc.dump" ; then
      diff -b -w "tmp_blosc.cdl" "tmp_blosc.dump"
  else
      echo "XFAIL: filter=blosc"
  fi
}

testset() {
# Which test cases to exercise
#    testdeflate $1
    testbzip2 $1
#    testblosc $1
}

testset nc

exit 0
