/*
 * Common test code for PIO C tests.
 *
 * Ed Hartnett
 */
#include <config.h>
#include <pio.h>
#include <pio_internal.h>
#include <pio_tests.h>

/* Test with async.
 *
 * @param my_rank rank of the task.
 * @param nprocs the size of the communicator.
 * @param num_flavors the number of PIO IO types that will be tested.
 * @param flavors array of the PIO IO types that will be tested.
 * @param test_comm communicator with all test tasks.
 * @returns 0 for success error code otherwise.
 */
int test_async2(int my_rank, int num_flavors, int *flavor, MPI_Comm test_comm,
                int component_count, int num_io_procs, int target_ntasks, char *test_name)
{
    int iosysid[component_count];  /* The ID for the parallel I/O system. */
    int num_procs[component_count]; /* Num procs in each component. */
    MPI_Comm io_comm;              /* Will get a duplicate of IO communicator. */
    MPI_Comm comp_comm[component_count]; /* Will get duplicates of computation communicators. */
    int mpierr;  /* Return code from MPI functions. */
    int ret;     /* Return code. */

    num_procs[0] = target_ntasks - 1;

    /* Is the current process a computation task? */
    int comp_task = my_rank < num_io_procs ? 0 : 1;

    /* Initialize the IO system. */
    if ((ret = PIOc_init_async(test_comm, num_io_procs, NULL, component_count,
                               num_procs, NULL, &io_comm, comp_comm, PIO_REARR_BOX, iosysid)))
        ERR(ERR_INIT);

    /* All the netCDF calls are only executed on the computation
     * tasks. The IO tasks have not returned from PIOc_Init_Intercomm,
     * and when the do, they should go straight to finalize. */
    if (comp_task)
    {
        for (int c = 0; c < component_count; c++)
        {
            if ((ret = test_all(iosysid[c], num_flavors, flavor, my_rank, comp_comm[0], 1)))
                return ret;

            /* Finalize the IO system. Only call this from the computation tasks. */
            if ((ret = PIOc_finalize(iosysid[c])))
                ERR(ret);
            if ((mpierr = MPI_Comm_free(&comp_comm[c])))
                MPIERR(mpierr);
        }
    }
    else
    {
        if ((mpierr = MPI_Comm_free(&io_comm)))
            MPIERR(mpierr);
    } /* endif comp_task */

    return PIO_NOERR;
}

/* Test without async.
 *
 * @param my_rank rank of the task.
 * @param num_flavors the number of PIO IO types that will be tested.
 * @param flavors array of the PIO IO types that will be tested.
 * @param test_comm communicator with all test tasks.
 * @returns 0 for success error code otherwise.
 */
int test_no_async2(int my_rank, int num_flavors, int *flavor, MPI_Comm test_comm, int target_ntasks,
                   int x_dim_len, int y_dim_len)
{
    int niotasks;    /* Number of processors that will do IO. */
    int ioproc_stride = 1;    /* Stride in the mpi rank between io tasks. */
    int ioproc_start = 0;     /* Zero based rank of first processor to be used for I/O. */
    PIO_Offset elements_per_pe; /* Array index per processing unit. */
    int iosysid;  /* The ID for the parallel I/O system. */
    int ioid;     /* The I/O description ID. */
    PIO_Offset *compdof; /* The decomposition mapping. */
    int slice_dimlen[2];
    int ret;      /* Return code. */

    slice_dimlen[0] = x_dim_len;
    slice_dimlen[1] = y_dim_len;

    /* keep things simple - 1 iotask per MPI process */
    niotasks = target_ntasks;

    /* Initialize the PIO IO system. This specifies how
     * many and which processors are involved in I/O. */
    if ((ret = PIOc_Init_Intracomm(test_comm, niotasks, ioproc_stride,
                                   ioproc_start, PIO_REARR_SUBSET, &iosysid)))
        return ret;

    /* Describe the decomposition. This is a 0-based array, so don't add 1! */
    elements_per_pe = x_dim_len * y_dim_len / target_ntasks;
    if (!(compdof = malloc(elements_per_pe * sizeof(PIO_Offset))))
        return PIO_ENOMEM;
    for (int i = 0; i < elements_per_pe; i++)
        compdof[i] = my_rank * elements_per_pe + i;

    /* Create the PIO decomposition for this test. */
    if ((ret = PIOc_init_decomp(iosysid, PIO_FLOAT, 2, slice_dimlen, (PIO_Offset)elements_per_pe,
                                compdof, &ioid, 0, NULL, NULL)))
        return ret;
    free(compdof);

    /* Run tests. */
    if ((ret = test_all(iosysid, num_flavors, flavor, my_rank, test_comm, 0)))
        return ret;

    /* Free the PIO decomposition. */
    if ((ret = PIOc_freedecomp(iosysid, ioid)))
        return ret;

    /* Finalize PIO system. */
    if ((ret = PIOc_finalize(iosysid)))
        return ret;

    return PIO_NOERR;
}

/*
 * Run the test code, executing the test_all() function without async,
 * and then with async.
 *
 * @param argc argument count passed into main.
 * @param argv argument array passed into main.
 * @param min_ntasks the min number of tasks this code need to run.
 * @param max_ntasks the code will run on this many tasks, extras will
 * be ignored.
 * @param log_level the level to set the logger at.
 * @param test_name the name of the test being run.
 * @param dim_len array of size 3 with the dimension sizes.
 * @param component_count number of computation components.
 * @param num_io_procs number of IO components.
 * @returns 0 for success, error code otherwise.
 */
int run_test_main(int argc, char **argv, int min_ntasks, int max_ntasks,
                  int log_level, char *test_name, int *dim_len, int component_count,
                  int num_io_procs)
{
    int my_rank;
    int ntasks;
    int num_flavors; /* Number of PIO netCDF flavors in this build. */
    int flavor[NUM_FLAVORS]; /* iotypes for the supported netCDF IO flavors. */
    MPI_Comm test_comm; /* A communicator for this test. */
    int ret;         /* Return code. */

    /* Initialize test. */
    if ((ret = pio_test_init2(argc, argv, &my_rank, &ntasks, min_ntasks,
                              max_ntasks, log_level, &test_comm)))
        ERR(ERR_INIT);

    if ((ret = PIOc_set_iosystem_error_handling(PIO_DEFAULT, PIO_RETURN_ERROR, NULL)))
        return ret;

    /* Only do something on max_ntasks tasks. */
    if (my_rank < max_ntasks)
    {
        /* Figure out iotypes. */
        if ((ret = get_iotypes(&num_flavors, flavor)))
            ERR(ret);

        /* Run tests without async feature. */
        if ((ret = test_no_async2(my_rank, num_flavors, flavor, test_comm, max_ntasks,
                                  dim_len[1], dim_len[2])))
            return ret;

        /* Run tests with async. */
        if ((ret = test_async2(my_rank, num_flavors, flavor, test_comm, component_count,
                               num_io_procs, max_ntasks, test_name)))
            return ret;

    } /* endif my_rank < TARGET_NTASKS */

    /* Finalize the MPI library. */
    if ((ret = pio_test_finalize(&test_comm)))
        return ret;

    printf("%d %s SUCCESS!!\n", my_rank, test_name);
    return 0;
}
