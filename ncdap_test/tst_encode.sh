#!/bin/sh
# Dennis Heimbigner

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

set -e
echo ""
echo "*** Testing #encode=" mechanism

#  raw: http://iridl.ldeo.columbia.edu/SOURCES/.Indices/.soi/.c8110/.anomaly/T/(Jan 1979)/VALUE/dods
${NCDUMP} -h 'http://iridl.ldeo.columbia.edu/SOURCES/.Indices/.soi/.c8110/.anomaly/T/%28Jan%201979%29/VALUE/dods?anomaly[0]'
