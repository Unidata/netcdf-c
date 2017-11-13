/** @file
 * Code to map IO to model decomposition.
 *
 * @author Jim Edwards
 */
#include <config.h>
#include <pio_internal.h>
#include <pio.h>

/**
 * Convert a 1-D index into a coordinate value in an arbitrary
 * dimension space. E.g., for index 4 into a array defined as a[3][2],
 * will return 2,0.
 *
 * Sometimes (from box_rearranger_create()) this function is called
 * with -1 for idx. Not clear if this makes sense.
 *
 * @param ndims number of dimensions.
 * @param gdimlen array of length ndims with the dimension sizes.
 * @param idx the index to convert. This is the index into a 1-D array
 * of data.
 * @param dim_list array of length ndims that will get the dimensions
 * corresponding to this index.
 * @author Jim Edwards
 */
void idx_to_dim_list(int ndims, const int *gdimlen, PIO_Offset idx,
                     PIO_Offset *dim_list)
{
    /* Check inputs. */
    pioassert(ndims >= 0 && gdimlen && idx >= -1 && dim_list, "invalid input",
              __FILE__, __LINE__);
    LOG((2, "idx_to_dim_list ndims = %d idx = %d", ndims, idx));

    /* Easiest to start from the right and move left. */
    for (int i = ndims - 1; i >= 0; --i)
    {
        int next_idx;

        /* This way of doing div/mod is slightly faster than using "/"
         * and "%". */
        next_idx = idx / gdimlen[i];
        dim_list[i] = idx - (next_idx * gdimlen[i]);
        LOG((3, "next_idx = %d idx = %d gdimlen[%d] = %d dim_list[%d] = %d",
             next_idx, idx, i, gdimlen[i], i, dim_list[i]));
        idx = next_idx;
    }
}

/**
 * Expand a region along dimension dim, by incrementing count[i] as
 * much as possible, consistent with the map.
 *
 * Once max_size is reached, the map is exhausted, or the next entries fail
 * to match, expand_region updates the count and calls itself with the next
 * outermost dimension, until the region has been expanded as much as
 * possible along all dimensions.
 *
 * Precondition: maplen >= region_size (thus loop runs at least
 * once).
 *
 * @param dim the dimension number to start with.
 * @param gdimlen array of global dimension lengths.
 * @param maplen the length of the map.
 * @param map array (length maplen) with the the 1-based compmap.
 * @param region_size ???
 * @param region_stride amount incremented along dimension.
 * @param max_size array of size dim + 1 that contains the maximum
 * sizes along that dimension.
 * @param count array of size dim + 1 that gets the new counts.
 * @author Jim Edwards
 */
void expand_region(int dim, const int *gdimlen, int maplen, const PIO_Offset *map,
                   int region_size, int region_stride, const int *max_size,
                   PIO_Offset *count)
{
    /* Flag used to signal that we can no longer expand the region
       along dimension dim. */
    int expansion_done = 0;

    /* Check inputs. */
    pioassert(dim >= 0 && gdimlen && maplen >= 0 && map && region_size >= 0 &&
              maplen >= region_size && region_stride >= 0 && max_size && count,
              "invalid input", __FILE__, __LINE__);

    /* Expand no greater than max_size along this dimension. */
    for (int i = 1; i <= max_size[dim]; ++i)
    {
        /* Count so far is at least i. */
        count[dim] = i;

        /* Now see if we can expand to i+1 by checking that the next
           region_size elements are ahead by exactly region_stride.
           Assuming monotonicity in the map, we could skip this for the
           innermost dimension, but it's necessary past that because the
           region does not necessarily comprise contiguous values. */
        for (int j = 0; j < region_size; j++)
        {
            int test_idx; /* Index we are testing. */

            test_idx = j + i * region_size;

            /* If we have exhausted the map, or the map no longer matches,
               we are done, break out of both loops. */
            if (test_idx >= maplen || map[test_idx] != map[j] + i * region_stride)
            {
                expansion_done = 1;
                break;
            }
        }
        if (expansion_done)
            break;
    }

    /* Move on to next outermost dimension if there are more left,
     * else return. */
    if (dim > 0)
        expand_region(dim - 1, gdimlen, maplen, map, region_size * count[dim],
                      region_stride * gdimlen[dim], max_size, count);
}

/**
 * Set start and count so that they describe the first region in
 * map.
 *
 * This function is used when creating the subset rearranger (it
 * is not used for the box rearranger). It is called by get_regions().
 *
 * Preconditions:
 * <ul>
 * <li>ndims is > 0
 * <li>maplen is > 0
 * <li>All elements of map are inside the bounds specified by gdimlen.
 * </ul>
 *
 * Note that the map array is 1 based, but calculations are 0 based.
 *
 * @param ndims the number of dimensions.
 * @param gdimlen an array length ndims with the sizes of the global
 * dimensions.
 * @param maplen the length of the map.
 * @param map
 * @param start array (length ndims) that will get start indicies of
 * found region.
 * @param count array (length ndims) that will get counts of found
 * region.
 * @returns length of the region found.
 * @author Jim Edwards
 */
PIO_Offset find_region(int ndims, const int *gdimlen, int maplen, const PIO_Offset *map,
                       PIO_Offset *start, PIO_Offset *count)
{
    PIO_Offset regionlen = 1;

    /* Check inputs. */
    pioassert(ndims > 0 && gdimlen && maplen > 0 && map && start && count,
              "invalid input", __FILE__, __LINE__);
    LOG((2, "find_region ndims = %d maplen = %d", ndims, maplen));

    int max_size[ndims];

    /* Convert the index which is the first element of map into global
     * data space. */
    idx_to_dim_list(ndims, gdimlen, map[0] - 1, start);

    /* Can't expand beyond the array edge. Set up max_size array for
     * expand_region call below. */
    for (int dim = 0; dim < ndims; ++dim)
    {
        max_size[dim] = gdimlen[dim] - start[dim];
        LOG((3, "max_size[%d] = %d", max_size[dim]));
    }

    /* For each dimension, figure out how far we can expand in that dimension
       while staying contiguous in the input array.

       Start with the innermost dimension (ndims-1), and it will recurse
       through to the outermost dimensions. */
    expand_region(ndims - 1, gdimlen, maplen, map, 1, 1, max_size, count);

    /* Calculate the number of data elements in this region. */
    for (int dim = 0; dim < ndims; dim++)
        regionlen *= count[dim];

    return regionlen;
}

/**
 * Convert a global coordinate value into a local array index.
 *
 * @param ndims the number of dimensions.
 * @param lcoord pointer to an offset.
 * @param count array of counts.
 * @returns the local array index.
 * @author Jim Edwards
 */
PIO_Offset coord_to_lindex(int ndims, const PIO_Offset *lcoord, const PIO_Offset *count)
{
    PIO_Offset lindex = 0;
    PIO_Offset stride = 1;

    /* Check inputs. */
    pioassert(ndims > 0 && lcoord && count, "invalid input", __FILE__, __LINE__);

    for (int i = ndims - 1; i >= 0; i--)
    {
        lindex += lcoord[i] * stride;
        stride = stride * count[i];
    }
    return lindex;
}

/**
 * Compute the max io buffer size needed for an iodesc. It is the
 * combined size (in number of data elements) of all the regions of
 * data stored in the buffer of this iodesc. The max size is then set
 * in the iodesc.
 *
 * @param io_comm the IO communicator
 * @param iodesc a pointer to the io_desc_t struct.
 * @returns 0 for success, error code otherwise.
 * @author Jim Edwards
 */
int compute_maxIObuffersize(MPI_Comm io_comm, io_desc_t *iodesc)
{
    PIO_Offset totiosize = 0;
    int mpierr; /* Return code from MPI calls. */

    pioassert(iodesc, "need iodesc", __FILE__, __LINE__);

    /*  compute the max io buffer size, for conveneance it is the
     *  combined size of all regions */
    for (io_region *region = iodesc->firstregion; region; region = region->next)
    {
        if (region->count[0] > 0)
        {
            PIO_Offset iosize = 1;
            for (int i = 0; i < iodesc->ndims; i++)
                iosize *= region->count[i];
            totiosize += iosize;
        }
    }
    LOG((2, "compute_maxIObuffersize got totiosize = %lld", totiosize));

    /* Share the max io buffer size with all io tasks. */
    if ((mpierr = MPI_Allreduce(MPI_IN_PLACE, &totiosize, 1, MPI_OFFSET, MPI_MAX, io_comm)))
        return check_mpi(NULL, mpierr, __FILE__, __LINE__);
    pioassert(totiosize > 0, "totiosize <= 0", __FILE__, __LINE__);
    LOG((2, "after allreduce compute_maxIObuffersize got totiosize = %lld", totiosize));

    /* Remember the result. */
    iodesc->maxiobuflen = totiosize;

    return PIO_NOERR;
}

/**
 * Create the derived MPI datatypes used for comp2io and io2comp
 * transfers. Used in define_iodesc_datatypes().
 *
 * @param mpitype The MPI type of data (MPI_INT, etc.).
 * @param msgcnt This is the number of MPI types that are created.
 * @param mindex An array (length numinds) of indexes into the data
 * array from the comp map. Will be NULL when count is zero.
 * @param mcount An array (length msgcnt) with the number of indexes
 * to be put on each mpi message/task.
 * @param mfrom A pointer to the previous structure in the read/write
 * list. This is always NULL for the BOX rearranger.
 * @param mtype pointer to an array (length msgcnt) which gets the
 * created datatypes. Will be NULL when iodesc->nrecvs == 0.
 * @returns 0 on success, error code otherwise.
 * @author Jim Edwards
 */
int create_mpi_datatypes(MPI_Datatype mpitype, int msgcnt,
                         const PIO_Offset *mindex, const int *mcount, int *mfrom,
                         MPI_Datatype *mtype)
{
    int blocksize;
    int numinds = 0;
    PIO_Offset *lindex = NULL;
    int mpierr; /* Return code from MPI functions. */

    /* Check inputs. */
    pioassert(msgcnt > 0 && mcount, "invalid input", __FILE__, __LINE__);

    PIO_Offset bsizeT[msgcnt];

    LOG((1, "create_mpi_datatypes mpitype = %d msgcnt = %d", mpitype, msgcnt));
    LOG((2, "MPI_BYTE = %d MPI_CHAR = %d MPI_SHORT = %d MPI_INT = %d MPI_FLOAT = %d MPI_DOUBLE = %d",
         MPI_BYTE, MPI_CHAR, MPI_SHORT, MPI_INT, MPI_FLOAT, MPI_DOUBLE));

    /* How many indicies in the array? */
    for (int j = 0; j < msgcnt; j++)
        numinds += mcount[j];
    LOG((2, "numinds = %d", numinds));

    if (mindex)
    {
        if (!(lindex = malloc(numinds * sizeof(PIO_Offset))))
            return pio_err(NULL, NULL, PIO_ENOMEM, __FILE__, __LINE__);
        memcpy(lindex, mindex, (size_t)(numinds * sizeof(PIO_Offset)));
        LOG((3, "allocated lindex, copied mindex"));
    }

    bsizeT[0] = 0;
    mtype[0] = PIO_DATATYPE_NULL;
    int pos = 0;
    int ii = 0;

    /* Determine the blocksize. This is done differently for the
     * rearrangers. (If mfrom is NULL, this is the box rearranger.) */
    if (mfrom == NULL)
    {
        LOG((3, "mfrom is NULL"));
        for (int i = 0; i < msgcnt; i++)
        {
            if (mcount[i] > 0)
            {
                /* Look for the largest block of data for io which
                 * can be expressed in terms of start and
                 * count. */
                bsizeT[ii] = GCDblocksize(mcount[i], lindex + pos);
                ii++;
                pos += mcount[i];
            }
        }
        blocksize = (int)lgcd_array(ii, bsizeT);
    }
    else
    {
        blocksize = 1;
    }
    LOG((3, "blocksize = %d", blocksize));

    /* pos is an index to the start of each message block. */
    pos = 0;
    for (int i = 0; i < msgcnt; i++)
    {
        if (mcount[i] > 0)
        {
            int len = mcount[i] / blocksize;
            int displace[len];
            LOG((3, "blocksize = %d i = %d mcount[%d] = %d len = %d", blocksize, i, i,
                 mcount[i], len));
            if (blocksize == 1)
            {
                if (!mfrom)
                {
                    /* Box rearranger. */
                    for (int j = 0; j < len; j++)
                        displace[j] = (int)(lindex[pos + j]);
                }
                else
                {
                    /* Subset rearranger. */
                    int k = 0;
                    for (int j = 0; j < numinds; j++)
                        if (mfrom[j] == i)
                            displace[k++] = (int)(lindex[j]);
                }

            }
            else
            {
                for (int j = 0; j < mcount[i]; j++)
                    (lindex + pos)[j]++;

                for (int j = 0; j < len; j++)
                    displace[j] = ((lindex + pos)[j * blocksize] - 1);
            }

#if PIO_ENABLE_LOGGING
            for (int j = 0; j < len; j++)
                LOG((3, "displace[%d] = %d", j, displace[j]));
#endif /* PIO_ENABLE_LOGGING */

            LOG((3, "calling MPI_Type_create_indexed_block len = %d blocksize = %d "
                 "mpitype = %d", len, blocksize, mpitype));
            /* Create an indexed datatype with constant-sized blocks. */
            if ((mpierr = MPI_Type_create_indexed_block(len, blocksize, displace,
                                                        mpitype, &mtype[i])))
                return check_mpi(NULL, mpierr, __FILE__, __LINE__);

            if (mtype[i] == PIO_DATATYPE_NULL)
                return pio_err(NULL, NULL, PIO_EINVAL, __FILE__, __LINE__);

            /* Commit the MPI data type. */
            LOG((3, "about to commit type"));
            if ((mpierr = MPI_Type_commit(&mtype[i])))
                return check_mpi(NULL, mpierr, __FILE__, __LINE__);
            pos += mcount[i];
        }
    }

    /* Free resources. */
    if (lindex)
        free(lindex);

    LOG((3, "done with create_mpi_datatypes()"));
    return PIO_NOERR;
}

/**
 * If needed, create the derived MPI datatypes used for comp2io and
 * io2comp transfers.
 *
 * If iodesc->stype and iodesc->rtype arrays already exist, this
 * function does nothing. This function is called from
 * rearrange_io2comp() and rearrange_comp2io().
 *
 * NOTE from Jim: I am always oriented toward write so recieve
 * always means io tasks and send always means comp tasks. The
 * opposite relationship is actually the case for reading. I've
 * played with different ways of referring to things to get rid of
 * this orientation bias in the documentation as well as in
 * variable names, but I haven't found anything that I found more
 * satisfactory.
 *
 * @param ios pointer to the iosystem_desc_t struct.
 * @param iodesc a pointer to the io_desc_t struct.
 * @returns 0 on success, error code otherwise.
 * @author Jim Edwards
 */
int define_iodesc_datatypes(iosystem_desc_t *ios, io_desc_t *iodesc)
{
    int ret; /* Return value. */

    pioassert(ios && iodesc, "invalid input", __FILE__, __LINE__);
    LOG((1, "define_iodesc_datatypes ios->ioproc = %d iodesc->rtype is %sNULL, iodesc->nrecvs",
         ios->ioproc, iodesc->rtype ? "not " : "", iodesc->nrecvs));

    /* Set up the to transfer data to and from the IO tasks. */
    if (ios->ioproc)
    {
        /* If the types for the IO tasks have not been created, then
         * create them. */
        if (!iodesc->rtype)
        {
            if (iodesc->nrecvs > 0)
            {
                /* Allocate memory for array of MPI types for the IO tasks. */
                if (!(iodesc->rtype = malloc(iodesc->nrecvs * sizeof(MPI_Datatype))))
                    return pio_err(ios, NULL, PIO_ENOMEM, __FILE__, __LINE__);
                LOG((2, "allocated memory for IO task MPI types iodesc->nrecvs = %d "
                     "iodesc->rearranger = %d", iodesc->nrecvs, iodesc->rearranger));

                /* Initialize data types to NULL. */
                for (int i = 0; i < iodesc->nrecvs; i++)
                    iodesc->rtype[i] = PIO_DATATYPE_NULL;

                /* The different rearrangers get different values for mfrom. */
                int *mfrom = iodesc->rearranger == PIO_REARR_SUBSET ? iodesc->rfrom : NULL;

                /* Create the MPI datatypes. */
                if ((ret = create_mpi_datatypes(iodesc->mpitype, iodesc->nrecvs, iodesc->rindex,
                                                iodesc->rcount, mfrom, iodesc->rtype)))
                    return pio_err(ios, NULL, ret, __FILE__, __LINE__);
            }
        }
    }

    /* Define the datatypes for the computation components if they
     * don't exist. (These will be the send side in a write
     * operation.) */
    if (ios->compproc)
    {
        if (!iodesc->stype)
        {
            int ntypes;

            /* Subset rearranger gets one type; box rearranger gets one
             * type per IO task. */
            ntypes = iodesc->rearranger == PIO_REARR_SUBSET ? 1 : ios->num_iotasks;

            /* Allocate memory for array of MPI types for the computation tasks. */
            if (!(iodesc->stype = malloc(ntypes * sizeof(MPI_Datatype))))
                return pio_err(ios, NULL, PIO_ENOMEM, __FILE__, __LINE__);
            LOG((3, "allocated memory for computation MPI types ntypes = %d", ntypes));

            /* Initialize send types to NULL. */
            for (int i = 0; i < ntypes; i++)
                iodesc->stype[i] = PIO_DATATYPE_NULL;

            /* Remember how many types we created for the send side. */
            iodesc->num_stypes = ntypes;

            /* Create the MPI data types. */
            LOG((3, "about to call create_mpi_datatypes for computation MPI types"));
            if ((ret = create_mpi_datatypes(iodesc->mpitype, ntypes, iodesc->sindex,
                                            iodesc->scount, NULL, iodesc->stype)))
                return pio_err(ios, NULL, ret, __FILE__, __LINE__);
        }
    }

    LOG((3, "done with define_iodesc_datatypes()"));
    return PIO_NOERR;
}

/**
 * Completes the mapping for the box rearranger. This function is
 * called from box_rearrange_create(). It is not used for the subset
 * rearranger.
 *
 * This function:
 * <ul>
 * <li>Allocates and inits iodesc->scount, an array (length
 * ios->num_iotasks) containing number of data elements sent to each
 * IO task from current compute task.
 * <li>Uses pio_swapm() to send iodesc->scount array from each
 * computation task to all IO tasks.
 * <li>On IO tasks, allocates and inits iodesc->rcount and
 * iodesc->rfrom arrays (length max(1, nrecvs)) which holds the amount
 * of data to expect from each compute task and the rank of that
 * task. .
 * <li>Allocates and inits iodesc->sindex arrays (length iodesc->ndof)
 * which holds indecies for computation tasks.
 * <li>On IO tasks, allocates and inits iodesc->rindex (length
 * totalrecv) with indices of the data to be sent/received from this
 * io task to each compute task.
 * <li>Uses pio_swapm() to send list of indicies on each compute task
 * to the IO tasks.
 * </ul>
 *
 * @param ios pointer to the iosystem_desc_t struct.
 * @param iodesc a pointer to the io_desc_t struct.
 * @param dest_ioproc an array (length maplen) of IO task numbers.
 * @param dest_ioindex an array (length maplen) of IO indicies.
 * @returns 0 on success, error code otherwise.
 * @author Jim Edwards
 */
int compute_counts(iosystem_desc_t *ios, io_desc_t *iodesc,
                   const int *dest_ioproc, const PIO_Offset *dest_ioindex)
{
    int *recv_buf = NULL;
    int nrecvs = 0;
    int ierr;

    /* Check inputs. */
    pioassert(ios && iodesc && dest_ioproc && dest_ioindex &&
              iodesc->rearranger == PIO_REARR_BOX && ios->num_uniontasks > 0,
              "invalid input", __FILE__, __LINE__);
    LOG((1, "compute_counts ios->num_uniontasks = %d ios->compproc %d ios->ioproc %d",
         ios->num_uniontasks, ios->compproc, ios->ioproc));

    /* Arrays for swapm all to all gather calls. */
    MPI_Datatype sr_types[ios->num_uniontasks];
    int send_counts[ios->num_uniontasks];
    int send_displs[ios->num_uniontasks];
    int recv_counts[ios->num_uniontasks];
    int recv_displs[ios->num_uniontasks];

    /* The list of indeces on each compute task */
    PIO_Offset s2rindex[iodesc->ndof];

    /* Allocate memory for the array of counts and init to zero. */
    if (!(iodesc->scount = calloc(ios->num_iotasks, sizeof(int))))
        return pio_err(ios, NULL, PIO_ENOMEM, __FILE__, __LINE__);

    /* iodesc->scount is the number of data elements sent to each IO
     * task from the current compute task. dest_ioindex[i] may be
     * -1. */
    if (ios->compproc)
        for (int i = 0; i < iodesc->ndof; i++)
            if (dest_ioindex[i] >= 0)
                (iodesc->scount[dest_ioproc[i]])++;

    /* Initialize arrays used in swapm call. */
    for (int i = 0; i < ios->num_uniontasks; i++)
    {
        send_counts[i] = 0;
        send_displs[i] = 0;
        recv_counts[i] = 0;
        recv_displs[i] = 0;
        sr_types[i] = MPI_INT;
    }

    /* Setup for the swapm call. iodesc->scount is the amount of data
     * this compute task will transfer to/from each iotask. For the
     * box rearranger there can be more than one IO task per compute
     * task. This provides enough information to know the size of data
     * on the iotask, so at line 557 we allocate arrays to hold the
     * map on the iotasks. iodesc->rcount is an array of the amount of
     * data to expect from each compute task and iodesc->rfrom is the
     * rank of that task. */
    if (ios->compproc)
    {
        for (int i = 0; i < ios->num_iotasks; i++)
        {
            send_counts[ios->ioranks[i]] = 1;
            send_displs[ios->ioranks[i]] = i * sizeof(int);
            LOG((3, "send_counts[%d] = %d send_displs[%d] = %d", ios->ioranks[i],
                 send_counts[ios->ioranks[i]], ios->ioranks[i], send_displs[ios->ioranks[i]]));
        }
    }

    /* IO tasks need to know how many data elements they will receive
     * from each compute task. Allocate space for that, and set up
     * swapm call. */
    if (ios->ioproc)
    {
        /* Allocate memory to hold array of the scounts from all
         * computation tasks. */
        if (!(recv_buf = calloc(ios->num_comptasks, sizeof(int))))
            return pio_err(ios, NULL, PIO_ENOMEM, __FILE__, __LINE__);

        /* Initialize arrays that keep track of counts and
         * displacements for the all-to-all gather. */
        for (int i = 0; i < ios->num_comptasks; i++)
        {
            recv_counts[ios->compranks[i]] = 1;
            recv_displs[ios->compranks[i]] = i * sizeof(int);
            LOG((3, "recv_counts[%d] = %d recv_displs[%d] = %d", ios->compranks[i],
                 recv_counts[ios->compranks[i]], ios->compranks[i],
                 recv_displs[ios->compranks[i]]));
        }
    }

    LOG((2, "about to share scount from each compute task to all IO tasks."));
    /* Share the iodesc->scount from each compute task to all IO
     * tasks. The scounts will end up in array recv_buf. */
    if ((ierr = pio_swapm(iodesc->scount, send_counts, send_displs, sr_types,
                          recv_buf, recv_counts, recv_displs, sr_types, ios->union_comm,
                          &iodesc->rearr_opts.comp2io)))
        return pio_err(ios, NULL, ierr, __FILE__, __LINE__);

    /* On IO tasks, set up data receives. */
    if (ios->ioproc)
    {
        /* Count the number of non-zero scounts from the compute
         * tasks. */
        for (int i = 0; i < ios->num_comptasks; i++)
        {
            if (recv_buf[i] != 0)
                nrecvs++;
            LOG((3, "recv_buf[%d] = %d", i, recv_buf[i]));
        }

        /* Get memory to hold the count of data receives. */
        if (!(iodesc->rcount = calloc(max(1, nrecvs), sizeof(int))))
            return pio_err(ios, NULL, PIO_ENOMEM, __FILE__, __LINE__);

        /* Get memory to hold the list of task data was from. */
        if (!(iodesc->rfrom = calloc(max(1, nrecvs), sizeof(int))))
            return pio_err(ios, NULL, PIO_ENOMEM, __FILE__, __LINE__);
        LOG((3, "allocared rfrom max(1, nrecvs) = %d", max(1, nrecvs)));

        nrecvs = 0;
        for (int i = 0; i < ios->num_comptasks; i++)
        {
            if (recv_buf[i] != 0)
            {
                iodesc->rcount[nrecvs] = recv_buf[i];
                iodesc->rfrom[nrecvs] = ios->compranks[i];
                nrecvs++;
            }
        }
        free(recv_buf);
    }

    /* ??? */
    iodesc->nrecvs = nrecvs;
    LOG((3, "iodesc->nrecvs = %d", iodesc->nrecvs));

    /* Allocate an array for indicies on the computation tasks (the
     * send side when writing). */
    if (iodesc->sindex == NULL && iodesc->ndof > 0)
        if (!(iodesc->sindex = malloc(iodesc->ndof * sizeof(PIO_Offset))))
            return pio_err(ios, NULL, PIO_ENOMEM, __FILE__, __LINE__);
    LOG((2, "iodesc->ndof = %d ios->num_iotasks = %d", iodesc->ndof, ios->num_iotasks));

    int tempcount[ios->num_iotasks];
    int spos[ios->num_iotasks];

    /* ??? */
    spos[0] = 0;
    tempcount[0] = 0;
    for (int i = 1; i < ios->num_iotasks; i++)
    {
        spos[i] = spos[i - 1] + iodesc->scount[i - 1];
        tempcount[i] = 0;
        LOG((3, "spos[%d] = %d tempcount[%d] = %d", i, spos[i], i, tempcount[i]));
    }

    /* ??? */
    for (int i = 0; i < iodesc->ndof; i++)
    {
        int iorank;
        int ioindex;

        LOG((3, "dest_ioproc[%d] = %d dest_ioindex[%d] = %d", i, dest_ioproc[i], i,
             dest_ioindex[i]));
        iorank = dest_ioproc[i];
        ioindex = dest_ioindex[i];
        if (iorank > -1)
        {
            /* this should be moved to create_box */
            iodesc->sindex[spos[iorank] + tempcount[iorank]] = i;

            s2rindex[spos[iorank] + tempcount[iorank]] = ioindex;
            (tempcount[iorank])++;
            LOG((3, "iorank = %d ioindex = %d tempcount[iorank] = %d", iorank, ioindex,
                 tempcount[iorank]));
        }
    }

    /* Initialize arrays to zeros. */
    for (int i = 0; i < ios->num_uniontasks; i++)
    {
        send_counts[i] = 0;
        send_displs[i] = 0;
        recv_counts[i] = 0;
        recv_displs[i] = 0;
    }

    /* ??? */
    for (int i = 0; i < ios->num_iotasks; i++)
    {
        /* Subset rearranger needs one type, box rearranger needs one for
         * each IO task. */
        send_counts[ios->ioranks[i]] = iodesc->scount[i];
        if (send_counts[ios->ioranks[i]] > 0)
            send_displs[ios->ioranks[i]] = spos[i] * SIZEOF_MPI_OFFSET;
        LOG((3, "ios->ioranks[i] = %d iodesc->scount[%d] = %d spos[%d] = %d",
             ios->ioranks[i], i, iodesc->scount[i], i, spos[i]));
    }

    /* Only do this on IO tasks. */
    if (ios->ioproc)
    {
        int totalrecv = 0;
        for (int i = 0; i < nrecvs; i++)
        {
            recv_counts[iodesc->rfrom[i]] = iodesc->rcount[i];
            totalrecv += iodesc->rcount[i];
        }

        recv_displs[0] = 0;
        for (int i = 1; i < nrecvs; i++)
        {
            recv_displs[iodesc->rfrom[i]] = recv_displs[iodesc->rfrom[i - 1]] +
                iodesc->rcount[i - 1] * SIZEOF_MPI_OFFSET;
            LOG((3, "iodesc->rfrom[%d] = %d recv_displs[iodesc->rfrom[i]] = %d", i,
                 iodesc->rfrom[i], recv_displs[iodesc->rfrom[i]]));
        }

        /* rindex is an array of the indices of the data to be sent from
           this io task to each compute task. */
        LOG((3, "totalrecv = %d", totalrecv));
        if (totalrecv > 0)
        {
            totalrecv = iodesc->llen;  /* can reduce memory usage here */
            if (!(iodesc->rindex = calloc(totalrecv, sizeof(PIO_Offset))))
                return pio_err(ios, NULL, PIO_ENOMEM, __FILE__, __LINE__);
            LOG((3, "allocated totalrecv elements in rindex array"));
        }
    }

    /* For the swapm call below, init the types to MPI_OFFSET. */
    for (int i = 0; i < ios->num_uniontasks; i++)
        sr_types[i] = MPI_OFFSET;

    /* Here we are sending the mapping from the index on the compute
     * task to the index on the io task. */
    /* s2rindex is the list of indeces on each compute task */
    LOG((3, "sending mapping"));
    if ((ierr = pio_swapm(s2rindex, send_counts, send_displs, sr_types, iodesc->rindex,
                          recv_counts, recv_displs, sr_types, ios->union_comm,
                          &iodesc->rearr_opts.comp2io)))
        return pio_err(ios, NULL, ierr, __FILE__, __LINE__);

    return PIO_NOERR;
}

/**
 * Moves data from compute tasks to IO tasks. This is called from
 * PIOc_write_darray_multi().
 *
 * @param ios pointer to the iosystem_desc_t struct.
 * @param iodesc a pointer to the io_desc_t struct.
 * @param sbuf send buffer. May be NULL.
 * @param rbuf receive buffer. May be NULL.
 * @param nvars number of variables.
 * @returns 0 on success, error code otherwise.
 * @author Jim Edwards
 */
int rearrange_comp2io(iosystem_desc_t *ios, io_desc_t *iodesc, void *sbuf,
                      void *rbuf, int nvars)
{
    int ntasks;       /* Number of tasks in communicator. */
    int niotasks;     /* Number of IO tasks. */
    MPI_Comm mycomm;  /* Communicator that data is transferred over. */
    int mpierr;       /* Return code from MPI calls. */
    int ret;

#ifdef TIMING
    GPTLstart("PIO:rearrange_comp2io");
#endif

    /* Caller must provide these. */
    pioassert(ios && iodesc && nvars > 0, "invalid input", __FILE__, __LINE__);

    LOG((1, "rearrange_comp2io nvars = %d iodesc->rearranger = %d", nvars,
         iodesc->rearranger));

    /* Different rearraangers use different communicators. */
    if (iodesc->rearranger == PIO_REARR_BOX)
    {
        mycomm = ios->union_comm;
        niotasks = ios->num_iotasks;
    }
    else
    {
        mycomm = iodesc->subset_comm;
        niotasks = 1;
    }

    /* Get the number of tasks. */
    if ((mpierr = MPI_Comm_size(mycomm, &ntasks)))
        return check_mpi(NULL, mpierr, __FILE__, __LINE__);

    /* These are parameters to pio_swapm to send data from compute to
     * IO tasks. */
    int sendcounts[ntasks];
    int recvcounts[ntasks];
    int sdispls[ntasks];
    int rdispls[ntasks];
    MPI_Datatype sendtypes[ntasks];
    MPI_Datatype recvtypes[ntasks];

    /* Initialize pio_swapm parameter arrays. */
    for (int i = 0; i < ntasks; i++)
    {
        sendcounts[i] = 0;
        recvcounts[i] = 0;
        sdispls[i] = 0;
        rdispls[i] = 0;
        recvtypes[i] = PIO_DATATYPE_NULL;
        sendtypes[i] =  PIO_DATATYPE_NULL;
    }
    LOG((3, "ntasks = %d iodesc->mpitype_size = %d niotasks = %d", ntasks,
         iodesc->mpitype_size, niotasks));

    /* If it has not already been done, define the MPI data types that
     * will be used for this io_desc_t. */
    if ((ret = define_iodesc_datatypes(ios, iodesc)))
        return pio_err(ios, NULL, ret, __FILE__, __LINE__);

    /* If this io proc, we need to exchange data with compute
     * tasks. Create a MPI DataType for that exchange. */
    LOG((2, "ios->ioproc %d iodesc->nrecvs = %d", ios->ioproc, iodesc->nrecvs));
    if (ios->ioproc && iodesc->nrecvs > 0)
    {
        for (int i = 0; i < iodesc->nrecvs; i++)
        {
            if (iodesc->rtype[i] != PIO_DATATYPE_NULL)
            {
                LOG((3, "iodesc->rtype[%d] = %d iodesc->rearranger = %d", i, iodesc->rtype[i],
                     iodesc->rearranger));
                if (iodesc->rearranger == PIO_REARR_SUBSET)
                {
                    LOG((3, "exchanging data for subset rearranger"));
                    recvcounts[i] = 1;

                    /*  Create an MPI derived data type from equally
                     *  spaced blocks of the same size. The block size
                     *  is 1, the stride here is the length of the
                     *  collected array (llen). */
#if PIO_USE_MPISERIAL
                    if ((mpierr = MPI_Type_hvector(nvars, 1, (MPI_Aint)iodesc->llen * iodesc->mpitype_size,
                                                   iodesc->rtype[i], &recvtypes[i])))
                        return check_mpi(NULL, mpierr, __FILE__, __LINE__);
#else
                    if ((mpierr = MPI_Type_create_hvector(nvars, 1, (MPI_Aint)iodesc->llen * iodesc->mpitype_size,
                                                          iodesc->rtype[i], &recvtypes[i])))
                        return check_mpi(NULL, mpierr, __FILE__, __LINE__);
#endif /* PIO_USE_MPISERIAL */
                    pioassert(recvtypes[i] != PIO_DATATYPE_NULL, "bad mpi type", __FILE__, __LINE__);

                    if ((mpierr = MPI_Type_commit(&recvtypes[i])))
                        return check_mpi(NULL, mpierr, __FILE__, __LINE__);
                }
                else
                {
                    recvcounts[iodesc->rfrom[i]] = 1;
                    LOG((3, "exchanging data for box rearranger i = %d iodesc->rfrom[i] = %d "
                         "recvcounts[iodesc->rfrom[i]] = %d", i, iodesc->rfrom[i],
                         recvcounts[iodesc->rfrom[i]]));

#if PIO_USE_MPISERIAL
                    if ((mpierr = MPI_Type_hvector(nvars, 1, (MPI_Aint)iodesc->llen * iodesc->mpitype_size,
                                                   iodesc->rtype[i], &recvtypes[iodesc->rfrom[i]])))
                        return check_mpi(NULL, mpierr, __FILE__, __LINE__);
#else
                    if ((mpierr = MPI_Type_create_hvector(nvars, 1, (MPI_Aint)iodesc->llen * iodesc->mpitype_size,
                                                          iodesc->rtype[i], &recvtypes[iodesc->rfrom[i]])))
                        return check_mpi(NULL, mpierr, __FILE__, __LINE__);
#endif /* PIO_USE_MPISERIAL */
                    pioassert(recvtypes[iodesc->rfrom[i]] != PIO_DATATYPE_NULL,  "bad mpi type",
                              __FILE__, __LINE__);

                    if ((mpierr = MPI_Type_commit(&recvtypes[iodesc->rfrom[i]])))
                        return check_mpi(NULL, mpierr, __FILE__, __LINE__);

                    rdispls[iodesc->rfrom[i]] = 0;
                }
            }
        }
    }

    /* On compute tasks loop over iotasks and create a data type for
     * each exchange.  */
    for (int i = 0; i < niotasks; i++)
    {
        int io_comprank = ios->ioranks[i];
        LOG((3, "ios->ioranks[%d] = %d", i, ios->ioranks[i]));
        if (iodesc->rearranger == PIO_REARR_SUBSET)
            io_comprank = 0;

        LOG((3, "i = %d iodesc->scount[i] = %d", i, iodesc->scount[i]));
        if (iodesc->scount[i] > 0 && sbuf)
        {
            LOG((3, "io task %d creating sendtypes[%d]", i, io_comprank));
            sendcounts[io_comprank] = 1;
#if PIO_USE_MPISERIAL
            if ((mpierr = MPI_Type_hvector(nvars, 1, (MPI_Aint)iodesc->ndof * iodesc->mpitype_size,
                                           iodesc->stype[i], &sendtypes[io_comprank])))
                return check_mpi(NULL, mpierr, __FILE__, __LINE__);
#else
            if ((mpierr = MPI_Type_create_hvector(nvars, 1, (MPI_Aint)iodesc->ndof * iodesc->mpitype_size,
                                                  iodesc->stype[i], &sendtypes[io_comprank])))
                return check_mpi(NULL, mpierr, __FILE__, __LINE__);
#endif /* PIO_USE_MPISERIAL */
            pioassert(sendtypes[io_comprank] != PIO_DATATYPE_NULL,  "bad mpi type", __FILE__, __LINE__);

            if ((mpierr = MPI_Type_commit(&sendtypes[io_comprank])))
                return check_mpi(NULL, mpierr, __FILE__, __LINE__);
        }
        else
        {
            sendcounts[io_comprank] = 0;
        }
    }

    /* Data in sbuf on the compute nodes is sent to rbuf on the ionodes */
    LOG((2, "about to call pio_swapm for sbuf"));
    if ((ret = pio_swapm(sbuf, sendcounts, sdispls, sendtypes,
                         rbuf, recvcounts, rdispls, recvtypes, mycomm,
                         &iodesc->rearr_opts.comp2io)))
        return pio_err(ios, NULL, ret, __FILE__, __LINE__);

    /* Free the MPI types. */
    for (int i = 0; i < ntasks; i++)
    {
        LOG((3, "freeing MPI types for task %d", i));
        if (sendtypes[i] != PIO_DATATYPE_NULL)
            if ((mpierr = MPI_Type_free(&sendtypes[i])))
                return check_mpi(NULL, mpierr, __FILE__, __LINE__);

        if (recvtypes[i] != PIO_DATATYPE_NULL)
            if ((mpierr = MPI_Type_free(&recvtypes[i])))
                return check_mpi(NULL, mpierr, __FILE__, __LINE__);
    }

#ifdef TIMING
    GPTLstop("PIO:rearrange_comp2io");
#endif

    return PIO_NOERR;
}

/**
 * Moves data from IO tasks to compute tasks. This function is used in
 * PIOc_read_darray().
 *
 * @param ios pointer to the iosystem_desc_t struct.
 * @param iodesc a pointer to the io_desc_t struct.
 * @param sbuf send buffer.
 * @param rbuf receive buffer.
 * @returns 0 on success, error code otherwise.
 * @author Jim Edwards
 */
int rearrange_io2comp(iosystem_desc_t *ios, io_desc_t *iodesc, void *sbuf,
                      void *rbuf)
{
    MPI_Comm mycomm;
    int ntasks;
    int niotasks;
    int mpierr; /* Return code from MPI calls. */
    int ret;

    /* Check inputs. */
    pioassert(ios && iodesc, "invalid input", __FILE__, __LINE__);

#ifdef TIMING
    GPTLstart("PIO:rearrange_io2comp");
#endif

    /* Different rearrangers use different communicators and number of
     * IO tasks. */
    if (iodesc->rearranger == PIO_REARR_BOX)
    {
        mycomm = ios->union_comm;
        niotasks = ios->num_iotasks;
    }
    else
    {
        mycomm = iodesc->subset_comm;
        niotasks = 1;
    }
    LOG((3, "niotasks = %d", niotasks));

    /* Get the size of this communicator. */
    if ((mpierr = MPI_Comm_size(mycomm, &ntasks)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);

    /* Define the MPI data types that will be used for this
     * io_desc_t. */
    if ((ret = define_iodesc_datatypes(ios, iodesc)))
        return pio_err(ios, NULL, ret, __FILE__, __LINE__);

    /* Allocate arrays needed by the pio_swapm() function. */
    int sendcounts[ntasks];
    int recvcounts[ntasks];
    int sdispls[ntasks];
    int rdispls[ntasks];
    MPI_Datatype sendtypes[ntasks];
    MPI_Datatype recvtypes[ntasks];

    /* Initialize arrays. */
    for (int i = 0; i < ntasks; i++)
    {
        sendcounts[i] = 0;
        recvcounts[i] = 0;
        sdispls[i] = 0;
        rdispls[i] = 0;
        sendtypes[i] = PIO_DATATYPE_NULL;
        recvtypes[i] = PIO_DATATYPE_NULL;
    }

    /* In IO tasks set up sendcounts/sendtypes for pio_swapm() call
     * below. */
    if (ios->ioproc)
    {
        for (int i = 0; i < iodesc->nrecvs; i++)
        {
            if (iodesc->rtype[i] != PIO_DATATYPE_NULL)
            {
                if (iodesc->rearranger == PIO_REARR_SUBSET)
                {
                    if (sbuf)
                    {
                        sendcounts[i] = 1;
                        sendtypes[i] = iodesc->rtype[i];
                    }
                }
                else
                {
                    sendcounts[iodesc->rfrom[i]] = 1;
                    sendtypes[iodesc->rfrom[i]] = iodesc->rtype[i];
                }
            }
        }
    }

    /* In the box rearranger each comp task may communicate with
     * multiple IO tasks here we are setting the count and data type
     * of the communication of a given compute task with each io
     * task. */
    for (int i = 0; i < niotasks; i++)
    {
        int io_comprank = ios->ioranks[i];

        if (iodesc->rearranger == PIO_REARR_SUBSET)
            io_comprank = 0;

        if (iodesc->scount[i] > 0 && iodesc->stype[i] != PIO_DATATYPE_NULL)
        {
            recvcounts[io_comprank] = 1;
            recvtypes[io_comprank] = iodesc->stype[i];
        }
    }

    /* Data in sbuf on the ionodes is sent to rbuf on the compute nodes */
    if ((ret = pio_swapm(sbuf, sendcounts, sdispls, sendtypes, rbuf, recvcounts,
                         rdispls, recvtypes, mycomm, &iodesc->rearr_opts.io2comp)))
        return pio_err(ios, NULL, ret, __FILE__, __LINE__);

#ifdef TIMING
    GPTLstop("PIO:rearrange_io2comp");
#endif

    return PIO_NOERR;
}

/**
 * Determine whether fill values are needed. This function compares
 * how much data we have to how much data is in a record (or
 * non-record var). If we have enough data to completely fill the
 * variable, then fill is not needed.
 *
 * @param ios pointer to the iosystem_desc_t struct.
 * @param iodesc a pointer to the io_desc_t struct.
 * @param gdimlen pointer to an array length iodesc->ndims with the
 * global array sizes for one record (for record vars) or for the
 * entire var (for non-record vars).
 * @param compmap only used for the box communicator.
 * @returns 0 on success, error code otherwise.
 * @author Jim Edwards
 */
int determine_fill(iosystem_desc_t *ios, io_desc_t *iodesc, const int *gdimlen,
                   const PIO_Offset *compmap)
{
    PIO_Offset totalllen = 0;
    PIO_Offset totalgridsize = 1;
    int mpierr; /* Return code from MPI calls. */

    /* Check inputs. */
    pioassert(ios && iodesc && gdimlen && compmap, "invalid input",
              __FILE__, __LINE__);

    /* Determine size of data space. */
    for (int i = 0; i < iodesc->ndims; i++)
        totalgridsize *= gdimlen[i];

    /* Determine how many values we have locally. */
    if (iodesc->rearranger == PIO_REARR_SUBSET)
        totalllen = iodesc->llen;
    else
        for (int i = 0; i < iodesc->ndof; i++)
            if (compmap[i] > 0)
                totalllen++;

    /* Add results accross communicator. */
    LOG((2, "determine_fill before allreduce totalllen = %d totalgridsize = %d",
         totalllen, totalgridsize));
    if ((mpierr = MPI_Allreduce(MPI_IN_PLACE, &totalllen, 1, PIO_OFFSET, MPI_SUM,
                                ios->union_comm)))
        check_mpi(NULL, mpierr, __FILE__, __LINE__);
    LOG((2, "after allreduce totalllen = %d", totalllen));

    /* If the total size of the data provided to be written is < the
     * total data size then we need fill values. */
    iodesc->needsfill = totalllen < totalgridsize;

    /*  TURN OFF FILL for timing test
        iodesc->needsfill=false; */

    return PIO_NOERR;
}

/**
 * The box rearranger computes a mapping between IO tasks and compute
 * tasks such that the data on IO tasks can be written with a single
 * call to the underlying netCDF library. This may involve an
 * all-to-all rearrangement in the mapping, but should minimize data
 * movement in lower level libraries.
 *
 * On each compute task the application program passes a compmap array
 * of length ndof. This array describes the arrangement of data in
 * memory on that compute task.
 *
 * These arrays are gathered and rearranged to the IO-tasks (which are
 * sometimes collocated with compute tasks), each IO task contains
 * data from the compmap of one or more compute tasks in the iomap
 * array and the length of that array is llen.
 *
 * This function:
 * <ul>
 * <li>For IO tasks, determines llen.
 * <li>Determine whether fill values will be needed.
 * <li>Do an allgather of llen values into array iomaplen.
 * <li>For each IO task, send starts/counts to all compute tasks.
 * <li>Find dest_ioindex and dest_ioproc for each element in the map.
 * <li>Call compute_counts().
 * <li>On IO tasks, compute the max IO buffer size.
 * </ul>
 *
 * @param ios pointer to the iosystem_desc_t struct.
 * @param maplen the length of the map. This is the number of data
 * elements on the compute task.
 * @param compmap a 1 based array of offsets into the global space. A
 * 0 in this array indicates a value which should not be transfered.
 * @param gdimlen an array length ndims with the sizes of the global
 * dimensions.
 * @param ndims the number of dimensions.
 * @param iodesc a pointer to the io_desc_t struct, which must be
 * allocated before this function is called.
 * @returns 0 on success, error code otherwise.
 * @author Jim Edwards
 */
int box_rearrange_create(iosystem_desc_t *ios, int maplen, const PIO_Offset *compmap,
                         const int *gdimlen, int ndims, io_desc_t *iodesc)
{
    int ret;

    /* Check inputs. */
    pioassert(ios && maplen >= 0 && compmap && gdimlen && ndims > 0 && iodesc,
              "invalid input", __FILE__, __LINE__);
    LOG((1, "box_rearrange_create maplen = %d ndims = %d ios->num_comptasks = %d "
         "ios->num_iotasks = %d", maplen, ndims, ios->num_comptasks, ios->num_iotasks));

    /* Allocate arrays needed for this function. */
    int dest_ioproc[maplen]; /* Destination IO task for each data element on compute task. */
    PIO_Offset dest_ioindex[maplen];    /* Offset into IO task array for each data element. */
    int sendcounts[ios->num_uniontasks]; /* Send counts for swapm call. */
    int sdispls[ios->num_uniontasks];    /* Send displacements for swapm. */
    int recvcounts[ios->num_uniontasks]; /* Receive counts for swapm. */
    int rdispls[ios->num_uniontasks];    /* Receive displacements for swapm. */
    MPI_Datatype dtypes[ios->num_uniontasks]; /* Array of MPI_OFFSET types for swapm. */
    PIO_Offset iomaplen[ios->num_iotasks];   /* Gets the llen of all IO tasks. */

    /* This is the box rearranger. */
    iodesc->rearranger = PIO_REARR_BOX;

    /* Number of elements of data on compute node. */
    iodesc->ndof = maplen;

    /* Initialize array values. */
    for (int i = 0; i < maplen; i++)
    {
        dest_ioproc[i] = -1;
        dest_ioindex[i] = -1;
    }

    /* Initialize arrays used in swapm. */
    for (int i = 0; i < ios->num_uniontasks; i++)
    {
        sendcounts[i] = 0;
        sdispls[i] = 0;
        recvcounts[i] = 0;
        rdispls[i] = 0;
        dtypes[i] = MPI_OFFSET;
    }

    /* For IO tasks, determine llen, the length of the data array on
     * the IO task. For computation tasks, llen will remain at 0. Also
     * set up arrays for the allgather which will give every IO task a
     * complete list of llens for each IO task. */
    LOG((3, "ios->ioproc = %d ios->num_uniontasks = %d", ios->ioproc,
         ios->num_uniontasks));
    pioassert(iodesc->llen == 0, "error", __FILE__, __LINE__);
    if (ios->ioproc)
    {
        /* Set up send counts for sending llen in all to all
         * gather. We are sending to all tasks, IO and computation. */
        for (int i = 0; i < ios->num_comptasks; i++)
            sendcounts[ios->compranks[i]] = 1;
        for (int i = 0; i < ios->num_iotasks; i++)
            sendcounts[ios->ioranks[i]] = 1;

        /* Determine llen, the lenght of the data array on this IO
         * node, by multipliying the counts in the
         * iodesc->firstregion. */
        iodesc->llen = 1;
        for (int i = 0; i < ndims; i++)
        {
            iodesc->llen *= iodesc->firstregion->count[i];
            LOG((3, "iodesc->firstregion->start[%d] = %d iodesc->firstregion->count[%d] = %d",
                 i, iodesc->firstregion->start[i], i, iodesc->firstregion->count[i]));
        }
        LOG((2, "iodesc->llen = %d", iodesc->llen));
    }

    /* Determine whether fill values will be needed. */
    if ((ret = determine_fill(ios, iodesc, gdimlen, compmap)))
        return pio_err(ios, NULL, ret, __FILE__, __LINE__);
    LOG((2, "iodesc->needsfill = %d ios->num_iotasks = %d", iodesc->needsfill,
         ios->num_iotasks));

    /* Set up receive counts and displacements to for an AllToAll
     * gather of llen. */
    for (int i = 0; i < ios->num_iotasks; i++)
    {
        recvcounts[ios->ioranks[i]] = 1;
        rdispls[ios->ioranks[i]] = i * SIZEOF_MPI_OFFSET;
        LOG((3, "i = %d ios->ioranks[%d] = %d recvcounts[%d] = %d rdispls[%d] = %d",
             i, i, ios->ioranks[i], ios->ioranks[i], recvcounts[ios->ioranks[i]],
             ios->ioranks[i], rdispls[ios->ioranks[i]]));
    }

    /* All-gather the llen to all tasks into array iomaplen. */
    LOG((3, "calling pio_swapm to allgather llen into array iomaplen, ndims = %d dtypes[0] = %d",
         ndims, dtypes));
    if ((ret = pio_swapm(&iodesc->llen, sendcounts, sdispls, dtypes, iomaplen, recvcounts,
                         rdispls, dtypes, ios->union_comm, &iodesc->rearr_opts.io2comp)))
        return pio_err(ios, NULL, ret, __FILE__, __LINE__);
    LOG((3, "iodesc->llen = %d", iodesc->llen));
#if PIO_ENABLE_LOGGING
    for (int i = 0; i < ios->num_iotasks; i++)
        LOG((3, "iomaplen[%d] = %d", i, iomaplen[i]));
#endif /* PIO_ENABLE_LOGGING */

    /* For each IO task send starts/counts to all compute tasks. */
    for (int i = 0; i < ios->num_iotasks; i++)
    {
        /* The ipmaplen contains the llen (number of data elements)
         * for this IO task. */
        LOG((2, "iomaplen[%d] = %d", i, iomaplen[i]));

        /* If there is data for this IO task, send start/count to all
         * compute tasks. */
        if (iomaplen[i] > 0)
        {
            PIO_Offset start[ndims];
            PIO_Offset count[ndims];

            /* Set up send/recv parameters for all to all gather of
             * counts and starts. */
            for (int j = 0; j < ios->num_uniontasks; j++)
            {
                sendcounts[j] = 0;
                sdispls[j] = 0;
                rdispls[j] = 0;
                recvcounts[j] = 0;
                if (ios->union_rank == ios->ioranks[i])
                    sendcounts[j] = ndims;
            }
            recvcounts[ios->ioranks[i]] = ndims;

            /* The count array from iotask i is sent to all compute tasks. */
            LOG((3, "about to call pio_swapm with count from iotask %d ndims = %d",
                 i, ndims));
            if ((ret = pio_swapm(iodesc->firstregion->count, sendcounts, sdispls, dtypes, count,
                                 recvcounts, rdispls, dtypes, ios->union_comm,
                                 &iodesc->rearr_opts.io2comp)))
                return pio_err(ios, NULL, ret, __FILE__, __LINE__);

            /* The start array from iotask i is sent to all compute tasks. */
            LOG((3, "about to call pio_swapm with start from iotask %d ndims = %d",
                 i, ndims));
            if ((ret = pio_swapm(iodesc->firstregion->start,  sendcounts, sdispls, dtypes,
                                 start, recvcounts, rdispls, dtypes, ios->union_comm,
                                 &iodesc->rearr_opts.io2comp)))
                return pio_err(ios, NULL, ret, __FILE__, __LINE__);

#if PIO_ENABLE_LOGGING
            for (int d = 0; d < ndims; d++)
                LOG((3, "start[%d] = %lld count[%d] = %lld", d, start[d], d, count[d]));
#endif /* PIO_ENABLE_LOGGING */

            /* For each element of the data array on the compute task,
             * find the IO task to send the data element to, and its
             * offset into the global data array. */
            for (int k = 0; k < maplen; k++)
            {
                PIO_Offset gcoord[ndims], lcoord[ndims];
                bool found = true;

                /* The compmap array is 1 based but calculations are 0 based */
                LOG((3, "about to call idx_to_dim_list ndims = %d ", ndims));
                idx_to_dim_list(ndims, gdimlen, compmap[k] - 1, gcoord);
#if PIO_ENABLE_LOGGING
                for (int d = 0; d < ndims; d++)
                    LOG((3, "gcoord[%d] = %lld", d, gcoord[d]));
#endif /* PIO_ENABLE_LOGGING */

                /* Find a destination for each entry in the compmap. */
                for (int j = 0; j < ndims; j++)
                {
                    if (gcoord[j] >= start[j] && gcoord[j] < start[j] + count[j])
                    {
                        lcoord[j] = gcoord[j] - start[j];
                    }
                    else
                    {
                        found = false;
                        break;
                    }
                }

                /* Did we find a destination IO task for this element
                 * of the computation task data array? If so, remember
                 * the destination IO task, and determine the index
                 * for that element in the IO task data. */
                if (found)
                {
                    dest_ioindex[k] = coord_to_lindex(ndims, lcoord, count);
                    dest_ioproc[k] = i;
                    LOG((3, "found dest_ioindex[%d] = %d dest_ioproc[%d] = %d", k, dest_ioindex[k],
                         k, dest_ioproc[k]));
                }
            }
        }
    }

    /* Check that a destination is found for each compmap entry. */
    for (int k = 0; k < maplen; k++)
        if (dest_ioproc[k] < 0 && compmap[k] > 0)
            return pio_err(ios, NULL, PIO_EINVAL, __FILE__, __LINE__);

    /* Completes the mapping for the box rearranger. */
    LOG((2, "calling compute_counts maplen = %d", maplen));
    if ((ret = compute_counts(ios, iodesc, dest_ioproc, dest_ioindex)))
        return pio_err(ios, NULL, ret, __FILE__, __LINE__);

    /* Compute the max io buffer size needed for an iodesc. */
    if (ios->ioproc)
    {
        if ((ret = compute_maxIObuffersize(ios->io_comm, iodesc)))
            return pio_err(ios, NULL, ret, __FILE__, __LINE__);
        LOG((3, "iodesc->maxiobuflen = %d", iodesc->maxiobuflen));
    }

    return PIO_NOERR;
}

/**
 * Compare offsets is used by the sort in the subset rearranger. This
 * function is passed to qsort.
 *
 * @param a pointer to an offset.
 * @param b pointer to another offset.
 * @returns 0 if offsets are the same or either pointer is NULL.
 * @author Jim Edwards
 */
int compare_offsets(const void *a, const void *b)
{
    mapsort *x = (mapsort *)a;
    mapsort *y = (mapsort *)b;
    if (!x || !y)
        return 0;
    return (int)(x->iomap - y->iomap);
}

/**
 * Calculate start and count regions for the subset rearranger. This
 * function is not used in the box rearranger.
 *
 * Each region is a block of output which can be represented in a
 * single call to the underlying netcdf library.  This can be as small
 * as a single data point, but we hope we've aggragated better than
 * that.
 *
 * @param ndims the number of dimensions
 * @param gdimlen an array length ndims with the sizes of the global
 * dimensions.
 * @param maplen the length of the map
 * @param map may be NULL (when ???).
 * @param maxregions
 * @param firstregion pointer to the first region.
 * @returns 0 on success, error code otherwise.
 * @author Jim Edwards
 */
int get_regions(int ndims, const int *gdimlen, int maplen, const PIO_Offset *map,
                int *maxregions, io_region *firstregion)
{
    int nmaplen = 0;
    int regionlen;
    io_region *region;
    int ret;

    /* Check inputs. */
    pioassert(ndims >= 0 && gdimlen && maplen >= 0 && maxregions && firstregion,
              "invalid input", __FILE__, __LINE__);
    LOG((1, "get_regions ndims = %d maplen = %d", ndims, maplen));

    region = firstregion;
    if (map)
    {
        while (map[nmaplen++] <= 0)
        {
            LOG((3, "map[%d] = %d", nmaplen, map[nmaplen]));
            ;
        }
        nmaplen--;
    }
    region->loffset = nmaplen;
    LOG((2, "region->loffset = %d", region->loffset));

    *maxregions = 1;

    while (nmaplen < maplen)
    {
        /* Here we find the largest region from the current offset
           into the iomap. regionlen is the size of that region and we
           step to that point in the map array until we reach the
           end. */
        for (int i = 0; i < ndims; i++)
            region->count[i] = 1;

        /* Set start/count to describe first region in map. */
        regionlen = find_region(ndims, gdimlen, maplen-nmaplen,
                                &map[nmaplen], region->start, region->count);
        pioassert(region->start[0] >= 0, "failed to find region", __FILE__, __LINE__);

        nmaplen = nmaplen + regionlen;
        LOG((2, "regionlen = %d nmaplen = %d", regionlen, nmaplen));

        /* If we need to, allocate the next region. */
        if (region->next == NULL && nmaplen < maplen)
        {
            LOG((2, "allocating next region"));
            if ((ret = alloc_region2(NULL, ndims, &region->next)))
                return ret;

            /* The offset into the local array buffer is the sum of
             * the sizes of all of the previous regions (loffset) */
            region = region->next;
            region->loffset = nmaplen;

            /* The calls to the io library are collective and so we
               must have the same number of regions on each io task
               maxregions will be the total number of regions on this
               task. */
            (*maxregions)++;
            LOG((2, "*maxregions = %d", *maxregions));
        }
    }

    return PIO_NOERR;
}

/**
 * Create the MPI communicators needed by the subset rearranger.
 *
 * The subset rearranger needs a mapping from compute tasks to IO
 * task, the only requirement is that each compute task map to one and
 * only one IO task.  This mapping groups by mpi task id others are
 * possible and may be better for certain decompositions
 *
 * The as yet unrealized vision here is that the user would be able to
 * supply an alternative subset partitioning function. Requirements of
 * this function are that there be exactly one io task per compute
 * task group.
 *
 * @param ios pointer to the iosystem_desc_t struct.
 * @param iodesc a pointer to the io_desc_t struct.
 * @returns 0 on success, error code otherwise.
 * @author Jim Edwards
 */
int default_subset_partition(iosystem_desc_t *ios, io_desc_t *iodesc)
{
    int color;
    int key;
    int mpierr; /* Return value from MPI functions. */

    pioassert(ios && iodesc, "invalid input", __FILE__, __LINE__);
    LOG((1, "default_subset_partition ios->ioproc = %d ios->io_rank = %d "
         "ios->comp_rank = %d", ios->ioproc, ios->io_rank, ios->comp_rank));

    /* Create a new comm for each subset group with the io task in
       rank 0 and only 1 io task per group */
    if (ios->ioproc)
    {
        key = 0;
        color= ios->io_rank;
    }
    else
    {
        int taskratio = ios->num_comptasks / ios->num_iotasks;
        key = max(1, ios->comp_rank % taskratio + 1);
        color = min(ios->num_iotasks - 1, ios->comp_rank / taskratio);
    }
    LOG((3, "key = %d color = %d", key, color));

    /* Create new communicators. */
    if ((mpierr = MPI_Comm_split(ios->comp_comm, color, key, &iodesc->subset_comm)))
        return check_mpi(NULL, mpierr, __FILE__, __LINE__);

    return PIO_NOERR;
}

/**
 * Create the subset rearranger.
 *
 * The subset rearranger computes a mapping between IO tasks and
 * compute tasks such that each compute task communicates with one and
 * only one IO task.
 *
 * This function is called from PIOc_InitDecomp().
 *
 * This function:
 * <ul>
 * <li>Calls default_subset_partition() to create subset_comm.
 * <li>For IO tasks, allocates iodesc->rcount array (length ntasks).
 * <li>Allocates iodesc->scount array (length 1)
 * <li>Determins value of iodesc->scount[0], the number of data
 * elements on this compute task which are read/written.
 * <li>Allocated and inits iodesc->sindex (length iodesc->scount[0]),
 * init it to contain indicies to data.
 * <li>Pass the reduced maplen (without holes) from each compute task
 * to its associated IO task.
 * <li>On IO tasks, determine llen.
 * <li>Determine whether fill values will be needed.
 * <li>Pass iodesc->sindex from each compute task to its associated IO
 * task.
 * <li>Create shrtmap, which is compmap without the holes.
 * <li>Gather shrtmaps from each task into iomap.
 * <li>On IO tasks, sort the mapping, this will transpose the data
 * into IO order.
 * <li>On IO tasks, allocate and init iodesc->rindex and iodesc->rfrom
 * (length iodesc->llen).
 * <li>On IO tasks, handle fill values, if needed.
 * <li>On IO tasks, scatter values of srcindex to subset communicator.
 * <li>On IO tasks, call get_regions() and distribute the max
 * maxregions to all tasks in IO communicator.
 * <li>On IO tasks, call compute_maxIObuffersize().
 * </ul>
 *
 * @param ios pointer to the iosystem_desc_t struct.
 * @param maplen the length of the map.
 * @param compmap a 1 based array of offsets into the array record on
 * file. A 0 in this array indicates a value which should not be
 * transfered.
 * @param gdimlen an array length ndims with the sizes of the global
 * dimensions.
 * @param ndims the number of dimensions.
 * @param iodesc a pointer to the io_desc_t struct.
 * @returns 0 on success, error code otherwise.
 * @author Jim Edwards
 */
int subset_rearrange_create(iosystem_desc_t *ios, int maplen, PIO_Offset *compmap,
                            const int *gdimlen, int ndims, io_desc_t *iodesc)
{
    int i, j;
    PIO_Offset *iomap = NULL;
    mapsort *map = NULL;
    PIO_Offset totalgridsize;
    PIO_Offset *srcindex = NULL;
    PIO_Offset *myfillgrid = NULL;
    int maxregions;
    int rank, ntasks;
    int rcnt = 0;
    int mpierr; /* Return call from MPI function calls. */
    int ret;

    /* Check inputs. */
    pioassert(ios && maplen >= 0 && compmap && gdimlen && ndims >= 0 && iodesc,
              "invalid input", __FILE__, __LINE__);

    LOG((2, "subset_rearrange_create maplen = %d ndims = %d", maplen, ndims));

    /* subset partitions each have exactly 1 io task which is task 0
     * of that subset_comm */
    /* TODO: introduce a mechanism for users to define partitions */
    if ((ret = default_subset_partition(ios, iodesc)))
        return pio_err(ios, NULL, ret, __FILE__, __LINE__);
    iodesc->rearranger = PIO_REARR_SUBSET;

    /* Get size of this subset communicator and rank of this task in it. */
    if ((mpierr = MPI_Comm_rank(iodesc->subset_comm, &rank)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Comm_size(iodesc->subset_comm, &ntasks)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);

    /* Check rank for correctness. */
    if (ios->ioproc)
        pioassert(rank == 0, "Bad io rank in subset create", __FILE__, __LINE__);
    else
        pioassert(rank > 0 && rank < ntasks, "Bad comp rank in subset create",
                  __FILE__, __LINE__);

    /* Remember the maplen for this computation task. */
    iodesc->ndof = maplen;

    if (ios->ioproc)
    {
        /* Allocate space to hold count of data to be received in pio_swapm(). */
        if (!(iodesc->rcount = malloc(ntasks * sizeof(int))))
            return pio_err(ios, NULL, PIO_ENOMEM, __FILE__, __LINE__);

        rcnt = 1;
    }

    /* Allocate space to hold count of data to be sent in pio_swapm(). */
    if (!(iodesc->scount = malloc(sizeof(int))))
        return pio_err(ios, NULL, PIO_ENOMEM, __FILE__, __LINE__);

    iodesc->scount[0] = 0;

    /* Find the total size of the global data array. */
    totalgridsize = 1;
    for (i = 0; i < ndims; i++)
        totalgridsize *= gdimlen[i];

    /* Determine scount[0], the number of data elements in the
     * computation task that are to be written, by looking at
     * compmap. */
    for (i = 0; i < maplen; i++)
    {
        /*  turns out this can be allowed in some cases
            pioassert(compmap[i]>=0 && compmap[i]<=totalgridsize, "Compmap value out of bounds",
            __FILE__,__LINE__); */
        if (compmap[i] > 0)
            (iodesc->scount[0])++;
    }

    /* Allocate an array for indicies on the computation tasks (the
     * send side when writing). */
    if (iodesc->scount[0] > 0)
        if (!(iodesc->sindex = calloc(iodesc->scount[0], sizeof(PIO_Offset))))
            return pio_err(ios, NULL, PIO_ENOMEM, __FILE__, __LINE__);

    j = 0;
    for (i = 0; i < maplen; i++)
        if (compmap[i] > 0)
            iodesc->sindex[j++] = i;

    /* Pass the reduced maplen (without holes) from each compute task
     * to its associated IO task. */
    if ((mpierr = MPI_Gather(iodesc->scount, 1, MPI_INT, iodesc->rcount, rcnt,
                             MPI_INT, 0, iodesc->subset_comm)))
        return check_mpi(NULL, mpierr, __FILE__, __LINE__);

    iodesc->llen = 0;

    int rdispls[ntasks];
    int recvcounts[ntasks];

    /* On IO tasks determine llen. */
    if (ios->ioproc)
    {
        for (i = 0; i < ntasks; i++)
        {
            iodesc->llen += iodesc->rcount[i];
            rdispls[i] = 0;
            recvcounts[i] = iodesc->rcount[i];
            if (i > 0)
                rdispls[i] = rdispls[i - 1] + iodesc->rcount[i - 1];
        }

        if (iodesc->llen > 0)
        {
            if (!(srcindex = calloc(iodesc->llen, sizeof(PIO_Offset))))
                return pio_err(ios, NULL, PIO_ENOMEM, __FILE__, __LINE__);

            for (i = 0; i < iodesc->llen; i++)
                srcindex[i] = 0;
        }
    }
    else
    {
        for (i = 0; i < ntasks; i++)
        {
            recvcounts[i] = 0;
            rdispls[i] = 0;
        }
    }

    /* Determine whether fill values will be needed. */
    if ((ret = determine_fill(ios, iodesc, gdimlen, compmap)))
        return pio_err(ios, NULL, ret, __FILE__, __LINE__);

    /* Pass the sindex from each compute task to its associated IO task. */
    if ((mpierr = MPI_Gatherv(iodesc->sindex, iodesc->scount[0], PIO_OFFSET,
                              srcindex, recvcounts, rdispls, PIO_OFFSET, 0,
                              iodesc->subset_comm)))
        return check_mpi(NULL, mpierr, __FILE__, __LINE__);

    /* On IO tasks which need it, allocate memory for the map and the
     * iomap. */
    if (ios->ioproc && iodesc->llen > 0)
    {
        if (!(map = calloc(iodesc->llen, sizeof(mapsort))))
            return pio_err(ios, NULL, PIO_ENOMEM, __FILE__, __LINE__);

        if (!(iomap = calloc(iodesc->llen, sizeof(PIO_Offset))))
            return pio_err(ios, NULL, PIO_ENOMEM, __FILE__, __LINE__);
    }

    /* Now pass the compmap, skipping the holes. */
    PIO_Offset *shrtmap;
    if (maplen > iodesc->scount[0] && iodesc->scount[0] > 0)
    {
        if (!(shrtmap = calloc(iodesc->scount[0], sizeof(PIO_Offset))))
            return pio_err(ios, NULL, PIO_ENOMEM, __FILE__, __LINE__);

        j = 0;
        for (int i = 0; i < maplen; i++)
            if (compmap[i] > 0)
                shrtmap[j++] = compmap[i];
    }
    else
    {
        shrtmap = compmap;
    }

    /* Gather shrtmap from each task in the subset communicator, and
     * put gathered results into iomap. */
    if ((mpierr = MPI_Gatherv(shrtmap, iodesc->scount[0], PIO_OFFSET, iomap, recvcounts,
                              rdispls, PIO_OFFSET, 0, iodesc->subset_comm)))
        return check_mpi(NULL, mpierr, __FILE__, __LINE__);

    if (shrtmap != compmap)
        free(shrtmap);

    /* On IO tasks that have data in the local array ??? */
    if (ios->ioproc && iodesc->llen > 0)
    {
        int pos = 0;
        int k = 0;
        mapsort *mptr;
        for (i = 0; i < ntasks; i++)
        {
            for (j = 0; j < iodesc->rcount[i]; j++)
            {
                mptr = &map[k];
                mptr->rfrom = i;
                mptr->soffset = srcindex[pos + j];
                mptr->iomap = iomap[pos + j];
                k++;
            }
            pos += iodesc->rcount[i];
        }

        /* sort the mapping, this will transpose the data into IO order */
        qsort(map, iodesc->llen, sizeof(mapsort), compare_offsets);

        if (!(iodesc->rindex = calloc(1, iodesc->llen * sizeof(PIO_Offset))))
            return pio_err(ios, NULL, PIO_ENOMEM, __FILE__, __LINE__);

        if (!(iodesc->rfrom = calloc(1, iodesc->llen * sizeof(int))))
            return pio_err(ios, NULL, PIO_ENOMEM, __FILE__, __LINE__);
    }

    int cnt[ntasks];
    for (i = 0; i < ntasks; i++)
    {
        cnt[i] = rdispls[i];
    }

    /* For IO tasks init rfrom and rindex arrays (compute tasks have
     * llen of 0). */
    for (i = 0; i < iodesc->llen; i++)
    {
        mapsort *mptr = &map[i];
        iodesc->rfrom[i] = mptr->rfrom;
        iodesc->rindex[i] = i;
        iomap[i] = mptr->iomap;
        srcindex[(cnt[iodesc->rfrom[i]])++] = mptr->soffset;
    }

    /* Handle fill values if needed. */
    if (ios->ioproc && iodesc->needsfill)
    {
        /* we need the list of offsets which are not in the union of iomap */
        PIO_Offset thisgridsize[ios->num_iotasks];
        PIO_Offset thisgridmin[ios->num_iotasks], thisgridmax[ios->num_iotasks];
        int nio;
        PIO_Offset *myusegrid = NULL;
        int gcnt[ios->num_iotasks];
        int displs[ios->num_iotasks];

        thisgridmin[0] = 1;
        thisgridsize[0] =  totalgridsize / ios->num_iotasks;
        thisgridmax[0] = thisgridsize[0];
        int xtra = totalgridsize - thisgridsize[0] * ios->num_iotasks;

        for (nio = 0; nio < ios->num_iotasks; nio++)
        {
            int cnt = 0;
            int imin = 0;
            if (nio > 0)
            {
                thisgridsize[nio] =  totalgridsize / ios->num_iotasks;
                if (nio >= ios->num_iotasks - xtra)
                    thisgridsize[nio]++;
                thisgridmin[nio] = thisgridmax[nio - 1] + 1;
                thisgridmax[nio]= thisgridmin[nio] + thisgridsize[nio] - 1;
            }
            for (int i = 0; i < iodesc->llen; i++)
            {
                if (iomap[i] >= thisgridmin[nio] && iomap[i] <= thisgridmax[nio])
                {
                    cnt++;
                    if (cnt == 1)
                        imin = i;
                }
            }

            /* Gather cnt from all tasks in the IO communicator into array gcnt. */
            if ((mpierr = MPI_Gather(&cnt, 1, MPI_INT, gcnt, 1, MPI_INT, nio, ios->io_comm)))
                return check_mpi(NULL, mpierr, __FILE__, __LINE__);

            if (nio == ios->io_rank)
            {
                displs[0] = 0;
                for (i = 1; i < ios->num_iotasks; i++)
                    displs[i] = displs[i - 1] + gcnt[i - 1];

                /* Allocate storage for the grid. */
                if (!(myusegrid = malloc(thisgridsize[nio] * sizeof(PIO_Offset))))
                    return pio_err(ios, NULL, PIO_ENOMEM, __FILE__, __LINE__);

                /* Initialize the grid to all -1. */
                for (i = 0; i < thisgridsize[nio]; i++)
                    myusegrid[i] = -1;
            }

            if ((mpierr = MPI_Gatherv(&iomap[imin], cnt, PIO_OFFSET, myusegrid, gcnt,
                                      displs, PIO_OFFSET, nio, ios->io_comm)))
                return check_mpi(NULL, mpierr, __FILE__, __LINE__);
        }

        /* Allocate and initialize a grid to fill in missing values. ??? */
        PIO_Offset grid[thisgridsize[ios->io_rank]];
        for (i = 0; i < thisgridsize[ios->io_rank]; i++)
            grid[i] = 0;

        int cnt = 0;
        for (i = 0; i < thisgridsize[ios->io_rank]; i++)
        {
            int j = myusegrid[i] - thisgridmin[ios->io_rank];
            pioassert(j < thisgridsize[ios->io_rank], "out of bounds array index",
                      __FILE__, __LINE__);
            if (j >= 0)
            {
                grid[j] = 1;
                cnt++;
            }
        }
        if (myusegrid)
            free(myusegrid);

        iodesc->holegridsize = thisgridsize[ios->io_rank] - cnt;
        if (iodesc->holegridsize > 0)
        {
            /* Allocate space for the fillgrid. */
            if (!(myfillgrid = malloc(iodesc->holegridsize * sizeof(PIO_Offset))))
                return pio_err(ios, NULL, PIO_ENOMEM, __FILE__, __LINE__);
        }

        /* Initialize the fillgrid. */
        for (i = 0; i < iodesc->holegridsize; i++)
            myfillgrid[i] = -1;

        j = 0;
        for (i = 0; i < thisgridsize[ios->io_rank]; i++)
        {
            if (grid[i] == 0)
            {
                if (myfillgrid[j] == -1)
                    myfillgrid[j++] = thisgridmin[ios->io_rank] + i;
                else
                    return pio_err(ios, NULL, PIO_EINVAL, __FILE__, __LINE__);
            }
        }
        maxregions = 0;
        iodesc->maxfillregions = 0;
        if (myfillgrid)
        {
            /* Allocate a data region to hold fill values. */
            if ((ret = alloc_region2(ios, iodesc->ndims, &iodesc->fillregion)))
                return pio_err(ios, NULL, ret, __FILE__, __LINE__);
            if ((ret = get_regions(iodesc->ndims, gdimlen, iodesc->holegridsize, myfillgrid,
                                   &iodesc->maxfillregions, iodesc->fillregion)))
                return pio_err(ios, NULL, ret, __FILE__, __LINE__);
            free(myfillgrid);
            maxregions = iodesc->maxfillregions;
        }

        /* Get the max maxregions, and distribute it to all tasks in
         * the IO communicator. */
        if ((mpierr = MPI_Allreduce(MPI_IN_PLACE, &maxregions, 1, MPI_INT, MPI_MAX,
                                    ios->io_comm)))
            return check_mpi(NULL, mpierr, __FILE__, __LINE__);
        iodesc->maxfillregions = maxregions;

        /* Get the max maxholegridsize, and distribute it to all tasks
         * in the IO communicator. */
	iodesc->maxholegridsize = iodesc->holegridsize;
        if ((mpierr = MPI_Allreduce(MPI_IN_PLACE, &(iodesc->maxholegridsize), 1, MPI_INT,
                                    MPI_MAX, ios->io_comm)))
            return check_mpi(NULL, mpierr, __FILE__, __LINE__);
    }

    /* Scatter values of srcindex to subset communicator. ??? */
    if ((mpierr = MPI_Scatterv((void *)srcindex, recvcounts, rdispls, PIO_OFFSET,
                               (void *)iodesc->sindex, iodesc->scount[0],  PIO_OFFSET,
                               0, iodesc->subset_comm)))
        return check_mpi(NULL, mpierr, __FILE__, __LINE__);

    if (ios->ioproc)
    {
        iodesc->maxregions = 0;
        if ((ret = get_regions(iodesc->ndims, gdimlen, iodesc->llen, iomap,
                               &iodesc->maxregions, iodesc->firstregion)))
            return pio_err(ios, NULL, ret, __FILE__, __LINE__);
        maxregions = iodesc->maxregions;

        /* Get the max maxregions, and distribute it to all tasks in
         * the IO communicator. */
        if ((mpierr = MPI_Allreduce(MPI_IN_PLACE, &maxregions, 1, MPI_INT, MPI_MAX, ios->io_comm)))
            return check_mpi(NULL, mpierr, __FILE__, __LINE__);
        iodesc->maxregions = maxregions;

        /* Free resources. */
        if (iomap)
            free(iomap);

        if (map)
            free(map);

        if (srcindex)
            free(srcindex);

        /* Compute the max io buffer size needed for an iodesc. */
        if ((ret = compute_maxIObuffersize(ios->io_comm, iodesc)))
            return pio_err(ios, NULL, ret, __FILE__, __LINE__);

        iodesc->nrecvs = ntasks;
    }

    return PIO_NOERR;
}

/**
 * Performance tuning rearranger.
 *
 * @param ios pointer to the iosystem description struct.
 * @param iodesc pointer to the IO description struct.
 * @returns 0 on success, error code otherwise.
 * @author Jim Edwards
 */
void performance_tune_rearranger(iosystem_desc_t *ios, io_desc_t *iodesc)
{
#ifdef TIMING
#ifdef PERFTUNE
    double *wall, usr[2], sys[2];
    void *cbuf, *ibuf;
    int tsize;
    int myrank;
    int mpierr; /* Return code for MPI calls. */

    assert(iodesc);

    if ((mpierr = MPI_Type_size(iodesc->mpitype, &tsize)))
        return check_mpi(NULL, mpierr, __FILE__, __LINE__);
    cbuf = NULL;
    ibuf = NULL;
    if (iodesc->ndof > 0)
        if (!(cbuf = bget(iodesc->ndof * tsize)))
            return pio_err(ios, file, PIO_ENOMEM, __FILE__, __LINE__);

    if (iodesc->llen > 0)
        if (!(ibuf = bget(iodesc->llen * tsize)))
            return pio_err(ios, file, PIO_ENOMEM, __FILE__, __LINE__);

    if (iodesc->rearranger == PIO_REARR_BOX)
        mycomm = ios->union_comm;
    else
        mycomm = iodesc->subset_comm;

    if ((mpierr = MPI_Comm_size(mycomm, &nprocs)))
        return check_mpi(NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Comm_rank(mycomm, &myrank)))
        return check_mpi(NULL, mpierr, __FILE__, __LINE__);

    int log2 = log(nprocs) / log(2) + 1;
    if (!(wall = bget(2 * 4 * log2 * sizeof(double))))
        return pio_err(ios, file, PIO_ENOMEM, __FILE__, __LINE__);
    double mintime;
    int k = 0;

    if ((mpierr = MPI_Barrier(mycomm)))
        return check_mpi(NULL, mpierr, __FILE__, __LINE__);
    GPTLstamp(&wall[0], &usr[0], &sys[0]);
    rearrange_comp2io(ios, iodesc, cbuf, ibuf, 1);
    rearrange_io2comp(ios, iodesc, ibuf, cbuf);
    GPTLstamp(&wall[1], &usr[1], &sys[1]);
    mintime = wall[1]-wall[0];
    if ((mpierr = MPI_Allreduce(MPI_IN_PLACE, &mintime, 1, MPI_DOUBLE, MPI_MAX, mycomm)))
        return check_mpi(NULL, mpierr, __FILE__, __LINE__);

    handshake = iodesc->rearr_opts.comp2io.hs;
    isend = iodesc->isend;
    maxreqs = iodesc->max_requests;

    for (int i = 0; i < 2; i++)
    {
        if (i == 0)
            iodesc->rearr_opts.comp2io.hs = false;
        else
            iodesc->rearr_opts.comp2io.hs = true;

        for (int j = 0; j < 2; j++)
        {
            if (j == 0)
                iodesc->isend = false;
            else
                iodesc->isend = true;

            iodesc->max_requests = 0;

            for (nreqs = nprocs; nreqs >= 2; nreqs /= 2)
            {
                iodesc->max_requests = nreqs;
                if ((mpierr = MPI_Barrier(mycomm)))
                    return check_mpi(NULL, mpierr, __FILE__, __LINE__);
                GPTLstamp(wall, usr, sys);
                rearrange_comp2io(ios, iodesc, cbuf, ibuf, 1);
                rearrange_io2comp(ios, iodesc, ibuf, cbuf);
                GPTLstamp(wall+1, usr, sys);
                wall[1] -= wall[0];
                if ((mpierr = MPI_Allreduce(MPI_IN_PLACE, wall + 1, 1, MPI_DOUBLE, MPI_MAX,
                                            mycomm)))
                    return check_mpi(NULL, mpierr, __FILE__, __LINE__);

                if (wall[1] < mintime * 0.95)
                {
                    handshake = iodesc->rearr_opts.comp2io.hs;
                    isend = iodesc->isend;
                    maxreqs = nreqs;
                    mintime = wall[1];
                }
                else if (wall[1] > mintime * 1.05)
                {
                    exit;
                }
            }
        }
    }

    iodesc->rearr_opts.comp2io.hs = handshake;
    iodesc->isend = isend;
    iodesc->max_requests = maxreqs;

    LOG((1, "spmd optimization: maxreqs: %d handshake:%d isend:%d mintime=%f\n",
         maxreqs,handshake,isend,mintime));

    /* Free memory. */
    brel(wall);
    brel(cbuf);
    brel(ibuf);
#endif
#endif
}
