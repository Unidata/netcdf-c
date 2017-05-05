#!/bin/bash
# This shell script runs the nc-dap tests.
# $Id: run_ncdap_tests.sh,v 1.5 2009/04/02 22:03:52 dmh Exp $
#set -o xtrace
# if this is part of a distcheck action, then this script
# will be executed in a different directory
# than the ontaining it; so capture the path to this script
# as the location of the source directory.

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

# Locate the expected directory
expected="${srcdir}/expected"

# Define success/fail returns
OKRETURN=0
# For now, always return success
FAILRETURN=1

# The opendap server appears to be sometimes difficult to reach,
# so for now, always report success.

. $srcdir/util.sh
. $srcdir/urls.sh

echo "*** Testing libnc-dap."

headertests=1
datatests=1

while getopts "vfhdr" opt
do
  case "$opt" in
    v) verbose=1;;
    f) tryfailed=1;;
    h) headertests=1; datatests=;;
    d) headertests=; datatests=1;;
    r) reportonly=1;;
   esac
done

mkdir -p ./output

if test -n "$headertests" ; then
reset
. $srcdir/header.1.sh
ret=0; if ! report "Head"; then ret=1; fi
if test "$ret" -eq 1 -a -z "$reportonly" ; then exit $FAILRETURN; fi
fi

if test -n "$datatests" ; then
reset
. $srcdir/data.1.sh
ret=0; if ! report "Data"; then ret=1; fi
if test "$ret" -eq 1 -a -z "$reportonly" ; then exit $FAILRETURN; fi
fi
exit $OKRETURN
