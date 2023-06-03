#!/bin/sh

if test "x$srcdir" = "x"; then srcdir=`dirname $0`; fi
export srcdir;

. ../test_common.sh

set -e

. ${srcdir}/d4test_common.sh

set -e

# This specific set of tests verifies the legal URLs for accessing
# a DAP4 server.

echo "test_dap4url.sh:"

SVCD4TS="remotetest.unidata.ucar.edu"
SVCTHREDDS="${SVCD4TS}"
#SVCD4TS="192.168.56.1:8083"
#SVCTHREDDS="192.168.56.1:8081"

FD4TS="\
test_one_var.nc \
"

FTH="\
"

D4TSTEMPLATES="\
http://%svc/d4ts/testfiles/%file#dap4 \
dap4://%svc/d4ts/testfiles/%file \
"

THTEMPLATES="\
http://%svc/thredds/dap4/testAll/%file \
"
failure() {
      echo "*** Fail: $1"
      exit 1
}

builddap4url() {
  svc="$1"
  file="$2"
  template="$3"
  URL=`echon $3 | sed -e "s|%file|$file|g" -e "s|%svc|$svc|g"`
}

setresultdir results_test_dap4url

testd4ts() {
for f in $FD4TS ; do
  for t in $D4TSTEMPLATES ; do
    builddap4url "$SVCD4TS" "$f" "$t"
    echo "testing: $URL"
    if ! ${NCDUMP} ${DUMPFLAGS} -h "${URL}" > ./results_test_dap4url/${f}.d4ts; then
      failure "${URL}"
    fi
  done
done
}

testthredds() {
for f in $FTH ; do
  for t in $THTEMPLATES ; do
    builddap4url "$SVCTHREDDS" "$f" "$t"
    echo "testing: $URL"
    if ! ${NCDUMP} ${DUMPFLAGS} -h "${URL}" > ./results_test_dap4url/${f}.thredds; then
      failure "${URL}"
    fi
  done
done
}

testd4ts
testthredds
echo "*** Pass"
exit 0
