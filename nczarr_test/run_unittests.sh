#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

set -e
set -x

# Control which test sets are executed
# possible sets: mapnc4 json proj
#TESTS="mapnc4 json proj"
TESTS="mapnc4 json proj"

echo ""
echo "*** Unit Testing"

for T in $TESTS ; do
case "$T" in

mapnc4)
echo ""; echo "*** Test zmap_nc4"
CMD="${execdir}/ut_mapnc4${ext} -c"
$CMD create
ncdump ./testmapnc4.ncz >./ut_mapnc4_create.cdl
diff -wb ${srcdir}/ref_ut_mapnc4_create.cdl ./ut_mapnc4_create.cdl
$CMD delete
if test -f testmapnc4.ncz; then
    echo "delete did not delete testmapnc4.ncz"
    exit 1
fi
$CMD writemeta
ncdump ./testmapnc4.ncz >./ut_mapnc4_wmeta.cdl
diff -wb ${srcdir}/ref_ut_mapnc4_wmeta.cdl ./ut_mapnc4_wmeta.cdl
$CMD writemeta2 # depends on writemeta
ncdump ./testmapnc4.ncz >./ut_mapnc4_wmeta2.cdl
diff -wb ${srcdir}/ref_ut_mapnc4_wmeta2.cdl ./ut_mapnc4_wmeta2.cdl
$CMD readmeta > ut_mapnc4_rmeta.txt # depends on writemeta2
diff -wb ${srcdir}/ref_ut_mapnc4_rmeta.txt ./ut_mapnc4_rmeta.txt
$CMD writedata # depends on writemeta2
ncdump ./testmapnc4.ncz >./ut_mapnc4_write.cdl
diff -wb ${srcdir}/ref_ut_mapnc4_write.cdl ./ut_mapnc4_write.cdl
$CMD readdata # depends on writemeta2
$CMD search > ut_mapnc4_search.txt
diff -wb ${srcdir}/ref_ut_mapnc4_search.txt ./ut_mapnc4_search.txt
;;

json)
echo ""; echo "*** Test zjson"
CMD="${execdir}/ut_json${ext} -c"
$CMD build > ut_json_build.txt
diff -wb ${srcdir}/ref_ut_json_build.txt ./ut_json_build.txt
$CMD parse > ut_json_parse.txt
diff -wb ${srcdir}/ref_ut_json_parse.txt ./ut_json_parse.txt
;;

proj)
echo ""; echo "*** Test projection computations"
echo ""; echo "*** Test 1"
${execdir}/ut_projections${ext} -r 1 -d 4 -c 2 -s "[0:4:1]" > ut_proj1.txt
diff -wb ${srcdir}/ref_ut_proj1.txt ./ut_proj1.txt
;;

*) echo "Unknown test set: $T"; exit 1 ;;

esac
done

exit 0
