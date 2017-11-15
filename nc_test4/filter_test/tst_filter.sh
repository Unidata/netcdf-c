#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../../test_common.sh

set -e
export HDF5_PLUGIN_PATH=`pwd`

# Which test cases to exercise
API=1
NG=1
NCP=1
UNK=1
NGC=1
MISC=1

# Function to remove selected -s attributes from file;
# These attributes might be platform dependent
sclean() {
cat $1 \
  | sed -e '/var:_Endianness/d' \
  | sed -e '/_NCProperties/d' \
  | sed -e '/_SuperblockVersion/d' \
  | sed -e '/_IsNetcdf4/d' \
  | cat > $2
}

# Function to extract _Filter attribute from a file
# These attributes might be platform dependent
getfilterattr() {
sed -e '/var:_Filter/p' -ed <$1 >$2
}

trimleft() {
sed -e 's/[ 	]*\([^ 	].*\)/\1/' <$1 >$2
}

if test "x$API" = x1 ; then
echo "*** Testing dynamic filters using API"
rm -f ./bzip2.nc ./bzip2.dump ./tmp
${execdir}/test_filter
$NCDUMP -s ./bzip2.nc > ./tmp
# Remove irrelevant -s output
sclean ./tmp ./bzip2.dump
diff -b -w ${srcdir}/bzip2.cdl ./bzip2.dump
echo "*** Pass: API dynamic filter"
fi

if test "x$MISC" = x1 ; then
echo
echo "*** Testing dynamic filters parameter passing"
rm -f ./testmisc.nc tmp tmp2
${execdir}/test_misc
# Verify the parameters via ncdump
$NCDUMP -s ./testmisc.nc > ./tmp
# Extract the parameters
getfilterattr ./tmp ./tmp2
rm -f ./tmp
trimleft ./tmp2 ./tmp
rm -f ./tmp2
cat >./tmp2 <<EOF
var:_Filter = "32768,1,4294967279,23,4294967271,27,77,93,1145389056,697067329,2723935171,128,16777216,4294967295,4294967295" ;
EOF
diff -b -w ./tmp ./tmp2
echo "*** Pass: parameter passing"
fi

if test "x$NG" = x1 ; then
echo "*** Testing dynamic filters using ncgen"
rm -f ./bzip2.nc ./bzip2.dump ./tmp
$NCGEN -lb -4 -o bzip2.nc ${srcdir}/bzip2.cdl
$NCDUMP -s ./bzip2.nc > ./tmp
# Remove irrelevant -s output
sclean ./tmp ./bzip2.dump
diff -b -w ${srcdir}/bzip2.cdl ./bzip2.dump
echo "*** Pass: ncgen dynamic filter"
fi

if test "x$NCP" = x1 ; then
echo "*** Testing dynamic filters using nccopy"
rm -f ./unfiltered.nc ./filtered.nc ./filtered.dump ./tmp
$NCGEN -4 -lb -o unfiltered.nc ${srcdir}/unfiltered.cdl
$NCCOPY -F "/g/var,307,9,4" unfiltered.nc filtered.nc
$NCDUMP -s ./filtered.nc > ./tmp
# Remove irrelevant -s output
sclean ./tmp ./filtered.dump
diff -b -w ${srcdir}/filtered.cdl ./filtered.dump
echo "*** Pass: nccopy dynamic filter"
fi

if test "x$UNK" = x1 ; then
echo "*** Testing access to filter info when filter dll is not available"
rm -f bzip2.nc ./tmp
# build bzip2.nc
$NCGEN -lb -4 -o bzip2.nc ${srcdir}/bzip2.cdl
for f in cygbzip2.dll libbzip2.so ; do
  if test -f ${execdir}/$f ; then LIBNAME=${execdir}/$f; fi;
done
# dump and clean bzip2.nc header only when filter is avail
$NCDUMP -hs ./bzip2.nc > ./tmp
# Remove irrelevant -s output
sclean ./tmp bzip2.dump
# Now hide the filter code
mv ${LIBNAME} ${LIBNAME}.save
# dump and clean bzip2.nc header only when filter is not avail
rm -f ./tmp
$NCDUMP -hs ./bzip2.nc > ./tmp
# Remove irrelevant -s output
sclean ./tmp bzip2x.dump
# Restore the filter code
mv ${LIBNAME}.save ${LIBNAME}
diff -b -w ./bzip2.dump ./bzip2x.dump
echo "*** Pass: ncgen dynamic filter"
fi

if test "x$NGC" = x1 ; then
rm -f ./test_bzip2.c
echo "*** Testing dynamic filters using ncgen with -lc"
$NCGEN -lc -4 ${srcdir}/bzip2.cdl > test_bzip2.c
diff -b -w ${srcdir}/ref_bzip2.c ./test_bzip2.c
echo "*** Pass: ncgen dynamic filter"
fi

#cleanup
rm -f ./bzip*.nc ./unfiltered.nc ./filtered.nc ./tmp ./tmp2 *.dump bzip*hdr.*
rm -fr ./test_bzip2.c
rm -fr ./testmisc.nc
exit 0
