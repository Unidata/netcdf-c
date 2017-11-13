/**
 * @file
 * Compute start and count arrays for the box rearranger
 *
 * @author Jim Edwards
 * @date  2014
 */
#include <config.h>
#include <pio.h>
#include <pio_internal.h>

/** The default target blocksize for each io task when the box
 * rearranger is used. */
#define DEFAULT_BLOCKSIZE 1024

/** The target blocksize for each io task when the box rearranger is
 * used. */
int blocksize = DEFAULT_BLOCKSIZE;

/**
 * Recursive Standard C Function: Greatest Common Divisor.
 *
 * @param a
 * @param b
 * @returns greates common divisor.
 * @author Jim Edwards
 */
int gcd(int a, int b )
{
    if (a == 0)
        return b;
    return gcd(b % a, a);
}

/**
 * Recursive Standard C Function: Greatest Common Divisor for 64 bit
 * ints.
 *
 * @param a
 * @param b
 * @returns greates common divisor.
 * @author Jim Edwards
 */
long long lgcd(long long a, long long b)
{
    if (a == 0)
        return b;
    return lgcd(b % a, a);
}

/**
 * Return the greatest common devisor of array ain as int_64.
 *
 * @param nain number of elements in ain.
 * @param ain array of length nain.
 * @returns GCD of elements in ain.
 * @author Jim Edwards
 */
long long lgcd_array(int nain, long long *ain)
{
    int i;
    long long bsize = 1;

    for (i = 0; i < nain; i++)
        if(ain[i] <= 1)
            return bsize;

    bsize = ain[0];
    i = 1;
    while (i < nain && bsize > 1)
    {
        bsize = gcd(bsize, ain[i]);
        i++;
    }

    return bsize;
}

/**
 * Compute one element (dimension) of start and count arrays. This
 * function is used by CalcStartandCount().
 *
 * @param gdim global size of one dimension.
 * @param ioprocs number of io tasks.
 * @param rank IO rank of this task.
 * @param start pointer to PIO_Offset that will get the start value.
 * @param count pointer to PIO_Offset that will get the count value.
 * @author Jim Edwards
 */
void compute_one_dim(int gdim, int ioprocs, int rank, PIO_Offset *start,
                     PIO_Offset *count)
{
    int irank;     /* The IO rank for this task. */
    int remainder;
    int adds;
    PIO_Offset lstart, lcount;

    /* Check inputs. */
    pioassert(gdim >= 0 && ioprocs > 0 && rank >= 0 && start && count,
              "invalid input", __FILE__, __LINE__);

    /* Determin which IO task to use. */
    irank = rank % ioprocs;

    /* Each IO task will have its share of the global dim. */
    lcount = (long int)(gdim / ioprocs);

    /* Find the start for this task. */
    lstart = (long int)(lcount * irank);

    /* Is there anything left over? */
    remainder = gdim - lcount * ioprocs;

    /* Distribute left over data to some IO tasks. */
    if (remainder >= ioprocs - irank)
    {
        lcount++;
        if ((adds = irank + remainder - ioprocs) > 0)
            lstart += adds;
    }

    /* Return results to caller. */
    *start = lstart;
    *count = lcount;
}

/**
 * Look for the largest block of data for io which can be expressed in
 * terms of start and count.
 *
 * @param arrlen
 * @param arr_in
 * @returns the size of the block
 * @author Jim Edwards
 */
PIO_Offset GCDblocksize(int arrlen, const PIO_Offset *arr_in)
{
    int numblks = 0;  /* Number of blocks. */
    int numtimes = 0; /* Number of times adjacent arr_in elements differ by != 1. */
    int numgaps = 0;  /* Number of gaps. */
    int j;  /* Loop counter. */
    int ii; /* Loop counter. */
    int n;
    PIO_Offset bsize;     /* Size of the block. */
    PIO_Offset bsizeg;    /* Size of gap block. */
    PIO_Offset blklensum; /* Sum of all block lengths. */
    PIO_Offset del_arr[arrlen - 1]; /* Array of deltas between adjacent elements in arr_in. */
    PIO_Offset loc_arr[arrlen - 1];

    /* Check inputs. */
    pioassert(arrlen > 0 && arr_in, "invalid input", __FILE__, __LINE__);

    /* Count the number of contiguous blocks in arr_in. If any if
       these blocks is of size 1, we are done and can return.
       Otherwise numtimes is the number of blocks. */
    for (int i = 0; i < arrlen - 1; i++)
    {
        del_arr[i] = arr_in[i + 1] - arr_in[i];
        if (del_arr[i] != 1)
        {
            numtimes++;
            if ( i > 0 && del_arr[i - 1] > 1)
                return(1);
        }
    }

    /* If numtimes is 0 the all of the data in arr_in is contiguous
     * and numblks=1. Not sure why I have three different variables
     * here, seems like n,numblks and numtimes could be combined. */
    numblks = numtimes + 1;
    if (numtimes == 0)
        n = numblks;
    else
        n = numtimes;

    /* If numblks==1 then the result is arrlen and you can return. */
    bsize = (PIO_Offset)arrlen;
    if (numblks > 1)
    {
        PIO_Offset blk_len[numblks];
        PIO_Offset gaps[numtimes];

        /* If numblks > 1 then numtimes must be > 0 and this if block
         * isn't needed. */
        if (numtimes > 0)
        {
            ii = 0;
            for (int i = 0; i < arrlen - 1; i++)
                if (del_arr[i] > 1)
                    gaps[ii++] = del_arr[i] - 1;
            numgaps = ii;
        }

        j = 0;
        for (int i = 0; i < n; i++)
            loc_arr[i] = 1;

        for (int i = 0; i < arrlen - 1; i++)
            if(del_arr[i] != 1)
                loc_arr[j++] = i;

        blk_len[0] = loc_arr[0];
        blklensum = blk_len[0];
        for(int i = 1; i < numblks - 1; i++)
        {
            blk_len[i] = loc_arr[i] - loc_arr[i - 1];
            blklensum += blk_len[i];
        }
        blk_len[numblks - 1] = arrlen - blklensum;

        /* Get the GCD in blk_len array. */
        bsize = lgcd_array(numblks, blk_len);

        /* I don't recall why i needed these next two blocks, I
         * remember struggling to get this right in all cases and I'm
         * afraid that the end result is that bsize is almost always
         * 1. */
        if (numgaps > 0)
        {
            bsizeg = lgcd_array(numgaps, gaps);
            bsize = lgcd(bsize, bsizeg);
        }

        /* ??? */
        if (arr_in[0] > 0)
            bsize = lgcd(bsize, arr_in[0]);
    }

    return bsize;
}

/**
 * Compute start and count values for each io task. This is used in
 * PIOc_InitDecomp() for the box rearranger only.
 *
 * @param pio_type the PIO data type used in this decompotion.
 * @param ndims the number of dimensions in the variable, not
 * including the unlimited dimension.
 * @param gdims an array of global size of each dimension.
 * @param num_io_procs the number of IO tasks.
 * @param myiorank rank of this task in IO communicator.
 * @param start array of length ndims with data start values.
 * @param count array of length ndims with data count values.
 * @param num_aiotasks the number of IO tasks used(?)
 * @returns 0 for success, error code otherwise.
 * @author Jim Edwards
 */
int CalcStartandCount(int pio_type, int ndims, const int *gdims, int num_io_procs,
                      int myiorank, PIO_Offset *start, PIO_Offset *count, int *num_aiotasks)
{
    int minbytes;
    int maxbytes;
    int minblocksize; /* Like minbytes, but in data elements. */
    int basesize;     /* Size in bytes of base data type. */
    int use_io_procs;
    int i;
    long int pgdims;
    bool converged;
    int iorank;
    int ldims;
    int tiorank;
    int ioprocs;
    int tioprocs;
    int mystart[ndims], mycount[ndims];
    long int pknt;
    long int tpsize = 0;
    int ret;

    /* Check inputs. */
    pioassert(pio_type > 0 && ndims > 0 && gdims && num_io_procs > 0 && start && count,
              "invalid input", __FILE__, __LINE__);
    LOG((1, "CalcStartandCount pio_type = %d ndims = %d num_io_procs = %d myiorank = %d",
         pio_type, ndims, num_io_procs, myiorank));

    /* We are trying to find start and count indices for each iotask
     * such that each task has approximately blocksize data to write
     * (read). The number of iotasks participating in the operation is
     * blocksize/global_size. */
    minbytes = blocksize - 256;
    maxbytes = blocksize + 256;

    /* Determine the size of the data type. */
    if ((ret = find_mpi_type(pio_type, NULL, &basesize)))
        return ret;

    /* Determine the minimum block size. */
    minblocksize = minbytes / basesize;

    /* Find the total size of the data. */
    pgdims = 1;
    for (i = 0; i < ndims; i++)
        pgdims *= (long int)gdims[i];

    /* Find the number of ioprocs that are needed so that we have
     * blocksize data on each iotask*/
    use_io_procs = max(1, min((int)((float)pgdims / (float)minblocksize + 0.5), num_io_procs));

    /* Initialize to 0. */
    converged = 0;
    for (i = 0; i < ndims; i++)
    {
        mystart[i] = 0;
        mycount[i] = 0;
    }

    /* Use_io_procs is the number of ioprocs that are needed so that
     * we have blocksize data on each iotask, now find start and count
     * values needed on each of these tasks. */
    while (!converged)
    {
        long int p;

        for (iorank = 0; iorank < use_io_procs; iorank++)
        {
            for (i = 0; i < ndims; i++)
            {
                start[i] = 0;
                count[i] = gdims[i];
            }
            ldims = ndims - 1;
            p = basesize;
            for (i = ndims - 1; i >= 0; i--)
            {
                p = p * gdims[i];
                if (p / use_io_procs > maxbytes)
                {
                    ldims = i;
                    break;
                }
            }

            if (gdims[ldims] < use_io_procs)
            {
                if (ldims > 0 && gdims[ldims - 1] > use_io_procs)
                    ldims--;
                else
                    use_io_procs -= (use_io_procs % gdims[ldims]);
            }

            ioprocs = use_io_procs;
            tiorank = iorank;
            for (i = 0; i <= ldims; i++)
            {
                if (gdims[i] >= ioprocs)
                {
                    compute_one_dim(gdims[i], ioprocs, tiorank, &start[i], &count[i]);
                    if (start[i] + count[i] > gdims[i] + 1)
                    {
                        piodie("Start plus count exceeds dimension bound",__FILE__,__LINE__);
                    }
                }
                else if(gdims[i] > 1)
                {
                    tioprocs = gdims[i];
                    tiorank = (iorank * tioprocs) / ioprocs;
                    compute_one_dim(gdims[i], tioprocs, tiorank, &start[i], &count[i]);
                    ioprocs = ioprocs / tioprocs;
                    tiorank  = iorank % ioprocs;
                }

            }

            if (myiorank == iorank)
            {
                for (i = 0; i < ndims; i++)
                {
                    mystart[i] = start[i];
                    mycount[i] = count[i];
                }
            }
            pknt = 1;

            for(i = 0; i < ndims; i++)
                pknt *= count[i];

            tpsize += pknt;

            if (tpsize == pgdims && use_io_procs == iorank + 1)
            {
                converged = true;
                break;
            }
            else if(tpsize >= pgdims)
            {
                break;
            }
        }

        if (!converged)
        {
            tpsize = 0;
            use_io_procs--;
        }
    }

    /* On IO tasks, set the start/count arrays to computed values. On
     * non-io tasks set start/count to zero. */
    if (myiorank < use_io_procs)
    {
        for (i = 0; i < ndims; i++)
        {
            start[i] = mystart[i];
            count[i] = mycount[i];
        }
    }
    else
    {
        for (i = 0; i < ndims; i++)
        {
            start[i] = 0;
            count[i] = 0;
        }
    }

    /* Return the number of IO procs used to the caller. */
    *num_aiotasks = use_io_procs;

    return PIO_NOERR;
}
