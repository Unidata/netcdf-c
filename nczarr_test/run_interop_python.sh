#!/bin/sh

set -e

if test "x$srcdir" = x ; then srcdir=`pwd`; fi 
. ../test_common.sh

. "$srcdir/test_nczarr.sh"

set -e

metaonly="-h"

s3isolate "testdir_interop_python"
THISDIR=`pwd`
cd $ISOPATH

# Running python means activate the virtual environment
if [ -d "${TOPBUILDDIR}/venv" ]; then
    . ${TOPBUILDDIR}/venv/bin/activate
    trap "deactivate" EXIT
else
    echo "Error: Virtual environment not found. Did you run the build step?"
    exit 1
fi

testxarray() {
  # Create a dataset with xarray and convert it to both:
  # 1. netcdf4
  # 2. zarr
  # Then compare the output of ncdump on both files
  # to make sure they are the same
  
  echo "	o Running xarray Testcase:	(${@})"
  # Create files
  testing-tools create xarray $1 

  ncdump -n same ${PWD}/xarray-$1.nc > nc.out
  ncdump -n same file://${PWD}/xarray-$1.zarr/#mode=zarr,file > zarr.out
  diff -b nc.out zarr.out
}

# This shell script tests compatibility between
# this implementation and other implementations
# by means of files constructed by that other implementation

testallcases() {
zext=$1
case "$zext" in 
    file)
      testxarray "filename"
	;;
    zip)
	;;
    s3)
 	;;
    *) echo "unimplemented kind: $1" ; exit 1;;
esac
}


testallcases file
