#!/bin/sh

if test "x$SETX" != x; then set -x; fi

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

extfor() {
    case "$1" in
    nc4) zext="nz4" ;;
    nz4) zext="nz4" ;;
    nzf) zext="nzf" ;;
    s3) zext="s3" ;;
    *) echo "unknown kind: $1" ; exit 1;;
    esac
}

deletemap() {
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
	if "./zmapio $fileurl" ; then mapexists=1; else mapexists=0; fi
        ;;
    *) echo unknown format: $1 : abort ; exit 1 ;;
    esac
    if test $mapexists = 1 ; then
      echo "delete did not delete $1"
    fi
}

fileargs() {
  if test "x$zext" = xs3 ; then
    if test "x$NCS3PATH" = x ; then
	S3PATH="https://stratus.ucar.edu/unidata-netcdf-zarr-testing"
    else
	S3PATH="${NCS3PATH}"
    fi
    fileurl="${S3PATH}/test$tag#mode=nczarr,$zext"
    file=$fileurl
    S3HOST=`${execdir}/zs3parse -h $S3PATH`
    S3BUCKET=`${execdir}/zs3parse -b $S3PATH`
    S3PREFIX=`${execdir}/zs3parse -p $S3PATH`
  else
    file="test$tag.$zext"
    fileurl="file://test$tag.$zext#mode=nczarr,$zext"
  fi
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

ZMD="${execdir}/zmapio"
