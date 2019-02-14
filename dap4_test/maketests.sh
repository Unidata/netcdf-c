#!/bin/sh

# The purpose of this program is to fill in the
# testdata files: dmrtestfiles, daptestfiles, and
# cdltestfiles.
# 
# It obtains that data from the output of a thredds
# test called GenerateRaw.java. See the description
# in that program of its actions.
# 
# It is assumed that the SRC environment variable
# (below) points to the directory in the thredds
# tree containing the directories named
# rawtestfiles and testfiles.
# 
# This program copies the files
# ${SRC}/dmrtestfiles/*.dmr to the dmrtestfiles
# directory in this (dap4_test) directory.
# Similarly it copies ${SRC}/daptestfiles/*.dap to
# the local daptestfiles directory. Additionally,
# it looks at ${SRC}/testfiles/*.cdl to see if
# there are any .cdl files corresponding (by name)
# to the files in daptestfiles.

if test $# != 0 ; then
for arg in "$@"; do
  case "${arg}" in
  debug) DEBUG=1 ;;
  git) GITADD=1 ;;
  *) echo unknown argument $arg ;;
  esac
done
fi

SRC="d:/git/thredds/dap4/d4tests/src/test/data/resources"

# Compare files instead of recreating
if test "x$DEBUG" = x1 ; then
  pushd ./daptestfiles
  F=`ls -1 *.dap | sed -e 's/[.]dap//' |tr '\r\n' '  '`
  popd
  for f in ${F} ; do
    if ! diff -wBb ${SRC}/dmrtestfiles/${f}.dmr ./dmrtestfiles/${f}.dmr > /dev/null 2>&1 ; then
      echo diff -wBb ${SRC}/dmrtestfiles/${f}.dmr ./dmrtestfiles/${f}.dmr
      diff -wBb ${SRC}/dmrtestfiles/${f}.dmr ./dmrtestfiles/${f}.dmr
    fi
  done
  for f in ${F} ; do
    diff -wBb ${SRC}/daptestfiles/${f}.dapr ./dmrtestfiles/${f}.dap
  done
  exit
fi

# Recreate the <testfiles> directories
rm -fr ./daptestfiles ./cdltestfiles ./dmrtestfiles
mkdir ./daptestfiles
mkdir ./dmrtestfiles
mkdir ./cdltestfiles

# Copy the .dap and .dmr files
cp ${SRC}/daptestfiles/*.dap ./daptestfiles
cp ${SRC}/dmrtestfiles/*.dmr ./dmrtestfiles

# Get the list of daptestfiles with the extensions removed
pushd ./daptestfiles
F=`ls -1 *.nc.dap | sed -e 's/[.]nc[.]dap//' |tr '\r\n' '  '`
popd


# For each .dap file, see if a corresponding .cdl file
# exists in ${SRC}/testfiles; if so, copy it over.
for f in ${F} ; do
if test -e ${SRC}/testfiles/${f}.cdl ; then
  cp ${SRC}/testfiles/${f}.cdl ./cdltestfiles
else
  echo "Not found: ${SRC}/testfiles/${f}.cdl"
fi
done

# Compile the cdl files
pushd ./cdltestfiles
F=`ls -1 *.cdl | sed -e 's/[.]cdl//' |tr '\r\n' '  '`
popd
if ! test -e nctestfiles ; then mkdir nctestfiles; fi
for f in $F ; do
    ../ncgen/ncgen -4 -o nctestfiles/${f}.nc cdltestfiles/${f}.cdl 
done

# Fix the permissions; this is cosmetic
pushd daptestfiles ; chmod a-x * ; popd
pushd dmrtestfiles ; chmod a-x * ; popd
pushd cdltestfiles ; chmod a-x * ; popd
pushd nctestfiles ; chmod a-x * ; popd

# If invoked with the argument "git", this
# program will add the files to the git repo.
if test "x${GITADD}" = x1 ; then
git add daptestfiles/*.dap
git add daptestfiles/*.dmr
git add cdltestfiles/*.cdl
git add nctestfiles/*.nc
fi

exit
