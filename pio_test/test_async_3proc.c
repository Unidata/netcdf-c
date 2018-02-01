/*
 * Tests for PIOc_Intercomm. This tests basic asynch I/O capability.
 *
 * This very simple test runs on 32 ranks. Eight are used for IO, the
 * other 24 for computation. The netCDF sample files are created and
 * checked.
 *
 * @author Ed Hartnett
 */
#include <config.h>
#include <pio.h>
#include <pio_tests.h>

/* The number of tasks this test should run on. */
#define TARGET_NTASKS 3

/* The name of this test. */
#define TEST_NAME "test_async_3proc"

/* Number of different combonations of IO and computation processor
 * numbers we will try in this test. */
#define NUM_COMBOS 2

/* Number of computational components to create. */
#define COMPONENT_COUNT 1

/* Run async tests. */
int main(int argc, char **argv)
{
    int my_rank; /* Zero-based rank of processor. */
    int ntasks; /* Number of processors involved in current execution. */
    int iosysid[COMPONENT_COUNT]; /* The ID for the parallel I/O system. */
    int num_flavors; /* Number of PIO netCDF flavors in this build. */
    int flavor[NUM_FLAVORS]; /* iotypes for the supported netCDF IO flavors. */
    int ret; /* Return code. */
    MPI_Comm test_comm; /* comm for test */

    /* Num procs for IO and computation. */
    int num_procs[NUM_COMBOS][COMPONENT_COUNT] = {{1}, {2}};

    /* Number of processors that will do IO. */
    int num_io_procs[NUM_COMBOS] = {2, 1};

    /* Initialize test. */
    if ((ret = pio_test_init2(argc, argv, &my_rank, &ntasks, TARGET_NTASKS, TARGET_NTASKS,
                              -1, &test_comm)))
        ERR(ERR_INIT);

    /* Test code runs on TARGET_NTASKS tasks. The left over tasks do
     * nothing. */
    if (my_rank < TARGET_NTASKS)
    {
        /* Figure out iotypes. */
        if ((ret = get_iotypes(&num_flavors, flavor)))
            ERR(ret);

        for (int combo = 0; combo < NUM_COMBOS; combo++)
        {
            /* Is the current process a computation task? */
            int comp_task = my_rank < num_io_procs[combo] ? 0 : 1;

            /* Initialize the IO system. */
            if ((ret = PIOc_init_async(test_comm, num_io_procs[combo], NULL, COMPONENT_COUNT,
                                       num_procs[combo], NULL, NULL, NULL, PIO_REARR_BOX, iosysid)))
                ERR(ERR_INIT);

            /* All the netCDF calls are only executed on the computation
             * tasks. The IO tasks have not returned from PIOc_Init_Intercomm,
             * and when the do, they should go straight to finalize. */
            if (comp_task)
            {
                for (int flv = 0; flv < num_flavors; flv++)
                {
                    char filename[NC_MAX_NAME + 1]; /* Test filename. */
                    int my_comp_idx = 0; /* Index in iosysid array. */

                    for (int sample = 0; sample < NUM_SAMPLES; sample++)
                    {
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
                {
                    if ((ret = PIOc_finalize(iosysid[c])))
                        ERR(ret);
                }
            } /* endif comp_task */

            /* Wait for everyone to catch up. */
            MPI_Barrier(test_comm);
        } /* next combo */
    } /* endif my_rank < TARGET_NTASKS */

    /* Finalize test. */
    if ((ret = pio_test_finalize(&test_comm)))
        return ERR_AWFUL;

    printf("%d %s SUCCESS!!\n", my_rank, TEST_NAME);

    return 0;
}
