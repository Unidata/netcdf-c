#!/bin/bash

if test "x$srcdir" = x ; then srcdir=`pwd`; fi 
. ../test_common.sh

set -e

# Test scoping rules for types and dimensions

# Type test cases
# group_only - type defined in same group as var/attr
# ancestor_only - type defined in some ancestor group of var/attr
# ancestor_subgroup - type defined in both ancestor group and subgroup
# preorder - type defined in some preceding, non ancestor group

# Dimension test cases
# group_only - dim defined in same group as var
# ancestor_only - dim defined in some ancestor group of var
# ancestor_subgroup - dim defined in both ancestor group and subgroup

TSTS="scope_group_only scope_ancestor_only scope_ancestor_subgroup scope_preorder"

setup() {
    ${NCGEN} -4 -lb ${srcdir}/$1.cdl
}

testcycle() {
${NCCOPY} ${execdir}/$1.nc ${execdir}/$1_copy.nc
${NCDUMP} -h -n $1 ${execdir}/$1_copy.nc > copy_$1.cdl
diff -wB ${srcdir}/$1.cdl ${execdir}/copy_$1.cdl
}

typescope() {
ls -l ${execdir}/printfqn* ${execdir}/$1.nc ${execdir}/$1_copy.nc
REFT=`${execdir}/printfqn -f ${execdir}/$1.nc -v test_variable -t`
COPYT=`${execdir}/printfqn -f ${execdir}/$1_copy.nc -v test_variable -t`
if test "x$REFT" != "x$COPYT" ; then
  echo "***Fail: ref=${REFT} copy=${COPYT}"
  exit 1
fi
}

dimscope() {
REFT=`${execdir}/printfqn -f ${execdir}/$1.nc -v test_variable -d`
COPYT=`${execdir}/printfqn -f ${execdir}/$1_copy.nc -v test_variable -d`
if test "x$REFT" != "x$COPYT" ; then
  echo "***Fail: ref=${REFT} copy=${COPYT}"
fi
}

for t in $TSTS ; do setup $t; done
for t in $TSTS ; do testcycle $t; done
for t in $TSTS ; do typescope $t; done
for t in $TSTS ; do dimscope $t; done

exit 0
