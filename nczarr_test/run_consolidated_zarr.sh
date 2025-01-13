#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi 
. ../test_common.sh

. "$srcdir/test_nczarr.sh"

set -e

s3isolate "testdir_consolidated_zarr"
THISDIR=`pwd`
cd $ISOPATH

# Force use of consolidated metadata
unset NCNOZMETADATA

# Locate the plugin path
if ! findplugin h5blosc ; then exit 0; fi
if ! avail blosc; then exit 0; fi

# This shell script tests support for:
# 1. consolidated zarr (noxarray) read-only
# 2. xarray read

testcase() {
    zext=$1
    echo "*** Test: consolidated zarr write then read; format=$zext"
    fileargs tmp_consolidated_zarr "mode=zarr,noxarray,zmetadata,$zext"
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
    fileargs tmp_nczarr "mode=nczarr,noxarray,zmetadata,$zext"
    deletemap $zext $file
    ${NCGEN} -4 -b -o "$fileurl" $srcdir/ref_whole.cdl
    fileargs tmp_nczarr "mode=zarr,$zext"
    ${NCDUMP} -n nczarr2zarr $fileurl > tmp_nczarr_${zext}.cdl
    diff -b ${srcdir}/ref_nczarr2zarr.cdl tmp_nczarr_${zext}.cdl
}

testcase_csl_vs_no(){
    zext=$1
    echo "*** Test: consolidated pure python zarr read; format=$zext"
    deletemap $zext $file
    tar -zxf $srcdir/ref_consolidated_zarr_2.18.2_python.zarr.tgz
    cp -r ref_consolidated_zarr_2.18.2_python.zarr ref_consolidated_zarr_2.18.2_python.zarr.$zext
    cp -r ref_consolidated_zarr_2.18.2_python.zarr ref_zarr_2.18.2_python.zarr.$zext
    rm -f ref_zarr_2.18.2_python.zarr.$zext/.zmetadata
    fileargs ref_consolidated_zarr_2.18.2_python.zarr "mode=zarr,${zext}"
echo ">>> (1)"
P=`pwd`
ls -lrtd *.file
${ZMD} -h $fileurl
export NCTRACING=10
    ${NCDUMP} -n same_name $fileurl |tee tmp_consolidated_python_zarr_${zext}.cdl
    fileargs ref_zarr_2.18.2_python.zarr "mode=zarr,$zext"
echo ">>> (2)"
ls -lrtd *.zarr
    ${NCDUMP} -n same_name $fileurl > tmp_python_zarr_${zext}.cdl
    rm -f diff.txt
    diff -b tmp_consolidated_python_zarr_${zext}.cdl tmp_python_zarr_${zext}.cdl > diff.txt
}

testcase file
testcase_csl_vs_no file
if test "x$FEATURE_NCZARR_ZIP" = xyes ; then testcase zip; fi
if test "x$FEATURE_S3TESTS" = xyes ; then testcase s3; fi
