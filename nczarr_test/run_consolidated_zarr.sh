#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi 
. ../test_common.sh

. "$srcdir/test_nczarr.sh"

set -e

isolate "testdir_consolidated_zarr"
s3isolate "testdir_consolidated_zarr"
THISDIR=`pwd`
cd $ISOPATH

# This shell script tests support for:
# 1. consolidated zarr (noxarray) read-only
# 2. xarray read

testcase() {
    zext=$1

    set -x
    echo "*** Test: consolidated zarr write then read; format=$zext"
    fileargs tmp_consolidated_zarr "mode=zarr,noxarray,$zext"
    deletemap $zext $file
    ${NCGEN} -4 -b -o "$fileurl" $srcdir/ref_consolidated_zarr_base.cdl
    ${NCDUMP} $fileurl > tmp_consolidated_zarr_${zext}.cdl
    diff -b ${srcdir}/ref_consolidated_zarr.cdl tmp_consolidated_zarr_${zext}.cdl

    echo "*** Test: xarray zarr write then read; format=$zext"
    fileargs tmp_xarray "mode=zarr,$zext"
    #deletemap $zext $file
    ${NCGEN} -4 -b -o "$fileurl" $srcdir/ref_consolidated_zarr_base.cdl
    ${NCDUMP} $fileurl > tmp_xarray_${zext}.cdl
    diff -b ${srcdir}/ref_xarray.cdl tmp_xarray_${zext}.cdl

    echo "*** Test: consolidated zarr reading nczarr; format=$zext"
    fileargs tmp_nczarr "mode=nczarr,noxarray,$zext"
    deletemap $zext $file
    ${NCGEN} -4 -b -o "$fileurl" $srcdir/ref_whole.cdl
    fileargs tmp_nczarr "mode=zarr,$zext"
    ${NCDUMP} -n nczarr2zarr $fileurl > tmp_nczarr_${zext}.cdl
    diff -b ${srcdir}/ref_nczarr2zarr.cdl tmp_nczarr_${zext}.cdl
}

testcase_csl_vs_no(){
    zext=$1

    set -x
    echo "*** Test: consolidated pure python zarr read; format=$zext"
    deletemap $zext $file
    cp -r $srcdir/ref_consolidated_zarr_2.18.2_python.zarr/. ref_consolidated_zarr_2.18.2_python.zarr.$zext
    cp -r $srcdir/ref_consolidated_zarr_2.18.2_python.zarr/. ref_zarr_2.18.2_python.zarr.$zext
    rm -f ref_zarr_2.18.2_python.zarr.$zext/.zmetadata
    fileargs ref_consolidated_zarr_2.18.2_python.zarr "mode=zarr"
    echo "${NCDUMP} $fileurl > tmp_consolidated_python_zarr_${zext}.cdl"
    ${NCDUMP} -n same_name $fileurl > tmp_consolidated_python_zarr_${zext}.cdl
    fileargs ref_zarr_2.18.2_python.zarr "mode=zarr,$zext"
    ${NCDUMP} -n same_name $fileurl > tmp_python_zarr_${zext}.cdl
    diff -b tmp_consolidated_python_zarr_${zext}.cdl tmp_python_zarr_${zext}.cdl 
    set +x
}

testcase_csl_fallback() {
    zext=$1
    set -x
    echo "*** Test: reading dataset no,empty and invalids .zmetadata file; format=$zext"
    cp -r $srcdir/ref_consolidated_zarr_2.18.2_python.zarr/. ref_consolidated_zarr_2.18.2_python.zarr.$zext
    fileargs ref_consolidated_zarr_2.18.2_python.zarr "mode=zarr"

    ${NCDUMP} $fileurl > expected.cdl
    
    local mdfile="ref_consolidated_zarr_2.18.2_python.zarr.$zext/.zmetadata"
    # no .zmetadata file
    rm "${mdfile}"
    NCZARR_CONSOLIDATED=TRUE ${NCDUMP} -L0 $fileurl > no-metadata.cdl 2> no-metadata.log
    diff -b expected.cdl no-metadata.cdl
    cat no-metadata.log
    grep -q "NOTE: Dataset not consolidated! Doing so will improve performance" no-metadata.log

    # empty .zmetadata file
    truncate -s 0 "${mdfile}"
    NCZARR_CONSOLIDATED=TRUE ${NCDUMP} -L0 $fileurl > empty-metadata.cdl 2> empty-metadata.log
    diff -b expected.cdl empty-metadata.cdl
    cat empty-metadata.log
    grep -q "NOTE: Dataset not consolidated! Doing so will improve performance" empty-metadata.log

    # invalid .zmetadata file
    echo "{}" > "${mdfile}"
    NCZARR_CONSOLIDATED=TRUE ${NCDUMP} -L0 $fileurl > invalid-metadata.cdl 2> invalid-metadata.log
    diff -b expected.cdl invalid-metadata.cdl
    grep -q "WARN: Consolidated metadata is invalid, ignoring it!" invalid-metadata.log

    # empty dataset only on .zmetadata file
    echo "{"metadata":{},"zarr_consolidated_format":1}" > "${mdfile}"
    NCZARR_CONSOLIDATED=TRUE ${NCDUMP} -L0 $fileurl > empty-dataset.cdl 2> empty-dataset.log
    diff -b expected.cdl empty-dataset.cdl
    grep -q "WARN: Consolidated metadata is invalid, ignoring it!" empty-dataset.log

    set +x
}

testcase file
testcase_csl_vs_no file
testcase_csl_fallback file
if test "x$FEATURE_NCZARR_ZIP" = xyes ; then testcase zip; fi
if test "x$FEATURE_S3TESTS" = xyes ; then testcase s3; fi

