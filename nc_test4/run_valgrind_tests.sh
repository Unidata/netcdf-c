#!/bin/sh

# This shell runs the tests with valgrind.

# $Id: run_valgrind_tests.sh,v 1.9 2010/01/26 20:24:18 ed Exp $

set -e
echo ""
echo "Testing programs with valgrind..."

# These are my test programs.
list='tst_v2 '\
'tst_vars2 tst_atts tst_atts2 tst_h_vl2'

# These don't work yet: tst_h_vars3 
# tst_h_strings tst_h_atts3 tst_h_vars2 tst_vars tst_fills tst_chunks
# tst_coords tst_xplatform2

for tst in $list; do
    echo ""
    echo "Memory testing with $tst:"
    valgrind -q --error-exitcode=2 --leak-check=full ./$tst
done

echo "SUCCESS!!!"

exit 0
