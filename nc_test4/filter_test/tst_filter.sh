#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../../test_common.sh

set -e

export HDF5_PLUGIN_PATH=`pwd`

API=1
NG=1

if test "x$API" = x1 ; then
echo "*** Testing dynamic filters using API"
rm -f ./bzip2.nc ./bzip2.dump ./tmp
${execdir}/test_filter
$NCDUMP -s ./bzip2.nc > ./tmp
# Process bzip2.dump to remove extraneous info
sed -e '/var:_Endianness/d' -e '/var:_NoFill/d' <./tmp > bzip2.dump
rm -f ./tmp
diff -b -w ${srcdir}/bzip2.cdl ./bzip2.dump
echo "*** Pass: API dynamic filter"
fi

if test "x$NG" = x1 ; then
echo "*** Testing dynamic filters using ncgen"
rm -f ./bzip2.nc ./bzip2.dump ./tmp
$NCGEN -lb -4 -o bzip2.nc ${srcdir}/bzip2.cdl
$NCDUMP -s ./bzip2.nc > ./tmp
# Process bzip2.dump to remove extraneous info
sed -e '/var:_Endianness/d' -e '/var:_NoFill/d' <./tmp > bzip2.dump
rm -f ./tmp
diff -b -w ${srcdir}/bzip2.cdl ./bzip2.dump
echo "*** Pass: ncgen dynamic filter"
fi

#cleanup
rm -f ./bzip2.nc ./bzip2.dump

exit 0
