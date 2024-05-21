#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi 
. ../test_common.sh

set -e

isolate "results_earthdata_dap4"
THISDIR=`pwd`
cd $ISOPATH

# This shell script tests reading of
# publically accessible DAP4 datasets on earthdata

# Setup
# Assume environment variable USERPWD="username:password"
USR=`echo "$USERPWD" | cut -d':' -f1 | tr -d '\n\r'`
PWD=`echo "$USERPWD" | cut -d':' -f2 | tr -d '\n\r'`
rm -fr ./.netrc ./.ncrc ./urscookies
echo "machine urs.earthdata.nasa.gov login $USR password $PWD" > ./.netrc
echo "HTTP.COOKIEJAR=${ISOPATH}/urscookies" > ./.ncrc
echo "HTTP.NETRC=${ISOPATH}/.netrc" >> ./.ncrc

TESTCASES="opendap1;dap4://opendap.earthdata.nasa.gov/collections/C2532426483-ORNL_CLOUD/granules/Daymet_Daily_V4R1.daymet_v4_daily_hi_tmax_2010.nc"

testcase() {
NM=`echo "$1" | cut -d';' -f1`
URL=`echo "$1" | cut -d';' -f2`
echo "*** Test: $NM = $URL"
rm -f "tmp_$NM.cdl"
${NCDUMP} -h -n $NM $URL > "tmp_${NM}.cdl"
}

for t in $TESTCASES ; do
testcase "$t"
done
