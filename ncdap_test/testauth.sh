#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

# Since this involves a shared resource: the .rc files in current working directory,
# we need to isolate from any other test.

# Make sure execdir and srcdir absolute paths are available
WD=`pwd`
cd $srcdir ; abs_srcdir=`pwd` ; cd $WD
cd $execdir ; abs_execdir=`pwd` ; cd $WD

# Now create a special directory
# And enter it to execute tests
rm -fr rcauthdir
mkdir rcauthdir
cd rcauthdir
WD=`pwd`

#SHOW=1
#DBG=1

if test "x$NCAUTH_HOMETEST" != x ; then
    RCHOME=1
fi

COOKIES="${WD}/.cookies_test"

RC=.daprc

if test "x$FPISMSVC" = x ; then
NETRC=.netrc_test
NETRCIMP=.netrc
else
NETRC=_netrc_test
NETRCIMP=_netrc
fi

LOCALRCFILES="$WD/.dodsrc $WD/.daprc $WD/.ncrc $WD/$NETRC $WD/$NETRCIMP"
HOMERCFILES="$HOME/.dodsrc $HOME/.daprc $HOME/.ncrc $HOME/$NETRC $HOME/$NETRCIMP"
NETRCFILE=$WD/$NETRC
DAPRCFILE=$WD/$RC
if test "x$FP_ISMSVC" != x ; then
    LOCALRCFILES=`${NCPATHCVT} "$LOCALRCFILES"`
    HOMERCFILES=`${NCPATHCVT} "$HOMERCFILES"`
    NETRCFILE=`${NCPATHCVT} "$NETRCFILE"`
    DAPRCFILE=`${NCPATHCVT} "$DAPRCFILE"`
fi

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
    local NETRCFILE
  RCP="$1"
  if test "x$RCP" = x ; then
    echo "createrc: no rc specified"
    exit 1
  fi
  shift
  NETRCPATH="$1"
  echo "Creating rc file $RCP"
  if test "x${DBG}" = x1 ; then
    echo "HTTP.VERBOSE=1" >>$RCP
  fi	
  if test "x$NETRCPATH" = x ; then
    echo "HTTP.CREDENTIALS.USERPASSWORD=${BASICCOMBO}" >>$RCP
  elif test "x$NETRCPATH" != ximplicit ; then
    echo "HTTP.NETRC=${NETRCPATH}" >>$RCP
  elif test "x$NETRCPATH" = ximplicit ; then
    echo "HTTP.NETRC=" >>$RCP
  fi
  echo "HTTP.COOKIEJAR=${COOKIES}" >>$RCP
}

createnetrc() {
    local NETRCPATH
  NETRCPATH="$1"; # netrc file path
  if test "x$NETRCPATH" = x ; then return; fi
  echo "Creating netrc file $NETRCPATH"
#  echo -n "${PROTO}://$URLSERVER/$URLPATH" >>$NETRCPATH
  echo -n "machine $URLSERVER" >>$NETRCPATH
  echo  -n "  login $BASICUSER password $BASICPWD" >>$NETRCPATH
  echo "" >>$NETRCPATH
  chmod go-rwx $NETRCPATH
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
rclocal1() {
  echo "***Testing rc file in local directory"
  reset
  # Create the rc file in ./
  createrc $LOCALRC
  # Invoke ncdump to extract a file using the URL
  ${NCDUMP} -h "$URL" > testauthoutput
}

# Case: local daprc local netrc no embed
rclocal2() {
  echo "***Testing rc file + .netrc in local directory"
  reset
  # Create the rc file and (optional) netrc file in ./
  createnetrc $LOCALNETRC
  createrc $LOCALRC $LOCALNETRC
  # Invoke ncdump to extract a file using the URL
  ${NCDUMP} -h "$URL" > testauthoutput
}

# Case: home rc no netrc no embed
rchome1() {
  echo "***Testing home rc file no netrc in home directory"
  reset
  # Create the rc file file in ./
  createrc $HOMERC
  # Invoke ncdump to extract a file the URL
  ${NCDUMP} -h "$URL" > testauthoutput
}

# Case: home daprc implicit home netrc no embed
rchome2() {
  echo "***Testing .netrc file in home directory"
  reset
  createnetrc $HOME/$NETRCIMP
  createrc $HOMERC implicit
  # Invoke ncdump to extract a file using the URL
  ${NCDUMP} -h "$URL" > testauthoutput
}

# Case: local rc explicit netrc no embed
rchome3() {
  echo "***Testing local rc file and .netrc explicit in home directory"
  reset
  # Create the rc file and (optional) netrc file in ./
  createnetrc $HOME/$NETRC
  createrc $LOCALRC $HOME/$NETRC
  # Invoke ncdump to extract a file the URL
  ${NCDUMP} -h "$URL" > testauthoutput
}

# Case: local rc implicit netrc no embed
rchome4() {
  echo "***Testing local rc file and .netrc implicit in home directory"
  reset
  # Create the rc file and (optional) netrc file in ./
  createnetrc $HOME/$NETRCIMP
  createrc $LOCALRC implicit
  # Invoke ncdump to extract a file the URL
  ${NCDUMP} -h "$URL" > testauthoutput
}

reset() {
  if test "x$RCHOME" = x1 ; then
      rm -f $HOMERCFILES
  fi
  rm -f $LOCALRCFILES
  unset DAPRCFILE
  rm -f ./testauthoutput
}

rcembed

# Next set tests assume these defaults
URL="${PROTO}://${URLSERVER}/$URLPATH"

rclocal1
rclocal2

# Do not do this unless you know what you are doing
if test "x$RCHOME" = x1 ; then
    rchome1
    rchome2
    rchome3
    rchome4
fi

reset
exit

