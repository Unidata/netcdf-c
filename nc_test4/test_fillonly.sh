#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi 
. ../test_common.sh

# This shell script tests bug reported in github issue 
# https://github.com/Unidata/netcdf-c/issues/1826

set -e

echo ""
echo "*** Testing data conversions when a variable has fill value but never written"

${NCGEN} -4 -b -o tmp_fillonly.nc $srcdir/ref_fillonly.cdl
${execdir}/tst_fillonly${ext}

exit 0
