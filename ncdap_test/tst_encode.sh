#!/bin/sh
# Dennis Heimbigner

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

set -e
echo ""
echo "*** Testing #encode=" mechanism

#${NCDUMP} -h 'http://opendap2.oceanbrowser.net/thredds/dodsC/data/emodnet1-domains/tmp%20test.nc?lon[0:8]#encode=none'
#  raw: http://iridl.ldeo.columbia.edu/SOURCES/.Indices/.soi/.c8110/.anomaly/T/(Jan 1979)/VALUE/dods
${NCDUMP} -h 'http://iridl.ldeo.columbia.edu/SOURCES/.Indices/.soi/.c8110/.anomaly/T/%28Jan%201979%29/VALUE/dods?anomaly[0]'
