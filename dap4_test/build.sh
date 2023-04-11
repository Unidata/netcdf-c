#!/bin/bash

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh
# Override option processing
#NOOPTIONS=1
#. ${srcdir}/d4test_common.sh

set +e

defaults() {
  # Set defaults
  CHECKSUM="false"
  SVC="192.168.56.1:8083"
  #SVC="remotetest.unidata.ucar.edu"
  fileset
  constrainedfileset
}

usage() {
echo "usage: bt.sh \
[-h|--help]\
[-S|--svc <server>]\
[-I|--input \"<input-files>\"]\
[-C|--ce \"<constraint-files>\"]\
[-c|-checksum]\
[--data]\
[actions]\
"
echo "where actions is a sequence of: \
reset\
|build\
|dmr\
|dap\
|cdl\
|install\
|cleanup\
"
exit 1
}

options() {
  while test "$#" -gt  0 ; do
  case "$1" in
    -h|--help) usage ; shift ;;
    -S|--svc) shift; SVC="$1"; shift ;;
    -I|--input) shift; unset CE; F="$1"; shift ;;
    -C|--ce) shift; unset F; CE="$1"; shift ;;
    -c|--checksum) CHECKSUM="true"; shift ;;
    --data) SHOWDATA="true"; shift ;;
    reset) ACTIONS="$ACTIONS reset" ; shift ;;
    build) ACTIONS="$ACTIONS build" ; shift ;;
    dmr) ACTIONS="$ACTIONS dmr"; shift ;;
    dap) ACTIONS="$ACTIONS dap" ; shift ;;
    cdl) ACTIONS="$ACTIONS cdl" ; shift ;;
    install) ACTIONS="$ACTIONS install"; shift ;;
    cleanup) ACTIONS="$ACTIONS cleanup"; shift ;;
    *)   echo "unknown option $1" ; break ;;
  esac
  done
}

setup() {
  if test "x$CHECKSUM" = xtrue ; then
    CSUM="dap4.checksum=true"
  else
    CSUM="dap4.checksum=false"
  fi
  AMPCSYM="&${CSUM}"
  excludedfiles
}

excludedfiles() {
# make sure each name is surrounded with a blank
# so we can do proper substring test
XCDL=" \
test_vlen9 \
test_vlen10 \
"
XDMR="$XCDL"
XDAP="$XCDL"
}

excluded() {
  local X KIND R EX
  X="$1"
  KIND="$2"
  R=0
  if test "x$KIND" = xdmr ; then EX="$XDMR" ; 
  elif test "x$KIND" = xdap ; then EX="$XDAP" ; 
  elif test "x$KIND" = xcdl ; then EX="$XCDL" ; 
  else
      echo "Unknown exclusion kined: $KIND"
      exit 1
  fi
  for ex in $EX ; do
      if test "x$ex" = "x$X" ; then R=1 ; fi
  done
  return $R
}

fileset() {
  local TMP x X
  # Total file set
  TMP=`ls -1 ${srcdir}/cdltestfiles/*.cdl`
  for x in $TMP ; do
    X=`basename $x .cdl`
    F="$F $X"
  done
  # Mac echo does not support -n flag
  F=`echon $F | cut -f1-`
}

constrainedfileset() {
  # Constrained tests
  CE=" \
test_atomic_array?/vu8[1][0:2:2];/vd[1];/vs[1][0];/vo[0][1]=1 \
test_atomic_array?/v16[0:1,3]=2 \
test_atomic_array?/v16[3,0:1]=3 \
test_one_vararray?/t[1]=4 \
test_one_vararray?/t[0:1]=5 \
test_enum_array?/primary_cloud[1:2:4]=6 \
test_opaque_array?/vo2[1][0:1]=7 \
test_struct_array?/s[0:2:3][0:1]=8 \
"
}

splitce() {
    local expr tmp
    expr="$1"
    CE_F=`echon $expr | cut -d'?' -f1`
    CE_INDEX=`echon $expr | cut -d'=' -f2`
    tmp=`echon $expr | cut -d'?' -f2`
    CE_CE=`echon $tmp | cut -d'=' -f1`
}

ceescape() {
    CEX=`echon "$1" | sed -e 's|\[|%5b|g' -e 's|]|%5d|g'`
}

build() {
    # Build the unconstrained .nc files and .ncdump files
    for f in $F ; do
      excluded ${f} cdl
      if test $? = 0 ; then
          ${NCGEN} -4 -o "${srcdir}/downloads/${f}.nc" "${srcdir}/cdltestfiles/${f}.cdl"
          ${NCDUMP} "${srcdir}/downloads/${f}.nc" > ${srcdir}/downloads/${f}.ncdump
      fi
    done
}

capturedmr() {
    local TARGET
    for f in $F ; do
      excluded ${f} dmr
      if test $? = 0 ; then
          TARGET="downloads/${f}.nc.dmr"
          wget -t2 -O ${TARGET} "http://$SVC/d4ts/testfiles/${f}.nc.dmr.xml?$CSUM"
          if test "x$?" != x0 ; then
            echo ">>> wget failed: ${TARGET}"
            rm -f ${TARGET}
          fi
      fi
    done
    for e in $CE ; do
      splitce $e # Split CE
      excluded ${CE_F} dmr
      if test $? = 0 ; then
          TARGET="downloads/${CE_F}.$CE_INDEX.nc.dmr"
	  ceescape ${CE_CE}
          wget -t2 -O ${TARGET} "http://$SVC/d4ts/testfiles/${CE_F}.nc.dmr.xml?$CSUM&dap4.ce=${CEX}"
          if test "x$?" != x0 ; then
            echo ">>> wget failed: ${TARGET}"
            rm -f ${TARGET}
          fi
      fi
     done
}

capturedap() {
    local TARGET
    for f in $F ; do
	excluded ${f} dap
	if test "x$?" = x0 ; then
          TARGET="downloads/${f}.nc.dap"
          wget -t2 -O ${TARGET} "http://$SVC/d4ts/testfiles/${f}.nc.dap?$CSUM"
	  if test "x$?" != x0 ; then
            echo ">>> wget failed: ${TARGET}"
            rm -f ${TARGET} 
          fi
        fi
    done
    for e in $CE ; do
      splitce $e # Split CE
      excluded ${CE_F} dap
      if test $? = 0 ; then
          TARGET="downloads/${CE_F}.$CE_INDEX.nc.dap"
	  ceescape "$CE_CE"
          wget -t2 -O ${TARGET} "http://$SVC/d4ts/testfiles/${CE_F}.nc.dap?$CSUM&dap4.ce=${CEX}"
          if test "x$?" != x0 ; then
            echo ">>> wget failed: ${TARGET}"
            rm -f ${TARGET}
          fi
      fi
     done
}

capturecdl() {
    local TARGET
    for f in $F ; do
	excluded ${f} cdl
	if test "x$?" = x0 ; then
          TARGET="downloads/${f}.nc.cdl"
	  if test "x$SHOWDATA" = xtrue ; then HOPT="-h" ; fi
	  ${NCDUMP} $HOPT "http://$SVC/d4ts/testfiles/${f}.nc?$CSUM#dap4" >${TARGET}
	  if test "x$?" != x0 ; then
            echo ">>> wget failed: ${TARGET}"
            rm -f ${TARGET} 
          fi
        fi
    done
    for e in $CE ; do
      splitce $e # Split CE
      excluded ${CE_F} dap
      if test $? = 0 ; then
          TARGET="downloads/${CE_F}.nc.$CE_INDEX.cdl"
	  if test "x$SHOWDATA" = xtrue ; then HOPT="-h" ; fi
	  ${NCDUMP} $HOPT "http://$SVC/d4ts/testfiles/${CE_F}.nc?$CSUM&dap4.ce=${CE_CE}#dap4" >${TARGET}
          if test "x$?" != x0 ; then
            echo ">>> wget failed: ${TARGET}"
            rm -f ${TARGET}
          fi
      fi
     done
}

install() {
    local ret
    ret=0
    if test "x$DMR" = x1 ; then
        rm -f dmrtestfiles/*.dmr
        cp downloads/*.dmr dmrtestfiles
    fi
    if test "x$DAP" = x1 ; then
        rm -f daptestfiles/*.dap
        cp downloads/*.dap daptestfiles
    fi
    return $ret
}

cleanup() {
    rm -fr downloads
}

actions() {
    for a in $ACTIONS ; do
    case "$a" in
	reset) rm -fr downloads; mkdir downloads ;;
	cleanup) cleanup ;;
	build) build ;;
	install) install ;;
	dmr) capturedmr ;;
	dap) capturedap ;;
	cdl) capturecdl ;;
    esac	
    done
}

main() {
  defaults
  options "$@"
  setup
  actions
}

main "$@"

