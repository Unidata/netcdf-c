#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

#SHOW=1
#DBG=1

if test -d "/home/dmh" ; then
    RCHOME=1
fi

WD=`pwd`

COOKIES="${WD}/.cookies_test"

RC=.daprc
NETRC=.netrc_test

NETRCFILE=$WD/$NETRC
DAPRCFILE=$WD/$RC

HOMENETRCFILE=$HOME/$NETRC
HOMEDAPRCFILE=$HOME/$RC

if test "x$DBG" = x1 ; then
SHOW=1
fi

# Major parameters

AUTHSERVER="thredds.ucar.edu"
BASICCOMBO="authtester:auth"
URLPATH="thredds/dodsC/test3/testData.nc"
PROTO=https
URLSERVER=${AUTHSERVER}

if test "x$DBG" = x1 ; then
URLPATH="${URLPATH}#log&show=fetch"
fi

# Split the combo
BASICUSER=`echo $BASICCOMBO | cut -d: -f1`
BASICPWD=`echo $BASICCOMBO | cut -d: -f2`

if test "x$TEMP" = x ; then
  TEMP="/tmp"
fi
TEMP=`echo "$TEMP" | sed -e "s|/$||"`

LOCALRC=${WD}/$RC
LOCALNETRC=${WD}/$NETRC
HOMERC=${HOME}/$RC
HOMERC=`echo "$HOMERC" | sed -e "s|//|/|g"`
HOMENETRC=${HOME}/$NETRC
HOMENETRC=`echo "$HOMENETRC" | sed -e "s|//|/|g"`

createrc() {
    local RCP
    local NETRC
  RCP="$1"
  if test "x$RCP" = x ; then
    echo "createrc: no rc specified"
    exit 1
  fi
  shift
  NETRC="$1"
  echo "Creating rc file $RCP"
  if test "x${DBG}" = x1 ; then
    echo "HTTP.VERBOSE=1" >>$RCP
  fi	
  if test "x$NETRC" = x ; then
    echo "HTTP.CREDENTIALS.USERPASSWORD=${BASICCOMBO}" >>$RCP
  elif test "x$NETRC" != ximplicit ; then
    echo "HTTP.NETRC=${NETRC}" >>$RCP
  fi
  echo "HTTP.COOKIEJAR=${COOKIES}" >>$RCP
}

createnetrc() {
    local NETRC
  NETRC="$1"; # netrc file path
  if test "x$NETRC" = x ; then return; fi
  echo "Creating netrc file $NETRC"
  echo -n "${PROTO}://$URLSERVER/$URLPATH" >>$NETRC
  echo  -n "  login $BASICUSER password $BASICPWD" >>$NETRC
  echo "" >>$NETRC
  chmod go-rwx $NETRC
}

# Test cases

# Case: !daprc !netrc embedded usr:pwd
rcembed() {
  echo "***Testing with embedded user:pwd"
  reset
  URL="${PROTO}://${BASICCOMBO}@${URLSERVER}/$URLPATH"
  # Invoke ncdump to extract a file the URL
  ${NCDUMP} -h "$URL" > testauthoutput
}

# Case: local daprc no netrc no embed
rclocal() {
  echo "***Testing rc file in local directory"
  reset
  # Create the rc file and (optional) netrc fil in ./
  createrc $LOCALRC $LOCALNETRC

  # Invoke ncdump to extract a file using the URL
  ${NCDUMP} -h "$URL" > testauthoutput
}

# Case: local daprc local netrc no embed
rclocalnetrc() {
  echo "***Testing rc file in local directory"
  reset
  # Create the rc file and (optional) netrc fil in ./
  createnetrc $LOCALNETRC
  createrc $LOCALRC $LOCALNETRC

  # Invoke ncdump to extract a file using the URL
  ${NCDUMP} -h "$URL" > testauthoutput
}

rchome() {
  echo "***Testing rc file and netrc in home directory"
  reset
  # Create the rc file and (optional) netrc file in ./
  createnetrc $HOMENETRC
  createrc $HOMERC $HOMENETRC

  # Invoke ncdump to extract a file the URL
  ${NCDUMP} -h "$URL" > testauthoutput
}

rchome2() {
  echo "***Testing local rc file and .netrc implicit in home directory"
  reset
  # Create the rc file and (optional) netrc file in ./
  createnetrc $HOME/.netrc
  createrc $HOMERC implicit

  # Invoke ncdump to extract a file the URL
  ${NCDUMP} -h "$URL" > testauthoutput
}

reset() {
if test "x$RCHOME" = x1 ; then rm -f $HOMENETRCFILE $HOMEDAPRCFILE; fi
rm -f $NETRCFILE $DAPRCFILE $LOCALRC $LOCALNETRC
unset DAPRCFILE
rm -f testauthoutput
}

rcembed

# Next set tests assume these defaults
URL="${PROTO}://${URLSERVER}/$URLPATH"

rclocal
rclocalnetrc

# Do not do this unless you know what you are doing
if test "x$RCHOME" = x1 ; then
    rchome
    rchome2
fi

reset

exit

