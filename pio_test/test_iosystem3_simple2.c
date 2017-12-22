/*
 * Tests the PIO library with multiple iosysids in use at the
 * same time.
 *
 * This is a simplified, C version of the fortran
 * pio_iosystem_tests3.F90.
 *
 * @author Ed Hartnett
 */
#include <config.h>
#include <pio.h>
#include <pio_tests.h>

/* The number of tasks this test should run on. */
#define TARGET_NTASKS 4

/* The name of this test. */
#define TEST_NAME "test_iosystem3"

/* Used when initializing PIO. */
#define STRIDE1 1
#define BASE0 0
#define NUM_IO4 4
#define REARRANGER 1

/* Run async tests. */
int main(int argc, char **argv)
{
    int my_rank; /* Zero-based rank of processor. */
    int ntasks; /* Number of processors involved in current execution. */
    int iosysid_world; /* The ID for the parallel I/O system. */
    char fname0[NC_MAX_NAME + 1];
    int ncid;
    int num_flavors; /* Number of PIO netCDF flavors in this build. */
    int flavor[NUM_FLAVORS]; /* iotypes for the supported netCDF IO flavors. */
    int ret; /* Return code. */
    MPI_Comm test_comm;

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

        /* Initialize PIO system on world. */
        if ((ret = PIOc_Init_Intracomm(test_comm, NUM_IO4, STRIDE1, BASE0, REARRANGER, &iosysid_world)))
            ERR(ret);

        for (int i = 0; i < num_flavors; i++)
        {
            /* Create the file. */
            sprintf(fname0, "test_iosystem3_simple2_%d.nc", i);
            if ((ret = PIOc_createfile(iosysid_world, &ncid, &flavor[i], fname0, NC_CLOBBER)))
                return ret;

            /* End define mode. */
            if ((ret = PIOc_enddef(ncid)))
                return ret;

            /* Close the file. */
            if ((ret = PIOc_closefile(ncid)))
                return ret;

            /* Now check the first file from WORLD communicator. */
            int mode = PIO_WRITE;

            /* Open the file. */
            if ((ret = PIOc_openfile(iosysid_world, &ncid, &flavor[i], fname0, mode)))
                return ret;

            /* Check the file. */
            int ndims;
            if ((ret = PIOc_inq(ncid, &ndims, NULL, NULL, NULL)))
                return ret;

            /* Close the file. */
            if ((ret = PIOc_closefile(ncid)))
                return ret;
        } /* next iotype */

        /* Finalize PIO systems. */
        if ((ret = PIOc_finalize(iosysid_world)))
            ERR(ret);
    } /* my_rank < TARGET_NTASKS */

    /* Finalize test. */
    if ((ret = pio_test_finalize(&test_comm)))
        return ERR_AWFUL;

    printf("%d %s SUCCESS!!\n", my_rank, TEST_NAME);

    return 0;
}
