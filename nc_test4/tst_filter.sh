#!/bin/bash

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

set -e

# Which test cases to exercise
API=1
NG=1
NCP=1
NGC=1
MISC=1
MULTI=1
REP=1
ORDER=1

# Load the findplugins function
. ${builddir}/findplugin.sh
echo "findplugin.sh loaded"

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

trimleft() {
sed -e 's/[ 	]*\([^ 	].*\)/\1/' <$1 >$2
}

# Locate the plugin dir and the library names; argument order is critical
# Find bzip2 and capture
findplugin h5bzip2
BZIP2LIB="${HDF5_PLUGIN_LIB}"
BZIP2DIR="${HDF5_PLUGIN_DIR}/${BZIP2LIB}"
# Find misc and capture
findplugin h5misc
MISCDIR="${HDF5_PLUGIN_DIR}/${HDF5_PLUGIN_LIB}"
# Find noop and capture
findplugin h5noop
NOOPLIB="${HDF5_PLUGIN_LIB}"
NOOPDIR="${HDF5_PLUGIN_DIR}/${HDF5_PLUGIN_LIB}"

echo "final HDF5_PLUGIN_DIR=${HDF5_PLUGIN_DIR}"
export HDF5_PLUGIN_DIR
export HDF5_PLUGIN_PATH="$HDF5_PLUGIN_DIR"

# Verify
if ! test -f ${BZIP2DIR} ; then echo "Unable to locate ${BZIP2DIR}"; exit 1; fi
if ! test -f ${MISCDIR} ; then echo "Unable to locate ${MISCDIR}"; exit 1; fi
if ! test -f ${NOOPDIR} ; then echo "Unable to locate ${NOOPDIR}"; exit 1; fi

# See if we have szip
if avail szip; then HAVE_SZIP=1; else HAVE_SZIP=0; fi

# Execute the specified tests

if test "x$API" = x1 ; then
echo "*** Testing dynamic filters using API"
rm -f ./tmp_bzip2.nc ./tmp_bzip2.dump ./tmp_filter.txt
${execdir}/test_filter
${NCDUMP} -s -n bzip2 tmp_bzip2.nc > ./tmp_filter.txt
# Remove irrelevant -s output
sclean ./tmp_filter.txt ./tmp_bzip2.dump
diff -b -w ${srcdir}/bzip2.cdl ./tmp_bzip2.dump
echo "*** Pass: API dynamic filter"
fi

if test "x$MISC" = x1 ; then
echo
echo "*** Testing dynamic filters parameter passing"
rm -f ./tmp_misc.nc tmp_filter.txt tmp_filter2.txt
${execdir}/test_filter_misc
# Verify the parameters via ncdump
${NCDUMP} -s tmp_misc.nc > ./tmp_misc.txt
# Extract the parameters
getfilterattr ./tmp_misc.txt ./tmp_filter2.txt
rm -f ./tmp_misc.txt
trimleft ./tmp_filter2.txt ./tmp_filter.txt
rm -f ./tmp_filter2.txt
cat >./tmp_filter2.txt <<EOF
var:_Filter = "32768,3,239,23,65511,27,77,93,1145389056,3287505826,1097305129,1,2147483648,4294967295,4294967295" ;
EOF
diff -b -w ./tmp_filter.txt ./tmp_filter2.txt
echo "*** Pass: parameter passing"
fi

if test "x$NG" = x1 ; then
echo "*** Testing dynamic filters using ncgen"
rm -f ./tmp_bzip2.nc ./tmp_bzip2.dump ./tmp_filter.txt
${NCGEN} -lb -4 -o tmp_bzip2.nc ${srcdir}/bzip2.cdl
${NCDUMP} -s -n bzip2 tmp_bzip2.nc > ./tmp_filter.txt
# Remove irrelevant -s output
sclean ./tmp_filter.txt ./tmp_bzip2.dump
diff -b -w ${srcdir}/bzip2.cdl ./tmp_bzip2.dump
echo "*** Pass: ncgen dynamic filter"
fi

if test "x$NCP" = x1 ; then
echo "*** Testing dynamic filters using nccopy"
rm -f ./tmp_unfiltered.nc ./tmp_filtered.nc ./tmp_filtered.dump ./tmp_filter.txt
# Create our input test files
${NCGEN} -4 -lb -o tmp_unfiltered.nc ${srcdir}/ref_unfiltered.cdl
${NCGEN} -4 -lb -o tmp_unfilteredvv.nc ${srcdir}/ref_unfilteredvv.cdl

echo "	*** Testing simple filter application"
${NCCOPY} -M0 -F "/g/var,307,9" tmp_unfiltered.nc tmp_filtered.nc
${NCDUMP} -s -n filtered tmp_filtered.nc > ./tmp_filter.txt
# Remove irrelevant -s output
sclean ./tmp_filter.txt ./tmp_filtered.dump
diff -b -w ${srcdir}/ref_filtered.cdl ./tmp_filtered.dump
echo "	*** Pass: nccopy simple filter"

echo "	*** Testing '*' filter application"
${NCCOPY} -M0 -F "*,307,9" tmp_unfilteredvv.nc tmp_filteredvv.nc
${NCDUMP} -s -n filteredvv tmp_filteredvv.nc > ./tmp_filtervv.txt
# Remove irrelevant -s output
sclean ./tmp_filtervv.txt ./tmp_filteredvv.dump
diff -b -w ${srcdir}/ref_filteredvv.cdl ./tmp_filteredvv.dump
echo "	*** Pass: nccopy '*' filter"

echo "	*** Testing 'v&v' filter application"
${NCCOPY} -M0 -F "var1&/g/var2,307,9" tmp_unfilteredvv.nc tmp_filteredvbar.nc
${NCDUMP} -n filteredvv -s tmp_filteredvbar.nc > ./tmp_filtervbar.txt
# Remove irrelevant -s output
sclean ./tmp_filtervbar.txt ./tmp_filteredvbar.dump
diff -b -w ${srcdir}/ref_filteredvv.cdl ./tmp_filteredvbar.dump
echo "	*** Pass: nccopy 'v|v' filter"

echo "	*** Testing pass-thru of filters"
rm -f ./tmp_filter.txt tmp_filter2.txt ./tmp_filter2.nc
# Prevent failure by allowing any chunk size
${NCCOPY} -M0 ./tmp_filtered.nc ./tmp_filter2.nc
${NCDUMP} -s tmp_filter2.nc > ./tmp_filter.txt
sed -e '/_Filter/p' -e d < ./tmp_filter.txt >tmp_filter2.txt
test -s tmp_filter2.txt
echo "	*** Pass: pass-thru of filters"

echo "	*** Testing -F none"
rm -f ./tmp_none.txt ./tmp_none2.txt ./tmp_none.nc
${NCCOPY} -M0 -F none ./tmp_filtered.nc ./tmp_none.nc
${NCDUMP} -hs tmp_none.nc > ./tmp_none.txt
sed -e '/_Filter/p' -e d < ./tmp_none.txt >./tmp_none2.txt
test ! -s tmp_none2.txt
echo "	*** Pass: -F none"

echo "	*** Testing -F var,none "
rm -f ./tmp_vnone.txt ./tmp_vnone.nc tmp_vnone2.txt
${NCCOPY} -M0 -F "/g/var,none" ./tmp_filtered.nc ./tmp_vnone.nc
${NCDUMP} -s tmp_vnone.nc > ./tmp_vnone.txt
sed -e '/_Filter/p' -e d < ./tmp_vnone.txt >tmp_vnone2.txt
test ! -s tmp_vnone2.txt
echo "	*** Pass: -F var,none"

echo "*** Pass: all nccopy filter tests"
fi

if test "x$NGC" = x1 ; then
rm -f ./tmp_bzip2.c
echo "*** Testing dynamic filters using ncgen with -lc"
${NCGEN} -lc -4 ${srcdir}/bzip2.cdl > tmp_bzip2.c
diff -b -w ${srcdir}/ref_bzip2.c ./tmp_bzip2.c
echo "*** Pass: ncgen dynamic filter"
fi

if test "x$MULTI" = x1 ; then
echo "*** Testing multiple filters"
rm -f ./tmp_multifilter.nc ./tmp_multi.txt ./tmp_smulti.cdl
rm -f tmp_nccopyF.cdl tmp_nccopyF.nc tmp_ncgenF.cdl tmp_ncgenF.nc
${execdir}/tst_multifilter
${NCDUMP} -hsF -n multifilter tmp_multifilter.nc >./tmp_multi.cdl
# Remove irrelevant -s output
sclean ./tmp_multi.cdl ./tmp_smulti.cdl
diff -b -w ${srcdir}/ref_multi.cdl ./tmp_smulti.cdl
echo "*** nccopy -F with multiple filters"
if ! test -f tmp_unfiltered.nc ; then
  ${NCGEN} -4 -lb -o tmp_unfiltered.nc ${srcdir}/ref_unfiltered.cdl
fi
${NCCOPY} "-F/g/var,307,4|40000" tmp_unfiltered.nc tmp_nccopyF.nc
${NCDUMP} -hs -n nccopyF tmp_nccopyF.nc > ./tmp_nccopyF.cdl
sclean tmp_nccopyF.cdl tmp_nccopyFs.cdl
diff -b -w ${srcdir}/ref_nccopyF.cdl ./tmp_nccopyFs.cdl
echo "*** ncgen with multiple filters"
${NCGEN} -4 -lb -o tmp_ncgenF.nc ${srcdir}/ref_nccopyF.cdl
# Need to fix name using -n
${NCDUMP} -hsF -n nccopyF tmp_ncgenF.nc > ./tmp_ncgenF.cdl
sclean tmp_ncgenF.cdl tmp_ncgenFs.cdl
diff -b -w ${srcdir}/ref_nccopyF.cdl ./tmp_ncgenFs.cdl
echo "*** Pass: multiple filters"
fi

if test "x$REP" = x1 ; then
echo "*** Testing filter re-definition invocation"
rm -f tmp_filterrepeat.txt
${execdir}/test_filter_repeat >tmp_filterrepeat.txt
diff -b -w ${srcdir}/ref_filter_repeat.txt tmp_filterrepeat.txt
fi

if test "x$ORDER" = x1 ; then
echo "*** Testing multiple filter order of invocation on create"
rm -f tmp_crfilterorder.txt
${execdir}/test_filter_order create >tmp_crfilterorder.txt
diff -b -w ${srcdir}/ref_filter_order_create.txt tmp_crfilterorder.txt

echo "*** Testing multiple filter order of invocation on read"
rm -f tmp_rdfilterorder.txt
${execdir}/test_filter_order read >tmp_rdfilterorder.txt
diff -b -w ${srcdir}/ref_filter_order_read.txt tmp_rdfilterorder.txt

fi

# Cleanup

echo "*** Pass: all selected tests passed"

exit 0
