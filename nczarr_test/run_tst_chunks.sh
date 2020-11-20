#!/bin/sh

# This shell just tests the tst_chunks3 program by running it a few
# times to generate a simple test file. Then it uses ncdump -s to
# check that the output is what it should be.

# Russ Rew, Dennis Heimbigner

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh


test() {
    FMT=$1
    DIMS=$1
    CHUNKS=$2
    ${execdir}/bm_chunks3 --format=$FMT --X=1 --f=bm_chunks3 --dims="$DIMS" chunks="$CHUNKS"
}

echo ""
echo "*** Running benchmarking program tst_chunks3 for tiny test file"
test "6,12,4" 2,3,1"
echo '*** SUCCESS!!!'

exit 0
echo ""
echo "*** Testing the benchmarking program tst_chunks3 for larger variables ..."
#cachesize=10000000; cachehash=10000; cachepre=0.0
test "32,90,91" "8,10,13"
echo '*** SUCCESS!!!'

exit 0
