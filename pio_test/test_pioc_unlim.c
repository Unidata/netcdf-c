/*
 * Tests for PIO Functions. In this test we use a simple 3D variable,
 * with an unlimited dimension. The data will have two timesteps, and
 * 4x4 elements each timestep.
 *
 * @author Ed Hartnett
 * @date 2/14/17
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
#define TEST_NAME "test_pioc_unlim"

/* Number of processors that will do IO. */
#define NUM_IO_PROCS 1

/* Number of computational components to create. */
#define COMPONENT_COUNT 1

/* The number of dimensions in the example data. In this test, we
 * are using three-dimensional data. */
#define NDIM 3

/* The length of our sample data along each dimension. */
#define X_DIM_LEN 4
#define Y_DIM_LEN 4

/* The number of timesteps of data to write. */
#define NUM_TIMESTEPS 2

/* The name of the variable in the netCDF output files. */
#define VAR_NAME "var_3D_with_unlim"

/* The meaning of life, the universe, and everything. */
#define START_DATA_VAL 42

/* The dimension names. */
char dim_name[NDIM][PIO_MAX_NAME + 1] = {"timestep", "x", "y"};

/* Length of the dimensions in the sample data. */
int dim_len[NDIM] = {NC_UNLIMITED, X_DIM_LEN, Y_DIM_LEN};

#define NETCDF4_UNLIM_FILE_NAME "netcdf4_unlim_file.nc"
#define DIM_NAME1 "dim1"
#define DIM_NAME2 "dim2"

/* Create the decomposition to divide the 3-dimensional sample data
 * between the 4 tasks.
 *
 * @param ntasks the number of available tasks
 * @param my_rank rank of this task.
 * @param iosysid the IO system ID.
 * @param dim1_len the length of the dimension.
 * @param ioid a pointer that gets the ID of this decomposition.
 * @returns 0 for success, error code otherwise.
 **/
int create_decomposition(int ntasks, int my_rank, int iosysid, int dim1_len,
                         int *ioid)
{
    PIO_Offset elements_per_pe;     /* Array elements per processing unit. */
    PIO_Offset *compdof;  /* The decomposition mapping. */
    int ret;

    /* How many data elements per task? */
    elements_per_pe = X_DIM_LEN * Y_DIM_LEN / ntasks;

    /* Allocate space for the decomposition array. */
    if (!(compdof = malloc(elements_per_pe * sizeof(PIO_Offset))))
        return PIO_ENOMEM;

    /* Describe the decomposition. This is a 1-based array, so add 1! */
    for (int i = 0; i < elements_per_pe; i++)
        compdof[i] = my_rank * elements_per_pe + i + 1;

    /* Create the PIO decomposition for this test. */
    if ((ret = PIOc_InitDecomp(iosysid, PIO_FLOAT, NDIM - 1, &dim_len[1], elements_per_pe,
                               compdof, ioid, NULL, NULL, NULL)))
        ERR(ret);

    /* Free the mapping. */
    free(compdof);

    return 0;
}

/* Create the test file. */
int create_test_file(int iosysid, int ioid, int iotype, int my_rank, int *ncid, int *varid)
{
    char filename[PIO_MAX_NAME + 1]; /* Name for the output files. */
    int dim_len[NDIM] = {NC_UNLIMITED, X_DIM_LEN, Y_DIM_LEN}; /* Length of the dimensions in the sample data. */
    int dimids[NDIM];      /* The dimension IDs. */
    int ret;       /* Return code. */

    /* Create the filename. */
    sprintf(filename, "%s_iotype_%d.nc", TEST_NAME, iotype);
    
    /* Create the netCDF output file. */
    if ((ret = PIOc_createfile(iosysid, ncid, &iotype, filename, PIO_CLOBBER)))
        ERR(ret);
    
    /* Define netCDF dimensions and variable. */
    for (int d = 0; d < NDIM; d++)
        if ((ret = PIOc_def_dim(*ncid, dim_name[d], (PIO_Offset)dim_len[d], &dimids[d])))
            ERR(ret);
    
    /* Define a variable. */
    if ((ret = PIOc_def_var(*ncid, VAR_NAME, PIO_FLOAT, NDIM, dimids, varid)))
        ERR(ret);
    
    /* End define mode. */
    if ((ret = PIOc_enddef(*ncid)))
        ERR(ret);
    
    return PIO_NOERR;
}

/* Tests with multiple unlimited dims. Only netcdf-4 IOTYPES support
 * multiple unlimited dims. */
int run_multiple_unlim_test(int iosysid, int ioid, int iotype, int my_rank,
                            MPI_Comm test_comm)
{
#define UDIM1_NAME "unlimited1"
#define UDIM2_NAME "unlimited2"
#define NUM_UNLIM_DIMS 2
    int ncid;
    char filename[PIO_MAX_NAME + 1];
    int dimid[NUM_UNLIM_DIMS];
    int nunlimdims;
    int unlimdimids[NUM_UNLIM_DIMS];
    int ret;

    /* Create filename. */
    sprintf(filename, "%s_multiple_unlim_dim_%d.nc", TEST_NAME, iotype);

    /* Create a test file. */
    if ((ret = PIOc_createfile(iosysid, &ncid, &iotype, filename, PIO_CLOBBER)))
        ERR(ret);

    /* Add unlimited dimension. */
    if ((ret = PIOc_def_dim(ncid, UDIM1_NAME, NC_UNLIMITED, &dimid[0])))
        ERR(ret);        

    /* Add another unlimited dimension. */
    if ((ret = PIOc_def_dim(ncid, UDIM2_NAME, NC_UNLIMITED, &dimid[1])))
        ERR(ret);        

    /* Check for correctness. */
    if ((ret = PIOc_inq_unlimdims(ncid, &nunlimdims, unlimdimids)))
        ERR(ret);
    if (nunlimdims != NUM_UNLIM_DIMS)
        ERR(ERR_WRONG);
    for (int d = 0; d < NUM_UNLIM_DIMS; d++)
    {
        if (unlimdimids[d] != dimid[d])
            ERR(ERR_WRONG);
    }

    /* Check some more stuff. */
    {
        int nunlimdims;
        int unlimdimids[NUM_UNLIM_DIMS];
        
        /* These should also work. */
        if ((ret = PIOc_inq_unlimdims(ncid, NULL, NULL)))
            ERR(ret);
        if ((ret = PIOc_inq_unlimdims(ncid, &nunlimdims, NULL)))
            ERR(ret);
        if (nunlimdims != NUM_UNLIM_DIMS)
            ERR(ERR_WRONG);
        if ((ret = PIOc_inq_unlimdims(ncid, NULL, unlimdimids)))
            ERR(ret);
        for (int d = 0; d < NUM_UNLIM_DIMS; d++)
        {
            if (unlimdimids[d] != dimid[d])
                ERR(ERR_WRONG);
        }
    }

    /* Now try to add a var with two unlimited dims. It will fail. */
    int varid;
    #define VAR_NAME2 "some_dumb_variable_name_def_var_will_fail_anyway"
    if (PIOc_def_var(ncid, VAR_NAME2, PIO_INT, NUM_UNLIM_DIMS, unlimdimids,
                     &varid) != PIO_EINVAL)
        ERR(ERR_WRONG);
    
    /* Close the file. */
    if ((PIOc_closefile(ncid)))
        return ret;

    /* Use netCDF-4 directly to create a file that PIO can not create
     * or read. */
    if (my_rank == 0)
    {
        int ncid;
        int dimids[NUM_UNLIM_DIMS];
        int varid;
        
        if ((ret = nc_create(NETCDF4_UNLIM_FILE_NAME, NC_CLOBBER|NC_NETCDF4, &ncid)))
            ERR(ret);
        
        if ((ret = nc_def_dim(ncid, DIM_NAME1, NC_UNLIMITED, &dimids[0])))
            ERR(ret);
        if ((ret = nc_def_dim(ncid, DIM_NAME2, NC_UNLIMITED, &dimids[1])))
            ERR(ret);
        if ((ret = nc_def_var(ncid, VAR_NAME, PIO_INT, NUM_UNLIM_DIMS, dimids, &varid)))
            ERR(ret);
        if ((ret = nc_close(ncid)))
            ERR(ret);
    }

    /* Other tasks wait for task 0 to write file using netCDF-4... */
    MPI_Barrier(test_comm);

    /* Try to read file. It will not work. */
    if (PIOc_openfile2(iosysid, &ncid, &iotype, NETCDF4_UNLIM_FILE_NAME,
                       0) != PIO_EINVAL)
        ERR(ERR_WRONG);

    return PIO_NOERR;
}

/* Run all the tests. */
int test_all(int iosysid, int num_flavors, int *flavor, int my_rank, MPI_Comm test_comm,
             int async)
{
    int ioid;
    int ncid;
    int varid;
    int my_test_size;
    char filename[NC_MAX_NAME + 1];
    int ret; /* Return code. */

    if ((ret = MPI_Comm_size(test_comm, &my_test_size)))
        MPIERR(ret);
    
    /* This will be our file name for writing out decompositions. */
    sprintf(filename, "decomp_%d.txt", my_rank);

    if (!async)
    {
        /* Decompose the data over the tasks. */
        if ((ret = create_decomposition(my_test_size, my_rank, iosysid, X_DIM_LEN, &ioid)))
            return ret;

        /* Use PIO to create the example file in each of the four
         * available ways. */
        for (int fmt = 0; fmt < num_flavors; fmt++)
        {
            if ((ret = create_test_file(iosysid, ioid, flavor[fmt], my_rank, &ncid, &varid)))
                return ret;
            if ((ret = PIOc_setframe(ncid, varid, 0)))
                return ret;
            if ((ret = PIOc_advanceframe(ncid, varid)))
                return ret;

            /* Look at the internals to check that the frame commands
             * worked. */
            file_desc_t *file;
            var_desc_t *vdesc;     /* Contains info about the variable. */

            if ((ret = pio_get_file(ncid, &file)))
                return ret;
            if ((ret = get_var_desc(varid, &file->varlist, &vdesc)))
                return ret;
            if (vdesc->record != 1)
                return ERR_WRONG;
            
            if ((PIOc_closefile(ncid)))
                return ret;

            /* Test file with multiple unlimited dims. Only netCDF-4
             * iotypes can run this test. */
            if (flavor[fmt] == PIO_IOTYPE_NETCDF4C || flavor[fmt] == PIO_IOTYPE_NETCDF4P)
                if ((ret = run_multiple_unlim_test(iosysid, ioid, flavor[fmt], my_rank, test_comm)))
                    return ret;
        }

        /* Free the PIO decomposition. */
        if ((ret = PIOc_freedecomp(iosysid, ioid)))
            ERR(ret);
    }

    return PIO_NOERR;
}

/* Run Tests for NetCDF-4 Functions. */
int main(int argc, char **argv)
{
    /* Change the 5th arg to 3 to turn on logging. */
    return run_test_main(argc, argv, MIN_NTASKS, TARGET_NTASKS, -1,
                         TEST_NAME, dim_len, COMPONENT_COUNT, NUM_IO_PROCS);
}
