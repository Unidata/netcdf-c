#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

#SHOW=1
#DBG=1
# Choose at most 1
#GDB=1
#VG=1

WD=`pwd`
COOKIES="${WD}/test_auth_cookies"

RC=.daprc

OCLOGFILE=stderr
if test "x$DBG" = x1 ; then
SHOW=1
fi

# Major parameters

PROTO=https
URLSERVER="localhost:8081"
URLPATH="/thredds/dodsC/containerauth/testData2.nc"

if test "x$DBG" = x1 ; then
PARAMS="#log&show=fetch"
fi

OUTPUT="./.output"
if test "x$TEMP" = x ; then
  TEMP="/tmp"
fi
TEMP=`echo "$TEMP" | sed -e "s|/$||"`

show() {
  if test "x$SHOW" = x1 ; then cat $OUTPUT; fi
  if test "x$OUTPUT" != "x"; then rm -f $OUTPUT; fi
}

# Assemble the ncdump command
if test "x$DBG" = x1; then
NCDUMP="$NCDUMP -D1"
fi

if test "x$GDB" = x1 ; then
  NCDUMP="gdb --args $NCDUMP"
fi
if test "x$VG" = x1 ; then
  NCDUMP="valgrind --leak-check=full $NCDUMP"
fi

# Rest of tests assume these defaults
URL="${PROTO}://${URLSERVER}/$URLPATH"

if test "x$DBG" = x1 ; then
URL="$URL$PARAMS"
fi

# Invoke ncdump to extract a file the URL
echo "command: ${NCDUMP} -h ${URL} > $OUTPUT"
${NCDUMP} -h "$URL" > $OUTPUT

show

exit

