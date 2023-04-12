# not executable

if test "x$SETX" = x1 ; then set -x ; fi

dap4_manifest="\
test_atomic_array \
test_atomic_types \
test_enum_1 \
test_enum_2 \
test_enum_3 \
test_enum_array \
test_fill \
test_groups1 \
test_misc1 \
test_one_var \
test_one_vararray \
test_opaque \
test_opaque_array \
test_struct1 \
test_struct_array \
test_struct_nested \
test_struct_nested3 \
test_struct_type \
test_test \
test_unlim \
test_unlim1 \
test_utf8 \
test_vlen1 \
test_vlen10 \
test_vlen11 \
test_vlen2 \
test_vlen3 \
test_vlen4 \
test_vlen5 \
test_vlen6 \
test_vlen7 \
test_vlen8 \
test_vlen9 \
test_zerodim \
"

constrained_manifest="\
test_atomic_array?/vu8[1][0:2:2];/vd[1];/vs[1][0];/vo[0][1]=1 \
test_atomic_array?/v16[0:1,3]=2 \
test_atomic_array?/v16[3,0:1]=3 \
test_one_vararray?/t[1]=4 \
test_one_vararray?/t[0:1]=5 \
test_enum_array?/primary_cloud[1:2:4]=6 \
test_opaque_array?/vo2[1][0:1]=7 \
test_struct_array?/s[0:2:3][0:1]=8 \
"
