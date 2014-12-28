#!/bin/sh
#set -x

#NOEMBED=1
#NOLOCAL=1
#NOHOME=1

SHOW=1
#DBG=1
#GDB=1

CMP=1

# if this is part of a distcheck action, then this script
# will be executed in a different directory
# than the one containing it; so capture the path to this script
# as the location of the source directory.

# capture the build directory
# Do a hack to remove e.g. c: for CYGWIN
builddir=`pwd`/..
if test "x$TOPSRCDIR" = x ; then
srcdir="$TOPSRCDIR/ncdap_test"
else
srcdir=`dirname $0`
fi
# canonical
cd $srcdir
srcdir=`pwd`

# Hack for CYGWIN
if [ `uname | cut -d "_" -f 1` = "MINGW32" ]; then
    srcdir=`echo $srcdir | sed 's/\/c\//c:\//g'`
    builddir=`echo $builddir | sed 's/\/c\//c:\//g'`
fi
cd ${builddir}/ncdap_test

if test "x$URS" != x ; then CMP= ; SHOW=1; fi

if test "x$CMP" = x1 ; then SHOW= ; fi

EXPECT=$srcdir/expected3/testData.nc.dmp

NETRCFILE=$builddir/netrc
# This is the control variable
NETRC=$NETRCFILE

OCLOGFILE=stderr
if test "x$DBG" = x1 ; then
SHOW=1
fi

# Major parameters

BASICCOMBO="tiggeUser:tigge"
URLSERVER="remotetest.unidata.ucar.edu"
URLPATH="thredds/dodsC/restrict/testData.nc"

COOKIES="$builddir/cookies"

# See if we need to override
if test "x$URS" != "x" ; then
#https://54.86.135.31/opendap/data/nc/fnoc1.nc.dds
URLSERVER="54.86.135.31"
URLPATH="opendap/data/nc/fnoc1.nc"
BASICCOMBO="$URS"
NOEMBED=1
NETRC=$NETRCFILE
else
NETRC=
fi

# Split the combo
BASICUSER=`echo $BASICCOMBO | cut -d: -f1`
BASICPWD=`echo $BASICCOMBO | cut -d: -f2`

NCDUMP=$builddir/ncdump/ncdump

RC=.ocrc

LOCALRC=./$RC
HOMERC=${HOME}/$RC
HOMERC=`echo "$HOMERC" | sed -e "s|//|/|g"`

function createrc {
if test "x$1" != x ; then
RCP=$1
rm -f $RCP
echo "Creating rc file $RCP"
if test "x${DBG}" != x ; then
echo "HTTP.VERBOSE=1" >>$RCP
fi	
echo "HTTP.COOKIEJAR=${COOKIES}" >>$RCP
if test "x${URS}" = x ; then
echo "HTTP.CREDENTIALS.USERPASSWORD=${BASICCOMBO}" >>$RCP
fi
if test "x${NETRC}" != x ; then
echo "HTTP.NETRC=${NETRC}" >>$RCP
fi
fi
}

function createnetrc {
if test "x$1" != x ; then
rm -f $1
echo "Creating netrc file $1"
echo "machine uat.urs.earthdata.nasa.gov login $BASICUSER password $BASICPWD" >>$1
#echo "machine 54.86.135.31 login $BASICUSER password $BASICPWD" >>$1
fi
}

function reset {
rm -f ./tmp ./.ocrc ./.dodsrc $HOME/.ocrc $HOME/.dodsrc $COOKIES $NETRC
}

function compare {
  if test "x$CMP" = x1 ; then
    if diff -w ./tmp $EXPECT ; then
      echo "***Pass"
    else
      echo "***FAIL"
      pass=1
    fi
  fi
}

pass=0

if test "x$GDB" = x1 ; then
NCDUMP="gdb --args $NCDUMP"
fi

# Initialize
reset

if test "x$NOEMBED" != x1 ; then
echo "***Testing rc file with embedded user:pwd"
URL="https://${BASICCOMBO}@${URLSERVER}/$URLPATH"
# Invoke ncdump to extract the URL
echo "command: ${NCDUMP} -h $URL"
${NCDUMP} -h "$URL" >./tmp
if test "x$CMP" = x1 ; then compare ; fi
if test "x${SHOW}" = x1 ; then cat ./tmp ; fi
fi

URL="https://${URLSERVER}/$URLPATH"
if test "x$NOLOCAL" != x1 ; then
echo "***Testing rc file in local directory"
# Create the rc file and (optional) netrc file in ./
reset
createnetrc $NETRC
createrc $LOCALRC

# Invoke ncdump to extract a file the URL
echo "command: ${NCDUMP} -h $URL"
${NCDUMP} -h "$URL" >./tmp
if test "x$CMP" = x1 ; then compare ; fi
if test "x${SHOW}" = x1 ; then cat ./tmp ; fi
fi

if test "x$NOHOME" != x1 ; then
echo "***Testing rc file in home directory"
# Create the rc file and (optional) netrc fil in ./
reset
createnetrc $NETRC
createrc $HOMERC

# Invoke ncdump to extract a file the URL
echo "command: ${NCDUMP} -h $URL"
${NCDUMP} -h "$URL" >./tmp
if test "x$CMP" = x1 ; then compare; fi
if test "x${SHOW}" = x1 ; then cat ./tmp ; fi
fi

#cleanup
reset

exit $pass
