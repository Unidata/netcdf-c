#!/bin/sh

if test "x$srcdir" = "x" ; then srcdir=`dirname $0`; fi; export srcdir

rm -f log.log
sh -x ${srcdir}/test_parse.sh > ./parse.log 2>&1
cat log.log /dev/tty

exit 0
