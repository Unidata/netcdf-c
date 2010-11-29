#!/bin/sh

# This shell runs the tests with valgrind.

# $Id: run_valgrind_tests.sh,v 1.9 2010/01/26 20:24:18 ed Exp $

set -e
echo ""
echo "Testing programs with valgrind..."

# These are my test programs.
list='tst_h_files tst_h_files2 tst_h_files4 tst_h_atts '\
'tst_h_atts3 tst_h_atts4 tst_h_vars tst_h_vars2 tst_h_vars3 tst_h_grps '\
'tst_h_compounds tst_h_compounds2 tst_h_wrt_cmp tst_h_rd_cmp tst_h_vl '\
'tst_h_opaques tst_h_strings tst_h_strings1 tst_h_strings2 tst_h_ints '\
'tst_h_dimscales tst_h_dimscales1 tst_h_dimscales2 tst_h_dimscales3 '\
'tst_h_enums tst_dims tst_dims2 tst_dims3 tst_files tst_files4 tst_vars '\
'tst_varms tst_unlim_vars tst_converts tst_converts2 tst_grps '\
'tst_compounds tst_compounds2 tst_compounds3 tst_opaques tst_strings '\
'tst_strings2 tst_interops tst_interops4 tst_interops5 tst_interops6 '\
'tst_enums tst_coords tst_coords2 tst_coords3 tst_vars3 tst_chunks '\
'tst_utf8 tst_fills tst_fills2 tst_fillbug tst_xplatform  '\
'tst_h_atts2 tst_endian_fill tst_atts'

# These don't work yet: tst_grps2 tst_xplatform2 

for tst in $list; do
    echo ""
    cmd1="valgrind -q --error-exitcode=2 --leak-check=full ./$tst"
    echo "$cmd1:"
    $cmd1
done

echo "SUCCESS!!!"

exit 0
