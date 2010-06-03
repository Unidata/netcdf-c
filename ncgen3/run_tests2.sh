#!/bin/sh
# This shell script runs the ncdump tests.
# $Id: run_tests2.sh,v 1.3 2009/09/24 18:19:11 dmh Exp $

echo "*** creating test0.nc from test0.cdl..."
if !(../ncdump/ncdump -n c1 $(srcdir)/ctest0.nc > ctest1.cdl); then
    echo "ncgen3 test failed! Sorry!" && exit 1
fi

echo "*** Tests successful!"
exit 0
