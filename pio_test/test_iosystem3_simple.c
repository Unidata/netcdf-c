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
#define TEST_NAME "test_iosystem3_simple"

/* Needed to init intracomm. */
#define STRIDE 1
#define BASE 0
#define REARRANGER 1

/* Used to devide up the tasks into MPI groups. */
#define OVERLAP_NUM_RANGES 2

/* Run test. */
int main(int argc, char **argv)
{
    int my_rank; /* Zero-based rank of processor. */
    int ntasks; /* Number of processors involved in current execution. */
    int iosysid_world; /* The ID for the parallel I/O system. */
    int overlap_iosysid; /* The ID for iosystem of overlap_comm. */
    MPI_Group world_group; /* An MPI group of world. */
    MPI_Group overlap_group; /* An MPI group of 0, 1, and 3. */
    MPI_Comm overlap_comm = MPI_COMM_NULL; /* Communicator for tasks 0, 1, 2. */
    int overlap_rank = -1; /* Tasks rank in communicator. */
    int overlap_size = 0; /* Size of communicator. */
    int ret; /* Return code. */
    MPI_Comm test_comm;

    /* Initialize test. */
    if ((ret = pio_test_init2(argc, argv, &my_rank, &ntasks, TARGET_NTASKS, TARGET_NTASKS,
                              0, &test_comm)))
        ERR(ERR_INIT);

    /* Test code runs on TARGET_NTASKS tasks. The left over tasks do
     * nothing. */
    if (my_rank < TARGET_NTASKS)
    {
        /* Initialize PIO system on world. */
        if ((ret = PIOc_Init_Intracomm(test_comm, 4, 1, 0, 1, &iosysid_world)))
            ERR(ret);

        /* Get MPI_Group of world comm. */
        if ((ret = MPI_Comm_group(test_comm, &world_group)))
            ERR(ret);

        /* Create a group with tasks 0, 1, 3. */
        int overlap_ranges[OVERLAP_NUM_RANGES][3] = {{0, 0, 1}, {1, 3, 2}};
        if ((ret = MPI_Group_range_incl(world_group, OVERLAP_NUM_RANGES,
                                        overlap_ranges, &overlap_group)))
            ERR(ret);

        /* Create a communicator from the overlap_group. */
        if ((ret = MPI_Comm_create(test_comm, overlap_group, &overlap_comm)))
            ERR(ret);

        /* Learn my rank and the total number of processors in overlap
         * group. */
        if (overlap_comm != MPI_COMM_NULL)
        {
            if ((ret = MPI_Comm_rank(overlap_comm, &overlap_rank)))
                MPIERR(ret);
            if ((ret = MPI_Comm_size(overlap_comm, &overlap_size)))
                MPIERR(ret);
        }

        /* Initialize PIO system for overlap comm. */
        if (overlap_comm != MPI_COMM_NULL)
        {
            if ((ret = PIOc_Init_Intracomm(overlap_comm, 1, 1, 0, 1, &overlap_iosysid)))
                ERR(ret);
        }

        /* Finalize PIO system. */
        if (overlap_comm != MPI_COMM_NULL)
            if ((ret = PIOc_finalize(overlap_iosysid)))
                ERR(ret);

        if ((ret = PIOc_finalize(iosysid_world)))
            ERR(ret);

        /* Free MPI resources used by test. */
        if ((ret = MPI_Group_free(&overlap_group)))
            ERR(ret);
        if ((ret = MPI_Group_free(&world_group)))
            ERR(ret);
        if (overlap_comm != MPI_COMM_NULL)
            if ((ret = MPI_Comm_free(&overlap_comm)))
                ERR(ret);
    }

    /* Finalize test. */
    if ((ret = pio_test_finalize(&test_comm)))
        return ERR_AWFUL;

    printf("%d %s SUCCESS!!\n", my_rank, TEST_NAME);

    return 0;
}
