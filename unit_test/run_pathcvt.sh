#!/bin/sh

if test "x$srcdir" = x ; then srcdir=`pwd`; fi 
. ../test_common.sh

set -e

# For testing purposes: we want MSYS_NO_PATHCONV and MSYS2_ARG_CONV_EXCL set.
#     Suppress converting paths of form /[a-z]/... to [a-z]:/...
      export MSYS_NO_PATHCONV=1
#     Disable automatic path conversions in MINGW shell:
      export MSYS2_ARG_CONV_EXCL='*'

${execdir}/test_pathcvt
