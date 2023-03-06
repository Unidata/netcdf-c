#!/bin/sh

# This shell just tests the tst_chunks3 program by running it a few
# times to generate a simple test file. Then it uses ncdump -s to
# check that the output is what it should be.
# Copied from nc_perf/.

# Russ Rew, Dennis Heimbigner

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh
. "$srcdir/test_nczarr.sh"

test1() {
    FMT=$1
    DIMS=$2
    CHUNKS=$3
    ${execdir}/bm_chunks3 --format=$FMT --f=bm_chunks3 --dims="$DIMS" --chunks="$CHUNKS"
}

testcases() {
echo ""
echo "*** Running benchmarking program bm_chunks3 for tiny test file"
test1 $1 "6,12,4" "2,3,1"
echo '*** SUCCESS!!!'
echo ""
echo "*** Testing the benchmarking program bm_chunks3 for larger variables ..."
#cachesize=10000000; cachehash=10000; cachepre=0.0
test1 $1 "32,90,91" "8,10,13"
echo '*** SUCCESS!!!'
}


testcases file
if test "x$FEATURE_NCZARR_ZIP" = xyes ; then testcases zip; fi
if test "x$FEATURE_S3TESTS" = xyes ; then testcases s3; fi

exit 0
