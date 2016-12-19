#!/bin/bash

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
# ${SRC}/rawtestfiles/*.dmr to the dmrtestfiles
# directory in this (dap4_test) directory.
# Similarly it copies ${SRC}/rawtestfiles/*.dap to
# the local daptestfiles directory. Additionally,
# it looks at ${SRC}/testfiles/*.cdl to see if
# there are any .cdl files corresponding (by name)
# to the files in daptestfiles.

SRC="d:/git/thredds/dap4/d4tests/src/test/data/resources"

# Recreate the testfiles directories
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
#exists in ${SRC}/testfiles; if so, copy it over.
for f in ${F} ; do
if test -f ${SRC}/testfiles/${f}.cdl ; then
  cp ${SRC}/testfiles/${f}.cdl ./cdltestfiles
else
  echo "Not found: ${SRC}/testfiles/${f}.cdl"
fi
done

# Fix the permissions; this is cosmetic
pushd daptestfiles ; chmod a-x * ; popd
pushd dmrtestfiles ; chmod a-x * ; popd
pushd cdltestfiles ; chmod a-x * ; popd

# If invoked with the argument "git", this
# program will add the files to the git repo.
if test "x$1" = xgit ; then
git add daptestfiles/*.dap
git add daptestfiles/*.dmr
git add cdltestfiles/*.cdl
fi

exit
