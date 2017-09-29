/*
 * This program tests some internal functions in the PIO library.
 *
 * @author Jim Edwards, Ed Hartnett
 * @date 11/23/16
 */
#include <config.h>
#include <pio.h>
#include <pio_tests.h>
#include <pio_internal.h>

/* The number of tasks this test should run on. */
#define TARGET_NTASKS 4

/* The minimum number of tasks this test should run on. */
#define MIN_NTASKS 1

/* The name of this test. */
#define TEST_NAME "test_spmd"

/* Number of test cases in inner loop of test. */
#define NUM_TEST_CASES 5

/* Test MPI_Alltoallw by having processor i send different amounts of
 * data to each processor.  The first test sends i items to processor
 * i from all processors. */
int run_spmd_tests(MPI_Comm test_comm)
{
    int my_rank;  /* 0-based rank in test_comm. */
    int ntasks;   /* Number of tasks in test_comm. */
    int num_elem; /* Number of elements in buffers. */
    int type_size; /* Size in bytes of an element. */
    int mpierr;   /* Return value from MPI calls. */
    int ret;      /* Return value. */

    /* Learn rank and size. */
    if ((mpierr = MPI_Comm_size(test_comm, &ntasks)))
        MPIERR(mpierr);
    if ((mpierr = MPI_Comm_rank(test_comm, &my_rank)))
        MPIERR(mpierr);

    /* Determine size of buffers. */
    num_elem = ntasks;

    int sbuf[ntasks];    /* The send buffer. */
    int rbuf[ntasks];    /* The receive buffer. */
    int sendcounts[ntasks]; /* Number of elements of data being sent from each task. */
    int recvcounts[ntasks]; /* Number of elements of data being sent from each task. */
    int sdispls[ntasks]; /* Displacements for sending data. */
    int rdispls[ntasks]; /* Displacements for receiving data. */
    MPI_Datatype sendtypes[ntasks]; /* MPI types of data being sent. */
    MPI_Datatype recvtypes[ntasks]; /* MPI types of data being received. */

    /* Load up the send buffer. */
    for (int i = 0; i < num_elem; i++)
        sbuf[i] = my_rank;

    /* Load up the receive buffer to make debugging easier. */
    for (int i = 0; i < num_elem; i++)
        rbuf[i] = -999;

    /* Get the size of the int type for MPI. (Should always be 4.) */
    if ((mpierr = MPI_Type_size(MPI_INT, &type_size)))
        return check_mpi(NULL, mpierr, __FILE__, __LINE__);
    assert(type_size == sizeof(int));

    /* Initialize the arrays. */
    for (int i = 0; i < ntasks; i++)
    {
        sendcounts[i] = 1;
        sdispls[i] = 0;
        sendtypes[i] = MPI_INT;
        recvcounts[i] = 1;
        rdispls[i] = i * type_size;
        recvtypes[i] = MPI_INT;
    }

    /* Perform tests for different values of msg_cnt. (BTW it hangs
     * with msg_cnt = 1!). */
    for (int msg_cnt = 0; msg_cnt < TARGET_NTASKS; msg_cnt = msg_cnt ? msg_cnt * 2 : 4)
    {
        for (int itest = 0; itest < NUM_TEST_CASES; itest++)
        {
            rearr_comm_fc_opt_t fc;
            fc.hs = false;
            fc.isend = false;

            /* Wait for all tasks. */
            MPI_Barrier(test_comm);

            /* Set the parameters different for each test case. */
            if (itest == 1)
            {
                fc.hs = true;
                fc.isend = true;
            }
            else if (itest == 2)
            {
                fc.hs = false;
                fc.isend = true;
            }
            else if (itest == 3)
            {
                fc.hs = false;
                fc.isend = false;
            }
            else if (itest == 4)
            {
                fc.hs = true;
                fc.isend = false;
            }

            /* Run the swapm function. */
            if ((ret = pio_swapm(sbuf, sendcounts, sdispls, sendtypes, rbuf, recvcounts,
                                 rdispls, recvtypes, test_comm, &fc)))
                return ret;

            /* Print results. */
            /* MPI_Barrier(test_comm); */
            /* for (int e = 0; e < num_elem; e++) */
            /*     printf("%d sbuf[%d] = %d\n", my_rank, e, sbuf[e]); */
            /* MPI_Barrier(test_comm); */
            /* for (int e = 0; e < num_elem; e++) */
            /*     printf("%d rbuf[%d] = %d\n", my_rank, e, rbuf[e]); */

            /* Check that rbuf has 0, 1, ..., ntasks-1. */
            for (int e = 0; e < num_elem; e++)
                if (((int *)rbuf)[e] != e)
                    return ERR_WRONG;
        }
    }

    return 0;
}

/* Test some of the functions in the file pioc_sc.c.
 *
 * @param test_comm the MPI communicator that the test code is running on.
 * @returns 0 for success, error code otherwise.
 */
int run_sc_tests(MPI_Comm test_comm)
{
#define SC_ARRAY_LEN 3
    int my_rank;  /* 0-based rank in test_comm. */
    int ntasks;   /* Number of tasks in test_comm. */
    int mpierr;   /* Return value from MPI calls. */
    long long larray1[SC_ARRAY_LEN] = {7, 42, 14};
    long long larray2[SC_ARRAY_LEN] = {2, 3, 7};
    long long larray3[SC_ARRAY_LEN] = {90, 180, 270};
    long long larray4[SC_ARRAY_LEN] = {1, 180, 270};

    /* Learn rank and size. */
    if ((mpierr = MPI_Comm_size(test_comm, &ntasks)))
        MPIERR(mpierr);
    if ((mpierr = MPI_Comm_rank(test_comm, &my_rank)))
        MPIERR(mpierr);

    /* Test the gcd() function. */
    if (gcd(0, 2) != 2)
        return ERR_WRONG;
    if (gcd(2, 2) != 2)
        return ERR_WRONG;
    if (gcd(42, 2) != 2)
        return ERR_WRONG;

    /* Test the long long version. */
    if (lgcd(0, 2) != 2)
        return ERR_WRONG;
    if (lgcd(2, 2) != 2)
        return ERR_WRONG;
    if (lgcd(42, 2) != 2)
        return ERR_WRONG;

    /* Test the gcd_array() function. */
    if (lgcd_array(SC_ARRAY_LEN, larray1) != 7)
        return ERR_WRONG;
    if (lgcd_array(SC_ARRAY_LEN, larray2) != 1)
        return ERR_WRONG;
    if (lgcd_array(SC_ARRAY_LEN, larray3) != 90)
        return ERR_WRONG;
    if (lgcd_array(SC_ARRAY_LEN, larray4) != 1)
        return ERR_WRONG;

    /* Test compute_one_dim. */
    PIO_Offset start, count;
    compute_one_dim(4, 4, my_rank, &start, &count);
    if (start != my_rank || count != 1)
        return ERR_WRONG;
    compute_one_dim(400, 4, my_rank, &start, &count);
    if (start != my_rank * 100 || count != 100)
        return ERR_WRONG;
    /* Left over data will go to task 3. */
    compute_one_dim(5, 4, my_rank, &start, &count);
    if (start != my_rank || count != (my_rank == 3 ? 2 : 1))
        return ERR_WRONG;
    return 0;
}

/* Test some list stuff. */
int test_lists()
{
    file_desc_t *fdesc;

    /* Test that bad input is correctly rejected. */
    if (pio_delete_iodesc_from_list(42) != PIO_EBADID)
        return ERR_WRONG;
    if (pio_delete_iosystem_from_list(42) != PIO_EBADID)
        return ERR_WRONG;
    if (pio_delete_file_from_list(42) != PIO_EBADID)
        return ERR_WRONG;
    if (pio_get_file(42, NULL) != PIO_EINVAL)
        return ERR_WRONG;
    if (pio_get_file(42, &fdesc) != PIO_EBADID)
        return ERR_WRONG;
    return 0;
}

/* Test some list stuff. */
int test_determine_procs()
{
#define ONE_COMPONENT 1
#define TWO_COMPONENTS 2
#define THREE_PROCS 3
    int ret;
    
    {
        int num_io_procs = 1;
        int component_count = ONE_COMPONENT;
        int num_procs_per_comp[ONE_COMPONENT] = {1};
        int *my_proc_list[ONE_COMPONENT];
        
        if ((ret = determine_procs(num_io_procs, component_count, num_procs_per_comp, NULL,
                                   my_proc_list)))
            return ret;

        /* Check results and free resources. */
        for (int c = 0; c < ONE_COMPONENT; c++)
        {
            if (my_proc_list[c][0] != 1)
                return ERR_WRONG;
            free(my_proc_list[c]);
        }
    }
    
    {
        int num_io_procs = 3;
        int component_count = TWO_COMPONENTS;
        int num_procs_per_comp[TWO_COMPONENTS] = {1, 1};
        int *my_proc_list[TWO_COMPONENTS];
        
        if ((ret = determine_procs(num_io_procs, component_count, num_procs_per_comp, NULL,
                                   my_proc_list)))
            return ret;
        
        /* Check results and free resources. */
        for (int c = 0; c < TWO_COMPONENTS; c++)
        {
            if (my_proc_list[c][0] != c + 3)
                return ERR_WRONG;
            free(my_proc_list[c]);
        }
    }
    
    {
        int num_io_procs = 3;
        int component_count = TWO_COMPONENTS;
        int num_procs_per_comp[TWO_COMPONENTS] = {THREE_PROCS, THREE_PROCS};
        int *my_proc_list[TWO_COMPONENTS];
        
        if ((ret = determine_procs(num_io_procs, component_count, num_procs_per_comp, NULL,
                                   my_proc_list)))
            return ret;
        
        /* Check results and free resources. */
        for (int c = 0; c < TWO_COMPONENTS; c++)
        {
            for (int p = 0; p < THREE_PROCS; p++)
                if (my_proc_list[c][p] != 3 + c * THREE_PROCS + p)
                    return ERR_WRONG;
            free(my_proc_list[c]);
        }
    }
    
    {
        int num_io_procs = 3;
        int component_count = TWO_COMPONENTS;
        int num_procs_per_comp[TWO_COMPONENTS] = {THREE_PROCS, THREE_PROCS};
        int proc_list_1[THREE_PROCS] = {8, 9, 10};
        int proc_list_2[THREE_PROCS] = {11, 12, 13};
        int *proc_list[TWO_COMPONENTS] = {proc_list_1, proc_list_2};
        int *my_proc_list[TWO_COMPONENTS];
        
        if ((ret = determine_procs(num_io_procs, component_count, num_procs_per_comp,
                                   (int **)proc_list, my_proc_list)))
            return ret;
        
        /* Check results and free resources. */
        for (int c = 0; c < TWO_COMPONENTS; c++)
        {
            for (int p = 0; p < THREE_PROCS; p++)
                if (my_proc_list[c][p] != proc_list[c][p])
                    return ERR_WRONG;
            free(my_proc_list[c]);
        }
    }
    
    return PIO_NOERR;
}

/*
 * Test some list stuff for varlists.
 *
 * @author Ed Hartnett
 */
int test_varlists()
{
    var_desc_t *varlist = NULL;
    var_desc_t *var_desc;
    int ret;

    /* Try to delete a non-existing var. */
    if (delete_var_desc(2, &varlist) != PIO_ENOTVAR)
        return ERR_WRONG;

    /* Add a var to the list. */
    if ((ret = add_to_varlist(0, 1, PIO_INT, 4, MPI_INT, 4, &varlist)))
        return ret;

    /* Find that var_desc_t. */
    if ((ret = get_var_desc(0, &varlist, &var_desc)))
        return ret;
    if (var_desc->varid != 0 || !var_desc->rec_var || var_desc->pio_type != PIO_INT ||
        var_desc->pio_type_size != 4 || var_desc->mpi_type != MPI_INT || var_desc->mpi_type_size != 4)
        return ERR_WRONG;

    /* Try to delete a non-existing var - should fail. */
    if (delete_var_desc(2, &varlist) != PIO_ENOTVAR)
        return ERR_WRONG;

    /* Delete it. */
    if ((ret = delete_var_desc(0, &varlist)))
        return ret;

    /* Make sure it is gone. */
    if (get_var_desc(0, &varlist, &var_desc) != PIO_ENOTVAR)
        return ERR_WRONG;

    return 0;
}

/*
 * Test some more list stuff for varlists.
 *
 * @author Ed Hartnett
 */
int test_varlists2()
{
    var_desc_t *varlist = NULL;
    var_desc_t *var_desc;
    int ret;

    /* Add some vars to the list. */
    if ((ret = add_to_varlist(0, 1, PIO_INT, 4, MPI_INT, 4, &varlist)))
        return ret;
    if ((ret = add_to_varlist(1, 0, PIO_DOUBLE, 8, MPI_DOUBLE, 8, &varlist)))
        return ret;
    if ((ret = add_to_varlist(2, 1, PIO_BYTE, 1, MPI_CHAR, 1, &varlist)))
        return ret;

    /* Find those var_desc_t. */
    if ((ret = get_var_desc(0, &varlist, &var_desc)))
        return ret;
    if (var_desc->varid != 0 || !var_desc->rec_var || var_desc->pio_type != PIO_INT ||
        var_desc->pio_type_size != 4 || var_desc->mpi_type != MPI_INT)
        return ERR_WRONG;

    if ((ret = get_var_desc(1, &varlist, &var_desc)))
        return ret;
    if (var_desc->varid != 1 || var_desc->rec_var || var_desc->pio_type != PIO_DOUBLE ||
        var_desc->pio_type_size != 8)
        return ERR_WRONG;

    if ((ret = get_var_desc(2, &varlist, &var_desc)))
        return ret;
    if (var_desc->varid != 2 || !var_desc->rec_var || var_desc->pio_type != PIO_BYTE ||
        var_desc->pio_type_size != 1)
        return ERR_WRONG;

    /* Try to delete a non-existing var - should fail. */
    if (delete_var_desc(3, &varlist) != PIO_ENOTVAR)
        return ERR_WRONG;

    /* Delete one of the vars. */
    if ((ret = delete_var_desc(0, &varlist)))
        return ret;

    /* Make sure it is gone. */
    if (get_var_desc(0, &varlist, &var_desc) != PIO_ENOTVAR)
        return ERR_WRONG;

    /* Make sure the others are still there. */
    var_desc = NULL;
    if ((ret = get_var_desc(1, &varlist, &var_desc)))
        return ret;
    if (var_desc->varid != 1 || var_desc->rec_var)
        return ERR_WRONG;

    var_desc = NULL;
    if ((ret = get_var_desc(2, &varlist, &var_desc)))
        return ret;
    if (var_desc->varid != 2 || !var_desc->rec_var)
        return ERR_WRONG;

    /* Delete the other two vars from the varlist. */
    if ((ret = delete_var_desc(1, &varlist)))
        return ret;
    if ((ret = delete_var_desc(2, &varlist)))
        return ret;

    return 0;
}

/*
 * Test even more list stuff for varlists.
 *
 * @author Ed Hartnett
 */
int test_varlists3()
{
    var_desc_t *varlist = NULL;
    var_desc_t *var_desc;
    int ret;

    /* Add some vars to the list. */
    if ((ret = add_to_varlist(0, 1, PIO_INT, 4, MPI_INT, 4, &varlist)))
        return ret;
    if ((ret = add_to_varlist(1, 0, PIO_INT, 4, MPI_INT, 4, &varlist)))
        return ret;
    if ((ret = add_to_varlist(2, 1, PIO_INT, 4, MPI_INT, 4, &varlist)))
        return ret;
    if ((ret = add_to_varlist(3, 0, PIO_INT, 4, MPI_INT, 4, &varlist)))
        return ret;

    /* Delete one of the vars. */
    if ((ret = delete_var_desc(1, &varlist)))
        return ret;

    /* Make sure it is gone. */
    if (get_var_desc(1, &varlist, &var_desc) != PIO_ENOTVAR)
        return ERR_WRONG;

    /* Make sure the others are still there. */
    var_desc = NULL;
    if ((ret = get_var_desc(0, &varlist, &var_desc)))
        return ret;
    if (var_desc->varid != 0 || !var_desc->rec_var)
        return ERR_WRONG;

    var_desc = NULL;
    if ((ret = get_var_desc(2, &varlist, &var_desc)))
        return ret;
    if (var_desc->varid != 2 || !var_desc->rec_var)
        return ERR_WRONG;

    var_desc = NULL;
    if ((ret = get_var_desc(3, &varlist, &var_desc)))
        return ret;
    if (var_desc->varid != 3 || var_desc->rec_var)
        return ERR_WRONG;

    /* Delete the other vars from the varlist. */
    if ((ret = delete_var_desc(0, &varlist)))
        return ret;
    if ((ret = delete_var_desc(2, &varlist)))
        return ret;
    if ((ret = delete_var_desc(3, &varlist)))
        return ret;

    /* Make sure they are gone. */
    if (get_var_desc(0, &varlist, &var_desc) != PIO_ENOTVAR)
        return ERR_WRONG;
    if (get_var_desc(2, &varlist, &var_desc) != PIO_ENOTVAR)
        return ERR_WRONG;
    if (get_var_desc(3, &varlist, &var_desc) != PIO_ENOTVAR)
        return ERR_WRONG;
    
    return 0;
}

/* Test the ceil2() and pair() functions. */
int test_ceil2_pair()
{
    /* Test the ceil2() function. */
    if (ceil2(1) != 1)
        return ERR_WRONG;
    if (ceil2(-100) != 1)
        return ERR_WRONG;
    if (ceil2(2) != 2)
        return ERR_WRONG;
    if (ceil2(3) != 4)
        return ERR_WRONG;
    if (ceil2(16) != 16)
        return ERR_WRONG;
    if (ceil2(17) != 32)
        return ERR_WRONG;

    /* Test the pair() function. */
    if (pair(4, 0, 0) != 1)
        return ERR_WRONG;
    if (pair(4, 2, 2) != 1)
        return ERR_WRONG;

    return 0;
}

/* Test the function that finds an MPI type to match a PIO type. */
int test_find_mpi_type()
{
    MPI_Datatype mpi_type;
    int type_size;
    int ret;

    /* This should not work. */
    if (find_mpi_type(PIO_BYTE + 42, &mpi_type, &type_size) != PIO_EBADTYPE)
        return ERR_WRONG;

    /* Try every atomic type. */
    if ((ret = find_mpi_type(PIO_BYTE, &mpi_type, &type_size)))
        return ret;
    if (mpi_type != MPI_BYTE || type_size != 1)
        return ERR_WRONG;

    if ((ret = find_mpi_type(PIO_CHAR, &mpi_type, &type_size)))
        return ret;
    if (mpi_type != MPI_CHAR || type_size != 1)
        return ERR_WRONG;

    if ((ret = find_mpi_type(PIO_SHORT, &mpi_type, &type_size)))
        return ret;
    if (mpi_type != MPI_SHORT || type_size != 2)
        return ERR_WRONG;

    if ((ret = find_mpi_type(PIO_INT, &mpi_type, &type_size)))
        return ret;
    if (mpi_type != MPI_INT || type_size != 4)
        return ERR_WRONG;

    if ((ret = find_mpi_type(PIO_FLOAT, &mpi_type, &type_size)))
        return ret;
    if (mpi_type != MPI_FLOAT || type_size != 4)
        return ERR_WRONG;

    if ((ret = find_mpi_type(PIO_DOUBLE, &mpi_type, &type_size)))
        return ret;
    if (mpi_type != MPI_DOUBLE || type_size != 8)
        return ERR_WRONG;

    /* These should also work. */
    if ((ret = find_mpi_type(PIO_INT, &mpi_type, NULL)))
        return ret;
    if (mpi_type != MPI_INT)
        return ERR_WRONG;
    if ((ret = find_mpi_type(PIO_INT, NULL, &type_size)))
        return ret;
    if (type_size != 4)
        return ERR_WRONG;
    if ((ret = find_mpi_type(PIO_INT, NULL, NULL)))
        return ret;

#ifdef _NETCDF4
    if ((ret = find_mpi_type(PIO_UBYTE, &mpi_type, &type_size)))
        return ret;
    if (mpi_type != MPI_UNSIGNED_CHAR || type_size != 1)
        return ERR_WRONG;

    if ((ret = find_mpi_type(PIO_USHORT, &mpi_type, &type_size)))
        return ret;
    if (mpi_type != MPI_UNSIGNED_SHORT || type_size != 2)
        return ERR_WRONG;

    if ((ret = find_mpi_type(PIO_UINT, &mpi_type, &type_size)))
        return ret;
    if (mpi_type != MPI_UNSIGNED || type_size != 4)
        return ERR_WRONG;

    if ((ret = find_mpi_type(PIO_INT64, &mpi_type, &type_size)))
        return ret;
    if (mpi_type != MPI_LONG_LONG || type_size != 8)
        return ERR_WRONG;

    if ((ret = find_mpi_type(PIO_UINT64, &mpi_type, &type_size)))
        return ret;
    if (mpi_type != MPI_UNSIGNED_LONG_LONG || type_size != 8)
        return ERR_WRONG;

    if ((ret = find_mpi_type(PIO_STRING, &mpi_type, &type_size)))
        return ret;
    if (mpi_type != MPI_CHAR || type_size != 1)
        return ERR_WRONG;

#endif /* _NETCDF4 */
    return PIO_NOERR;
}

int test_misc()
{
    wmulti_buffer wmb;

    /* This should not work. */
    if (flush_buffer(TEST_VAL_42, &wmb, 0) != PIO_EBADID)
        return ERR_WRONG;

    return 0;
}

/* This test code was recovered from main() in pioc_sc.c. */
int test_CalcStartandCount()
{
    int ndims = 2;
    int gdims[2] = {31, 777602};
    int num_io_procs = 24;
    bool converged = false;
    PIO_Offset start[ndims], kount[ndims];
    int iorank, numaiotasks = 0;
    long int tpsize = 0;
    long int psize;
    long int pgdims = 1;
    int scnt;
    int ret;

    for (int i = 0; i < ndims; i++)
        pgdims *= gdims[i];

    while (!converged)
    {
        for (iorank = 0; iorank < num_io_procs; iorank++)
        {
            if ((ret = CalcStartandCount(PIO_DOUBLE, ndims, gdims, num_io_procs, iorank,
                                         start, kount, &numaiotasks)))
                return ret;

            if (numaiotasks < 0)
                return numaiotasks;

            psize = 1;
            scnt = 0;
            for (int i = 0; i < ndims; i++)
            {
                psize *= kount[i];
                scnt += kount[i];
            }
            tpsize += psize;
        }

        if (tpsize == pgdims)
            converged = true;
        else
        {
            tpsize = 0;
            num_io_procs--;
        }
    }

    return 0;
}

/* Test the GDCblocksize() function. */
int run_GDCblocksize_tests(MPI_Comm test_comm)
{
    {
        int arrlen = 1;
        PIO_Offset arr_in[1] = {0};
        PIO_Offset blocksize;

        blocksize = GCDblocksize(arrlen, arr_in);
        if (blocksize != 1)
            return ERR_WRONG;
    }

    {
        int arrlen = 4;
        PIO_Offset arr_in[4] = {0, 1, 2, 3};
        PIO_Offset blocksize;

        blocksize = GCDblocksize(arrlen, arr_in);
        if (blocksize != 4)
            return ERR_WRONG;
    }

    {
        int arrlen = 4;
        PIO_Offset arr_in[4] = {0, 2, 3, 4};
        PIO_Offset blocksize;

        blocksize = GCDblocksize(arrlen, arr_in);
        if (blocksize != 1)
            return ERR_WRONG;
    }

    {
        int arrlen = 4;
        PIO_Offset arr_in[4] = {0, 1, 3, 4};
        PIO_Offset blocksize;

        blocksize = GCDblocksize(arrlen, arr_in);
        if (blocksize != 1)
            return ERR_WRONG;
    }

    {
        int arrlen = 4;
        PIO_Offset arr_in[4] = {0, 1, 2, 4};
        PIO_Offset blocksize;

        blocksize = GCDblocksize(arrlen, arr_in);
        if (blocksize != 1)
            return ERR_WRONG;
    }

    return 0;
}

/* Run Tests for pio_spmd.c functions. */
int main(int argc, char **argv)
{
    int my_rank; /* Zero-based rank of processor. */
    int ntasks;  /* Number of processors involved in current execution. */
    int ret;     /* Return code. */
    MPI_Comm test_comm; /* A communicator for this test. */

    /* Initialize test. */
    if ((ret = pio_test_init2(argc, argv, &my_rank, &ntasks, MIN_NTASKS,
                              TARGET_NTASKS, -1, &test_comm)))
        ERR(ERR_INIT);

    /* Test code runs on TARGET_NTASKS tasks. The left over tasks do
     * nothing. */
    if (my_rank < TARGET_NTASKS)
    {
        /* I don't need this iosystem, but it's the only way to get
         * the logs to write. */
        int iosysid;
        if ((ret = PIOc_Init_Intracomm(test_comm, TARGET_NTASKS, 1, 0, PIO_REARR_BOX, &iosysid)))
            return ret;

        if ((ret = run_sc_tests(test_comm)))
            return ret;

        if ((ret = run_GDCblocksize_tests(test_comm)))
            return ret;

        if ((ret = run_spmd_tests(test_comm)))
            return ret;

        if ((ret = test_CalcStartandCount()))
            return ret;

        if ((ret = test_lists()))
            return ret;

        if ((ret = test_determine_procs()))
            return ret;

        if ((ret = test_varlists()))
            return ret;

        if ((ret = test_varlists2()))
            return ret;

        if ((ret = test_varlists3()))
            return ret;

        if ((ret = test_ceil2_pair()))
            return ret;

        if ((ret = test_find_mpi_type()))
            return ret;

        if ((ret = test_misc()))
            return ret;

        /* Finalize PIO system. */
        if ((ret = PIOc_finalize(iosysid)))
            return ret;

    } /* endif my_rank < TARGET_NTASKS */

    /* Finalize the MPI library. */
    if ((ret = pio_test_finalize(&test_comm)))
        return ret;

    printf("%d %s SUCCESS!!\n", my_rank, TEST_NAME);

    return 0;
}
