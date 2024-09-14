#!/bin/bash

# Test the programmatic API for manipulating the plugin paths.
# WARNING: This file is also used to build nczarr_test/run_pluginpath.sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

set -e

IMPLS=
if test "x$FEATURE_HDF5" = xyes ; then IMPLS="$IMPLS hdf5"; fi
if test "x$FEATURE_NCZARR" = xyes ; then IMPLS="$IMPLS nczarr"; fi
# Remove leading blank
IMPLS=`echo "$IMPLS" | cut -d' ' -f2,3`
echo "IMPLS=|$IMPLS|"

#VERBOSE=1

DFALT="/zero;/one;/two;/three;/four"
DFALTHDF5="/zero;/one;/two;/hdf5;/three;/four"
DFALTNCZARR="/zero;/one;/two;/nczarr;three;/four;/five"

if test "x$TESTNCZARR" = x1 ; then
. "$srcdir/test_nczarr.sh"
s3isolate "testdir_pluginpath"
THISDIR=`pwd`
cd $ISOPATH
fi

TP="${execdir}/tst_pluginpaths"

filenamefor() {
  # tmp|ref_action
  eval "filename=${1}_$2"
}

dfaltfor() {
    case "$1" in
	hdf5) eval "dfalt=\"$DFALTHDF5\"" ;;
	nczarr) eval "dfalt=\"$DFALTNCZARR\"" ;;
	all) eval "dfalt=\"$DFALT\"" ;;
    esac
}

modfor() {
    local formatx="$1"
    local dfalt="$2"
    case "$formatx" in
	hdf5) mod="${dfalt};/modhdf5" ;;
	nczarr) mod="/modnczarr;${dfalt}" ;;
	all) mode="${dfalt}" ;;
    esac
}

#####

testread() {
    local formatx="$1"
    filenamefor tmp read
    dfaltfor $formatx
    if test "$formatx" = all ; then
	echo "testread(${formatx}): " >> ${filename}.txt
	for f in $IMPLS ; do testread $f ; done
    elif test "x$FEATURE_HDF5" && test "$formatx" = hdf5 ; then
	echon "testread(${formatx}): " >> ${filename}.txt
	${TP} -x "formatx:${formatx},write:${dfalt},read" >> ${filename}.txt ;
    elif test "x$FEATURE_NCZARR" && test "$formatx" = nczarr ; then
	echon "testread(${formatx}): " >> ${filename}.txt
	${TP} -x "formatx:${formatx},write:${dfalt},read" >> ${filename}.txt ;
    fi
}                           

testwrite() {
    local formatx="$1"
    filenamefor tmp write
    dfaltfor $formatx
    modfor $formatx "$dfalt"
    if test "$formatx" = all ; then
	echo "testwrite(${formatx}): " >> ${filename}.txt
	for f in $IMPLS ; do testwrite $f ; done
    elif test "x$FEATURE_HDF5" && test "$formatx" = hdf5 ; then
        echon "testwrite(${formatx}): before: " >> ${filename}.txt
	    ${TP} -x "formatx:${formatx},write:${dfalt},read"  >> ${filename}.txt
	echon "testwrite(${formatx}): after: " >> ${filename}.txt
	    ${TP} -x "formatx:${formatx},write:${mod},read"  >> ${filename}.txt
    elif test "x$FEATURE_NCZARR" && test "$formatx" = nczarr ; then
	echon "testwrite(${formatx}): before: " >> ${filename}.txt
	    ${TP} -x "formatx:${formatx},write:${dfalt},read"  >> ${filename}.txt
	echon "testwrite(${formatx}): after: " >> ${filename}.txt
	    ${TP} -x "formatx:${formatx},write:${mod},read"  >> ${filename}.txt
    fi
}                           

#########################

cleanup() {
    rm -f tmp_*.txt
}

init() {
    cleanup
}

# Verify output for a specific action
verify() {
    for action in read write ; do
        if diff -wBb ${srcdir}/ref_${action}.txt tmp_${action}.txt ; then
	    echo "***PASS: $action"
	else
	    echo "***FAIL: $action"
	    exit 1
        fi
    done
}

init
for fx in $IMPLS all ; do
    testread  $fx
    testwrite $fx
done
verify
cleanup
