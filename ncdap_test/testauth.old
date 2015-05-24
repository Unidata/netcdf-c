#!/bin/sh

#NOEMBED=1
#NOLOCAL=1
#NOHOME=1
#NOENV=1

#DBG=1
#SHOW=1

# Choose at most 1
#GDB=1
#VG=1

NFL=1

WD=`pwd`

NETRCFILE=$WD/test_auth_netrc
# This is the control variable
NETRC=$NETRCFILE

COOKIES="${WD}/test_auth_cookies"

RC=.daprc

NCLOGFILE=stderr
if test "x$DBG" = x1 ; then
SHOW=1
fi

# Major parameters

BASICCOMBO="tiggeUser:tigge"
URLSERVER="remotetest.unidata.ucar.edu"
URLPATH="thredds/dodsC/restrict/testData.nc"

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

if test "x$DBG" = x1 ; then
URLPATH="${URLPATH}#log&show=fetch"
fi

# Split the combo
BASICUSER=`echo $BASICCOMBO | cut -d: -f1`
BASICPWD=`echo $BASICCOMBO | cut -d: -f2`

NCDUMP=
for o in ./.libs/ncdump.exe ./.libs/ncdump ./ncdump.exe ./ncdump ; do
  if test -f $o ; then
  NCDUMP=$o
  break;
  fi
done
if test "x$NCDUMP" = x ; then
echo "no ncdump"
exit 1
fi

if test "x$SHOW" = x ; then
OUTPUT="> /dev/null"
else
OUTPUT=
fi

if test "x$TEMP" = x ; then
  TEMP="/tmp"
fi
TEMP=`echo "$TEMP" | sed -e "s|/$||"`

LOCALRC=./$RC
HOMERC=${HOME}/$RC
HOMERC=`echo "$HOMERC" | sed -e "s|//|/|g"`
ENVRC="$TEMP/$RC"

cd `pwd`
builddir=`pwd`
# Hack for CYGWIN
cd $srcdir
srcdir=`pwd`
cd ${builddir}

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
if test "x${NETRC}" != x && test "x$NFL" = x ; then
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

# Forcibly remove all and do not restore with save
function reset {
    for f in ./$RC $HOMERC $ENVRC $COOKIES $NETRC ; do
      rm -f ${f}
    done      
}

# Restore from .save files
function restore {
  for f in ./$RC $HOMERC $ENVRC $COOKIES $NETRC ; do
    rm -f ${f}
    if test -f ${f}.save ; then
      echo "restoring old ${f}"
      cp ${f}.save ${f}
    fi      
  done      
}

function save {
  for f in ./$RC $HOMERC $ENVRC $COOKIES $NETRC ; do
    if test -f $f ; then
      if test -f ${f}.save ; then
        ignore=1
      else
        echo "saving $f"
        cp ${f} ${f}.save
      fi
    fi      
  done      
}

export LD_LIBRARY_PATH="../liblib/.libs:/usr/local/lib:/usr/lib64:$LD_LIBRARY_PATH"

if test "x$GDB" = x1 ; then
NCDUMP="gdb --args $NCDUMP"
fi
if test "x$VG" = x1 ; then
NCDUMP="valgrind --leak-check=full $NCDUMP"
fi

# Initialize
save
reset

if test "x$NOEMBED" != x1 ; then
echo "***Testing rc file with embedded user:pwd"
URL="https://${BASICCOMBO}@${URLSERVER}/$URLPATH"
# Invoke ncdump to extract a file from the URL
echo "command: ${NCDUMP} -h $URL ${OUTPUT}"
${NCDUMP} -h "$URL" ${OUTPUT}
fi

URL="https://${URLSERVER}/$URLPATH"
if test "x$NOLOCAL" != x1 ; then
echo "***Testing rc file in local directory"
# Create the rc file and (optional) netrc file in ./
reset
createnetrc $NETRC
createrc $LOCALRC

# Invoke ncdump to extract a file the URL
echo "command: ${NCDUMP} -h $URL ${OUTPUT}"
${NCDUMP} -h "$URL" ${OUTPUT}
fi

if test "x$NOHOME" != x1 ; then
echo "***Testing rc file in home directory"
# Create the rc file and (optional) netrc fil in ./
reset
createnetrc $NETRC
createrc $HOMERC

# Invoke ncdump to extract a file the URL
echo "command: ${NCDUMP} -h $URL ${OUTPUT}"
${NCDUMP} -h "$URL" ${OUTPUT}
fi

if test "x$NOENV" != x1 ; then
echo "*** Testing rc file from env variable"
# Create the rc file and (optional) netrc file
reset
createnetrc $NETRC
export NCRCFILE=$ENVRC
createrc $NCRCFILE

# Invoke ncdump to extract a file the URL
echo "command: ${NCDUMP} -h $URL ${OUTPUT}"
${NCDUMP} -h "$URL" ${OUTPUT} 
fi

set +x
#restore

