#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

. "$srcdir/test_nczarr.sh"

set -e

# Test those map implementations where
# it is possible to look at the actual storage.
# .ncz and .nzf specifically. Note that we
# cannot easily look inside S3 storage
# except using the aws-cli, if available


# Common
CMD="${execdir}/ut_map${ext}"

testmapcreate() {
  echo ""; echo "*** Test zmap create -k $1"
  extfor "$1"
  tag="map"
  output="test$tag.$zext"

  deletemap $1 $output

  # Create the test file
  $CMD -k$1 -x create -o $output
  cdl="ut_${tag}_create_${zext}.cdl"
  dumpmap $zext $output ./$cdl
  diff -wb ${srcdir}/ref_$cdl ./$cdl
  # delete the test file
  $CMD -k$1 -x delete -f $output
  rm -f $cdl
  if test -f $output; then
    echo "delete did not delete $output"
    exit 1
  fi
  # re-create the test file
  $CMD -k$1 -x create -o $output
}

testmapmeta() {
  echo ""; echo "*** Test zmap read/write meta -k $1"
  extfor "$1"
  tag="map"
  file="test$tag.$zext"

  $CMD -k$1 -x writemeta -f $file
  cdl="ut_${tag}_writemeta_${zext}.cdl"
  dumpmap $zext $file ./$cdl
  diff -wb ${srcdir}/ref_$cdl ./$cdl

  $CMD -k$1 -x writemeta2 -o ./$file
  cdl="ut_${tag}_write2meta_${zext}.cdl"
  dumpmap $zext $file ./$cdl
  diff -wb ${srcdir}/ref_$cdl ./$cdl

  output="ut_${tag}_readmeta_$zext.txt"
  $CMD -k$1 -x readmeta -f $file > ./$output
  diff -wb ${srcdir}/ref_$output ./$output
}

testmapdata() {
  echo ""; echo "*** Test zmap read/write data -k $1"
  extfor "$1"
  tag="map"
  file="test$tag.$zext"

  $CMD -k$1 -x "writedata" -f $file
  cdl="ut_${tag}_writedata_${zext}.cdl"
  dumpmap $zext $file  ./$cdl
  diff -wb ${srcdir}/ref_$cdl ./$cdl

  # readata is verification only
  $CMD -k$1 -x readdata -f $file
}

testmapsearch() {
  echo ""; echo "*** Test zmap search -k $1"
  extfor "$1"
  tag="map"
  file="test$tag.$zext"

  txt=ut_${tag}_search_$zext.txt
  rm -f $txt
  $CMD -k$1 -x "search" -f $file > $txt
  diff -wb ${srcdir}/ref_$txt ./$txt
}

echo ""

echo "*** Map Unit Testing"

echo ""; echo "*** Test zmap_nzf"
testmapcreate nzf; testmapmeta nzf; testmapdata nzf; testmapsearch nzf

if test "x$FEATURE_HDF5" = xyes ; then
echo ""; echo "*** Test zmap_nz4"
testmapcreate nz4; testmapmeta nz4; testmapdata nz4; testmapsearch nz4
fi

exit 0
