/*
 * Tests for PIO distributed arrays. This tests cases when arrays do
 * not distribute evenly over the processors.
 *
 * @author Ed Hartnett
 * @date 3/6/17
 */
#include <config.h>
#include <pio.h>
#include <pio_internal.h>
#include <pio_tests.h>

/* The number of tasks this test should run on. */
#define TARGET_NTASKS 4

/* The minimum number of tasks this test should run on. */
#define MIN_NTASKS 4

/* The name of this test. */
#define TEST_NAME "test_darray_uneven"

/* Number of processors that will do IO. */
#define NUM_IO_PROCS 1

/* Number of computational components to create. */
#define COMPONENT_COUNT 1

/* This is for 3D data decompositions. */
#define NDIM3 3

/* Create the decomposition to divide the 4-dimensional sample data
 * between the 4 tasks. For the purposes of decomposition we are only
 * concerned with 3 dimensions - we ignore the unlimited dimension.
 *
 * @param ntasks the number of available tasks
 * @param my_rank rank of this task.
 * @param iosysid the IO system ID.
 * @param dim_len an array of length 3 with the dimension sizes.
 * @param pio_type the type for this decomposition.
 * @param ioid a pointer that gets the ID of this decomposition.
 * lengths.
 * @returns 0 for success, error code otherwise.
 **/
int create_decomposition_3d(int ntasks, int my_rank, int iosysid, int *dim_len,
                            int pio_type, int *ioid)
{
    PIO_Offset elements_per_pe;     /* Array elements per processing unit. */
    PIO_Offset remainder;     /* Left over array elements. */
    PIO_Offset *compdof;  /* The decomposition mapping. */
    PIO_Offset data_size = 1;
    int ret;

    /* How many data elements per task? In this example we will end up
     * with 4. */
    for (int d = 0; d < NDIM3; d++)
        data_size *= dim_len[d];
    elements_per_pe = data_size / ntasks;
    remainder = data_size % ntasks;

    /* Distribute the remaining elements. */
    if (my_rank < remainder)
        elements_per_pe++;

    /* Allocate space for the decomposition array. */
    if (!(compdof = malloc(elements_per_pe * sizeof(PIO_Offset))))
        return PIO_ENOMEM;

    /* Describe the decomposition. */
    for (int i = 0; i < elements_per_pe; i++)
    {
        int my_remainder = 0;
        if (my_rank >= remainder)
            my_remainder = remainder;
        compdof[i] = my_rank * elements_per_pe + i + my_remainder;
    }

    /* Create the PIO decomposition for this test. */
    if ((ret = PIOc_init_decomp(iosysid, pio_type, NDIM3, dim_len, elements_per_pe,
                                compdof, ioid, 0, NULL, NULL)))
        ERR(ret);

    /* Free the mapping. */
    free(compdof);

    return 0;
}

/**
 * Test the decomp read/write functionality. Given an ioid for a 3D
 * decomposition, this function will write a decomp file, then read it
 * in to ensure the correct values are read.
 *
 * @param iosysid the IO system ID.
 * @param ioid the ID of the decomposition.
 * @param num_flavors the number of IOTYPES available in this build.
 * @param flavor array of available iotypes.
 * @param my_rank rank of this task.
 * @param rearranger the rearranger to use (PIO_REARR_BOX or
 * PIO_REARR_SUBSET).
 * @param test_comm the MPI communicator for this test.
 * @param dim_len array of length 3 with dim lengths.
 * @param expected_maplen pointer to array of length TARGET_NTASKS
 * with the maplen we expect to get for each of the tasks running this
 * test.
 * @param pio_type the type we expect to be associated with
 * this decomposition.
 * @param full_maplen the length of the full map.
 * @param pointer to expected map, an array of TARGET_NTASKS *
 * max_maplen.
 * @returns 0 for success, error code otherwise.
 */
int test_decomp_read_write(int iosysid, int ioid, int num_flavors, int *flavor, int my_rank,
                           int rearranger, MPI_Comm test_comm, int *dim_len, int *expected_maplen,
                           int pio_type, int fill_maplen, int *expected_map)
{
#define TEST_DECOMP_TITLE "Decomposition data for test_darray_uneven.c in PIO library."
#define TEST_DECOMP_HISTORY "This file may be deleted; it is for test purposes only: "
    int ioid2;                         /* ID for decomp we read into. */
    char filename[PIO_MAX_NAME + 1];   /* Name for the output files. */
    char title[] = TEST_DECOMP_TITLE;
    char history[PIO_MAX_NAME + 1] = TEST_DECOMP_HISTORY;
    char title_in[PIO_MAX_NAME + 1];
    char history_in[PIO_MAX_NAME + 1];
    int fortran_order_in; /* Indicates fortran vs. c order. */
    int ret;              /* Return code. */

    /* Use PIO to create the decomp file in one of the four
     * available ways. */
    for (int fmt = 0; fmt < 1; fmt++)
    {
        /* Create the filename. */
        sprintf(filename, "decomp_%s_pio_type_%d_dims_%d_x_%d_x_%d.nc", TEST_NAME, pio_type,
                dim_len[0], dim_len[1], dim_len[2]);

        /* Create history string. */
        strncat(history, filename, NC_MAX_NAME - strlen(TEST_DECOMP_HISTORY));

        if ((ret = PIOc_write_nc_decomp(iosysid, filename, 0, ioid, title, history, 0)))
            return ret;

        /* Open the decomposition file with netCDF. */
        int ncid_in;
        int iotype = PIO_IOTYPE_NETCDF;
        if ((ret = PIOc_openfile(iosysid, &ncid_in, &iotype, filename, NC_NOWRITE)))
            return ret;

        /* Get the max maplen. */
        int max_maplen;
        if ((ret = PIOc_get_att_int(ncid_in, NC_GLOBAL, DECOMP_MAX_MAPLEN_ATT_NAME, &max_maplen)))
            return ret;

        /* Check dims. */
        PIO_Offset ndims_in;
        if ((ret = PIOc_inq_dim(ncid_in, 0, NULL, &ndims_in)))
            return ret;
        if (ndims_in != NDIM3)
            return ERR_WRONG;
        PIO_Offset ntasks_in;
        if ((ret = PIOc_inq_dim(ncid_in, 1, NULL, &ntasks_in)))
            return ret;
        if (ntasks_in != TARGET_NTASKS)
            return ERR_WRONG;

        /* Check the maplen. */
        int maplen_varid;
        int maplen_in[TARGET_NTASKS];
        if ((ret = PIOc_inq_varid(ncid_in, DECOMP_MAPLEN_VAR_NAME, &maplen_varid)))
            return ret;
        if ((ret = PIOc_get_var(ncid_in, maplen_varid, &maplen_in)))
            return ret;
        for (int t = 0; t < TARGET_NTASKS; t++)
        {
            if (maplen_in[t] != expected_maplen[t])
                return ERR_WRONG;
        }

        /* Check the map. */
        int map_varid;
        int map_in[TARGET_NTASKS][max_maplen];
        if ((ret = PIOc_inq_varid(ncid_in, DECOMP_MAP_VAR_NAME, &map_varid)))
            return ret;
        if ((ret = PIOc_get_var(ncid_in, map_varid, (int *)&map_in)))
            return ret;
        for (int t = 0; t < TARGET_NTASKS; t++)
        {
            for (int e = 0; e < max_maplen; e++)
            {
                if (map_in[t][e] != expected_map[t * max_maplen + e])
                    return ERR_WRONG;
            }
        }

        /* Close the decomposition file. */
        if ((ret = PIOc_closefile(ncid_in)))
            return ret;

        /* Read the decomposition file into PIO. */
        if ((ret = PIOc_read_nc_decomp(iosysid, filename, &ioid2, test_comm, pio_type,
                                       title_in, history_in, &fortran_order_in)))
            return ret;

        /* Check the results. */
        {
            iosystem_desc_t *ios;
            io_desc_t *iodesc;

            /* Get the IO system info. */
            if (!(ios = pio_get_iosystem_from_id(iosysid)))
                return pio_err(NULL, NULL, PIO_EBADID, __FILE__, __LINE__);

            /* Get the IO desc, which describes the decomposition. */
            if (!(iodesc = pio_get_iodesc_from_id(ioid2)))
                return pio_err(ios, NULL, PIO_EBADID, __FILE__, __LINE__);

            /* We need to find the MPI type we will expect to see in
             * iodesc. */
            MPI_Datatype expected_mpi_type;
            if ((ret = find_mpi_type(pio_type, &expected_mpi_type, NULL)))
                return ret;

            /* Check values in iodesc. */
            /* printf("ioid2 = %d iodesc->ioid = %d iodesc->maplen = %d iodesc->ndims = %d " */
            /*        "iodesc->ndof = %d iodesc->rearranger = %d iodesc->maxregions = %d " */
            /*        "iodesc->needsfill = %d iodesc->mpitype = %d expected_mpi_type = %d\n", */
            /*        ioid2, iodesc->ioid, iodesc->maplen, iodesc->ndims, iodesc->ndof, */
            /*        iodesc->rearranger, iodesc->maxregions, iodesc->needsfill, iodesc->mpitype, */
            /*        expected_mpi_type); */
            if (strcmp(title, title_in) || strcmp(history, history_in))
                return ERR_WRONG;
            if (iodesc->ioid != ioid2 || iodesc->rearranger != rearranger ||
                iodesc->mpitype != expected_mpi_type)
                return ERR_WRONG;
            if (iodesc->ndims != NDIM3)
                return ERR_WRONG;
            if (iodesc->maplen != expected_maplen[my_rank])
                return ERR_WRONG;
            if (iodesc->ndims != NDIM3 || iodesc->ndof != expected_maplen[my_rank])
                return ERR_WRONG;
            if (iodesc->needsfill)
                return ERR_WRONG;
            /* Don't forget to add 1! */
            for (int e = 0; e < iodesc->maplen; e++)
            {
                /* printf("%d e = %d max_maplen = %d iodesc->map[e] = %lld expected_map[my_rank * max_maplen + e] = %d\n", */
                /*        my_rank, e, max_maplen, iodesc->map[e], expected_map[my_rank * max_maplen + e]); */
                if (iodesc->map[e] != expected_map[my_rank * max_maplen + e] + 1)
                    return ERR_WRONG;
            }
            for (int d = 0; d < NDIM3; d++)
                if (iodesc->dimlen[d] != dim_len[d])
                    return ERR_WRONG;
        }

        /* Free the PIO decomposition. */
        if ((ret = PIOc_freedecomp(iosysid, ioid2)))
            ERR(ret);
    }
    return PIO_NOERR;
}

/* Run tests for darray functions. */
int main(int argc, char **argv)
{
/* #define NUM_TYPES_TO_TEST 3 */
/*     int test_type[NUM_TYPES_TO_TEST] = {PIO_INT, PIO_FLOAT, PIO_DOUBLE}; */
#define NUM_TYPES_TO_TEST 1
    int test_type[NUM_TYPES_TO_TEST] = {PIO_INT};
    int my_rank;
    int ntasks;
    int num_flavors; /* Number of PIO netCDF flavors in this build. */
    int flavor[NUM_FLAVORS]; /* iotypes for the supported netCDF IO flavors. */
    MPI_Comm test_comm; /* A communicator for this test. */
    int ret;         /* Return code. */

    /* Initialize test. */
    if ((ret = pio_test_init2(argc, argv, &my_rank, &ntasks, MIN_NTASKS,
                              MIN_NTASKS, -1, &test_comm)))
        ERR(ERR_INIT);

    if ((ret = PIOc_set_iosystem_error_handling(PIO_DEFAULT, PIO_RETURN_ERROR, NULL)))
        return ret;

    /* Only do something on max_ntasks tasks. */
    if (my_rank < TARGET_NTASKS)
    {
#define NUM_REARRANGERS_TO_TEST 2
        int rearranger[NUM_REARRANGERS_TO_TEST] = {PIO_REARR_BOX, PIO_REARR_SUBSET};
        int iosysid;  /* The ID for the parallel I/O system. */
        int ioproc_stride = 1;    /* Stride in the mpi rank between io tasks. */
        int ioproc_start = 0;     /* Zero based rank of first processor to be used for I/O. */
        int map_1x4x4[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
        int map_2x4x4[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
                           16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31};
        int map_3x4x4[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
                           24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47};
        int map_1x3x3[] = {0, 1, 2, 3, 4, PIO_FILL_INT, 5, 6, PIO_FILL_INT, 7, 8, PIO_FILL_INT};
        int map_1x2x3[] = {0, 1, 2, 3, 4, PIO_FILL_INT, 5, PIO_FILL_INT};
#define NUM_DIM_COMBOS_TO_TEST 5
        int dim_len[NUM_DIM_COMBOS_TO_TEST][NDIM3] = {{1, 4, 4},
                                                      {2, 4, 4},
                                                      {3, 4, 4},
                                                      {1, 3, 3},
                                                      {1, 2, 3}};
        int expected_maplen[NUM_DIM_COMBOS_TO_TEST][TARGET_NTASKS] = {{4, 4, 4, 4},
                                                                      {8, 8, 8, 8},
                                                                      {12, 12, 12, 12},
                                                                      {3, 2, 2, 2},
                                                                      {2, 2, 1, 1}};
        int *expected_map[NUM_DIM_COMBOS_TO_TEST] = {map_1x4x4, map_2x4x4, map_3x4x4, map_1x3x3, map_1x2x3};
        int ret;      /* Return code. */

        /* Figure out iotypes. */
        if ((ret = get_iotypes(&num_flavors, flavor)))
            ERR(ret);

        for (int r = 0; r < NUM_REARRANGERS_TO_TEST; r++)
        {
            int ioid; /* Decomposition ID. */

            /* Initialize the PIO IO system. This specifies how
             * many and which processors are involved in I/O. */
            if ((ret = PIOc_Init_Intracomm(test_comm, TARGET_NTASKS, ioproc_stride,
                                           ioproc_start, rearranger[r], &iosysid)))
                return ret;

            /* Run tests for each data type. */
            for (int t = 0; t < NUM_TYPES_TO_TEST; t++)
            {
                for (int dc = 0; dc < NUM_DIM_COMBOS_TO_TEST; dc++)
                {
                    /* What is length of map for this combo? */
                    int full_maplen = 1;
                    for (int d = 0; d < NDIM3; d++)
                        full_maplen *= dim_len[dc][d];

                    /* Decompose the data over the tasks. */
                    if ((ret = create_decomposition_3d(TARGET_NTASKS, my_rank, iosysid, dim_len[dc],
                                                       test_type[t], &ioid)))
                        return ret;

                    /* Test decomposition read/write. */
                    if ((ret = test_decomp_read_write(iosysid, ioid, num_flavors, flavor, my_rank,
                                                      rearranger[r], test_comm, dim_len[dc],
                                                      expected_maplen[dc], test_type[t], full_maplen,
                                                      expected_map[dc])))
                        return ret;

                    /* Free the PIO decomposition. */
                    if ((ret = PIOc_freedecomp(iosysid, ioid)))
                        ERR(ret);
                }
            }

            /* Finalize PIO system. */
            if ((ret = PIOc_finalize(iosysid)))
                return ret;

        } /* next rearranger */
    } /* endif my_rank < TARGET_NTASKS */

    /* Finalize the MPI library. */
    if ((ret = pio_test_finalize(&test_comm)))
        return ret;

    printf("%d %s SUCCESS!!\n", my_rank, TEST_NAME);
    return 0;
}
