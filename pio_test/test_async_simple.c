/*
 * Tests for PIOc_Intercomm. This tests basic asynch I/O capability.
 *
 * This very simple test runs on two ranks. One is used for
 * computation, the other for IO. A sample netCDF file is created and
 * checked.
 *
 * To run with valgrind, use this command:
 * <pre>mpiexec -n 4 valgrind -v --leak-check=full --suppressions=../../../tests/unit/valsupp_test.supp
 * --error-exitcode=99 --track-origins=yes ./test_async_simple</pre>
 *
 * @author Ed Hartnett
 */
#include <config.h>
#include <pio.h>
#include <pio_tests.h>

/* The number of tasks this test should run on. */
#define TARGET_NTASKS 2

/* The name of this test. */
#define TEST_NAME "test_async_simple"

/* Number of processors that will do IO. */
#define NUM_IO_PROCS 1

/* Number of computational components to create. */
#define COMPONENT_COUNT 1

/* Run simple async test. */
int main(int argc, char **argv)
{
#define NUM_IO_PROCS 1
#define NUM_COMP_PROCS 1
    int my_rank; /* Zero-based rank of processor. */
    int ntasks; /* Number of processors involved in current execution. */
    int iosysid[COMPONENT_COUNT]; /* The ID for the parallel I/O system. */
    int num_flavors; /* Number of PIO netCDF flavors in this build. */
    int flavor[NUM_FLAVORS]; /* iotypes for the supported netCDF IO flavors. */
    int ret; /* Return code. */
    int num_procs[COMPONENT_COUNT] = {1}; /* Num procs for IO and computation. */
    int io_proc_list[NUM_IO_PROCS] = {0};
    int comp_proc_list[NUM_COMP_PROCS] = {1};
    int *proc_list[COMPONENT_COUNT] = {comp_proc_list};
    MPI_Comm test_comm;

    /* Initialize test. */
    if ((ret = pio_test_init2(argc, argv, &my_rank, &ntasks, TARGET_NTASKS, TARGET_NTASKS,
                              -1, &test_comm)))
        ERR(ERR_INIT);

    /* Only do something on TARGET_NTASKS tasks. */
    if (my_rank < TARGET_NTASKS)
    {
        /* Figure out iotypes. */
        if ((ret = get_iotypes(&num_flavors, flavor)))
            ERR(ret);

        /* Is the current process a computation task? */
        int comp_task = my_rank < NUM_IO_PROCS ? 0 : 1;

        /* Check for invalid values. */
        if (PIOc_init_async(test_comm, NUM_IO_PROCS, NULL, COMPONENT_COUNT,
                            num_procs, NULL, NULL, NULL, PIO_REARR_BOX, NULL) != PIO_EINVAL)
            ERR(ERR_WRONG);
        if (PIOc_init_async(test_comm, NUM_IO_PROCS, NULL, COMPONENT_COUNT,
                            num_procs, NULL, NULL, NULL, TEST_VAL_42, iosysid) != PIO_EINVAL)
            ERR(ERR_WRONG);
        if (PIOc_init_async(test_comm, NUM_IO_PROCS, NULL, -1,
                            num_procs, NULL, NULL, NULL, PIO_REARR_BOX, iosysid) != PIO_EINVAL)
            ERR(ERR_WRONG);
        if (PIOc_init_async(test_comm, NUM_IO_PROCS, NULL, COMPONENT_COUNT,
                            NULL, NULL, NULL, NULL, PIO_REARR_BOX, iosysid) != PIO_EINVAL)
            ERR(ERR_WRONG);

        /* Initialize the IO system. */
        if ((ret = PIOc_init_async(test_comm, NUM_IO_PROCS, io_proc_list, COMPONENT_COUNT,
                                   num_procs, (int **)proc_list, NULL, NULL, PIO_REARR_BOX, iosysid)))
            ERR(ERR_INIT);

        /* All the netCDF calls are only executed on the computation
         * tasks. The IO tasks have not returned from PIOc_Init_Intercomm,
         * and when the do, they should go straight to finalize. */
        if (comp_task)
        {
            for (int flv = 0; flv < num_flavors; flv++)
            {
                int my_comp_idx = my_rank - 1; /* Index in iosysid array. */

                for (int sample = 0; sample < NUM_SAMPLES; sample++)
                {
                    char filename[NC_MAX_NAME + 1]; /* Test filename. */
                    char iotype_name[NC_MAX_NAME + 1];

                    /* Create a filename. */
                    if ((ret = get_iotype_name(flavor[flv], iotype_name)))
                        return ret;
                    sprintf(filename, "%s_%s_%d_%d.nc", TEST_NAME, iotype_name, sample, my_comp_idx);

                    /* Create sample file. */
                    if ((ret = create_nc_sample(sample, iosysid[my_comp_idx], flavor[flv], filename, my_rank, NULL)))
                        ERR(ret);

                    /* Check the file for correctness. */
                    if ((ret = check_nc_sample(sample, iosysid[my_comp_idx], flavor[flv], filename, my_rank, NULL)))
                        ERR(ret);
                }
            } /* next netcdf flavor */

            /* Finalize the IO system. Only call this from the computation tasks. */
            for (int c = 0; c < COMPONENT_COUNT; c++)
                if ((ret = PIOc_finalize(iosysid[c])))
                    ERR(ret);
        } /* endif comp_task */
    } /* endif my_rank < TARGET_NTASKS */

    /* Finalize test. */
    if ((ret = pio_test_finalize(&test_comm)))
        return ERR_AWFUL;

    printf("%d %s SUCCESS!!\n", my_rank, TEST_NAME);

    return 0;
}
