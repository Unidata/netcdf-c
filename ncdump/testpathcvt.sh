#!/bin/bash

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

set -e

# Disable automatic path conversions in MINGW shell:
export MSYS2_ARG_CONV_EXCL='*'

# We need to find the drive letter, if any
DL=`${NCPATHCVT} -c -e / | sed -e 's|/cygdrive/\([a-zA-Z]\)/.*|\1|'`
if test "x$DL" != x ; then
  # Lower case drive letter
  DLL=`echo "$DL" | tr '[:upper:]' '[:lower:]'`
  DL="-d $DLL"
fi


testcase1() {
T="$1"
P="$2"

echo -n "path: $T: |$P| => |" >>tmp_pathcvt.txt
${NCPATHCVT} -B"@" ${DL} "$T" -e "$P" >>tmp_pathcvt.txt
echo "|" >> tmp_pathcvt.txt
}

testcase() {
    testcase1 "-u" "$1"
    testcase1 "-c" "$1"
    testcase1 "-m" "$1"
    testcase1 "-w" "$1"
    testcase1 "-C" "$1"
}

rm -f tmp_pathcvt.txt

# '@' will get translated to embedded blank
PATHS="/xxx/x/y d:/x/y /cygdrive/d/x/y /d/x/y /cygdrive/d /d /cygdrive/d/git/netcdf-c/dap4_test/test_anon_dim.2.syn d:\\x\\y d:\\x\\y@w\\z"
for p in $PATHS ; do
testcase $p
done
exit

diff -w ${srcdir}/ref_pathcvt.txt ./tmp_pathcvt.txt

exit 0
