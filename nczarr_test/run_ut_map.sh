#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

. "$srcdir/test_nczarr.sh"

set -e

# Test those map implementations where
# it is possible to look at the actual storage.
# in some cases. Note that we
# cannot easily look inside S3 storage
# except using the aws-cli, if available


# Common
CMD="${execdir}/ut_map${ext}"

testmapcreate() {
  echo ""; echo "*** Test zmap create -k $1"
  extfor "$1"
  tag="map"
  output="tmp_$tag.$zext"

  deletemap $1 $output

  # Create the test file
  $CMD -k$1 -x create -o $output
  cdl="ut_${tag}_create_${zext}.cdl"
  ref="ref_ut_${tag}_create.cdl"
  dumpmap $zext $output ./$cdl
  diff -wb ${srcdir}/$ref ./$cdl
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
  file="tmp_$tag.$zext"

  $CMD -k$1 -x writemeta -f $file
  cdl="ut_${tag}_writemeta_${zext}.cdl"
  ref="ref_ut_${tag}_writemeta.cdl"
  dumpmap $zext $file ./$cdl
  diff -wb ${srcdir}/$ref ./$cdl

  $CMD -k$1 -x writemeta2 -o ./$file
  cdl="ut_${tag}_writemeta2_${zext}.cdl"
  ref="ref_ut_${tag}_writemeta2.cdl"
  dumpmap $zext $file ./$cdl
  diff -wb ${srcdir}/$ref ./$cdl

  output="ut_${tag}_readmeta_$zext.txt"
  outref="ref_ut_${tag}_readmeta.txt"
  $CMD -k$1 -x readmeta -f $file > ./$output
  diff -wb ${srcdir}/$outref ./$output

  output="ut_${tag}_readmeta2_$zext.txt"
  outref="ref_ut_${tag}_readmeta2.txt"
  $CMD -k$1 -x readmeta2 -f $file > ./$output
  diff -wb ${srcdir}/$outref ./$output
}

testmapdata() {
  echo ""; echo "*** Test zmap read/write data -k $1"
  extfor "$1"
  tag="map"
  file="tmp_$tag.$zext"

  $CMD -k$1 -x "writedata" -f $file
  cdl="ut_${tag}_writedata_${zext}.cdl"
  ref="ref_ut_${tag}_writedata.cdl"
  dumpmap $zext $file  ./$cdl
  diff -wb ${srcdir}/$ref ./$cdl
  # readata is verification only
  $CMD -k$1 -x readdata -f $file
}

testmapsearch() {
  echo ""; echo "*** Test zmap search -k $1"
  extfor "$1"
  tag="map"
  file="tmp_$tag.$zext"
  txt=ut_${tag}_search_$zext.txt
  ref=ref_ut_${tag}_search.txt
  rm -f $txt
  $CMD -k$1 -x "search" -f $file > $txt
  diff -wb ${srcdir}/$ref ./$txt
}

echo ""

echo "*** Map Unit Testing"

echo ""; echo "*** Test zmap_file"
testmapcreate file; testmapmeta file; testmapdata file; testmapsearch file

if test "x$FEATURE_NCZARR_ZIP" = xyes ; then
echo ""; echo "*** Test zmap_zip"
testmapcreate zip; testmapmeta zip; testmapdata zip; testmapsearch zip
fi

exit 0
