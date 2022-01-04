#!/bin/bash

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

. "$srcdir/test_nczarr.sh"

set -e

testset() {
# Which test cases to exercise
testapi $1
testng $1
testncp $1
testunk $1
testngc $1
testmisc $1
testmulti $1
testrep $1
testorder $1
}

# Function to remove selected -s attributes from file;
# These attributes might be platform dependent
sclean() {
    cat $1 \
 	| sed -e '/:_IsNetcdf4/d' \
	| sed -e '/:_Endianness/d' \
	| sed -e '/_NCProperties/d' \
	| sed -e '/_SuperblockVersion/d' \
	| cat > $2
}

# Function to extract _Filter attribute from a file
# These attributes might be platform dependent
getfilterattr() {
sed -e '/var.*:_Filter/p' -ed <$1 >$2
}

# Function to extract _Codecs attribute from a file
# These attributes might be platform dependent
getcodecsattr() {
sed -e '/var.*:_Codecs/p' -ed <$1 >$2
}

trimleft() {
sed -e 's/[ 	]*\([^ 	].*\)/\1/' <$1 >$2
}

# Locate the plugin path and the library names; argument order is critical
# Find bzip2 and capture
findplugin h5bzip2
BZIP2LIB="${HDF5_PLUGIN_LIB}"
BZIP2PATH="${HDF5_PLUGIN_PATH}/${BZIP2LIB}"
# Find misc and capture
findplugin h5misc
MISCPATH="${HDF5_PLUGIN_PATH}/${HDF5_PLUGIN_LIB}"

# Verify
if ! test -f ${BZIP2PATH} ; then echo "Unable to locate ${BZIP2PATH}"; exit 1; fi
if ! test -f ${MISCPATH} ; then echo "Unable to locate ${MISCPATH}"; exit 1; fi

echo "@@@@@@@@@@@"
find ${HDF5_PLUGIN_PATH}

# Execute the specified tests

testapi() {
zext=$1
echo "*** Testing dynamic filters using API for map=$zext"
fileargs tmp_api
deletemap $zext $file
${execdir}/testfilter $fileurl
${NCDUMP} -s -n bzip2 $fileurl > ./tmp_api_$zext.txt
# Remove irrelevant -s output
sclean ./tmp_api_$zext.txt ./tmp_api_$zext.dump
diff -b -w ${srcdir}/ref_bzip2.cdl ./tmp_api_$zext.dump
echo "*** Pass: API dynamic filter for map=$zext"
}

testmisc() {
zext=$1
echo "*** Testing dynamic filters parameter passing for map $zext"
fileargs tmp_misc
deletemap $zext $file
${execdir}/testfilter_misc $fileurl
# Verify the parameters via ncdump
${NCDUMP} -s $fileurl > ./tmp_misc_$zext.txt
# Extract the parameters
getfilterattr ./tmp_misc_$zext.txt ./tmp_misc2_$zext.txt
rm -f ./tmp_misc_$zext.txt
trimleft ./tmp_misc2_$zext.txt ./tmp_misc_$zext.txt
rm -f ./tmp_misc2_$zext.txt
cat >./tmp_misc2_$zext.txt <<EOF
var:_Filter = "32768,2,239,23,65511,27,77,93,1145389056,3287505826,1097305129,1,2147483648,4294967295,4294967295" ;
EOF
diff -b -w ./tmp_misc_$zext.txt ./tmp_misc_$zext.txt
echo "*** Pass: parameter passing for map $zext"
}

testng() {
zext=$1
echo "*** Testing dynamic filters using ncgen for map $zext"
fileargs tmp_misc
deletemap $zext $file
${NCGEN} -lb -4 -o $fileurl ${srcdir}/../nc_test4/bzip2.cdl
${NCDUMP} -s -n bzip2 $fileurl > ./tmp_ng_$zext.txt
# Remove irrelevant -s output
sclean ./tmp_ng_$zext.txt ./tmp_ng2_$zext.txt
diff -b -w ${srcdir}/ref_bzip2.cdl ./tmp_ng2_$zext.txt
echo "*** Pass: ncgen dynamic filter for map $zext"
}

testncp() {
zext=$1	
echo "*** Testing dynamic filters using nccopy for map $zext"
fileargs tmp_unfiltered
deletemap $zext $file
# Create our input test files
${NCGEN} -4 -lb -o $fileurl ${srcdir}/../nc_test4/ref_unfiltered.cdl
fileurl0=$fileurl
fileargs tmp_filtered
${NCCOPY} -M0 -F "/g/var,307,9,4" $fileurl0 $fileurl
${NCDUMP} -s -n filtered $fileurl > ./tmp_ncp_$zext.txt
# Remove irrelevant -s output
sclean ./tmp_ncp_$zext.txt ./tmp_ncp_$zext.dump
diff -b -w ${srcdir}/ref_filtered.cdl ./tmp_ncp_$zext.dump
echo "	*** Pass: nccopy simple filter for map $zext"
}

testunk() {
zext=$1	
echo "*** Testing access to filter info when filter implementation is not available for map $zext"
fileargs tmp_known
deletemap $zext $file
# build bzip2.nc
${NCGEN} -lb -4 -o $fileurl ${srcdir}/../nc_test4/bzip2.cdl
# dump and clean bzip2.nc header when filter is avail
${NCDUMP} -hs $fileurl > ./tmp_known_$zext.txt
# Remove irrelevant -s output
sclean ./tmp_known_$zext.txt tmp_known_$zext.dump
# Now hide the filter code
mv ${BZIP2PATH} ./${BZIP2LIB}.save
# dump and clean bzip2.nc header when filter is not avail
${NCDUMP} -hs $fileurl > ./tmp_unk_$zext.txt
# Restore the filter code
mv ./${BZIP2LIB}.save ${BZIP2PATH}
# Verify that the filter is no longer defined
UNK=`sed -e '/var:_Filter/p' -e d ./tmp_unk_$zext.txt`
test "x$UNK" = x
echo "*** Pass: ncgen dynamic filter for map $zext"
}

testngc() {
zext=$1	
echo "*** Testing dynamic filters using ncgen with -lc for map $zext"
fileargs tmp_ngc
deletemap $zext $file
${NCGEN} -lc -4 ${srcdir}/../nc_test4/bzip2.cdl > tmp_ngc.c
diff -b -w ${srcdir}/../nc_test4/../nc_test4/ref_bzip2.c ./tmp_ngc.c
echo "*** Pass: ncgen dynamic filter for map $zext"
}

testmulti() {
zext=$1	
echo "*** Testing multiple filters for map $zext"
fileargs tmp_multi
deletemap $zext $file
${execdir}/testfilter_multi $fileurl
${NCDUMP} -hs -n multifilter $fileurl >./tmp_multi_$zext.cdl
# Remove irrelevant -s output
sclean ./tmp_multi_$zext.cdl ./tmp_smulti_$zext.cdl
diff -b -w ${srcdir}/ref_multi.cdl ./tmp_smulti_$zext.cdl
echo "*** Pass: multiple filters for map $zext"
}

testrep() {
zext=$1	
echo "*** Testing filter re-definition invocation for map $zext"
fileargs tmp_rep
deletemap $zext $file
${execdir}/testfilter_repeat $fileurl >tmp_rep_$zext.txt
diff -b -w ${srcdir}/../nc_test4/ref_filter_repeat.txt tmp_rep_$zext.txt
}

testorder() {
zext=$1	
echo "*** Testing multiple filter order of invocation on create for map $zext"
fileargs tmp_order
deletemap $zext $file
${execdir}/testfilter_order create $fileurl >tmp_order_$zext.txt
diff -b -w ${srcdir}/../nc_test4/ref_filter_order_create.txt tmp_order_$zext.txt
echo "*** Testing multiple filter order of invocation on read for map $zext"
${execdir}/testfilter_order read $fileurl >tmp_order_rd_$zext.txt
diff -b -w ${srcdir}/../nc_test4/ref_filter_order_read.txt tmp_order_rd_$zext.txt
}

testset file
if test "x$FEATURE_NCZARR_ZIP" = xyes ; then testset zip ; fi
if test "x$FEATURE_S3TESTS" = xyes ; then testset s3 ; fi

exit 0
