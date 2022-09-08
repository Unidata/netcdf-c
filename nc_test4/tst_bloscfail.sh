#!/bin/bash

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

set -e

# Load the findplugins function
. ${builddir}/findplugin.sh
echo "findplugin.sh loaded"

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

trimleft() {
sed -e 's/[ 	]*\([^ 	].*\)/\1/' <$1 >$2
}

if test "x$TESTNCZARR" = x1 ; then
. "$srcdir/test_nczarr.sh"
fi

if ! avail blosc; then echo "Blosc compressor not found"; exit 0; fi

# Locate the plugin dir and the library names; argument order is critical
# Find bzip2 and capture
findplugin h5blosc
BLOSCLIB="${HDF5_PLUGIN_LIB}"
BLOSCDIR="${HDF5_PLUGIN_DIR}/${BZIP2LIB}"

echo "final HDF5_PLUGIN_DIR=${HDF5_PLUGIN_DIR}"
export HDF5_PLUGIN_DIR
export HDF5_PLUGIN_PATH="$HDF5_PLUGIN_DIR"

localclean() {
rm -f ./tmp_bloscx3.nc ./tmp_bloscx4.nc ./tmp_bloscx4_fail.nc
}

# Execute the specified tests

echo "*** Testing dynamic filters using API"
localclean
${NCGEN} -3 -o tmp_bloscx3.nc ${srcdir}/ref_bloscx.cdl
# This should pass
${NCCOPY} -4 -V three_dmn_rec_var -F *,32001,0,0,0,0,1,1,0 ./tmp_bloscx3.nc ./tmp_bloscx4.nc
# This should fail because shuffle is off
if ${NCCOPY} -4 -V three_dmn_rec_var -F *,32001,0,0,0,0,1,0,0 ./tmp_bloscx3.nc ./tmp_bloscx4_fail.nc ; then
    echo "*** not xfail: nccopy "
    exit 1;
else
    echo "*** xfail: nccopy "
fi

localclean

exit 0
