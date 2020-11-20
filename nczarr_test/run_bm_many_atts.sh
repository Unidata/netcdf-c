#!/bin/sh
# This is a metadata performance test for nczarr
# Dennis Heimbigner

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

. ./bm_common.sh

echo "Testing performance of nc_create and nc_open on file with large # of attributes"

CMD=bm_many_atts
EXC=${execdir}/bm_many_atts
ARGS="--ngroups=100 --ngroupattrs=100 --X=1"

echo "***Testing format: nc4"
FMT=nc4 ; $EXC --format=$FMT --f=$CMD $ARGS
echo "***Testing format: nzf"
FMT=nzf ; $EXC --format=$FMT --f=$CMD $ARGS
if test "x$FEATURE_S3TESTS" = xyes ; then
    echo "***Testing format: s3"
    FMT=s3 ; $EXC --format=$FMT --f=$CMD $ARGS
fi
