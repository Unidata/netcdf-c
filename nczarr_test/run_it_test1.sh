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
   deletemap ${x}        
   rm -f ${x}.dmp
   fileargs
   ${NCGEN} -b -${KFLAG} -N ref_${x} -o "${fileurl}" ${cdl}/ref_${x}.cdl
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

echo "*** Testing ncgen with -${KFLAG} and zmap=${zext}"

main() {
extfor $1
if test 'x$2' != x ; then CLOUD=$2; fi
RESULTSDIR="results.${zext}"
mkdir -p ${RESULTSDIR}
cd ${RESULTSDIR}
diffcycle
cd ..
}

# check settings
checksetting "NCZarr Support"
if test "x$HAVE_SETTING" = x1 ; then HAVENCZARR=1; fi
checksetting "NCZarr S3"
if test "x$HAVE_SETTING" = x1 ; then HAVES3=1; fi

rm -rf ${RESULTSDIR}

main nz4
main nzf

if test "x$NETCDF_S3_TESTS" != x ; then
    if test "x$HAVENCZARR" = x1 -a "x$HAVES3" = x1 ; then
      main s3 'https://stratus.ucar.edu/unidata-netcdf-zarr-testing'
    fi
fi
