#!/bin/bash

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

if test "x$TESTNCZARR" = x1 ; then
. "$srcdir/test_nczarr.sh"
fi

set -e

# Load the findplugins function
. ${builddir}/findplugin.sh
echo "findplugin.sh loaded"

# Set up HDF5_PLUGIN_PATH
export HDF5_PLUGIN_PATH=${HDF5_PLUGIN_DIR}

# Test operation with an unknown filter

# Function to remove selected -s attributes from file;
# These attributes might be platform dependent
sclean() {
    cat $1 \
 	| sed -e '/:_IsNetcdf4/d' \
	| sed -e '/:_Endianness/d' \
	| sed -e '/_NCProperties/d' \
	| sed -e '/_SuperblockVersion/d' \
	| cat > $2
}

# Function to extract _Filter attribute from a file
# These attributes might be platform dependent
getfilterattr() {
sed -e '/var.*:_Filter/p' -ed <$1 >$2
}

# Function to extract _Codecs attribute from a file
# These attributes might be platform dependent
getcodecsattr() {
sed -e '/var.*:_Codecs/p' -ed <$1 >$2
}

trimleft() {
sed -e 's/[ 	]*\([^ 	].*\)/\1/' <$1 >$2
}

# Locate the plugin path and the library names; argument order is critical

# Find noop and capture
findplugin h5unknown
UNKNOWNDIR="${HDF5_PLUGIN_DIR}"
UNKNOWNLIB="${HDF5_PLUGIN_LIB}"
UNKNOWNFILTER="${HDF5_PLUGIN_DIR}/${UNKNOWNLIB}"

# Getting the name  is especially tricky for dylib, which puts the version before the .dylib

# Verify
if ! test -f ${UNKNOWNFILTER} ; then echo "Unable to locate ${UNKNOWNFILTER}"; exit 1; fi

testunk() {
zext=$1	
echo "*** Testing access to filter info when filter implementation is not available for map $zext"
if test "x$TESTNCZARR" = x1 ; then
fileargs tmp_known
deletemap $zext $file
else
file="tmp_known_${zfilt}.nc"
rm -f $file
fileurl="$file"
fi
# build .nc file using unknown
${NCGEN} -lb -4 -o $fileurl ${srcdir}/../nc_test4/unknown.cdl
# dump and clean file when filter is avail
${NCDUMP} -hs $fileurl > ./tmp_known_$zext.txt
# Remove irrelevant -s output
sclean ./tmp_known_$zext.txt tmp_known_$zext.dump
# Hide the filter
rm -fr  ${UNKNOWNDIR}/save
mkdir -p  ${UNKNOWNDIR}/save
# Figure out all matching libs; make sure to remove .so, so.0, etc
LSRC=`${execdir}/../ncdump/ncpathcvt -F "${UNKNOWNDIR}"`
LDST=`${execdir}/../ncdump/ncpathcvt -F ${UNKNOWNDIR}/save`
mv ${LSRC}/*unknown* ${LDST}
# Verify that the filter is no longer defined
# Try to read the data; should xfail
if ${NCDUMP} -s $fileurl > ./tmp_unk_$zext.dmp ; then
echo "*** FAIL: filter found"
found=1
else
echo "*** XFAIL: filter not found"
found=0
fi
# Restore the filter
mv ${LDST}/*unknown* ${LSRC}
rm -fr  ${UNKNOWNDIR}/save
if test "x$found" = x1 ; then exit 1; fi
}

if test "x$TESTNCZARR" = x1 ; then
  testunk file
  if test "x$FEATURE_NCZARR_ZIP" = xyes ; then testunk zip ; fi
  if test "x$FEATURE_S3TESTS" = xyes ; then testunk s3 ; fi
else
testunk nc
fi

exit 0
