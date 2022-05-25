#!/bin/bash 

# Test the filter install
# This cannot be run as a regular test
# because installation will not have occurred

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

if test "x$TESTNCZARR" = x1 ; then
. ./test_nczarr.sh
fi

set -e

# Use this plugin path
export HDF5_PLUGIN_PATH="${FEATURE_PLUGIN_INSTALL_DIR}"

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

testbzip2() {
  zext=$1
  if ! avail bzip2; then return 0; fi
  runfilter $zext bzip2 '307,9' '[{\"id\": \"bz2\",\"level\": \"9\"}]'
  diff -b -w "tmp_filt_bzip2.cdl" "tmp_filt_bzip2.dump"
}

testset() {
# Which test cases to exercise
    testbzip2 $1
}

if test "x$TESTNCZARR" = x1 ; then
    testset file
    if test "x$FEATURE_NCZARR_ZIP" = xyes ; then testset zip ; fi
    if test "x$FEATURE_S3TESTS" = xyes ; then testset s3 ; fi
else
    testset nc
fi
