#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

set -e

testcase1() {
T="$1"
P="$2"
C=`${execdir}/pathcvt $T "$P"`
echo  "	$C"
}

testcase() {
#X=`echo -n "$1" | sed -e 's/\\\/\\\\\\\/g'`
X="$1"
echo "path:" "$X"
    testcase1 "-u" "$1"
    testcase1 "-c" "$1"
    testcase1 "-m" "$1"
    testcase1 "-ew" "$1" | sed -e 's/\\/\\\\/g'
}    

rm -f tmp_pathcvt.txt

testcase "/xxx/a/b" >> tmp_pathcvt.txt
testcase "d:/x/y" >> tmp_pathcvt.txt
testcase "/cygdrive/d/x/y" >> tmp_pathcvt.txt
testcase "/d/x/y" >> tmp_pathcvt.txt
testcase "/cygdrive/d" >> tmp_pathcvt.txt
testcase "/d" >> tmp_pathcvt.txt
testcase "/cygdrive/d/git/netcdf-c/dap4_test/test_anon_dim.2.syn" >> tmp_pathcvt.txt
testcase "d:\\x\\y" >> tmp_pathcvt.txt
testcase "d:\\x\\y w\\z" >> tmp_pathcvt.txt

diff -w ${srcdir}/ref_pathcvt.txt ./tmp_pathcvt.txt

exit 0
