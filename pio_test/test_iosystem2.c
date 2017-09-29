/*
 * Tests the PIO library with multiple iosysids in use at the
 * same time.
 *
 * This is a simplified, C version of the fortran pio_iosystem_tests2.F90.
 *
 * Ed Hartnett
 */
#include <config.h>
#include <pio.h>
#include <pio_tests.h>

/* The number of tasks this test should run on. */
#define TARGET_NTASKS 4

/* The name of this test. */
#define TEST_NAME "test_iosystem2"

/* Used to define netcdf test file. */
#define PIO_TF_MAX_STR_LEN 100
#define ATTNAME "filename"
#define DIMNAME "filename_dim"

/* This creates a netCDF file in the specified format, with some
 * sample values. */
int create_file(MPI_Comm comm, int iosysid, int format, char *filename,
                char *attname, char *dimname, int my_rank)
{
    int ncid, varid, dimid;
    int ret;

    /* Create the file. */
    if ((ret = PIOc_createfile(iosysid, &ncid, &format, filename, NC_CLOBBER)))
        return ret;

    /* Use the ncid to set the IO system error handler. This function
     * is deprecated. */
    PIOc_Set_File_Error_Handling(ncid, PIO_RETURN_ERROR);
    int method = PIOc_Set_File_Error_Handling(ncid, PIO_RETURN_ERROR);
    if (method != PIO_RETURN_ERROR)
        return ERR_WRONG;

    /* Define a dimension. */
    if ((ret = PIOc_def_dim(ncid, dimname, PIO_TF_MAX_STR_LEN, &dimid)))
        return ret;

    /* Define a 1-D variable. */
    if ((ret = PIOc_def_var(ncid, attname, NC_CHAR, 1, &dimid, &varid)))
        return ret;

    /* Write an attribute. */
    if ((ret = PIOc_put_att_text(ncid, varid, attname, strlen(filename), filename)))
        return ret;

    /* End define mode. */
    if ((ret = PIOc_enddef(ncid)))
        return ret;

    /* Close the file. */
    if ((ret = PIOc_closefile(ncid)))
        return ret;

    return PIO_NOERR;
}

/* This checks an already-open netCDF file. */
int check_file(MPI_Comm comm, int iosysid, int format, int ncid, char *filename,
               char *attname, char *dimname, int my_rank)
{
    int dimid;
    int ret;

    /* Check the file. */
    if ((ret = PIOc_inq_dimid(ncid, dimname, &dimid)))
        return ret;

    return PIO_NOERR;
}

/* This opens and checks a netCDF file. */
int open_and_check_file(MPI_Comm comm, int iosysid, int iotype, int *ncid, char *fname,
                        char *attname, char *dimname, int disable_close, int my_rank)
{
    int mode = PIO_WRITE;
    int ret;

    /* Open the file. */
    if ((ret = PIOc_openfile(iosysid, ncid, &iotype, fname, mode)))
        return ret;

    /* Check the file. */
    if ((ret = check_file(comm, iosysid, iotype, *ncid, fname, attname, dimname, my_rank)))
        return ret;

    /* Close the file, maybe. */
    if (!disable_close)
        if ((ret = PIOc_closefile(*ncid)))
            return ret;

    return PIO_NOERR;
}

/* Run async tests. */
int main(int argc, char **argv)
{
    int my_rank; /* Zero-based rank of processor. */
    int ntasks; /* Number of processors involved in current execution. */
    int iosysid; /* The ID for the parallel I/O system. */
    int iosysid_world; /* The ID for the parallel I/O system. */
    int ret; /* Return code. */
    int num_flavors;
    int iotypes[NUM_FLAVORS];
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
        if ((ret = get_iotypes(&num_flavors, iotypes)))
            ERR(ret);

        /* Split world into odd and even. */
        MPI_Comm newcomm;
        int even = my_rank % 2 ? 0 : 1;
        if ((ret = MPI_Comm_split(test_comm, even, 0, &newcomm)))
            MPIERR(ret);

        /* Get rank in new communicator and its size. */
        int new_rank, new_size;
        if ((ret = MPI_Comm_rank(newcomm, &new_rank)))
            MPIERR(ret);
        if ((ret = MPI_Comm_size(newcomm, &new_size)))
            MPIERR(ret);

        /* Initialize PIO system. */
        if ((ret = PIOc_Init_Intracomm(newcomm, 2, 1, 0, 1, &iosysid)))
            ERR(ret);

        /* This should fail. */
        if (PIOc_finalize(iosysid + TEST_VAL_42) != PIO_EBADID)
            ERR(ERR_WRONG);

        /* Initialize another PIO system. */
        if ((ret = PIOc_Init_Intracomm(test_comm, 4, 1, 0, 1, &iosysid_world)))
            ERR(ret);

        for (int i = 0; i < num_flavors; i++)
        {
            char fname0[] = "pio_iosys_test_file0.nc";
            char fname1[] = "pio_iosys_test_file1.nc";
            char fname2[] = "pio_iosys_test_file2.nc";

            if ((ret = create_file(test_comm, iosysid_world, iotypes[i], fname0, ATTNAME,
                                   DIMNAME, my_rank)))
                ERR(ret);

            if ((ret = create_file(test_comm, iosysid_world, iotypes[i], fname1, ATTNAME,
                                   DIMNAME, my_rank)))
                ERR(ret);

            if ((ret = create_file(test_comm, iosysid_world, iotypes[i], fname2, ATTNAME,
                                   DIMNAME, my_rank)))
                ERR(ret);

            MPI_Barrier(test_comm);

            /* Now check the first file. */
            int ncid;
            if ((ret = open_and_check_file(test_comm, iosysid_world, iotypes[i], &ncid, fname0,
                                           ATTNAME, DIMNAME, 1, my_rank)))
                ERR(ret);

            /* Now have the odd/even communicators each check one of the
             * remaining files. */
            int ncid2;
            char *fname = even ? fname1 : fname2;
            if ((ret = open_and_check_file(newcomm, iosysid, iotypes[i], &ncid2, fname,
                                           ATTNAME, DIMNAME, 1, my_rank)))
                ERR(ret);


            /* Close the still-open files. */
            if ((ret = PIOc_closefile(ncid)))
                ERR(ret);
            if ((ret = PIOc_closefile(ncid2)))
                ERR(ret);
        } /* next iotype */
        if ((ret = MPI_Comm_free(&newcomm)))
            MPIERR(ret);

        /* Finalize PIO system. */
        if ((ret = PIOc_finalize(iosysid)))
            ERR(ret);

        /* Finalize PIO system. */
        if ((ret = PIOc_finalize(iosysid_world)))
            ERR(ret);
    } /* my_rank < TARGET_NTASKS */

    /* Finalize test. */
    if ((ret = pio_test_finalize(&test_comm)))
        return ret;

    printf("%d %s SUCCESS!!\n", my_rank, TEST_NAME);

    return 0;
}
