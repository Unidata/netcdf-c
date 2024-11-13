#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

set -e

# Note, the test file for this is ref_fixedstring.h5
# and is generated on the fly by build_fixedstring.
${execdir}/build_fixedstring

echo "*** Test reading a file with HDF5 fixed length strings"
rm -f ./tmp_fixedstring.cdl
$NCDUMP  ${execdir}/ref_fixedstring.h5 > ./tmp_fixedstring.cdl
diff -b -w ${srcdir}/ref_fixedstring.cdl ./tmp_fixedstring.cdl
rm -f ${execdir}/ref_fixedstring.h5
