#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi 
. ../test_common.sh

. "$srcdir/test_nczarr.sh"

set -e

TESTSET="\
dimscope \
tst_group_data \
tst_solar_1 \
tst_nans \
tst_nul4 \
"       

cdl="${TOPSRCDIR}/ncdump/cdl"
expected="${TOPSRCDIR}/ncdump/expected"

KFLAG=4

# Functions

checkxfail() {
   # determine if this is an xfailtest
   isxfail=
   for t in ${ALLXFAIL} ; do
     if test "x${t}" = "x${x}" ; then isxfail=1; fi
   done
}

diffcycle() {
echo ""; echo "*** Test cycle zext=$1"
for x in ${TESTSET} ; do
   if test "x$verbose" = x1 ; then echo "*** Testing: ${x}" ; fi
   # determine if we need the specflag set
   specflag=
   headflag=
   for s in ${SPECIALTESTS} ; do
      if test "x${s}" = "x${x}" ; then specflag="-s"; headflag="-h"; fi
   done
   # determine if this is an xfailtest
   checkxfail ${x}
   deletemap ${execdir}/${x}        
   rm -f ${execdir}/${x}.dmp
   fileargs
   ${NCGEN} -b -${KFLAG} -N ref_${x} -o "${fileurl}" ${cdl}/ref_${x}.cdl
pwd
   ${NCDUMP} ${headflag} ${specflag} -n ref_${x} ${fileurl} > ${x}.dmp
   # compare the expected (silently if XFAIL)
   if test "x$isxfail" = "x1" -a "x$SHOWXFAILS" = "x" ; then
     if diff -b -w ${expected}/ref_${x}.dmp ${x}.dmp >/dev/null 2>&1; then ok=1; else ok=0; fi
   else
     if diff -b -w ${expected}/ref_${x}.dmp ${x}.dmp ; then ok=1; else ok=0; fi
   fi
   if test "x$ok" = "x1" ; then
     echo "*** SUCCEED: ${x}"
   elif test "x${isxfail}" = "x1" ; then
     echo "*** XFAIL : ${x}"
   else
     echo "*** FAIL: ${x}"
     exit 1
   fi
done
}

ittest() {
extfor $1
if test "x$2" != x ; then CLOUD=$2; fi
echo "*** Testing ncgen with -${KFLAG} and zmap=${zext}"
diffcycle $zext
}

main() {
ittest nzf
if test "x$FEATURE_HDF5" = xyes ; then
ittest nz4
fi
if test "x$FEATURE_S3TESTS" = xyes ; then
 ittest s3 'https://stratus.ucar.edu/unidata-netcdf-zarr-testing'
fi
}

main

