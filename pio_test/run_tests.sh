# Stop execution of script if error is returned.
set -e

# Stop loop if ctrl-c is pressed.
trap exit SIGINT SIGTERM

printf 'running PIO tests...\n'

# PIO_TESTS='test_async_mpi test_spmd test_rearr test_intercomm2 test_async_simple '\
# 'test_async_3proc test_async_4proc test_iosystem2_simple test_iosystem2_simple2 '\
# 'test_iosystem2 test_iosystem3_simple test_iosystem3_simple2 test_iosystem3 '\
# 'test_pioc_unlim test_pioc_putget test_pioc_fill test_darray_multi '\
# 'test_darray_multivar test_darray_multivar2 test_darray_multivar3 test_darray_1d '\
# 'test_darray_3d test_decomp_uneven test_decomps '
# PIO_TESTS='test_async_mpi test_spmd test_rearr test_intercomm2 test_async_simple '\
# 'test_async_3proc test_async_4proc test_iosystem2_simple test_iosystem2_simple2 '\
# 'test_iosystem2 test_iosystem3_simple test_iosystem3_simple2 test_iosystem3 test_pioc '\
# 'test_pioc_unlim test_pioc_putget test_pioc_fill test_darray test_darray_multi '\
# 'test_darray_multivar test_darray_multivar2 test_darray_multivar3 test_darray_1d '\
# 'test_darray_3d test_decomp_uneven test_decomps test_darray_async_simple '\
# 'test_darray_async test_darray_async_many test_darray_2sync test_async_multicomp '

#PIO_TESTS='test_darray_1d test_async_mpi test_spmd test_rearr test_intercomm2 test_async_simple'
PIO_TESTS='test_darray_1d test_async_mpi test_spmd  test_intercomm2 test_async_simple'

NC_PIO_TESTS='tst_darray_async tst_darray_1d'
success1=true
success2=true
for TEST in $PIO_TESTS
do
    success1=false
    echo "running ${TEST}"
    mpiexec -n 4 ./${TEST} && success1=true || break
done

for TEST in $NC_PIO_TESTS
do
    success2=false
    echo "running ${TEST}"
    mpiexec -n 4 ./${TEST} && success2=true || break
done

# PIO_TESTS_8='test_async_multi2'

# for TEST in $PIO_TESTS_8
# do
#     success=false
#     echo "running ${TEST}"
#     mpiexec -n 8 ./${TEST} && success=true || break
# done

# Did we succeed?
if test "x$success1" = xtrue -a "x$success2" = xtrue; then
    exit 0
fi
exit 1
