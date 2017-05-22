#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi
. ../test_common.sh

set -e

#X="-x"
#grind="checkleaks"

#exec sh $X ${srcdir}/tst_ncdap.sh "$srcdir" "$builddir" "file3" $grind
exec sh $X ${srcdir}/tst_ncdap.sh "$srcdir" "$builddir" "dds3" $grind

