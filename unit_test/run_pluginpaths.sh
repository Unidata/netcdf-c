#!/bin/bash

# Test the programmatic API for manipulating the plugin paths.
# This script is still full of cruft that needs to be removed

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

# Bash under windows/mingw has bad habit of translating '/' to '\\'
# for command line arguments
export MSYS2_ARG_CONV_EXCL="*" 

DFALT="/zero;/one;/two;/three;/four"
DFALTSET="/zero;/one;/mod;/two;/three;/four"
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

# Test that global state is same as that for HDF5 and NCZarr.
# It is difficult to test for outside interference, so not attempted.
testget() {
    filenamefor tmp get
    # print out the global state
    printf "testget(global): %s\n" `${TP} -x "set:${DFALT},get:global"` >> ${filename}.txt
    # print out the HDF5 state
    printf "testget(hdf5): %s\n" `${TP} -x "set:${DFALT},get:hdf5"` >> ${filename}.txt ;
    # print out the NCZarr state
    printf "testget(nczarr): %s\n" `${TP} -x "set:${DFALT},get:nczarr"` >> ${filename}.txt ;
}                           

# Set the global state to some value and verify that it was sync'd to hdf5 and nczarr
testset() {
    filenamefor tmp set
    # print out the global state, modify it and print again
    printf "testset(global): before: %s\n" `${TP} -x "set:${DFALT},get:global"` >> ${filename}.txt
    printf "testset(global): after: %s\n" `${TP} -x "set:${DFALT},set:${DFALTSET},get:global"` >> ${filename}.txt
    # print out the HDF5 state
    printf "testset(hdf5): before: %s\n" `${TP} -x "set:${DFALT},get:hdf5"` >> ${filename}.txt
    printf "testset(hdf5): after: %s\n" `${TP} -x "set:${DFALT},set:${DFALTSET},get:hdf5"` >> ${filename}.txt
    # print out the NCZarr state
    printf "testset(nczarr): before: %s\n" `${TP} -x "set:${DFALT},get:nczarr"` >> ${filename}.txt
    printf "testset(nczarr): after: %s\n" `${TP} -x "set:${DFALT},set:${DFALTSET},get:nczarr"` >> ${filename}.txt
}                           

# Test the ncaux set/get/clear functions */
testxget() {
    filenamefor tmp xget
    # print out the global state
    printf "testxget(global): %s\n" `${TP} -x "xset:${DFALT},xget:global"` >> ${filename}.txt
}                           

testxset() {
    filenamefor tmp xset
    # print out the global state, modify it and print again
    printf "testxset(global): before: %s\n" `${TP} -x "xset:${DFALT},xget:global"` >> ${filename}.txt
    printf "testxset(global): after: %s\n" `${TP} -x "xset:${DFALT},xset:${DFALTSET},xget:global"` >> ${filename}.txt
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
    for action in get set xget xset; do
        if diff -wBb ${srcdir}/ref_${action}.txt tmp_${action}.txt ; then
	    echo "***PASS: $action"
	else
	    echo "***FAIL: $action"
	    exit 1
        fi
    done
}

init
testget
testset
testxget
testxset
verify
cleanup
