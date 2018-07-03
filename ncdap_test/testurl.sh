#!/bin/sh

NOP=1 
NOS=1
NOB=1
MODE=1

#SHOW=1
#DBG=1
#GDB=1

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

set -x

OCLOGFILE=stderr
if test "x$DBG" = x1 ; then
SHOW=1
fi

URL="http://remotetest.unidata.ucar.edu/dts/test.03"

PREFIX="[log][show=fetch]"
SUFFIX="log&show=fetch"
BOTHP="[log][show=fetch]"
BOTHS="noprefetch&fetch=disk"
STRLEN="[maxstrlen=16]"
M5="[mode=5]"
M6="[mode=6]"

locreset () {
    rm -f ./tmp_testurl ./errtmp_testurl
}

# buildurl pre-url post-url
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

if test "x$NOP" = x1 ; then
echo "***Testing url prefix parameters"
buildurl $PREFIX ""
# Invoke ncdump to extract the URL

echo "command: ${NCDUMP} -h $url"

${NCDUMP} -h "$url" >./tmp_testurl 2> ./errtmp_testurl
if test "x${SHOW}" = x1 ; then cat ./tmp ; fi

# Test that maxstrlen works as alias for stringlength
echo "***Testing maxstrlen=stringlength alias"
buildurl $STRLEN ""
# Invoke ncdump to extract the URL
echo "command: ${NCDUMP} -h $url"
${NCDUMP} "$url" >./tmp_testurl 2> ./errtmp_testurl
if test "x${SHOW}" = x1 ; then cat ./tmp_testurl ; fi
# Look for the value of maxStrlen in output cdl
if ! fgrep -i "maxstrlen = 16" ./tmp_testurl ; then
echo "***Fail: maxStrlen not recognized"
fgrep -i "maxstrlen16 = 16" ./tmp_testurl > ./errtmp_testurl
fi

fi

locreset

if test "x$NOS" = x1 ; then
echo "***Testing url suffix parameters"
buildurl "" $SUFFIX
# Invoke ncdump to extract the URL
${NCDUMP} -h "$url" >./tmp_testurl  2> ./errtmp_testurl
if test "x${SHOW}" = x1 ; then cat ./tmp_testurl ; fi
fi

locreset

if test "x$NOB" = x1 ; then
echo "***Testing url prefix+suffix parameters"
buildurl $BOTHP $BOTHS
# Invoke ncdump to extract the URL
${NCDUMP} -h "$url" >./tmp_testurl 2> ./errtmp_testurl
if test "x${SHOW}" = x1 ; then cat ./tmp_testurl ; fi
fi

locreset

if test "x$MODE" = x1 ; then
echo "***Testing mode parameters"
buildurl $M5
# Invoke ncdump to extract the URL and test mode
format=`${NCDUMP} -k "$url"`
if test "x${SHOW}" = x1 ; then echo $format ; fi
# If cdf5 is not enabled, then it will default to classic
if test "x${format}" != xcdf5 && test "x${format}" != xclassic ; then pass=0 ; fi
buildurl $M6
# Invoke ncdump to extract the URL and test mode
format=`${NCDUMP} -k "$url"`
if test "x${SHOW}" = x1 ; then echo "format=$format" ; fi
if test "x${format}" != 'x64-bit offset' ; then pass=0 ; fi
fi

if test "x$pass" = x0 ; then
  echo "***FAIL"
  exit 1
fi
echo "***PASS"
exit 0
