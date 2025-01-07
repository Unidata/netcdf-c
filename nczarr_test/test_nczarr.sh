#!/bin/sh

# This file must kept in sync with
#	nczarr_test/test_nczarr.sh
# and
#	v3_nczarr_test/test_nczarr.sh

# Load only once
if test "x$TEST_NCZARR_SH" = x ; then
export TEST_NCZARR_SH=1

if test "x$SETX" != x; then set -x; fi

# Get the directory in which we are running
TMP=`pwd`
NCZARRDIR=`basename $TMP`
if test "x$NCZARRDIR" = xnczarr_test ; then
    export NCZARRFORMAT=2; export NCNOZMETADATA=1
elif test "x$NCZARRDIR" = x3_nczarr_test ; then
    export NCZARRFORMAT=3; export NCNOZMETADATA=1
else
    unset NCZARRFORMAT; unset NCNOZMETADATA
fi
echo "@@@ NCZARRFORMAT=$NCZARRFORMAT NCNOZMETADATA=$NCNOZMETADATA"

# Figure out which cloud repo to use
if test "x$NCZARR_S3_TEST_HOST" = x ; then
#    export NCZARR_S3_TEST_HOST=stratus.ucar.edu
    export NCZARR_S3_TEST_HOST="${S3ENDPOINT:-s3.us-east-1.amazonaws.com}"
fi
if test "x$NCZARR_S3_TEST_BUCKET" = x ; then
    export NCZARR_S3_TEST_BUCKET="${S3TESTBUCKET}"
fi
export NCZARR_S3_TEST_URL="https://${NCZARR_S3_TEST_HOST}/${NCZARR_S3_TEST_BUCKET}"

# TAG for zarr format to use; uses the environment variable NCZARRFORMAT
if test "x${NCZARRFORMAT}" = x3 ; then
    export ZDF="_v3"
else
    export ZDF=""
fi

# Fix execdir
NCZARRDIR="${execdir}/../nczarr_test"

ZMD="${NCZARRDIR}/${DL}zmapio"
S3UTIL="${NCZARRDIR}/${DL}s3util"
ZS3PARSE="${NCZARRDIR}/${DL}zs3parse"
NCDUMPCHUNKS="${NCZARRDIR}/${DL}ncdumpchunks"
ZHEX="${NCZARRDIR}/${DL}zhex"
ZISJSON="${NCZARRDIR}/${DL}zisjson"

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
    file) zext="file" ;;
    zip) zext="zip" ;;
    s3) zext="s3" ;;
    *) echo "unknown kind: $1" ; exit 1;;
    esac
}

deletemap() {
    case "$1" in
    file) rm -fr $2;;
    zip) rm -f $2;;
    s3) S3KEY=`${ZS3PARSE} -k $2`; s3sdkdelete $S3KEY ;;
    *) echo "unknown kind: $1" ; exit 1;;
    esac
}

mapstillexists() {
    mapstillexists=0
    if "${ZMD} $fileurl" &> /dev/null ; then
      echo "delete failed: $1"
      mapstillexists=1
    fi
}

fileargs() {
  f="$1"
  frag="$2"
  if test "x$frag" = x ; then frag="mode=nczarr,$zext" ; fi
  case "$zext" in
  s3)
    S3PATH="${NCZARR_S3_TEST_URL}/${S3ISOPATH}"
    fileurl="${S3PATH}/${f}#${frag}"
    file=$fileurl
    S3HOST=`${ZS3PARSE} -h $S3PATH`
    S3BUCKET=`${ZS3PARSE} -b $S3PATH`
    S3PREFIX=`${ZS3PARSE} -k $S3PATH`
    ;;
  *)
    file="${f}.$zext"
    fileurl="file://${f}.$zext#${frag}"
    ;;
  esac
}

dumpmap() {
    zext=$1
    zbase=`basename $2 ".$zext"`
    fileargs $zbase
    ${ZMD} -t int -x objdump $fileurl > $3
}

# Function to remove selected -s attributes from file;
# These attributes might be platform dependent
sclean() {
sed -i.bak -e '/:_IsNetcdf4/d' $1
sed -i.bak -e '/:_Endianness/d' $1
sed -i.bak -e '/_NCProperties/d' $1
sed -i.bak -e '/_SuperblockVersion/d' $1
}

# s3clean plus remove additional lines
scleanplus() {
sclean $1
sed -i.bak -e '/_Format/d' $1
sed -i.bak -e '/_global attributes:/d' $1 
}

# Function to rewrite selected key values in a zmapio output.
# because these values might be platform dependent
zmapclean() {
sed -i.bak -e 's|^\([^(]*\)([0-9][0-9]*)|\1()|' $1
sed -i.bak -e 's/"_NCProperties":[ ]*"version=\([0-9]\),[^"]*"/"_NCProperties": "version=\1,netcdf=0.0.0,nczarr=0.0.0"/g' $1
sed -i.bak -e 's/"_nczarr_superblock":[ ]*{[^}]*}/"_nczarr_superblock": {"version": "0.0.0", "format": 2}/g' $1
sed -i.bak -e 's/"_nczarr_superblock":[ ]*{[^}]*}/"_nczarr_superblock": {"version": "0.0.0", "format": 2}/g' $1
}

# Make sure execdir and srcdir absolute paths are available
WD=`pwd`
cd $srcdir ; abs_srcdir=`pwd` ; cd $WD
cd $execdir ; abs_execdir=`pwd` ; cd $WD

# Clear out any existing .rc files
WD=`pwd`
if test "x$NCAUTH_HOMETEST" != x ; then RCHOME=1; fi

# Set plugin path

if test "x$FP_USEPLUGINS" = xyes; then
# Load the findplugins function
. ${builddir}/findplugin.sh
echo "findplugin.sh loaded"

# Locate the plugin path and the library names; argument order is critical
# Find misc in order to determine HDF5_PLUGIN+PATH.
# Assume all test filters are in same plugin dir
if ! findplugin h5misc ; then exit 0; fi

echo "final HDF5_PLUGIN_DIR=${HDF5_PLUGIN_DIR}"
export HDF5_PLUGIN_PATH="${HDF5_PLUGIN_DIR}"
fi # USEPLUGINS

resetrc() {
  if test "x$RCHOME" = x1 ; then
      rm -f ${HOME}/.dodsrc ${HOME}/.daprc ${HOME}/.ncrc
  fi
  rm -f ${WD}/.dodsrc ${WD}/.daprc ${WD}/.ncrc
  unset NCRCENV_IGNORE
  unset NCRCENV_RC
  unset DAPRCFILE
}

s3sdkdelete() {
if test -f ${S3UTIL} ; then
  ${S3UTIL} ${PROFILE} -u "${NCZARR_S3_TEST_URL}" -k "$1" clear
elif which aws ; then
  aws s3api delete-object --endpoint-url=https://${NCZARR_S3_TEST_HOST} --bucket=${NCZARR_S3_TEST_BUCKET} --key="/${S3ISOPATH}/$1"
else
  echo "**** Could not delete ${NCZAR_S3_TEST_URL}"
fi
}

s3sdkcleanup() {
if test -f ${S3UTIL} ; then
  ${S3UTIL} ${PROFILE} -u "${NCZARR_S3_TEST_URL}" -k "$1" clear
elif which aws ; then
  aws s3api delete-object --endpoint-url=https://${NCZARR_S3_TEST_HOST} --bucket=${NCZARR_S3_TEST_BUCKET} --key="/${S3ISOPATH}/$1"
else
  echo "**** Could not delete ${NCZAR_S3_TEST_URL}"
fi
}

# Create an isolation path for S3; build on the isolation directory
s3isolate() {
  if test "x${S3ISOPATH}" = x ; then
    if test "x${ISOPATH}" = x ; then isolate "$1"; fi
    # Need isolation path to include the test directory
    BNAME=`basename $srcdir`
    S3ISODIR="${BNAME}_${TESTUID}/${ISODIR}"
    S3ISOPATH="${S3TESTSUBTREE}/${S3ISODIR}"
  fi
}

GDBB="gdb -batch -ex r -ex bt -ex q --args"

resetrc

fi #TEST_NCZARR_SH
