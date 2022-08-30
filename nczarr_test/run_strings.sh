#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi 
. ../test_common.sh

. "$srcdir/test_nczarr.sh"

# This shell script tests support for the NC_STRING type

set -e

# Cvt stringattr to single char string
stringfixsa() {
rm -f $2
sed -e '/:stringattr/ s|string :|:|' -e '/:stringattr/ s|", "||g' < $1 > $2
}

# Cvt stringattr to JSON format string
stringfixjsa() {
rm -f $2
sed -e '/:stringattr/ s|string :|:|' -e '/:stringattr/ s|"|\\"|g' -e '/:stringattr/ s|= \(.*\);|= "\[\1\]" ;|' < $1 > $2
}

# Cvt v var data to single char string
stringfixv() {
rm -f $2
sed -e '/v = / s|", "||g' < $1 > $2
}

# Cvt charattr to single char string
stringfixca() {
rm -f $2
sed -e '/:charattr/ s|", "||g' <$1 > $2
}

# Cvt c var data to single char string
stringfixc() {
rm -f $2
sed -e '/c = / s|", "||g' < $1 > $2
}

testcase() {
zext=$1

echo "*** Test: nczarr string write then read; format=$zext"
# Get pure zarr args
fileargs tmp_string_zarr "mode=zarr,$zext"
zarrurl="$fileurl"
zarrfile="$file"
# Get nczarr args
fileargs tmp_string_nczarr "mode=nczarr,$zext"
nczarrurl="$fileurl"
nczarrfile="$file"

# setup
deletemap $zext $zarrfile
deletemap $zext $nczarrfile

# Create alternate ref files
echo "*** create pure zarr file"
${NCGEN} -4 -b -o "$zarrurl" $srcdir/ref_string.cdl
echo "*** create nczarr file"
${NCGEN} -4 -b -o "$nczarrurl" $srcdir/ref_string.cdl

echo "*** read purezarr"
${NCDUMP} -n ref_string $zarrurl > tmp_string_zarr_${zext}.cdl
${ZMD} -h $zarrurl > tmp_string_zarr_${zext}.txt
echo "*** read nczarr"
${NCDUMP} -n ref_string $nczarrurl > tmp_string_nczarr_${zext}.cdl
${ZMD} -h $nczarrurl > tmp_string_nczarr_${zext}.txt

echo "*** convert for nczarr comparison"
stringfixca ${srcdir}/ref_string.cdl tmp_ref_string_ca.cdl
stringfixc tmp_ref_string_ca.cdl tmp_ref_string_cac.cdl
   
echo "*** convert for zarr comparison"
stringfixjsa tmp_ref_string_cac.cdl tmp_ref_string_cacsa.cdl

echo "*** verify"
diff -bw tmp_ref_string_cac.cdl tmp_string_nczarr_${zext}.cdl
diff -bw tmp_ref_string_cacsa.cdl tmp_string_zarr_${zext}.cdl
}

testcase file
if test "x$FEATURE_NCZARR_ZIP" = xyes ; then testcase zip; fi
if test "x$FEATURE_S3TESTS" = xyes ; then testcase s3; fi

exit 0
