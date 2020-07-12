#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

set -e

# Control which test sets are executed
# possible sets: mapnc4 json proj walk
TESTS=walk

# Functions

extfor() {
    case "$1" in
    nc4) zext="nz4" ;;
    nz4) zext="nz4" ;;
    nzf) zext="nzf" ;;
    *) echo "unknown kind: $1" ; exit 1;;
    esac
}

dumpmap1() {
    tmp=
    if test -f $1 ; then
      if test -s $1 ; then
	  ftype=`file -b $1`
	  case "$ftype" in
	  [Aa][Ss][Cc]*) tmp=`cat $1 | tr '\r\n' '  '` ;;
	  data*) tmp=`hexdump -v -e '1/1 " %1x"' ./testmap.nzf/data1` ;;
	  *) echo fail ; exit 1 ;;
	  esac
      fi
      echo "$1 : |" "$tmp" "|" >> $2
    else
      echo "$1" >> $2
    fi
}

dumpmap() {
    case "$1" in
    nz4) rm -f $3 ; ${NCDUMP} $2 > $3 ;;
    nzf)
	rm -f $3;
	lr=`find $2 | tr  '\r\n' '  '`
	for f in $lr ; do  dumpmap1 $f $3 ; done
	;;
    esac
}

deletemap() {
    rm -fr $2
}

# Common
CMD="${execdir}/ut_map${ext}"

testmapcreate() {
  echo ""; echo "*** Test zmap create -k $1"
  extfor "$1"
  tag="map"
  output="test$tag.$zext"

  deletemap $1 $output

  # Create the test file
  $CMD -k$1 -x create -o ./$output
  cdl="ut_${tag}_create_${zext}.cdl"
  dumpmap $zext ./$output ./$cdl
  diff -wb ${srcdir}/ref_$cdl ./$cdl
  # delete the test file
  $CMD -k$1 -x delete -f $output
  rm -f $cdl
  if test -f $output; then
    echo "delete did not delete $output"
    exit 1
  fi
  # re-create the test file
  $CMD -k$1 -x create -o ./$output
}

testmapmeta() {
  echo ""; echo "*** Test zmap read/write meta -k $1"
  extfor "$1"
  tag="map"
  file="test$tag.$zext"

  $CMD -k$1 -x writemeta -f ./$file
  cdl="ut_${tag}_writemeta_${zext}.cdl"
  dumpmap $zext ./$file ./$cdl
  diff -wb ${srcdir}/ref_$cdl ./$cdl

  $CMD -k$1 -x writemeta2 -o ${srcdir}/$file
  cdl="ut_${tag}_write2meta_${zext}.cdl"
  dumpmap $zext ./$file ./$cdl
  diff -wb ${srcdir}/ref_$cdl ./$cdl

  output="ut_${tag}_readmeta_$zext.txt"
  $CMD -k$1 -x readmeta -f $file > ${srcdir}/$output
  diff -wb ${srcdir}/ref_$output ./$output
}

testmapdata() {
  echo ""; echo "*** Test zmap read/write data -k $1"
  extfor "$1"
  tag="map"
  file="test$tag.$zext"

  $CMD -k$1 -x "writedata" -f ./$file
  cdl="ut_${tag}_writedata_${zext}.cdl"
  dumpmap $zext ./$file  ./$cdl
  diff -wb ${srcdir}/ref_$cdl ./$cdl

  # readata is verification only
  $CMD -k$1 -x readdata -f ./$file
}

testmapsearch() {
  echo ""; echo "*** Test zmap search -k $1"
  extfor "$1"
  tag="map"
  file="test$tag.$zext"

  txt=ut_${tag}_search_$zext.txt
  rm -f $txt
  $CMD -k$1 -x "search" -f ./$file > $txt
  diff -wb ${srcdir}/ref_$txt ./$txt
}

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

testproj() {
  file="ut_proj.txt"
  rm -f $file
  CMD="${execdir}/ut_projections${ext}"
  $CMD -ddim1=4 -v "int v(dim1/2)" -s "[0:4:1]" > $file
  diff -wb ${srcdir}/ref_$file ./$file
}

testwalk() {
  file="ut_walk.txt"
  rm -f $file
  CMD="${execdir}/ut_walk${ext}"
  $CMD -ddim1=4 -v "int v(dim1/2)" -s "[0:4:1]" > $file
  diff -wb ${srcdir}/ref_$file ./$file
}

echo ""

echo "*** Unit Testing"

for T in $TESTS ; do
case "$T" in

map)
#echo ""; echo "*** Test zmap_nz4"
#testmapcreate nz4; testmapmeta nz4; testmapdata nz4; testmapsearch nz4
echo ""; echo "*** Test zmap_nzf"
testmapcreate nzf; testmapmeta nzf; testmapdata nzf; testmapsearch nzf
;;

json)
echo ""; echo "*** Test zjson"
testjson
;;

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
