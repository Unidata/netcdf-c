#!/bin/sh

if test "x$srcdir" = "x"; then srcdir=`dirname $0`; fi
export srcdir;

. ../test_common.sh

. ${srcdir}/d4test_common.sh


set -e
echo "test_thredds.sh:"

FRAG="#checksummode=ignore"

F="\
harvey/goes16/CONUS/Channel01/20170821/GOES16_CONUS_20170821_020218_0.47_1km_33.3N_91.4W.nc4?dap4.ce=y \
"

FBAD="harvey/model/gfs_ana/GFS_Global_0p5deg_ana_20170820_0000.grib2 \
      harvey/nexrad/KCRP/20170820/Level2_KCRP_20170820_020558.ar2v"

failure() {
      echo "*** Fail: $1"
      exit 1
}

setresultdir results_test_thredds

if test "x${RESET}" = x1 ; then rm -fr ${BASELINEH}/*.thredds ; fi
for f in $F ; do
    makeurl "dap4://thredds-test.unidata.ucar.edu/thredds/dap4/casestudies" "$f"
    echo "testing: $URL"
    if ! ${NCDUMP} "${URL}" > ./results_test_thredds/${base}.thredds; then
        failure "${URL}"
    fi
    if test "x${TEST}" = x1 ; then
	if ! diff -wBb ${BASELINETH}/${base}.thredds ./results_test_thredds/${base}.thredds ; then
	    failure "diff ${base}.thredds"
	fi
    elif test "x${RESET}" = x1 ; then
	echo "${f}:" 
	cp ./results_test_thredds/${base}.thredds ${BASELINETH}/${base}.thredds
    fi
done

echo "*** Pass"
exit 0

