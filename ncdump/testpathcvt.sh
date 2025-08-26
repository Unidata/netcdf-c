#!/bin/bash

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

set -e

# For testing purposes: we want MSYS_NO_PATHCONV and MSYS2_ARG_CONV_EXCL set.
#     Suppress converting paths of form /[a-z]/... to [a-z]:/...
      export MSYS_NO_PATHCONV=1
#     Disable automatic path conversions in MINGW shell:
      export MSYS2_ARG_CONV_EXCL='*'

# '@' will get translated to embedded blank
TESTPATHS1=
TESTPATHS1="${TESTPATHS1} /xxx/x/y"
TESTPATHS1="${TESTPATHS1} d:/x/y"
TESTPATHS1="${TESTPATHS1} /cygdrive/d/x/y"
TESTPATHS1="${TESTPATHS1} /d/x/y"
TESTPATHS1="${TESTPATHS1} /cygdrive/d"
TESTPATHS1="${TESTPATHS1} /d"
TESTPATHS1="${TESTPATHS1} d:\\\\x\\\\y"
TESTPATHS1="${TESTPATHS1} d:\\\\x\\\\y@w\\\\z"
# Trim
TESTPATHS1=`echo "${TESTPATHS1}"|sed -e 's|^[ 	]\+||'`

TESTPATHS2=
TESTPATHS2="${TESTPATHS2} /xxx/x/y;/cygdrive/d/x/y"
TESTPATHS2="${TESTPATHS2} /d/x/y;/cygdrive/d"
TESTPATHS2="${TESTPATHS2} /d/x/y;d:\\\\x\\\\y"
TESTPATHS2="${TESTPATHS2} d:\\\\x\\\\y@w\\\\z"
# Trim
TESTPATHS2=`echo "${TESTPATHS2}"|sed -e 's|^[ 	]\+||'`

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
    testcaseD "-m" "$1"
}

testcase2() {
    testcaseP "-u" "$1"
    testcaseP "-c" "$1"
    testcaseP "-w" "$1"
    testcaseP "-m" "$1"
}

rm -f tmp_pathcvt.txt

for p in $TESTPATHS1 ; do
testcase1 "$p"
done

for p in $TESTPATHS2 ; do
testcase2 "$p"
done

diff -w ${srcdir}/ref_pathcvt.txt ./tmp_pathcvt.txt

exit 0
