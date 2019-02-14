#!/bin/sh
# This shell script runs the ncdump tests.
# $Id: run_tests2.sh,v 1.2 2005/11/07 20:25:32 ed Exp $

#Ensure srcdir
if test "x$srcdir" = x ; then srcdir=`pwd`; fi

. ../test_common.sh

echo "*** creating test0.nc from test0.cdl..."
if !(${NCDUMP} -n c1 $(srcdir)/ctest0.nc > ctest1.cdl); then
    echo "ncgen test failed! Sorry!" && exit 1
fi

echo "*** Tests successful!"
exit 0
