#!/bin/sh

#NOP=1
#NOS=1
#NOB=1

#SHOW=1
#DBG=1
#GDB=1

# if this is part of a distcheck action, then this script
# will be executed in a different directory
# than the one containing it; so capture the path to this script
# as the location of the source directory.

# capture the build directory
# Do a hack to remove e.g. c: for CYGWIN

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

OCLOGFILE=stderr
if test "x$DBG" = x1 ; then
SHOW=1
fi

URL="http://remotetest.unidata.ucar.edu/dts/test.03"

PREFIX="[log][show=fetch]"
SUFFIX="log&show=fetch"
BOTHP="[log][show=fetch]"
BOTHS="noprefetch&fetch=disk"

locreset () {
    rm -f ./tmp_testurl ./errtmp_testurl
}

buildurl () {
  front="$1"
  back="$2"
  url="${front}${URL}"
  if test "x$back" != x ; then
    url="${url}#${back}"
  fi
}

pass=1

if test "x$GDB" = x1 ; then
NCDUMP="gdb --args $NCDUMP"
fi

# Initialize
locreset

if test "x$NOP" != x1 ; then
echo "***Testing url prefix parameters"
buildurl $PREFIX ""
# Invoke ncdump to extract the URL
echo "command: ${NCDUMP} -h $url"
${NCDUMP} -h "$url" >./tmp_testurl 2> ./errtmp_testurl
if test "x${SHOW}" = x1 ; then cat ./tmp_testurl ; fi
fi

locreset
if test "x$NOS" != x1 ; then
echo "***Testing url suffix parameters"
buildurl "" $SUFFIX
# Invoke ncdump to extract the URL
echo "command: ${NCDUMP} -h $url"
${NCDUMP} -h "$url" >./tmp_testurl  2> ./errtmp_testurl
if test "x${SHOW}" = x1 ; then cat ./tmp_testurl ; fi
fi

locreset
if test "x$NOB" != x1 ; then
echo "***Testing url prefix+suffix parameters"
buildurl $BOTHP $BOTHS
# Invoke ncdump to extract the URL
echo "command: ${NCDUMP} -h $url"
${NCDUMP} -h "$url" >./tmp_testurl 2> ./errtmp_testurl
if test "x${SHOW}" = x1 ; then cat ./tmp_testurl ; fi
fi

locreset

if test "x$pass" = x0 ; then
  echo "***FAIL"
  exit 1
fi
echo "***PASS"
exit 0


