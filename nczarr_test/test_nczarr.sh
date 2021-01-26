#!/bin/sh

if test "x$SETX" != x; then set -x; fi

ZMD="${execdir}/zmapio"

awsdelete() {
aws s3api delete-object --endpoint-url=https://stratus.ucar.edu --bucket=unidata-netcdf-zarr-testing --key="$1"
}

# Check settings
checksetting() {
if test -f ${TOPBUILDDIR}/libnetcdf.settings ; then
    local PATTERN
    PATTERN="${1}:[ 	]*yes"
    if grep "$PATTERN" <${TOPBUILDDIR}/libnetcdf.settings ; then
       HAVE_SETTING=1
    else
       unset HAVE_SETTING
    fi
fi
}

checkprops() {
   specflag=
   headflag=
   isxfail=
   # determine if this is an xfailtest
   for t in ${XFAILTESTS} ; do
     if test "x${t}" = "x${x}" ; then isxfail=1; fi
   done
   for t in ${SPECTESTS} ; do
      if test "x${t}" = "x${f}" ; then specflag="-s"; fi
   done
   for t in ${HEADTESTS} ; do
      if test "x${t}" = "x${f}" ; then headflag="-h"; fi
   done
}

extfor() {
    case "$1" in
    nz4) zext="nz4" ;;
    nzf) zext="nzf" ;;
    s3) zext="s3" ;;
    *) echo "unknown kind: $1" ; exit 1;;
    esac
}

deletemap() {
    case "$1" in
    nz4) rm -fr $2;;
    nzf) rm -fr $2;;
    s3) S3KEY=`${execdir}/zs3parse -k $2`; awsdelete $S3KEY;;
    *) echo "unknown kind: $1" ; exit 1;;
    esac
}

mapexists() {
    mapexists=1
    case "$1" in
    nz4) if test -f $file; then mapexists=0; fi ;;
    nzf) if test -f $file; then mapexists=0; fi ;;		 
    s3)
	if "./zmapio $fileurl" ; then mapexists=1; else mapexists=0; fi
        ;;
    *) echo unknown format: $1 : abort ; exit 1 ;;
    esac
    if test $mapexists = 1 ; then
      echo "delete failed: $1"
    fi
}

fileargs() {
  f="$1"
  case "$zext" in
  s3)
    if test "x$NCS3PATH" = x ; then
	S3PATH="https://stratus.ucar.edu/unidata-netcdf-zarr-testing"
    else
	S3PATH="${NCS3PATH}"
    fi
    fileurl="${S3PATH}/${f}#mode=nczarr,$zext"
    file=$fileurl
    S3HOST=`${execdir}/zs3parse -h $S3PATH`
    S3BUCKET=`${execdir}/zs3parse -b $S3PATH`
    S3PREFIX=`${execdir}/zs3parse -k $S3PATH`
    ;;
  *)
    file="${f}.$zext"
    fileurl="file://${f}.$zext#mode=nczarr,$zext"
    ;;
  esac
}

dumpmap1() {
    local ISJSON
    tmp=
    if test -f $1 ; then
      ISJSON=`${execdir}/zisjson <$1`
      if test "x$ISJSON" = x1 ; then
	  tmp=`tr '\r\n' '  ' <$1`
      else
	  tmp=`${execdir}/zhex <$1`
      fi
      echo "$1 : |$tmp|" >> $2
    else
      echo "$1" >> $2
    fi
}

dumpmap() {
    case "$1" in
    nz4) rm -f $3 ; ${NCDUMP} $2 > $3 ;;
    nzf)
	rm -f $3
	export LC_ALL=C
	lr=`find $2 | sort| tr  '\r\n' '  '`
	for f in $lr ; do  dumpmap1 $f $3 ; done
	;;
    s3)
        aws s3api list-objects --endpoint-url=$S3HOST --bucket=$S3BUCKET
	;;
    *) echo "dumpmap failed" ; exit 1;
    esac
}

difftest() {
echo ""; echo "*** Test zext=$zext"
for t in ${TESTS} ; do
   echo "*** Testing: ${t}"
   # determine if we need the specflag set
   # determine properties
   checkprops ${t}
   ref="ref_${t}"
   rm -fr ${t}.$zext
   rm -f tmp_${t}.dmp
   fileargs $t
   ${NCGEN} -4 -lb -o ${fileurl} ${cdl}/${ref}.cdl
   ${NCDUMP} ${headflag} ${specflag} -n ${ref} ${fileurl} > tmp_${t}.dmp
   # compare the expected (silently if XFAIL)
   if diff -b -w ${expected}/${ref}.dmp tmp_${t}.dmp > ${t}.diff ; then ok=1; else ok=0; fi
   if test "x$ok" = "x1" ; then
     echo "*** SUCCEED: ${t}"
   elif test "x${isxfail}" = "x1" ; then
     echo "*** XFAIL : ${t}"
   else
     echo "*** FAIL: ${t}"
     exit 1
   fi
done
}
