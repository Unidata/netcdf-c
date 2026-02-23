#!/bin/bash
set -x 
if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

set -e

. "$srcdir/test_nczarr.sh"

s3isolate "testdir_error_checks"
THISDIR=`pwd`
cd $ISOPATH

testfiltererror() {
    # Mess with the plugin
    set -x
    cp -r ${srcdir}/ref_data.zarr .
    chmod -R 755 ./ref_data.zarr
    codec='faultycodecname'
    sed -i.bak 's/blosc/'${codec}'/g' ref_data.zarr/data/.zarray && rm ref_data.zarr/data/.zarray.bak
    (${NCDUMP} -v data -L0  "file://ref_data.zarr#mode=zarr" \
        2>&1 || true )  \
        | grep -o "Variable .* has unsupported codec: (${codec})"
    set +x
}


testfiltererror
