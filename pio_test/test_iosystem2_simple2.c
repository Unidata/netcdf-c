/*
 * Tests the PIO library with multiple iosysids in use at the
 * same time.
 *
 * This is a simplified, C version of the fortran pio_iosystem_tests2.F90.
 *
 * @author Ed Hartnett
 */
#include <config.h>
#include <pio.h>
#include <pio_tests.h>

/* The number of tasks this test should run on. */
#define TARGET_NTASKS 4

/* The name of this test. */
#define TEST_NAME "test_iosystem2_simple2"

/* Number of test files generated. */
#define NUM_FILES 3

/* Used to define netcdf test file. */
#define PIO_TF_MAX_STR_LEN 100
#define ATTNAME "filename"
#define DIMNAME "filename_dim"

/* Needed to init intracomm. */
#define STRIDE 1
#define BASE 0
#define REARRANGER 1

/* Run test. */
int main(int argc, char **argv)
{
    int my_rank; /* Zero-based rank of processor. */
    int ntasks; /* Number of processors involved in current execution. */
    int iosysid; /* The ID for the parallel I/O system. */
    int iosysid_world; /* The ID for the parallel I/O system. */
    int num_flavors; /* Number of PIO netCDF flavors in this build. */
    int flavor[NUM_FLAVORS]; /* iotypes for the supported netCDF IO flavors. */
    int ret; /* Return code. */
    MPI_Comm test_comm;

    /* Initialize test. */
    if ((ret = pio_test_init2(argc, argv, &my_rank, &ntasks, TARGET_NTASKS, TARGET_NTASKS,
                              -1, &test_comm)))
        ERR(ERR_INIT);

    /* Only do something on the first TARGET_NTASKS tasks. */
    if (my_rank < TARGET_NTASKS)
    {
        /* Figure out iotypes. */
        if ((ret = get_iotypes(&num_flavors, flavor)))
            ERR(ret);

        /* Split world into odd and even. */
        MPI_Comm newcomm;
        int even = my_rank % 2 ? 0 : 1;
        if ((ret = MPI_Comm_split(test_comm, even, 0, &newcomm)))
            MPIERR(ret);

        /* Get size of new communicator. */
        int new_size;
        if ((ret = MPI_Comm_size(newcomm, &new_size)))
            MPIERR(ret);

        /* Initialize an intracomm for evens/odds. */
        if ((ret = PIOc_Init_Intracomm(newcomm, new_size, STRIDE, BASE, REARRANGER, &iosysid)))
            ERR(ret);

        /* Initialize an intracomm for all processes. */
        if ((ret = PIOc_Init_Intracomm(test_comm, TARGET_NTASKS, STRIDE, BASE, REARRANGER,
                                       &iosysid_world)))
            ERR(ret);

        for (int flv = 0; flv < num_flavors; flv++)
        {
            char filename[NUM_SAMPLES][NC_MAX_NAME + 1]; /* Test filename. */
            int sample_ncid[NUM_SAMPLES];

            for (int sample = 0; sample < NUM_SAMPLES; sample++)
            {
                char iotype_name[NC_MAX_NAME + 1];

                /* Create a filename. */
                if ((ret = get_iotype_name(flavor[flv], iotype_name)))
                    return ret;
                sprintf(filename[sample], "%s_%s_%d_%d.nc", TEST_NAME, iotype_name, sample, 0);

                /* Create sample file. */
                if ((ret = create_nc_sample(sample, iosysid_world, flavor[flv], filename[sample],
                                            my_rank, NULL)))
                    ERR(ret);

                /* Check the file for correctness. */
                if ((ret = check_nc_sample(sample, iosysid_world, flavor[flv], filename[sample],
                                           my_rank, &sample_ncid[sample])))
                    ERR(ret);

            }

            /* Now check the files with the other iosysid. Even and odd
             * processes will check different files. */
            int this_sample = even ? 0 : 1;
            char *file1 = filename[this_sample];
            int ncid2;
            if ((ret = check_nc_sample(this_sample, iosysid, flavor[flv], file1, my_rank, &ncid2)))
                ERR(ret);

            /* Now close the open files. */
            for (int sample = 0; sample < NUM_SAMPLES; sample++)
                if ((ret = PIOc_closefile(sample_ncid[sample])))
                    ERR(ret);

            if ((ret = PIOc_closefile(ncid2)))
                ERR(ret);
        } /* next iotype */
        if ((ret = MPI_Comm_free(&newcomm)))
            MPIERR(ret);

        /* Finalize PIO odd/even intracomm. */
        if ((ret = PIOc_finalize(iosysid)))
            ERR(ret);

        /* Finalize PIO world intracomm. */
        if ((ret = PIOc_finalize(iosysid_world)))
            ERR(ret);
    }  /* my_rank < TARGET_NTASKS */

    /* Wait for all task before finalizing. */
    MPI_Barrier(MPI_COMM_WORLD);

    /* Finalize test. */
    if ((ret = pio_test_finalize(&test_comm)))
        return ERR_AWFUL;

    printf("%d %s SUCCESS!!\n", my_rank, TEST_NAME);

    return 0;
}
