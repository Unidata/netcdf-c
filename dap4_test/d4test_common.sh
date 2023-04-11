# not executable

. ${srcdir}/d4manifest.sh

if test "x$SETX" = x1 ; then set -x ; fi

if test "x$NOOPTIONS" = x ; then
if test $# = 0 ; then
TEST=1
else
for arg in "$@"; do
  case "${arg}" in
  test) TEST=1 ;;
  reset) RESET=1 ;;
  diff) DIFF=1 ;;
  cdl) CDLDIFF=1 ;;
  log) ;;
  *) echo unknown argument $arg ;;
  esac
done
fi
fi

# Define input paths
WD=`pwd`
cd ${top_srcdir}/dap4_test/cdltestfiles; CDLTESTFILES=`pwd` ; cd ${WD}
cd ${top_srcdir}/dap4_test/rawtestfiles; RAWTESTFILES=`pwd` ; cd ${WD}
cd ${top_srcdir}/dap4_test/baseline; BASELINE=`pwd` ; cd ${WD}
cd ${top_srcdir}/dap4_test/baselineraw; BASELINERAW=`pwd` ; cd ${WD}
cd ${top_srcdir}/dap4_test/baselineremote; BASELINEREM=`pwd` ; cd ${WD}
cd ${top_srcdir}/dap4_test/baselinehyrax; BASELINEHY=`pwd` ; cd ${WD}
cd ${top_srcdir}/dap4_test/baselinethredds; BASELINETH=`pwd` ; cd ${WD}

setresultdir() {
rm -fr ${builddir}/$1
mkdir -p ${builddir}/$1
}

FAILURES=
failure() {
  echo "*** Fail: $1"
  FAILURES=1
  if test "x$2" != x ; then
    exit 1
  fi
}

PUSHD() {
pushd $1 >>/dev/null
}
POPD() {
popd >>/dev/null
}

filesexist() {
    for x in "$@" ; do
	if ! test -e $x ; then
	  failure "missing file: $x"
	fi
    done
}

urlbasename() {
    constraint=`echo "$1" | cut -d '?' -f2`
    unconstrained=`echo "$1" | cut -d '?' -f1`
    base=`basename $unconstrained`
    prefix=`dirname $unconstrained`
}

makeurl() {
    urlbasename "$2"
    if test "x$constraint" = "x$unconstrained" ; then
        URL="$1/${prefix}/${base}${FRAG}"
    else
	URL="$1/${prefix}/${base}?$constraint${FRAG}"
    fi
}


finish() {
if test "x$FAILURES" = x1 ; then
echo "*** Fail"
exit 1
else
echo "*** Pass"
exit 0
fi
}

# Return the complete set of arguments minus any found in $SUPPRESS
suppress() {
  local args="$@"
  suppress=
  for a in $args; do
  for s in $SUPPRESS; do if test "x$a" != "x$s" ; then suppress="$suppress $a"; fi;  done
  done        
}

# Compute the set of testable names using the manifest.
EXCLUDEDFILES="test_vlen9 test_vlen10"

computetestablefiles() {
  local F0 firs excluded
  # remove untestable files  
  F=
  first=0  
  for f in ${dap4_manifest} ; do
    excluded=0
    for x in $EXCLUDEDFILES ; do
      if test "x$x" = "x$f" ; then excluded=1; break; fi
    done
    if test "x$excluded" = x0 ; then
      if test "x$first" = x1 ; then F="${f}.nc"; first=0; else F="$F ${f}.nc"; fi
    fi
   done
}

# Split a d4manifest.sh constraint line
# Result is to set 4 variables: FILE QUERY INDEX FRAG
splitconstraint() {
  local tpl
  tpl="$1"
  FILE=`echo $tpl | cut -d"?" -f1`
  tpl=`echo $tpl | cut -d"?" -f2`
  QUERY=`echo $tpl | cut -d"=" -f1`
  INDEX=`echo $tpl | cut -d"=" -f2`
  FRAG="#dap4&log&show=fetch"
}

VG="valgrind --leak-check=full --error-exitcode=1 --num-callers=100"
if test "x$USEVG" = x ; then VG=; fi

DUMPFLAGS="-XF"


