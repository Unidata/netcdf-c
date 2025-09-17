#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi 
. ../test_common.sh

. "$srcdir/test_nczarr.sh"

set -e

isolate "testdir_oldkeys"
THISDIR=`pwd`
cd $ISOPATH

testcase() {
zext=$1
fileargs ref_oldkeys "mode=nczarr,$zext"
# need to unpack the nczarr file
rm -fr ref_oldkeys.file
#echo ""
#echo "=================================="
#echo ""
#echo "Debug Stanza"
#echo "ENV:"
#env | sort
#echo ""
#echo "=================================="
#echo ""
#echo "srcdir: ${srcdir}"
#ls ${srcdir}
#echo ""
#echo "=================================="
#echo ""
#echo "unzip: $(which unzip)"
#echo ""
#echo "=================================="
#echo "End Debug Stanza"
#echo "=================================="
unzip ${srcdir}/ref_oldkeys.file.zip >> tmp_ignore.txt
${NCDUMP} $fileurl > tmp_oldkeys_${zext}.cdl
${ZMD} -t int $fileurl > tmp_oldkeys_${zext}.zmap
diff -b ${srcdir}/ref_oldkeys.cdl tmp_oldkeys_${zext}.cdl
diff -b ${srcdir}/ref_oldkeys.zmap tmp_oldkeys_${zext}.zmap
}

# Only test file case
testcase file
