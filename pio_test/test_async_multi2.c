/*
 * This tests async with multiple computation components. This is a
 * more comprehensive test than test_async_multicomp.c.
 *
 * @author Ed Hartnett
 * @date 9/12/17
 */
#include <config.h>
#include <pio.h>
#include <pio_tests.h>

/* The number of tasks this test should run on. */
#define TARGET_NTASKS 3

/* The name of this test. */
#define TEST_NAME "test_async_multi2"

/* Number of processors that will do IO. */
#define NUM_IO_PROCS 1

/* Number of tasks in each computation component. */
#define NUM_COMP_PROCS 1

/* Number of computational components to create. */
#define COMPONENT_COUNT 2

/* These are in test_common.c. */
extern int *pio_type;

/* Run simple async test. */
int main(int argc, char **argv)
{
    int my_rank; /* Zero-based rank of processor. */
    int ntasks; /* Number of processors involved in current execution. */
    int iosysid[COMPONENT_COUNT]; /* The ID for the parallel I/O system. */
    int num_iotypes; /* Number of PIO netCDF iotypes in this build. */
    int iotype[NUM_IOTYPES]; /* iotypes for the supported netCDF IO iotypes. */
    int num_procs[COMPONENT_COUNT] = {1, 1}; /* Num procs for IO and computation. */
    int io_proc_list[NUM_IO_PROCS] = {0};
    int comp_proc_list1[NUM_COMP_PROCS] = {1};
    int comp_proc_list2[NUM_COMP_PROCS] = {2};
    int *proc_list[COMPONENT_COUNT] = {comp_proc_list1, comp_proc_list2};
    MPI_Comm test_comm;
    int verbose = 1;
    int ret; /* Return code. */

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
                char filename[NC_MAX_NAME + 1]; /* Test filename. */
                int my_comp_idx = my_rank - 1; /* Index in iosysid array. */
                int num_types = (iotype[i] == PIO_IOTYPE_NETCDF4C ||
                                 iotype[i] == PIO_IOTYPE_NETCDF4P) ? NUM_NETCDF_TYPES - 1 : NUM_CLASSIC_TYPES;

                /* Create a decomposition. */
                /* int dim_len_2d[NDIM2] = {DIM_LEN2, DIM_LEN3}; */
                /* int ioid[num_types]; */
                /* for (int t = 0; t < num_types; t++) */
                /*     if ((ret = create_decomposition_2d(NUM_COMP_PROCS, my_rank, iosysid[my_comp_idx], dim_len_2d, &ioid[t], pio_type[t]))) */
                /*         ERR(ret); */
            
                /* Create sample file. */
                if ((ret = create_nc_sample_4(iosysid[my_comp_idx], iotype[i], my_rank, my_comp_idx,
                                              filename, TEST_NAME, verbose, num_types)))
                    ERR(ret);

                /* Check the file for correctness. */
                if ((ret = check_nc_sample_4(iosysid[my_comp_idx], iotype[i], my_rank, my_comp_idx,
                                             filename, verbose, num_types)))
                    ERR(ret);

                /* Free the decompositions. */
                /* for (int t = 0; t < num_types; t++) */
                /*     if ((ret = PIOc_freedecomp(iosysid[my_comp_idx], ioid[t]))) */
                /*         ERR(ret); */
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
