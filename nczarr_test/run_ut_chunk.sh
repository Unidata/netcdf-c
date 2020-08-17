#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

set -e

# Test chunking code

# Control which test sets are executed
# possible sets: proj walk
TESTS=proj

# Functions

testproj() {
  # Do a single output file
  file="ut_proj.txt"
  rm -f $file
  CMD="${execdir}/ut_projections${ext}"
  $CMD -ddim1=4 -v "int v(dim1/2)" -s "[0:4:1]" >> $file
  $CMD -ddim1=6 -v "int v(dim1/3)" -s "[0:5]" >> $file
  $CMD -ddim1=4 -ddim2=2 -v "int v(dim1/2,dim2/2)" -s "[2:3][0:2]" >> $file
  diff -wb ${srcdir}/ref_$file ./$file
}

testwalk() {
  file="ut_walk.txt"
  rm -f $file
  CMD="${execdir}/ut_walk${ext}"
  $CMD -ddim1=4 -v "int v(dim1/2)" -s "[0:4:1] -W "0,1,2,3" " > $file
  $CMD -ddim1=6 -v "int v(dim1/3)" -s "[0:5]" -W "0,1,2,3,4,5" >> $file
  $CMD -ddim1=4 -ddim2=2 -v "int v(dim1/2,dim2/2)" -s "[2:3][0:2]" -W "0,1,2,3,4,5,6,7">> $file
  diff -wb ${srcdir}/ref_$file ./$file
}

echo ""

echo "*** Chunking Unit Testing"

for T in $TESTS ; do
case "$T" in

proj)
echo ""; echo "*** Test projection computations"
echo ""; echo "*** Test 1"
testproj
;;

walk)
echo ""; echo "*** Test chunk walkings"
testwalk
;;

*) echo "Unknown test set: $T"; exit 1 ;;

esac
done

exit 0
