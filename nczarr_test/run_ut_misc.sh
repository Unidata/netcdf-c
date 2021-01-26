#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

. "$srcdir/test_nczarr.sh"

set -e

# Test misc functionality

# Control which test sets are executed
# possible sets: json
TESTS=json

# Functions

testjson() {
  file="ut_json_build.txt"
  rm -f $file
  CMD="${execdir}/ut_json${ext}"
  $CMD -x build > $file
  diff -wb ${srcdir}/ref_$file ./$file
  file="ut_json_parse.txt"
  rm -f $file
  $CMD -x parse > $file
  diff -wb ${srcdir}/ref_$file ./$file
}

echo ""

echo "*** Misc. Unit Testing"

for T in $TESTS ; do
case "$T" in

json)
echo ""; echo "*** Test zjson"
testjson
;;

*) echo "Unknown test set: $T"; exit 1 ;;

esac
done

exit 0
