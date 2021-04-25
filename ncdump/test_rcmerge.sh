#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

# Test that all available .rc files are merged and overrides are handled.
# The internal .rc is constructed as follows:
#
# 1. Use NCRCENV_RC environment variable exclusively if defined
# 2. If NCRCENV_RC is not defined then merge the set of rc files in this order:
#    1. $HOME/.ncrc
#    2. $HOME/.daprc
#    3. $HOME/.docsrc
#    4. $CWD/.ncrc
#    5. $CWD/.daprc
#    6. $CWD/.docsrc
#    Entries in later files override any of the >earlier >files

if test "x$NCAUTH_HOMETEST" != x ; then
    RCHOME=1
fi

WD=`pwd`

HOMERCFILES="$HOME/.ncrc $HOME/.daprc $HOME/.dodsrc"
LOCALRCFILES="$WD/.ncrc $WD/.daprc $WD/.dodsrc"

resetrc() {
  if test "x$RCHOME" = x1 ; then
      rm -f $HOMERCFILES
  fi
  rm -f $LOCALRCFILES
  unset NCRCENV_RC
}

mergecase1() {
    # create everything with different keys to test merge 
    resetrc
    rm -f tmp_rcmerge.txt tmpoutput.txt
    for r in "ncrc" "daprc" "dodsrc" ; do
        if test "x$RCHOME" = x1 ; then echo "${r}_home=${r}" >> $HOME/".${r}"; fi
        echo "${r}_local=${r}" >> $WD/".${r}"
    done;
    ${execdir}/tst_rcmerge > tmpoutput.txt
    if test "x$RCHOME" = x1 ; then
	cp ${srcdir}/ref_rcmerge1.txt tmp_rcmerge1.txt
    else
	sed -e '/_local/p' -e d <${srcdir}/ref_rcmerge1.txt > tmp_rcmerge1.txt
    fi
    diff -b tmp_rcmerge1.txt tmpoutput.txt
}

mergecase2() {
    # create with some same keys to test override
    resetrc
    rm -f tmp_rcmerge.txt tmpoutput.txt
    for r in "ncrc" "daprc" "dodsrc" ; do
        if test "x$RCHOME" = x1 ; then echo "${r}=${r}" >> $HOME/".${r}"; fi
        echo "${r}=${r}" >> $WD/".${r}"
    done;
    ${execdir}/tst_rcmerge > tmpoutput.txt
    diff -b ${srcdir}/ref_rcmerge2.txt tmpoutput.txt
}

mergecase3() {
    # Test cross file overrides
    resetrc
    rm -f tmp_rcmerge.txt tmpoutput.txt
    if test "x$RCHOME" = x1 ; then
	echo "ncrc=ncrc1" >> $HOME/.ncrc
	echo "ncrcx=ncrcx" >> $HOME/.ncrc
	echo "ncrc=ncrc2" >> $HOME/.dodsrc
	echo "daprc=daprc" >> $HOME/.daprc
    else
	echo "ncrc=ncrc1" >> $WD/.ncrc
	echo "ncrcx=ncrcx" >> $WD/.ncrc
	echo "ncrc=ncrc2" >> $WD/.dodsrc
	echo "daprc=daprc" >> $WD/.daprc
    fi
    echo "daprc=daprc" >> $WD/.dodsrc
    echo "ncrcx=ncrcy" >> $WD/.dodsrc

    ${execdir}/tst_rcmerge > tmpoutput.txt
    diff -b ${srcdir}/ref_rcmerge3.txt tmpoutput.txt
}

resetrc

mergecase1
mergecase2
mergecase3

resetrc
