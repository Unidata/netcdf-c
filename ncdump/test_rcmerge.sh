#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

# Test that all available .rc files are merged and overrides are handled.
# The internal .rc is constructed as follows:
#
# 1. Use NCRCENV_RC environment variable exclusively if defined
# 2. If NCRCENV_RC is not defined then merge the set of rc files in this order:
#    1. $RCHOME/.ncrc
#    2. $RCHOME/.daprc
#    3. $RCHOME/.docsrc
#    4. $CWD/.ncrc
#    5. $CWD/.daprc
#    6. $CWD/.docsrc
#    Entries in later files override any of the >earlier >files
# RCHOME overrides HOME when searching for .rc files.

# Since this involves a shared resource: the .rc files in current working directory,
# we need to isolate from any other test.

# Make sure execdir and srcdir absolute paths are available
WD=`pwd`
cd $srcdir ; abs_srcdir=`pwd` ; cd $WD
cd $execdir ; abs_execdir=`pwd` ; cd $WD

#DEBUG=1
#TRUEHOME=1

# Create RCHOME
if test "x$TRUEHOME" = x1 ; then
RCHOME="$HOME"
else
rm -fr rchome
mkdir rchome
cd rchome
RCHOME=`pwd`
cd ..
export NCRCENV_HOME="$RCHOME"
fi

# Now create a special directory
# And enter it to execute tests
rm -fr rcmergedir
mkdir rcmergedir
cd rcmergedir
WD=`pwd`

HOMERCFILES="$RCHOME/.ncrc $RCHOME/.daprc $RCHOME/.dodsrc"
LOCALRCFILES="$WD/.ncrc $WD/.daprc $WD/.dodsrc"

resetrc() {
  rm -fr $HOMERCFILES
  rm -f $LOCALRCFILES
  unset NCRCENV_RC
  rm -f tmp_rcmerge.txt
  rm -f tmp_rcapi.txt
  rm -f allfiles1 allfiles2 allfiles3
}

union() {
if test "x$DEBUG" = x1 ; then
  rm -f ../allfiles$1
  for f in $HOMERCFILES $LOCALRCFILES; do
     if test -f $f ; then cat  $f >> ../allfiles$1 ; fi
  done
fi
}

mergecase1() {
    # create everything with different keys to test merge 
    resetrc
    rm -f tmp_rcmerge.txt tmp_rcmerge.txt
    echo  "for r=ncrc daprc dodsrc"
    for r in "ncrc" "daprc" "dodsrc"; do
        echo "${r}_home=${r}" >> $RCHOME/".${r}";
        echo "${r}_local=${r}" >> $WD/".${r}"
    done;
    union 1
    ${abs_execdir}/tst_rcmerge |sort > tmp_rcmerge.txt
#    echo ">>merge1"; cat ${abs_srcdir}/ref_rcmerge1.txt;
#    echo "====="; cat tmp_rcmerge.txt
    diff -b ${abs_srcdir}/ref_rcmerge1.txt tmp_rcmerge.txt
}

mergecase2() {
    # create with some same keys to test override
    resetrc
    rm -f tmp_rcmerge.txt tmp_rcmerge.txt
    echo "Create in $RCHOME"
    for r in "ncrc" "daprc" "dodsrc" ; do
        echo "${r}=${r}" >> $RCHOME/".${r}";
    done;
    echo "Create in $WD"
    for r in "ncrc" "daprc" "dodsrc" ; do
        echo "${r}=${r}" >> $WD/".${r}"
    done;
    union 2
    ${abs_execdir}/tst_rcmerge |sort > tmp_rcmerge.txt
    diff -b ${abs_srcdir}/ref_rcmerge2.txt tmp_rcmerge.txt
}

mergecase3() {
    # Test cross file overrides
    resetrc
    rm -f tmp_rcmerge.txt tmp_rcmerge.txt
    echo "ncrc=ncrc1" >> $HOME/.ncrc
    echo "ncrcx=ncrcx" >> $RCHOME/.ncrc
    echo "ncrc=ncrc2" >> $RCHOME/.dodsrc
    echo "daprc=daprc" >> $RCHOME/.daprc
    echo "ncrc=ncrc1" >> $WD/.ncrc
    echo "ncrcx=ncrcx" >> $WD/.ncrc
    echo "ncrc=ncrc2" >> $WD/.dodsrc
    echo "daprc=daprc" >> $WD/.daprc
    echo "daprc=daprc" >> $WD/.dodsrc
    echo "ncrcx=ncrcy" >> $WD/.dodsrc
    union 3
    ${abs_execdir}/tst_rcmerge |sort -d > tmp_rcmerge.txt
    diff -b ${abs_srcdir}/ref_rcmerge3.txt tmp_rcmerge.txt
}

rcapi1() {
    resetrc
    echo "[http://github.com/a/b/c]ncrc=ncrc1" >> $WD/.ncrc
    echo "ncrc=ncrc2" >> $WD/.ncrc
    echo "[http://github.com:8080/a/b/c]key0=v0" >> $WD/.ncrc
    echo "[http://github.com]key0=v1" >> $WD/.ncrc
    ${abs_execdir}/tst_rcapi > tmp_rcapi.txt
    diff -b ${abs_srcdir}/ref_rcapi.txt ./tmp_rcapi.txt
}

resetrc

mergecase1
mergecase2
mergecase3
# Test the .rc api
rcapi1

# Test the .rc api
rcapi1

resetrc
