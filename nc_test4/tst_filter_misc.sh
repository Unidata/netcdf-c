#!/bin/bash 

# Test the filter install
# This cannot be run as a regular test
# because installation will not have occurred

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

if test "x$TESTNCZARR" = x1; then
. $srcdir/test_nczarr.sh
fi

set -e

isolate "testdir_filter_misc"
THISDIR=`pwd`
cd $ISOPATH

if test "x$TESTNCZARR" = x1; then
s3isolate
fi

# Load the findplugins function
. ${builddir}/findplugin.sh
echo "findplugin.sh loaded"

if ! filteravail bzip2 ; then 
   echo ">>> Filter bzip2 not available; discontinuing test"
   exit 0;
fi

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

testavail() {
  zext=$1
  if ! filteravail bzip2; then return 0; fi
  if test "x$TESTNCZARR" = x1 ; then
      ${execdir}/test_filter_avail
  else
      ${execdir}/tst_filter_avail
  fi
}

testvlen() {
  zext=$1
  if test "x$TESTNCZARR" = x1 ; then
      ${execdir}/test_filter_vlen
  else
      ${execdir}/tst_filter_vlen
  fi
}

testset() {
# Which test cases to exercise
    testavail $1
    testvlen $1
}

if test "x$TESTNCZARR" = x1 ; then
    testset file
    if test "x$FEATURE_NCZARR_ZIP" = xyes ; then testset zip ; fi
    if test "x$FEATURE_S3TESTS" = xyes ; then testset s3 ; fi
else
    testset nc
fi
