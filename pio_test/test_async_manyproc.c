/*
 * This tests async with multiple computation components. This test
 * uses more processors than test_async_multicomp.c. In this test, the
 * IO component has 3 processors, and the computational components
 * each have 2 processors, so the test uses 7 total.
 *
 * @author Ed Hartnett
 * @date 9/13/17
 */
#include <config.h>
#include <pio.h>
#include <pio_tests.h>

/* The number of tasks this test should run on. */
#define TARGET_NTASKS 7

/* The name of this test. */
#define TEST_NAME "test_async_manyproc"

/* Number of processors that will do IO. */
#define NUM_IO_PROCS 3

/* Number of tasks in each computation component. */
#define NUM_COMP_PROCS 2

/* Number of computational components to create. */
#define COMPONENT_COUNT 2

/* Number of dims in test file. */
#define NDIM2 2

/* Run simple async test. */
int main(int argc, char **argv)
{
    int my_rank; /* Zero-based rank of processor. */
    int ntasks; /* Number of processors involved in current execution. */
    int iosysid[COMPONENT_COUNT]; /* The ID for the parallel I/O system. */
    int num_iotypes; /* Number of PIO netCDF iotypes in this build. */
    int iotype[NUM_IOTYPES]; /* iotypes for the supported netCDF IO iotypes. */
    int num_procs[COMPONENT_COUNT] = {NUM_COMP_PROCS, NUM_COMP_PROCS}; /* Num procs for IO and computation. */
    int io_proc_list[NUM_IO_PROCS];
    int comp_proc_list1[NUM_COMP_PROCS] = {NUM_IO_PROCS, NUM_IO_PROCS + 1};
    int comp_proc_list2[NUM_COMP_PROCS] = {NUM_IO_PROCS + 2, NUM_IO_PROCS + 3};
    int *proc_list[COMPONENT_COUNT] = {comp_proc_list1, comp_proc_list2};
    MPI_Comm test_comm;
    int verbose = 0;
    int ret; /* Return code. */

    /* Initialize our list of IO tasks. */
    for (int p = 0; p < NUM_IO_PROCS; p++)
        io_proc_list[p] = p;

    /* Initialize test. */
    if ((ret = pio_test_init2(argc, argv, &my_rank, &ntasks, TARGET_NTASKS, TARGET_NTASKS,
                              -1, &test_comm)))
        ERR(ERR_INIT);

    /* Is the current process a computation task? */    
    int comp_task = my_rank < NUM_IO_PROCS ? 0 : 1;
    
    /* Only do something on TARGET_NTASKS tasks. */
    if (my_rank < TARGET_NTASKS)
    {
        /* Figure out iotypes. */
        if ((ret = get_iotypes(&num_iotypes, iotype)))
            ERR(ret);

        /* Initialize the IO system. The IO task will not return from
         * this call, but instead will go into a loop, listening for
         * messages. */
        if ((ret = PIOc_init_async(test_comm, NUM_IO_PROCS, io_proc_list, COMPONENT_COUNT,
                                   num_procs, (int **)proc_list, NULL, NULL, PIO_REARR_BOX, iosysid)))
            ERR(ERR_INIT);
        if (verbose)
            for (int c = 0; c < COMPONENT_COUNT; c++)
                printf("my_rank %d cmp %d iosysid[%d] %d\n", my_rank, c, c, iosysid[c]);

        /* All the netCDF calls are only executed on the computation
         * tasks. */
        if (comp_task)
        {
            for (int i = 0; i < num_iotypes; i++)
            {
                /* char filename[NC_MAX_NAME + 1]; /\* Test filename. *\/ */
                /* /\* Ranks 0, 1, 2 are IO. 3, 4 are the first */
                /*  * computation component. 5, 6 are the second. *\/ */
                /* int my_comp_idx = my_rank < NUM_IO_PROCS + NUM_COMP_PROCS ? 0 : 1;  /\* Index in iosysid array. *\/ */

                /* /\* Create sample file. *\/ */
                /* if ((ret = create_nc_sample_3(iosysid[my_comp_idx], iotype[i], my_rank, my_comp_idx, */
                /*                               filename, TEST_NAME, verbose, 0, 0))) */
                /*     ERR(ret); */

                /* /\* Check the file for correctness. *\/ */
                /* if ((ret = check_nc_sample_3(iosysid[my_comp_idx], iotype[i], my_rank, my_comp_idx, */
                /*                              filename, verbose, 0, 0))) */
                /*     ERR(ret); */
            } /* next netcdf iotype */

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
