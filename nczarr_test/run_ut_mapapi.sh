#!/bin/sh

set -x

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

. "$srcdir/test_nczarr.sh"

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
  tag="mapapi"
  fileargs

  deletemap $1 $file

  # Create the test file
  $CMD -k$1 -x create -f $file
  cdl="ut_${tag}_create_${zext}.cdl"
  ${ZMD} $fileurl > ./$cdl
  diff -wb ${srcdir}/ref_$cdl ./$cdl
  # delete the test file
  $CMD -k$1 -x delete -f $file
  rm -f $cdl

  mapexists $1
  if test mapexists = 0 ; then exit 1;  fi
  # re-create the test file
  $CMD -k$1 -x create -o $file
}

testmapmeta() {
  echo ""; echo "*** Test zmap read/write meta -k $1"
  extfor "$1"
  tag="mapapi"
  fileargs
  $CMD -k$1 -x simplemeta -f $file
  cdl="ut_${tag}_meta_${zext}.cdl"
  ${ZMD} $fileurl > ./$cdl
  diff -wb ${srcdir}/ref_$cdl ./$cdl
}

testmapdata() {
  echo ""; echo "*** Test zmap read/write data -k $1"
  extfor "$1"
  tag="mapapi"
  fileargs
  $CMD -k$1 -x "simpledata" -f $file
  cdl="ut_${tag}_data_${zext}.cdl"
  ${ZMD} $fileurl > ./$cdl
  diff -wb ${srcdir}/ref_$cdl ./$cdl
}

testmapsearch() {
  echo ""; echo "*** Test zmap search -k $1"
  extfor "$1"
  tag="mapapi"
  fileargs
  txt=ut_${tag}_search_$zext.txt
  rm -f $txt
  $CMD -k$1 -x "search" -f $file > $txt
  diff -wb ${srcdir}/ref_$txt ./$txt
}

# check settings
checksetting "NCZarr Support"
if test "x$HAVE_SETTING" = x1 ; then HAVENCZARR=1; fi
checksetting "NCZarr S3"
if test "x$HAVE_SETTING" = x1 ; then HAVES3=1; fi

echo ""

echo "*** Map Unit Testing"

echo ""; echo "*** Test zmap_nz4"
testmapcreate nz4; testmapmeta nz4; testmapdata nz4; testmapsearch nz4
echo ""; echo "*** Test zmap_nzf"
testmapcreate nzf; testmapmeta nzf; testmapdata nzf; testmapsearch nzf

if test "x$NETCDF_S3_TESTS" != x ; then
if test "x$HAVENCZARR" = x1 -a "x$HAVES3" = x1 ; then
echo ""; echo "*** Test zmap_s3sdk"
testmapcreate s3; testmapmeta s3; testmapdata s3; testmapsearch s3
fi
fi

exit 0
