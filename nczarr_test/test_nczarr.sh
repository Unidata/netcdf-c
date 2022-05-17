#!/bin/sh

if test "x$SETX" != x; then set -x; fi

# Figure out which cloud repo to use
if test "x$NCZARR_S3_TEST_HOST" = x ; then
#    export NCZARR_S3_TEST_HOST=stratus.ucar.edu
    export NCZARR_S3_TEST_HOST=s3.us-east-1.amazonaws.com
fi
if test "x$NCZARR_S3_TEST_BUCKET" = x ; then
#    export NCZARR_S3_TEST_BUCKET=unidata-netcdf-zarr-testing
    export NCZARR_S3_TEST_BUCKET=unidata-zarr-test-data
fi
export NCZARR_S3_TEST_URL="https://${NCZARR_S3_TEST_HOST}/${NCZARR_S3_TEST_BUCKET}"

ZMD="${execdir}/zmapio"

awsdelete() {
${execdir}/s3util ${PROFILE} -u "${NCZARR_S3_TEST_URL}" -k "$1" clear
# aws s3api delete-object --endpoint-url=https://${NCZARR_S3_TEST_HOST} --bucket=${NCZARR_S3_TEST_BUCKET} --key="netcdf-c/$1"
X=
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
    s3) S3KEY=`${execdir}/zs3parse -k $2`; awsdelete $S3KEY;;
    *) echo "unknown kind: $1" ; exit 1;;
    esac
}

mapstillexists() {
    mapstillexists=0
    if "./zmapio $fileurl" ; then
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
    S3PATH="${NCZARR_S3_TEST_URL}/netcdf-c"
    fileurl="${S3PATH}/${f}#${frag}"
    file=$fileurl
    S3HOST=`${execdir}/zs3parse -h $S3PATH`
    S3BUCKET=`${execdir}/zs3parse -b $S3PATH`
    S3PREFIX=`${execdir}/zs3parse -k $S3PATH`
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
    ${execdir}/zmapio -t int -x objdump $fileurl > $3
}

# Function to remove selected -s attributes from file;
# These attributes might be platform dependent
sclean() {
    cat $1 \
 	| sed -e '/:_IsNetcdf4/d' \
	| sed -e '/:_Endianness/d' \
	| sed -e '/_NCProperties/d' \
	| sed -e '/_SuperblockVersion/d' \
	| cat > $2
}

# Make sure execdir and srcdir absolute paths are available
WD=`pwd`
cd $srcdir ; abs_srcdir=`pwd` ; cd $WD
cd $execdir ; abs_execdir=`pwd` ; cd $WD

# Clear out any existing .rc files
WD=`pwd`
if test "x$NCAUTH_HOMETEST" != x ; then RCHOME=1; fi

# Set plugin path

#cd ../plugins; make clean all >/dev/null; cd ../nczarr_test

if test "x$FP_USEPLUGINS" = xyes; then
# Load the findplugins function
. ${builddir}/findplugin.sh
echo "findplugin.sh loaded"

# Locate the plugin path and the library names; argument order is critical
# Find misc in order to determine HDF5_PLUGIN+PATH.
# Assume all test filters are in same plugin dir
findplugin h5misc

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

resetrc
