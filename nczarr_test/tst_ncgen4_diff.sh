#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi 
. ../test_common.sh


TESTSET="\
ref_dimscope \
ref_typescope \
ref_tst_string_data \
ref_tst_comp \
ref_tst_comp2 \
ref_tst_comp3 \
ref_tst_group_data \
ref_tst_opaque_data \
ref_tst_solar_1 \
ref_tst_solar_2 \
ref_tst_enum_data \
ref_tst_special_atts \
ref_tst_nans \
ref_solar \
unlimtest2 \
ref_niltest \
ref_tst_h_scalar \
ref_tst_nul4 \
"       

# Functions

extfor() {
    case "$1" in
    nc4) zext="nz4" ;;
    nz4) zext="nz4" ;;
    nzf) zext="nzf" ;;
    s3) zext="s3" ;;
    *) echo "unknown kind: $1" ; exit 1;;
    esac
}

deletefile() {
    case "$1" in
    nc4) rm -fr $2;;
    nz4) rm -fr $2;;
    nzf) rm -fr $2;;
    esac
}

mapexists() {
    mapexists=1
    case "$1" in
    nz4) if test -f $file; then mapexists=0; fi ;;
    nzf) if test -f $file; then mapexists=0; fi ;;		 
    s3)
	if "./zmapdump $fileurl" ; then mapexists=1; else mapexists=0; fi
        ;;
    *) echo unknown format: $1 : abort ; exit 1 ;;
    esac
    if test $mapexists = 1 ; then
      echo "delete did not delete $1"
    fi
}

fileargs() {
  if test "x$zext" = xs3 ; then
      fileurl="https://stratus.ucar.edu/unidata-netcdf-zarr-testing/test$tag#mode=nczarr,$zext"
      file=$fileurl
  else
      file="test$tag.$zext"
      fileurl="file://test$tag.$zext#mode=$zext"
  fi
}

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
   if test $verbose = 1 ; then echo "*** Testing: ${x}" ; fi
   # determine if we need the specflag set
   specflag=
   headflag=
   for s in ${SPECIALTESTS} ; do
      if test "x${s}" = "x${x}" ; then specflag="-s"; headflag="-h"; fi
   done
   # determine if this is an xfailtest
   checkxfail ${x}
   deletefile ${x}        
   rm -f ${x}.dmp
   fileargs
   ${NCGEN} -b -k${KFLAG} -o ${fileurl} ${cdl}/${x}.cdl
   ${NCDUMP} ${headflag} ${specflag} -n ${x} ${fileurl} > ${x}.dmp
   # compare the expected (silently if XFAIL)
   if test "x$isxfail" = "x1" -a "x$SHOWXFAILS" = "x" ; then
     if diff -b -bw ${expected}/${x}.dmp ${x}.dmp >/dev/null 2>&1; then ok=1; else ok=0; fi
   else
     if diff -b -w ${expected}/${x}.dmp ${x}.dmp ; then ok=1; else ok=0; fi
   fi
   if test "x$ok" = "x1" ; then
     test $verbose = 1 && echo "*** SUCCEED: ${x}"
     passcount=`expr $passcount + 1`
   elif test "x${isxfail}" = "x1" ; then
     echo "*** XFAIL : ${x}"
     xfailcount=`expr $xfailcount + 1`
   else
     echo "*** FAIL: ${x}"
     failcount=`expr $failcount + 1`
   fi
done

echo "*** Testing ncgen with -k${KFLAG} and zmap=${zext}"

main() {
extfor $1
mkdir ${RESULTSDIR}.${zext}
cd ${RESULTSDIR}.${zext}
diffcycle
cd ..
totalcount=`expr $passcount + $failcount + $xfailcount`
okcount=`expr $passcount + $xfailcount`

echo "*** PASSED: zext=${zext} ${okcount}/${totalcount} ; ${xfailcount} expected failures ; ${failcount} unexpected failures"
}

rm -rf ${RESULTSDIR}
main nz4

if test $failcount -gt 0 ; then exit 1; else  exit 0; fi
