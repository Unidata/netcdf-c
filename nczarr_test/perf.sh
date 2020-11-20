#!/bin/sh
# This is a metadata performance test for nczarr
# Dennis Heimbigner

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

. ./bm_common.sh

echo "Testing performance of nc_create and nc_open on file with large metadata"

MEDARGS="--treedepth=2 \
--ngroups=2 \
--ngroupattrs=100 \
--ndims=100 \
--ntypes=10 \
--nvars=100 \
--varrank=2 \
--nvarattrs=500"

BIGARGS="--treedepth=6 \
--ngroups=2 \
--ngroupattrs=100 \
--ndims=100 \
--ntypes=10 \
--nvars=100 \
--varrank=2 \
--nvarattrs=500"

SMALLARGS="--treedepth=2 \
--ngroups=2 \
--ngroupattrs=100 \
--ndims=100 \
--ntypes=10 \
--nvars=10 \
--varrank=2 \
--nvarattrs=100"

ARGS="$SMALLARGS"

bmtest bm_bigmeta $ARGS
bmtest bm_openmeta $ARGS

