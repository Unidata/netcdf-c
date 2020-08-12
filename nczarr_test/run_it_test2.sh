#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi 
. ../test_common.sh

. "$srcdir/test_nczarr.sh"

set -e

# Functions

diffcycle() {
echo ""; echo "*** Test cycle zext=$1"
for x in ${TESTSET} ; do
   if test $verbose = 1 ; then echo "*** Testing: ${x}" ; fi
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
   ${NCGEN} -b -${KFLAG} -o "${fileurl}" ${cdl}/${x}.cdl
   ${NCDUMP} ${headflag} ${specflag} -n ${x} ${fileurl} > ${x}.dmp
   # compare the expected (silently if XFAIL)
   if test "x$isxfail" = "x1" -a "x$SHOWXFAILS" = "x" ; then
     if diff -b -bw ${expected}/${x}.dmp ${x}.dmp >/dev/null 2>&1; then ok=1; else ok=0; fi
   else
     if diff -b -w ${expected}/${x}.dmp ${x}.dmp ; then ok=1; else ok=0; fi
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
if test "x$2" != x ; then CLOUD="-c $2"; fi
${execdir}/tst_chunks -e $1 $CLOUD
${execdir}/tst_chunks2 -e $1 $CLOUD
}

main() {
ittest nzf
if test "xFEATURE_HDF5" = xyes ; then
ittest nz4
fi
if test "x$FEATURE_S3TESTS" = xyes ; then
 ittest s3 'https://stratus.ucar.edu/unidata-netcdf-zarr-testing'
fi
}

main

