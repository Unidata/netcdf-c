#!/bin/sh

set -e

# if this is part of a distcheck action, then this script
# will be executed in a different directory
# than the one containing it; so capture the path to this script
# as the location of the source directory.
srcdir=`dirname $0`
# Hack for CYGWIN
cd $srcdir
srcdir=`pwd`

# compute the build directory
builddir=`pwd`/..

if [ `uname | cut -d "_" -f 1` = "MINGW32" ]; then
    srcdir=`pwd | sed 's/\/c\//c:\//g'`
    builddir="$srcdir"/..
fi

# Hack for CYGWIN
cd $builddir
builddir=`pwd`

cd ${builddir}/ncdap_test

sh  ${srcdir}/tst_remote.sh "$srcdir" "$builddir" "3" "" ""
exit
