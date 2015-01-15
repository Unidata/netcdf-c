#!/bin/sh
#set -x
set -e
#S="-s"

# Compressions to test
TC="nozip zip szip bzip2 fpzip"

# if this is part of a distcheck action, then this script
# will be executed in a different directory
# than the ontaining it; so capture the path to this script
# as the location of the source directory.
srcdir=`dirname $0`
cd $srcdir
srcdir=`pwd`
echo "srcdir=${srcdir}"
# Also compute the build directory
builddir=`pwd`
#builddir=${srcdir}/..
echo "builddir=${builddir}"

# Known compressions
C="nozip zip bzip2 szip fpzip zfp"

if test -f "${builddir}/tst_compress.exe" ; then
EXE="${builddir}/tst_compress.exe"
else
EXE="${builddir}/tst_compress"
fi

function clean {
for c in $C ; do
rm -f $c.nc $c.cdl
done
}

function compare {
  ../ncdump/ncdump $S -n compress $1.nc > $1.cdl
  # diff against baseline.cdl
  if diff -wBb ${srcdir}/baseline.cdl $1.cdl ; then
    CODE=1
  else
    CODE=0
  fi
  if test "x$CODE" = "x1" ; then
    echo "   PASS: $1"
  else
    echo "   FAIL: $1"
  fi
  if test CODE = 0 ; then PASSFAIL=0; fi
}

function baseline {
  if ! ${EXE} zip ; then
    echo "***FAIL: tst_compress zip"
  else
    rm -f baseline.cdl
    ../ncdump/ncdump $S -n compress zip.nc > zip.cdl
    mv zip.cdl baseline.cdl
  fi
  clean
}

##################################################

clean

if test "x$1" = xbaseline ; then
  baseline
  exit 0
fi

# Main test
if test -f ${srcdir}/baseline.cdl ; then
    echo "baseline: ${srcdir}/baseline.cdl"
else
    echo "baseline: ${srcdir}/baseline.cdl"
  echo "No baseline file exists"
  exit 1
fi

PASSFAIL=1

# Create {zip,bzip2,szip}.nc
if ! ${EXE} $TC ; then
  echo "***FAIL: tst_compress"
  PASSFAIL=0
fi

# ncdump both files
if test -f nozip.nc ;   then compare nozip; fi
if test -f zip.nc ;   then compare zip; fi
if test -f szip.nc ;  then compare szip ; fi
if test -f bzip2.nc ; then compare bzip2; fi
if test -f fpzip.nc ; then compare fpzip ; fi
if test -f zfp.nc ; then compare zfp ; fi

clean

if test "x$PASSFAIL" = "x1" ; then
    echo "***PASS: run_compress"
    CODE=0
else
  echo "***FAIL: run_compress"
  CODE=1
fi

exit $CODE
