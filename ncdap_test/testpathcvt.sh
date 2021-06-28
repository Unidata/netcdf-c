#!/bin/bash

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

set -e

testcase1() {
T="$1"
P="$2"
echo -n "path: $T: |$P| => |" >>tmp_pathcvt.txt
${execdir}/pathcvt "$T" -e "$P" >>tmp_pathcvt.txt
echo "|" >> tmp_pathcvt.txt

}

testcase() {
    testcase1 "-u" "$1"
    testcase1 "-c" "$1"
    testcase1 "-m" "$1"
    testcase1 "-w" "$1"
}

rm -f tmp_pathcvt.txt

set -x

testcase "/xxx/x/y"
testcase "d:/x/y"
testcase "/cygdrive/d/x/y"
testcase "/d/x/y"
testcase "/cygdrive/d"
testcase "/d"
testcase "/cygdrive/d/git/netcdf-c/dap4_test/test_anon_dim.2.syn"
testcase "d:\\x\\y"
testcase "d:\\x\\y w\\z"

diff -w ${srcdir}/ref_pathcvt.txt ./tmp_pathcvt.txt

exit 0
