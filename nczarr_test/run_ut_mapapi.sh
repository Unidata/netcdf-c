#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

. "$srcdir/test_nczarr.sh"

#TR="-T10"

set -e

# Test map implementations for consistency at the zmap API
# level. This allows testing of implementations that do not admit
# of easy examination of the actual storage.  For example it is
# not easy to look inside S3 storage except using the aws-cli, if
# available.

CMD="${execdir}/ut_mapapi${ext}"

testmapcreate() {
  echo ""; echo "*** Test zmap create -k $1"
  extfor "$1"
  tag=mapapi
  base="tmp_$tag"
  fileargs $base

  deletemap $1 $file

  # Create the test file
  $CMD $TR -k$1 -x create -f $file
  cdl="ut_${tag}_create_${zext}.cdl"
  ref="ref_ut_${tag}_create.cdl"
  ${ZMD} $fileurl > ./$cdl
  diff -wb $srcdir/$ref ./$cdl
  # delete the test file
  $CMD $TR -k$1 -x delete -f $file
  rm -f $cdl

  mapstillexists $1
  if test mapstillexists = 0 ; then exit 1;  fi
  # re-create the test file
  $CMD -k$1 -x create -o $file
}

testmapmeta() {
  echo ""; echo "*** Test zmap read/write meta -k $1"
  extfor "$1"
  tag=mapapi
  base="tmp_$tag"
  fileargs $base
  $CMD $TR -k$1 -x simplemeta -f $file
  cdl="ut_${tag}_meta_${zext}.cdl"
  ref="ref_ut_${tag}_meta.cdl"
  ${ZMD} $fileurl > ./$cdl
  diff -wb ${srcdir}/$ref ./$cdl
}

testmapdata() {
  echo ""; echo "*** Test zmap read/write data -k $1"
  extfor "$1"
  tag=mapapi
  base="tmp_$tag"
  fileargs $base
  $CMD $TR -k$1 -x "simpledata" -f $file
  cdl="ut_${tag}_data_${zext}.cdl"
  ref="ref_ut_${tag}_data.cdl"
  ${ZMD} $fileurl > ./$cdl
  diff -wb ${srcdir}/$ref ./$cdl
}

testmapsearch() {
  echo ""; echo "*** Test zmap search -k $1"
  extfor "$1"
  tag=mapapi
  base="tmp_$tag"
  fileargs $base
  txt=ut_${tag}_search_$zext.txt
  ref=ref_ut_${tag}_search.txt
  rm -f $txt
  $CMD $TR -k$1 -x "search" -f $file > $txt
  diff -wb ${srcdir}/$ref ./$txt
}

main() {
echo ""
echo "*** Map Unit Testing"
echo ""; echo "*** Test zmap_file"
testmapcreate file; testmapmeta file; testmapdata file; testmapsearch file
if test "x$FEATURE_NCZARR_ZIP" = xyes ; then
    echo ""; echo "*** Test zmap_zip"
    testmapcreate zip; testmapmeta zip; testmapdata zip; testmapsearch zip
fi
if test "x$FEATURE_S3TESTS" = xyes ; then
  echo ""; echo "*** Test zmap_s3sdk"
  export PROFILE="-p default"
  testmapcreate s3; testmapmeta s3; testmapdata s3; testmapsearch s3
fi
}

main
