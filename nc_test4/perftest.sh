#!/bin/sh

BUILD=1
#PROF=1
#DEBUG=1
#MEM=1

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

set -e

echo "Testing performance of nc_create and nc_open on file with large metadata"

ARGS="--treedepth=6 \
--ngroups=2 \
--ngroupattrs=100 \
--ndims=100 \
--ntypes=10 \
--nvars=100 \
--varrank=2 \
--nvarattrs=500"

if test "x$MEM" = x1 ; then
CMD="valgrind --leak-check=full"
elif test "x$DEBUG" = x1 ; then
CMD="gdb --args "
fi

if test "x$BUILD" = x1 ; then
CFLAGS="-Wall -Wno-unused-variable -Wno-unused-function -I.. -I../include"
LDFLAGS="../liblib/.libs/libnetcdf.a -L/usr/local/lib -lhdf5_hl -lhdf5 -lz -ldl -lcurl -lm -lmfhdf -ldf"
CC=gcc
if test "x$PROF" = x1 ; then
CFLAGS="-pg $CFLAGS"
LDFLAGS="-pg $LDFLAGS"
fi
if test "x$DEBUG" = x1 ; then
CFLAGS="-g -O0 $CFLAGS"
LDFLAGS="-g -O0 $LDFLAGS"
fi
LLP="/usr/local/lib:${LD_LIBRARY_PATH}"
export LD_LIBRARY_PATH=${LLP}; export CFLAGS; export LDFLAGS
${CC} -o bigmeta ${CFLAGS} bigmeta.c  ${LDFLAGS}
${CC} -o openbigmeta ${CFLAGS} openbigmeta.c  ${LDFLAGS}
fi

echo "timing bigmeta:"
${CMD} ./bigmeta $ARGS
echo "timing openbigmeta:"
${CMD} ./openbigmeta
if test "x$PROF" = x1 ; then
rm -f perftest.txt
gprof openbigmeta gmon.out >perftest.txt
fi
