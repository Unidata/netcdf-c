#!/bin/bash

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

set -e

# Disable automatic path conversions in MINGW shell:
export MSYS2_ARG_CONV_EXCL='*'

# We need to find the drive letter, if any
DL=`${NCPATHCVT} -c -x / | sed -e 's|/cygdrive/\([a-zA-Z]\)/.*|\1|'`
if test "x$DL" != x ; then
  # Lower case drive letter
  DLL=`echon -e "$DL" | tr '[:upper:]' '[:lower:]'`
  DL="-D $DLL"
fi

testcaseD() {
T="$1"
P="$2"
# Fixup for shell handling of '\'
PX=`echon -n "$P" | sed -e 's/\\\\\\\\/\\\\/g'`
echon -e "path: $T: |$P| => |" >>tmp_pathcvt.txt
${NCPATHCVT} -B"@" ${DL} "$T" -x "$PX" >>tmp_pathcvt.txt
echo "|" >> tmp_pathcvt.txt
}

testcaseP() {
T="$1"
P="$2"
# Fixup for shell handling of '\'
PX=`echon -n "$P" | sed -e 's/\\\\\\\\/\\\\/g'`
echon -e "path: $T: |$P| => |" >>tmp_pathcvt.txt
${NCPATHCVT} -S';' -B"@" ${DL} "$T" -x "$PX" >>tmp_pathcvt.txt
echo "|" >> tmp_pathcvt.txt
}

# Note that -m is not tested as it is currently an alias for -w
testcase1() {
    testcaseD "-u" "$1"
    testcaseD "-c" "$1"
    testcaseD "-w" "$1"
}

testcase2() {
    testcaseP "-u" "$1"
    testcaseP "-c" "$1"
    testcaseP "-w" "$1"
}

rm -f tmp_pathcvt.txt

# '@' will get translated to embedded blank
TESTPATHS1="/xxx/x/y d:/x/y /cygdrive/d/x/y /d/x/y /cygdrive/d /d /cygdrive/d/git/netcdf-c/dap4_test/test_anon_dim.2.syn d:\\\\x\\\\y d:\\\\x\\\\y@w\\\\z"
for p in $TESTPATHS1 ; do
testcase1 "$p"
done

TESTPATHS2="/xxx/x/y;/cygdrive/d/x/y /d/x/y;/cygdrive/d cygdrive/d/git/netcdf-c/dap4_test/test_anon_dim.2.syn;d:\\\\x\\\\y d:\\\\x\\\\y@w\\\\z"
for p in $TESTPATHS2 ; do
testcase2 "$p"
done

diff -w ${srcdir}/ref_pathcvt.txt ./tmp_pathcvt.txt

exit 0
