/** @file
 *
 * Private functions to help read and write distributed arrays in PIO.
 *
 * When arrays are distributed, each processor holds some of the
 * array. Only by combining the distributed arrays from all processor
 * can the full array be obtained.
 *
 * @author Jim Edwards
 */

#include <config.h>
#include <pio.h>
#include <pio_internal.h>

/* 10MB default limit. */
extern PIO_Offset pio_buffer_size_limit;

/* Initial size of compute buffer. */
bufsize pio_cnbuffer_limit = 33554432;

/* Global buffer pool pointer. */
extern void *CN_bpool;

/* Maximum buffer usage. */
extern PIO_Offset maxusage;

/* handler for freeing the memory buffer pool */
void bpool_free(void *p)
{
    free(p);
    if(p == CN_bpool){
        CN_bpool = NULL;
    }
}

/**
 * Initialize the compute buffer to size pio_cnbuffer_limit.
 *
 * This routine initializes the compute buffer pool if the bget memory
 * management is used. If malloc is used (that is, PIO_USE_MALLOC is
 * non zero), this function does nothing.
 *
 * @param ios pointer to the iosystem descriptor which will use the
 * new buffer.
 * @returns 0 for success, error code otherwise.
 * @author Jim Edwards
 */
int compute_buffer_init(iosystem_desc_t *ios)
{
#if !PIO_USE_MALLOC

    if (!CN_bpool)
    {
        if (!(CN_bpool = malloc(pio_cnbuffer_limit)))
            return pio_err(ios, NULL, PIO_ENOMEM, __FILE__, __LINE__);

        bpool(CN_bpool, pio_cnbuffer_limit);
        if (!CN_bpool)
            return pio_err(ios, NULL, PIO_ENOMEM, __FILE__, __LINE__);

        bectl(NULL, malloc, bpool_free, pio_cnbuffer_limit);
    }
#endif
    LOG((2, "compute_buffer_init complete"));

    return PIO_NOERR;
}

/**
 * Fill start/count arrays for write_darray_multi_par(). This is an
 * internal funciton.
 *
 * @param ndims the number of dims in the decomposition.
 * @param fndims the number of dims in the file.
 * @param vdesc pointer to the var_desc_t info.
 * @param region pointer to a region.
 * @param frame array of record values.
 * @param start an already-allocated array which gets the start
 * values.
 * @param count an already-allocated array which gets the count
 * values.
 * @return 0 for success, error code otherwise.
 * @ingroup PIO_write_darray
 * @author Ed Hartnett
 */
int find_start_count(int ndims, int fndims, var_desc_t *vdesc,
                     io_region *region, const int *frame, size_t *start,
                     size_t *count)
{
    /* Init start/count arrays to zero. */
    for (int i = 0; i < fndims; i++)
    {
        start[i] = 0;
        count[i] = 0;
    }

    if (region)
    {
        if (vdesc->record >= 0)
        {
            /* This is a record based multidimensional
             * array. Figure out start/count for all but the
             * record dimension (dimid 0). */
            for (int i = fndims - ndims; i < fndims; i++)
            {
                start[i] = region->start[i - (fndims - ndims)];
                count[i] = region->count[i - (fndims - ndims)];
            }

            /* Now figure out start/count for record dimension. */
            if (fndims > 1 && ndims < fndims && count[1] > 0)
            {
                count[0] = 1;
                start[0] = frame[0];
            }
            else if (fndims == ndims)
            {
                /* ??? */
                start[0] += vdesc->record;
            }
        }
        else
        {
            /* This is a non record variable. */
            for (int i = 0; i < ndims; i++)
            {
                start[i] = region->start[i];
                count[i] = region->count[i];
            }
        }

#if PIO_ENABLE_LOGGING
        /* Log arrays for debug purposes. */
        for (int i = 0; i < ndims; i++)
            LOG((3, "start[%d] = %d count[%d] = %d", i, start[i], i, count[i]));
#endif /* PIO_ENABLE_LOGGING */
    }

    return PIO_NOERR;
}

/**
 * Write a set of one or more aggregated arrays to output file. This
 * function is only used with parallel-netcdf and netcdf-4 parallel
 * iotypes. Serial io types use write_darray_multi_serial().
 *
 * @param file a pointer to the open file descriptor for the file
 * that will be written to
 * @param nvars the number of variables to be written with this
 * decomposition.
 * @param vid: an array of the variable ids to be written.
 * @param iodesc pointer to the io_desc_t info.
 * @param fill Non-zero if this write is fill data.
 * @param frame the record dimension for each of the nvars variables
 * in iobuf. NULL if this iodesc contains non-record vars.
 * @return 0 for success, error code otherwise.
 * @ingroup PIO_write_darray
 * @author Jim Edwards, Ed Hartnett
 */
int write_darray_multi_par(file_desc_t *file, int nvars, int fndims, const int *varids,
                           io_desc_t *iodesc, int fill, const int *frame)
{
    iosystem_desc_t *ios;  /* Pointer to io system information. */
    var_desc_t *vdesc;    /* Pointer to var info struct. */
    int dsize;             /* Data size (for one region). */
    int ierr = PIO_NOERR;

    /* Check inputs. */
    pioassert(file && file->iosystem && varids && varids[0] >= 0 && varids[0] <= PIO_MAX_VARS &&
              iodesc, "invalid input", __FILE__, __LINE__);

    LOG((1, "write_darray_multi_par nvars = %d iodesc->ndims = %d iodesc->mpitype = %d "
         "iodesc->maxregions = %d iodesc->llen = %d", nvars, iodesc->ndims,
         iodesc->mpitype, iodesc->maxregions, iodesc->llen));

#ifdef TIMING
    /* Start timing this function. */
    GPTLstart("PIO:write_darray_multi_par");
#endif

    /* Get pointer to iosystem. */
    ios = file->iosystem;

    /* Point to var description scruct for first var. */
    if ((ierr = get_var_desc(varids[0], &file->varlist, &vdesc)))
        return pio_err(NULL, file, ierr, __FILE__, __LINE__);

    /* Set these differently for data and fill writing. */
    int num_regions = fill ? iodesc->maxfillregions: iodesc->maxregions;
    io_region *region = fill ? iodesc->fillregion : iodesc->firstregion;
    PIO_Offset llen = fill ? iodesc->holegridsize : iodesc->llen;
    void *iobuf = fill ? vdesc->fillbuf : file->iobuf;

    /* If this is an IO task write the data. */
    if (ios->ioproc)
    {
        int rrcnt = 0; /* Number of subarray requests (pnetcdf only). */
        void *bufptr;
        size_t start[fndims];
        size_t count[fndims];
        int ndims = iodesc->ndims;
        PIO_Offset *startlist[num_regions]; /* Array of start arrays for ncmpi_iput_varn(). */
        PIO_Offset *countlist[num_regions]; /* Array of count  arrays for ncmpi_iput_varn(). */

        LOG((3, "num_regions = %d", num_regions));

        /* Process each region of data to be written. */
        for (int regioncnt = 0; regioncnt < num_regions; regioncnt++)
        {
            /* Fill the start/count arrays. */
            if ((ierr = find_start_count(iodesc->ndims, fndims, vdesc, region, frame,
                                         start, count)))
                return pio_err(ios, file, ierr, __FILE__, __LINE__);

            /* IO tasks will run the netCDF/pnetcdf functions to write the data. */
            switch (file->iotype)
            {
#ifdef _NETCDF4
            case PIO_IOTYPE_NETCDF4P:
                /* For each variable to be written. */
                for (int nv = 0; nv < nvars; nv++)
                {
                    /* Set the start of the record dimension. (Hasn't
                     * this already been set above ???) */
                    if (vdesc->record >= 0 && ndims < fndims)
                        start[0] = frame[nv];

                    /* If there is data for this region, get a pointer to it. */
                    if (region)
                        bufptr = (void *)((char *)iobuf + iodesc->mpitype_size * (nv * llen + region->loffset));

                    /* Ensure collective access. */
                    ierr = nc_var_par_access(file->fh, varids[nv], NC_COLLECTIVE);

                    /* Write the data for this variable. */
                    if (!ierr)
                        ierr = nc_put_vara(file->fh, varids[nv], (size_t *)start, (size_t *)count, bufptr);
                }
                break;
#endif
#ifdef _PNETCDF
            case PIO_IOTYPE_PNETCDF:
                /* Get the total number of data elements we are
                 * writing for this region. */
                dsize = 1;
                for (int i = 0; i < fndims; i++)
                    dsize *= count[i];
                LOG((3, "dsize = %d", dsize));

                /* For pnetcdf's ncmpi_iput_varn() function, we need
                 * to provide arrays of arrays for start/count. */
                if (dsize > 0)
                {
                    /* Allocate storage for start/count arrays for
                     * this region. */
                    if (!(startlist[rrcnt] = calloc(fndims, sizeof(PIO_Offset))))
                        return pio_err(ios, file, PIO_ENOMEM, __FILE__, __LINE__);
                    if (!(countlist[rrcnt] = calloc(fndims, sizeof(PIO_Offset))))
                        return pio_err(ios, file, PIO_ENOMEM, __FILE__, __LINE__);

                    /* Copy the start/count arrays for this region. */
                    for (int i = 0; i < fndims; i++)
                    {
                        startlist[rrcnt][i] = start[i];
                        countlist[rrcnt][i] = count[i];
                        LOG((3, "startlist[%d][%d] = %d countlist[%d][%d] = %d", rrcnt, i,
                             startlist[rrcnt][i], rrcnt, i, countlist[rrcnt][i]));
                    }
                    rrcnt++;
                }

                /* Do this when we reach the last region. */
                if (regioncnt == num_regions - 1)
                {
                    /* For each variable to be written. */
                    for (int nv = 0; nv < nvars; nv++)
                    {
                        /* Get the var info. */
                        if ((ierr = get_var_desc(varids[nv], &file->varlist, &vdesc)))
                            return pio_err(NULL, file, ierr, __FILE__, __LINE__);

                        /* If this is a record var, set the start for
                         * the record dimension. */
                        if (vdesc->record >= 0 && ndims < fndims)
                            for (int rc = 0; rc < rrcnt; rc++)
                                startlist[rc][0] = frame[nv];

                        /* Get a pointer to the data. */
                        bufptr = (void *)((char *)iobuf + nv * iodesc->mpitype_size * llen);

                        if (vdesc->nreqs % PIO_REQUEST_ALLOC_CHUNK == 0)
                        {
                            if (!(vdesc->request = realloc(vdesc->request, sizeof(int) *
                                                           (vdesc->nreqs + PIO_REQUEST_ALLOC_CHUNK))))
                                return pio_err(ios, file, PIO_ENOMEM, __FILE__, __LINE__);

                            for (int i = vdesc->nreqs; i < vdesc->nreqs + PIO_REQUEST_ALLOC_CHUNK; i++)
                                vdesc->request[i] = NC_REQ_NULL;
                        }

                        /* Write, in non-blocking fashion, a list of subarrays. */
                        LOG((3, "about to call ncmpi_iput_varn() varids[%d] = %d rrcnt = %d, llen = %d",
                             nv, varids[nv], rrcnt, llen));
                        ierr = ncmpi_iput_varn(file->fh, varids[nv], rrcnt, startlist, countlist,
                                               bufptr, llen, iodesc->mpitype, &vdesc->request[vdesc->nreqs]);

                        /* keeps wait calls in sync */
                        if (vdesc->request[vdesc->nreqs] == NC_REQ_NULL)
                            vdesc->request[vdesc->nreqs] = PIO_REQ_NULL;

                        vdesc->nreqs++;
                    }

                    /* Free resources. */
                    for (int i = 0; i < rrcnt; i++)
                    {
                        free(startlist[i]);
                        free(countlist[i]);
                    }
                }
                break;
#endif
            default:
                return pio_err(ios, file, PIO_EBADIOTYPE, __FILE__, __LINE__);
            }

            /* Go to next region. */
            if (region)
                region = region->next;
        } /* next regioncnt */
    } /* endif (ios->ioproc) */

    /* Check the return code from the netCDF/pnetcdf call. */
    ierr = check_netcdf(file, ierr, __FILE__,__LINE__);

#ifdef TIMING
    /* Stop timing this function. */
    GPTLstop("PIO:write_darray_multi_par");
#endif

    return ierr;
}

/**
 * Fill the tmp_start and tmp_count arrays, which contain the start
 * and count arrays for all regions.
 *
 * This is an internal function which is only called on io tasks. It
 * is called by write_darray_multi_serial().
 *
 * @param region pointer to the first in a linked list of regions.
 * @param maxregions the number of regions in the list.
 * @param fndims the number of dimensions in the file.
 * @param iodesc_ndims the number of dimensions in the decomposition.
 * @param vdesc pointer to an array of var_desc_t for the vars being
 * written.
 * @param tmp_start pointer to an already allocaed array of length
 * fndims * maxregions. This array will get the start values for all
 * regions.
 * @param tmp_count pointer to an already allocaed array of length
 * fndims * maxregions. This array will get the count values for all
 * regions.
 * @returns 0 for success, error code otherwise.
 * @ingroup PIO_read_darray
 * @author Jim Edwards, Ed Hartnett
 **/
int find_all_start_count(io_region *region, int maxregions, int fndims,
                         int iodesc_ndims, var_desc_t *vdesc, size_t *tmp_start,
                         size_t *tmp_count)
{
    /* Check inputs. */
    pioassert(maxregions >= 0 && fndims > 0 && iodesc_ndims >= 0 && vdesc &&
              tmp_start && tmp_count, "invalid input", __FILE__, __LINE__);

    /* Find the start/count arrays for each region in the list. */
    for (int r = 0; r < maxregions; r++)
    {
        /* Initialize the start/count arrays for this region to 0. */
        for (int d = 0; d < fndims; d++)
        {
            tmp_start[d + r * fndims] = 0;
            tmp_count[d + r * fndims] = 0;
        }

        if (region)
        {
            if (vdesc->record >= 0)
            {
                /* This is a record based multidimensional
                 * array. Copy start/count for non-record
                 * dimensions. */
                for (int i = fndims - iodesc_ndims; i < fndims; i++)
                {
                    tmp_start[i + r * fndims] = region->start[i - (fndims - iodesc_ndims)];
                    tmp_count[i + r * fndims] = region->count[i - (fndims - iodesc_ndims)];
                    LOG((3, "tmp_start[%d] = %d tmp_count[%d] = %d", i + r * fndims,
                         tmp_start[i + r * fndims], i + r * fndims,
                         tmp_count[i + r * fndims]));
                }
            }
            else
            {
                /* This is not a record based multidimensional array. */
                for (int i = 0; i < iodesc_ndims; i++)
                {
                    tmp_start[i + r * fndims] = region->start[i];
                    tmp_count[i + r * fndims] = region->count[i];
                    LOG((3, "tmp_start[%d] = %d tmp_count[%d] = %d", i + r * fndims,
                         tmp_start[i + r * fndims], i + r * fndims,
                         tmp_count[i + r * fndims]));
                }
            }

            /* Move to next region. */
            region = region->next;

        } /* endif region */
    } /* next r */

    return PIO_NOERR;
}

/**
 * Internal function called by IO tasks other than IO task 0 to send
 * their tmp_start/tmp_count arrays to IO task 0.
 *
 * This is an internal function which is only called on io tasks other
 * than IO task 0. It is called by write_darray_multi_serial().
 *
 * @return 0 for success, error code otherwise.
 * @ingroup PIO_write_darray
 * @author Jim Edwards, Ed Hartnett
 */
int send_all_start_count(iosystem_desc_t *ios, io_desc_t *iodesc, PIO_Offset llen,
                         int maxregions, int nvars, int fndims, size_t *tmp_start,
                         size_t *tmp_count, void *iobuf)
{
    MPI_Status status;     /* Recv status for MPI. */
    int mpierr;  /* Return code from MPI function codes. */
    int ierr;    /* Return code. */

    /* Check inputs. */
    pioassert(ios && ios->ioproc && ios->io_rank > 0 && maxregions >= 0,
              "invalid inputs", __FILE__, __LINE__);

    /* Do a handshake. */
    if ((mpierr = MPI_Recv(&ierr, 1, MPI_INT, 0, 0, ios->io_comm, &status)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);

    /* Send local length of iobuffer for each field (all
     * fields are the same length). */
    if ((mpierr = MPI_Send((void *)&llen, 1, MPI_OFFSET, 0, ios->io_rank, ios->io_comm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    LOG((3, "sent llen = %d", llen));

    /* Send the number of data regions, the start/count for
     * all regions, and the data buffer with all the data. */
    if (llen > 0)
    {
        if ((mpierr = MPI_Send((void *)&maxregions, 1, MPI_INT, 0, ios->io_rank + ios->num_iotasks,
                               ios->io_comm)))
            return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
        if ((mpierr = MPI_Send(tmp_start, maxregions * fndims, MPI_OFFSET, 0,
                               ios->io_rank + 2 * ios->num_iotasks, ios->io_comm)))
            return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
        if ((mpierr = MPI_Send(tmp_count, maxregions * fndims, MPI_OFFSET, 0,
                               ios->io_rank + 3 * ios->num_iotasks, ios->io_comm)))
            return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
        if ((mpierr = MPI_Send(iobuf, nvars * llen, iodesc->mpitype, 0,
                               ios->io_rank + 4 * ios->num_iotasks, ios->io_comm)))
            return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
        LOG((3, "sent data for maxregions = %d", maxregions));
    }

    return PIO_NOERR;
}

/**
 * This is an internal function that is run only on IO proc 0. It
 * receives data from all the other IO tasks, and write that data to
 * disk. This is called from write_darray_multi_serial().
 *
 * @param file a pointer to the open file descriptor for the file
 * that will be written to.
 * @param varids an array of the variable ids to be written
 * @param frame the record dimension for each of the nvars variables
 * in iobuf.  NULL if this iodesc contains non-record vars.
 * @param iodesc pointer to the decomposition info.
 * @param llen length of the iobuffer on this task for a single
 * field.
 * @param maxregions max number of blocks to be written from this
 * iotask.
 * @param nvars the number of variables to be written with this
 * decomposition.
 * @param fndims the number of dimensions in the file.
 * @param tmp_start pointer to an already allocaed array of length
 * fndims * maxregions. This array will get the start values for all
 * regions.
 * @param tmp_count pointer to an already allocaed array of length
 * fndims * maxregions. This array will get the count values for all
 * regions.
 * @param iobuf the buffer to be written from this mpi task. May be
 * null. for example we have 8 ionodes and a distributed array with
 * global size 4, then at least 4 nodes will have a null iobuf. In
 * practice the box rearranger trys to have at least blocksize bytes
 * on each io task and so if the total number of bytes to write is
 * less than blocksize*numiotasks then some iotasks will have a NULL
 * iobuf.
 * @return 0 for success, error code otherwise.
 * @ingroup PIO_write_darray
 * @author Jim Edwards, Ed Hartnett
 */
int recv_and_write_data(file_desc_t *file, const int *varids, const int *frame,
                        io_desc_t *iodesc, PIO_Offset llen, int maxregions, int nvars,
                        int fndims, size_t *tmp_start, size_t *tmp_count, void *iobuf)
{
    iosystem_desc_t *ios;  /* Pointer to io system information. */
    size_t rlen;    /* Length of IO buffer on this task. */
    int rregions;   /* Number of regions in buffer for this task. */
    size_t start[fndims], count[fndims];
    size_t loffset;
    void *bufptr;
    var_desc_t *vdesc;    /* Contains info about the variable. */
    MPI_Status status;     /* Recv status for MPI. */
    int mpierr;  /* Return code from MPI function codes. */
    int ierr;    /* Return code. */

    /* Check inputs. */
    pioassert(file && varids && iodesc && tmp_start && tmp_count, "invalid input",
              __FILE__, __LINE__);

    LOG((2, "recv_and_write_data llen = %d maxregions = %d nvars = %d fndims = %d",
         llen, maxregions, nvars, fndims));

    /* Get pointer to IO system. */
    ios = file->iosystem;

    /* For each of the other tasks that are using this task
     * for IO. */
    for (int rtask = 0; rtask < ios->num_iotasks; rtask++)
    {
        /* From the remote tasks, we send information about
         * the data regions. and also the data. */
        if (rtask)
        {
            /* handshake - tell the sending task I'm ready */
            if ((mpierr = MPI_Send(&ierr, 1, MPI_INT, rtask, 0, ios->io_comm)))
                return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);

            /* Get length of iobuffer for each field on this
             * task (all fields are the same length). */
            if ((mpierr = MPI_Recv(&rlen, 1, MPI_OFFSET, rtask, rtask, ios->io_comm,
                                   &status)))
                return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
            LOG((3, "received rlen = %d", rlen));

            /* Get the number of regions, the start/count
             * values for all regions, and the data buffer. */
            if (rlen > 0)
            {
                if ((mpierr = MPI_Recv(&rregions, 1, MPI_INT, rtask, rtask + ios->num_iotasks,
                                       ios->io_comm, &status)))
                    return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
                if ((mpierr = MPI_Recv(tmp_start, rregions * fndims, MPI_OFFSET, rtask,
                                       rtask + 2 * ios->num_iotasks, ios->io_comm, &status)))
                    return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
                if ((mpierr = MPI_Recv(tmp_count, rregions * fndims, MPI_OFFSET, rtask,
                                       rtask + 3 * ios->num_iotasks, ios->io_comm, &status)))
                    return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
                if ((mpierr = MPI_Recv(iobuf, nvars * rlen, iodesc->mpitype, rtask,
                                       rtask + 4 * ios->num_iotasks, ios->io_comm, &status)))
                    return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
                LOG((3, "received data rregions = %d fndims = %d", rregions, fndims));
            }
        }
        else /* task 0 */
        {
            rlen = llen;
            rregions = maxregions;
        }
        LOG((3, "rtask = %d rlen = %d rregions = %d", rtask, rlen, rregions));

        /* If there is data from this task, write it. */
        if (rlen > 0)
        {
            loffset = 0;
            for (int regioncnt = 0; regioncnt < rregions; regioncnt++)
            {
                LOG((3, "writing data for region with regioncnt = %d", regioncnt));

                /* Get the start/count arrays for this region. */
                for (int i = 0; i < fndims; i++)
                {
                    start[i] = tmp_start[i + regioncnt * fndims];
                    count[i] = tmp_count[i + regioncnt * fndims];
                    LOG((3, "start[%d] = %d count[%d] = %d", i, start[i], i, count[i]));
                }

                /* Process each variable in the buffer. */
                for (int nv = 0; nv < nvars; nv++)
                {
                    LOG((3, "writing buffer var %d", nv));
                    if ((ierr = get_var_desc(varids[0], &file->varlist, &vdesc)))
                        return pio_err(NULL, file, ierr, __FILE__, __LINE__);

                    /* Get a pointer to the correct part of the buffer. */
                    bufptr = (void *)((char *)iobuf + iodesc->mpitype_size * (nv * rlen + loffset));

                    /* If this var has an unlimited dim, set
                     * the start on that dim to the frame
                     * value for this variable. */
                    if (vdesc->record >= 0)
                    {
                        if (fndims > 1 && iodesc->ndims < fndims && count[1] > 0)
                        {
                            count[0] = 1;
                            start[0] = frame[nv];
                        }
                        else if (fndims == iodesc->ndims)
                        {
                            start[0] += vdesc->record;
                        }
                    }

                    /* Call the netCDF functions to write the data. */
                    if ((ierr = nc_put_vara(file->fh, varids[nv], start, count, bufptr)))
                        return check_netcdf2(ios, NULL, ierr, __FILE__, __LINE__);

                } /* next var */

                /* Calculate the total size. */
                size_t tsize = 1;
                for (int i = 0; i < fndims; i++)
                    tsize *= count[i];

                /* Keep track of where we are in the buffer. */
                loffset += tsize;

                LOG((3, " at bottom of loop regioncnt = %d tsize = %d loffset = %d", regioncnt,
                     tsize, loffset));
            } /* next regioncnt */
        } /* endif (rlen > 0) */
    } /* next rtask */

    return PIO_NOERR;
}

/**
 * Write a set of one or more aggregated arrays to output file in
 * serial mode. This function is called for netCDF classic and
 * netCDF-4 serial iotypes. Parallel iotypes use
 * write_darray_multi_par().
 *
 * @param file a pointer to the open file descriptor for the file
 * that will be written to.
 * @param nvars the number of variables to be written with this
 * decomposition.
 * @param varids an array of the variable ids to be written
 * @param iodesc pointer to the decomposition info.
 * @param fill Non-zero if this write is fill data.
 * @param frame the record dimension for each of the nvars variables
 * in iobuf. NULL if this iodesc contains non-record vars.
 * @return 0 for success, error code otherwise.
 * @ingroup PIO_write_darray
 * @author Jim Edwards, Ed Hartnett
 */
int write_darray_multi_serial(file_desc_t *file, int nvars, int fndims, const int *varids,
                              io_desc_t *iodesc, int fill, const int *frame)
{
    iosystem_desc_t *ios;  /* Pointer to io system information. */
    var_desc_t *vdesc;     /* Contains info about the variable. */
    int ierr;              /* Return code. */

    /* Check inputs. */
    pioassert(file && file->iosystem && varids && varids[0] >= 0 &&
              varids[0] <= PIO_MAX_VARS && iodesc, "invalid input", __FILE__, __LINE__);

    LOG((1, "write_darray_multi_serial nvars = %d fndims = %d iodesc->ndims = %d "
         "iodesc->mpitype = %d", nvars, iodesc->ndims, fndims, iodesc->mpitype));

    /* Get the iosystem info. */
    ios = file->iosystem;

    /* Get the var info. */
    if ((ierr = get_var_desc(varids[0], &file->varlist, &vdesc)))
        return pio_err(NULL, file, ierr, __FILE__, __LINE__);

    /* Set these differently for data and fill writing. iobuf may be
     * null if array size < number of nodes. */
    int num_regions = fill ? iodesc->maxfillregions: iodesc->maxregions;
    io_region *region = fill ? iodesc->fillregion : iodesc->firstregion;
    PIO_Offset llen = fill ? iodesc->holegridsize : iodesc->llen;
    void *iobuf = fill ? vdesc->fillbuf : file->iobuf;

#ifdef TIMING
    /* Start timing this function. */
    GPTLstart("PIO:write_darray_multi_serial");
#endif

    /* Only IO tasks participate in this code. */
    if (ios->ioproc)
    {
        size_t tmp_start[fndims * num_regions]; /* A start array for each region. */
        size_t tmp_count[fndims * num_regions]; /* A count array for each region. */

        LOG((3, "num_regions = %d", num_regions));

        /* Fill the tmp_start and tmp_count arrays, which contain the
         * start and count arrays for all regions. */
        if ((ierr = find_all_start_count(region, num_regions, fndims, iodesc->ndims, vdesc,
                                         tmp_start, tmp_count)))
            return pio_err(ios, file, ierr, __FILE__, __LINE__);

        /* Tasks other than 0 will send their data to task 0. */
        if (ios->io_rank > 0)
        {
            /* Send the tmp_start and tmp_count arrays from this IO task
             * to task 0. */
            if ((ierr = send_all_start_count(ios, iodesc, llen, num_regions, nvars, fndims,
                                             tmp_start, tmp_count, iobuf)))
                return pio_err(ios, file, ierr, __FILE__, __LINE__);
        }
        else
        {
            /* Task 0 will receive data from all other IO tasks. */

            if ((ierr = recv_and_write_data(file, varids, frame, iodesc, llen, num_regions, nvars, fndims,
                                            tmp_start, tmp_count, iobuf)))
                return pio_err(ios, file, ierr, __FILE__, __LINE__);
        }
    }

#ifdef TIMING
    /* Stop timing this function. */
    GPTLstop("PIO:write_darray_multi_serial");
#endif

    return PIO_NOERR;
}

/**
 * Read an array of data from a file to the (parallel) IO library.
 *
 * @param file a pointer to the open file descriptor for the file
 * that will be written to
 * @param iodesc a pointer to the defined iodescriptor for the buffer
 * @param vid the variable id to be read
 * @param iobuf the buffer to be read into from this mpi task. May be
 * null. for example we have 8 ionodes and a distributed array with
 * global size 4, then at least 4 nodes will have a null iobuf. In
 * practice the box rearranger trys to have at least blocksize bytes
 * on each io task and so if the total number of bytes to write is
 * less than blocksize*numiotasks then some iotasks will have a NULL
 * iobuf.
 * @return 0 on success, error code otherwise.
 * @ingroup PIO_read_darray
 * @author Jim Edwards, Ed Hartnett
 */
int pio_read_darray_nc(file_desc_t *file, io_desc_t *iodesc, int vid, void *iobuf)
{
    iosystem_desc_t *ios;  /* Pointer to io system information. */
    var_desc_t *vdesc;     /* Information about the variable. */
    int ndims;             /* Number of dims in decomposition. */
    int fndims;            /* Number of dims for this var in file. */
    int ierr;              /* Return code from netCDF functions. */

    /* Check inputs. */
    pioassert(file && file->iosystem && iodesc && vid <= PIO_MAX_VARS, "invalid input",
              __FILE__, __LINE__);

#ifdef TIMING
    /* Start timing this function. */
    GPTLstart("PIO:read_darray_nc");
#endif

    /* Get the IO system info. */
    ios = file->iosystem;

    /* Get the variable info. */
    if ((ierr = get_var_desc(vid, &file->varlist, &vdesc)))
        return pio_err(NULL, file, ierr, __FILE__, __LINE__);

    /* Get the number of dimensions in the decomposition. */
    ndims = iodesc->ndims;

    /* Get the number of dims for this var in the file. */
    if ((ierr = PIOc_inq_varndims(file->pio_ncid, vid, &fndims)))
        return pio_err(ios, file, ierr, __FILE__, __LINE__);

    /* IO procs will actially read the data. */
    if (ios->ioproc)
    {
        io_region *region;
        size_t start[fndims];
        size_t count[fndims];
        size_t tmp_bufsize = 1;
        void *bufptr;
        int rrlen = 0;
        PIO_Offset *startlist[iodesc->maxregions];
        PIO_Offset *countlist[iodesc->maxregions];

        /* buffer is incremented by byte and loffset is in terms of
           the iodessc->mpitype so we need to multiply by the size of
           the mpitype. */
        region = iodesc->firstregion;

        /* ??? */
        if (fndims > ndims)
        {
            ndims++;
        }

        /* For each regions, read the data. */
        for (int regioncnt = 0; regioncnt < iodesc->maxregions; regioncnt++)
        {
            tmp_bufsize = 1;
            if (region == NULL || iodesc->llen == 0)
            {
                /* No data for this region. */
                for (int i = 0; i < fndims; i++)
                {
                    start[i] = 0;
                    count[i] = 0;
                }
                bufptr = NULL;
            }
            else
            {
                /* Get a pointer where we should put the data we read. */
                if (regioncnt == 0 || region == NULL)
                    bufptr = iobuf;
                else
                    bufptr=(void *)((char *)iobuf + iodesc->mpitype_size * region->loffset);

                LOG((2, "%d %d %d", iodesc->llen - region->loffset, iodesc->llen, region->loffset));

                /* Get the start/count arrays. */
                if (vdesc->record >= 0 && fndims > 1)
                {
                    /* This is a record var. The unlimited dimension
                     * (0) is handled specially. */
                    start[0] = vdesc->record;
                    for (int i = 1; i < ndims; i++)
                    {
                        start[i] = region->start[i-1];
                        count[i] = region->count[i-1];
                    }

                    /* Read one record. */
                    if (count[1] > 0)
                        count[0] = 1;
                }
                else
                {
                    /* Non-time dependent array */
                    for (int i = 0; i < ndims; i++)
                    {
                        start[i] = region->start[i];
                        count[i] = region->count[i];
                    }
                }
            }

            /* Do the read. */
            switch (file->iotype)
            {
#ifdef _NETCDF4
            case PIO_IOTYPE_NETCDF4P:
                /* ierr = nc_get_vara(file->fh, vid, start, count, bufptr); */
                switch (iodesc->piotype)
                {
                case PIO_BYTE:
                    ierr = nc_get_vara_schar(file->fh, vid, start, count, (signed char*)bufptr);
                    break;
                case PIO_CHAR:
                    ierr = nc_get_vara_text(file->fh, vid, start, count, (char*)bufptr);
                    break;
                case PIO_SHORT:
                    ierr = nc_get_vara_short(file->fh, vid, start, count, (short*)bufptr);
                    break;
                case PIO_INT:
                    ierr = nc_get_vara_int(file->fh, vid, start, count, (int*)bufptr);
                    break;
                case PIO_FLOAT:
                    ierr = nc_get_vara_float(file->fh, vid, start, count, (float*)bufptr);
                    break;
                case PIO_DOUBLE:
                    ierr = nc_get_vara_double(file->fh, vid, start, count, (double*)bufptr);
                    break;
                case PIO_UBYTE:
                    ierr = nc_get_vara_uchar(file->fh, vid, start, count, (unsigned char*)bufptr);
                    break;
                case PIO_USHORT:
                    ierr = nc_get_vara_ushort(file->fh, vid, start, count, (unsigned short*)bufptr);
                    break;
                case PIO_UINT:
                    ierr = nc_get_vara_uint(file->fh, vid, start, count, (unsigned int*)bufptr);
                    break;
                case PIO_INT64:
                    ierr = nc_get_vara_longlong(file->fh, vid, start, count, (long long*)bufptr);
                    break;
                case PIO_UINT64:
                    ierr = nc_get_vara_ulonglong(file->fh, vid, start, count, (unsigned long long*)bufptr);
                    break;
                case PIO_STRING:
                    ierr = nc_get_vara_string(file->fh, vid, start, count, (char**)bufptr);
                    break;
                default:
                    return pio_err(ios, file, PIO_EBADTYPE, __FILE__, __LINE__);
                }
                break;
#endif
#ifdef _PNETCDF
            case PIO_IOTYPE_PNETCDF:
            {
                tmp_bufsize = 1;
                for (int j = 0; j < fndims; j++)
                    tmp_bufsize *= count[j];

                if (tmp_bufsize > 0)
                {
                    startlist[rrlen] = bget(fndims * sizeof(PIO_Offset));
                    countlist[rrlen] = bget(fndims * sizeof(PIO_Offset));

                    for (int j = 0; j < fndims; j++)
                    {
                        startlist[rrlen][j] = start[j];
                        countlist[rrlen][j] = count[j];
                    }
                    rrlen++;
                }

                /* Is this is the last region to process? */
                if (regioncnt == iodesc->maxregions - 1)
                {
                    /* Read a list of subarrays. */
                    ierr = ncmpi_get_varn_all(file->fh, vid, rrlen, startlist,
                                              countlist, iobuf, iodesc->llen, iodesc->mpitype);

                    /* Release the start and count arrays. */
                    for (int i = 0; i < rrlen; i++)
                    {
                        brel(startlist[i]);
                        brel(countlist[i]);
                    }
                }
            }
            break;
#endif
            default:
                return pio_err(ios, file, PIO_EBADIOTYPE, __FILE__, __LINE__);
            }

            /* Check return code. */
            if (ierr)
                return check_netcdf(file, ierr, __FILE__,__LINE__);

            /* Move to next region. */
            if (region)
                region = region->next;
        } /* next regioncnt */
    }

#ifdef TIMING
    /* Stop timing this function. */
    GPTLstop("PIO:read_darray_nc");
#endif

    return PIO_NOERR;
}

/**
 * Read an array of data from a file to the (serial) IO library. This
 * function is only used with netCDF classic and netCDF-4 serial
 * iotypes.
 *
 * @param file a pointer to the open file descriptor for the file
 * that will be written to
 * @param iodesc a pointer to the defined iodescriptor for the buffer
 * @param vid the variable id to be read.
 * @param iobuf the buffer to be written from this mpi task. May be
 * null. for example we have 8 ionodes and a distributed array with
 * global size 4, then at least 4 nodes will have a null iobuf. In
 * practice the box rearranger trys to have at least blocksize bytes
 * on each io task and so if the total number of bytes to write is
 * less than blocksize * numiotasks then some iotasks will have a NULL
 * iobuf.
 * @returns 0 for success, error code otherwise.
 * @ingroup PIO_read_darray
 * @author Jim Edwards, Ed Hartnett
 */
int pio_read_darray_nc_serial(file_desc_t *file, io_desc_t *iodesc, int vid,
                              void *iobuf)
{
    iosystem_desc_t *ios;  /* Pointer to io system information. */
    var_desc_t *vdesc;    /* Information about the variable. */
    int ndims;             /* Number of dims in decomposition. */
    int fndims;            /* Number of dims for this var in file. */
    MPI_Status status;
    int mpierr;  /* Return code from MPI functions. */
    int ierr;

    /* Check inputs. */
    pioassert(file && file->iosystem && iodesc && vid >= 0 && vid <= PIO_MAX_VARS,
              "invalid input", __FILE__, __LINE__);

    LOG((2, "pio_read_darray_nc_serial vid = %d", vid));
#ifdef TIMING
    /* Start timing this function. */
    GPTLstart("PIO:read_darray_nc_serial");
#endif
    ios = file->iosystem;

    /* Get var info for this var. */
    if ((ierr = get_var_desc(vid, &file->varlist, &vdesc)))
        return pio_err(NULL, file, ierr, __FILE__, __LINE__);

    /* Get the number of dims in our decomposition. */
    ndims = iodesc->ndims;

    /* Get number of dims for this var. */
    if ((ierr = PIOc_inq_varndims(file->pio_ncid, vid, &fndims)))
        return pio_err(ios, file, ierr, __FILE__, __LINE__);

    /* Confirm that we are being called with the correct ndims. */
    pioassert((fndims == ndims && vdesc->record < 0) ||
              (fndims == ndims + 1 && vdesc->record >= 0),
              "unexpected record", __FILE__, __LINE__);

    if (ios->ioproc)
    {
        io_region *region;
        size_t start[fndims];
        size_t count[fndims];
        size_t tmp_start[fndims * iodesc->maxregions];
        size_t tmp_count[fndims * iodesc->maxregions];
        size_t tmp_bufsize;
        void *bufptr;

        /* buffer is incremented by byte and loffset is in terms of
           the iodessc->mpitype so we need to multiply by the size of
           the mpitype. */
        region = iodesc->firstregion;

        /* Put together start/count arrays for all regions. */
        for (int regioncnt = 0; regioncnt < iodesc->maxregions; regioncnt++)
        {
            if (!region || iodesc->llen == 0)
            {
                /* Nothing to write for this region. */
                for (int i = 0; i < fndims; i++)
                {
                    tmp_start[i + regioncnt * fndims] = 0;
                    tmp_count[i + regioncnt * fndims] = 0;
                }
                bufptr = NULL;
            }
            else
            {
                if (vdesc->record >= 0 && fndims > 1)
                {
                    /* This is a record var. Find start for record dims. */
                    tmp_start[regioncnt * fndims] = vdesc->record;

                    /* Find start/count for all non-record dims. */
                    for (int i = 1; i < fndims; i++)
                    {
                        tmp_start[i + regioncnt * fndims] = region->start[i - 1];
                        tmp_count[i + regioncnt * fndims] = region->count[i - 1];
                    }

                    /* Set count for record dimension. */
                    if (tmp_count[1 + regioncnt * fndims] > 0)
                        tmp_count[regioncnt * fndims] = 1;
                }
                else
                {
                    /* Non-time dependent array */
                    for (int i = 0; i < fndims; i++)
                    {
                        tmp_start[i + regioncnt * fndims] = region->start[i];
                        tmp_count[i + regioncnt * fndims] = region->count[i];
                    }
                }
            }

#if PIO_ENABLE_LOGGING
            /* Log arrays for debug purposes. */
            LOG((3, "region = %d", region));
            for (int i = 0; i < fndims; i++)
                LOG((3, "tmp_start[%d] = %d tmp_count[%d] = %d", i + regioncnt * fndims, tmp_start[i + regioncnt * fndims],
                     i + regioncnt * fndims, tmp_count[i + regioncnt * fndims]));
#endif /* PIO_ENABLE_LOGGING */

            /* Move to next region. */
            if (region)
                region = region->next;
        } /* next regioncnt */

        /* IO tasks other than 0 send their starts/counts and data to
         * IO task 0. */
        if (ios->io_rank > 0)
        {
            if ((mpierr = MPI_Send(&iodesc->llen, 1, MPI_OFFSET, 0, ios->io_rank, ios->io_comm)))
                return check_mpi(file, mpierr, __FILE__, __LINE__);
            LOG((3, "sent iodesc->llen = %d", iodesc->llen));

            if (iodesc->llen > 0)
            {
                if ((mpierr = MPI_Send(&(iodesc->maxregions), 1, MPI_INT, 0,
                                       ios->num_iotasks + ios->io_rank, ios->io_comm)))
                    return check_mpi(file, mpierr, __FILE__, __LINE__);
                if ((mpierr = MPI_Send(tmp_count, iodesc->maxregions * fndims, MPI_OFFSET, 0,
                                       2 * ios->num_iotasks + ios->io_rank, ios->io_comm)))
                    return check_mpi(file, mpierr, __FILE__, __LINE__);
                if ((mpierr = MPI_Send(tmp_start, iodesc->maxregions * fndims, MPI_OFFSET, 0,
                                       3 * ios->num_iotasks + ios->io_rank, ios->io_comm)))
                    return check_mpi(file, mpierr, __FILE__, __LINE__);
                LOG((3, "sent iodesc->maxregions = %d tmp_count and tmp_start arrays", iodesc->maxregions));

                if ((mpierr = MPI_Recv(iobuf, iodesc->llen, iodesc->mpitype, 0,
                                       4 * ios->num_iotasks + ios->io_rank, ios->io_comm, &status)))
                    return check_mpi(file, mpierr, __FILE__, __LINE__);
                LOG((3, "received %d elements of data", iodesc->llen));
            }
        }
        else if (ios->io_rank == 0)
        {
            /* This is IO task 0. Get starts/counts and data from
             * other IO tasks. */
            int maxregions = 0;
            size_t loffset, regionsize;
            size_t this_start[fndims * iodesc->maxregions];
            size_t this_count[fndims * iodesc->maxregions];

            for (int rtask = 1; rtask <= ios->num_iotasks; rtask++)
            {
                if (rtask < ios->num_iotasks)
                {
                    if ((mpierr = MPI_Recv(&tmp_bufsize, 1, MPI_OFFSET, rtask, rtask, ios->io_comm, &status)))
                        return check_mpi(file, mpierr, __FILE__, __LINE__);
                    LOG((3, "received tmp_bufsize = %d", tmp_bufsize));

                    if (tmp_bufsize > 0)
                    {
                        if ((mpierr = MPI_Recv(&maxregions, 1, MPI_INT, rtask, ios->num_iotasks + rtask,
                                               ios->io_comm, &status)))
                            return check_mpi(file, mpierr, __FILE__, __LINE__);
                        if ((mpierr = MPI_Recv(this_count, maxregions * fndims, MPI_OFFSET, rtask,
                                               2 * ios->num_iotasks + rtask, ios->io_comm, &status)))
                            return check_mpi(file, mpierr, __FILE__, __LINE__);
                        if ((mpierr = MPI_Recv(this_start, maxregions * fndims, MPI_OFFSET, rtask,
                                               3 * ios->num_iotasks + rtask, ios->io_comm, &status)))
                            return check_mpi(file, mpierr, __FILE__, __LINE__);
                        LOG((3, "received maxregions = %d this_count, this_start arrays ", maxregions));
                    }
                }
                else
                {
                    maxregions = iodesc->maxregions;
                    tmp_bufsize = iodesc->llen;
                }
                LOG((3, "maxregions = %d tmp_bufsize = %d", maxregions, tmp_bufsize));

                /* Now get each region of data. */
                loffset = 0;
                for (int regioncnt = 0; regioncnt < maxregions; regioncnt++)
                {
                    /* Get pointer where data should go. */
                    bufptr = (void *)((char *)iobuf + iodesc->mpitype_size * loffset);
                    regionsize = 1;

                    /* ??? */
                    if (rtask < ios->num_iotasks)
                    {
                        for (int m = 0; m < fndims; m++)
                        {
                            start[m] = this_start[m + regioncnt * fndims];
                            count[m] = this_count[m + regioncnt * fndims];
                            regionsize *= count[m];
                        }
                    }
                    else
                    {
                        for (int m = 0; m < fndims; m++)
                        {
                            start[m] = tmp_start[m + regioncnt * fndims];
                            count[m] = tmp_count[m + regioncnt * fndims];
                            regionsize *= count[m];
                        }
                    }
                    loffset += regionsize;

                    /* Read the data. */
                    /* ierr = nc_get_vara(file->fh, vid, start, count, bufptr); */
                    switch (iodesc->piotype)
                    {
                    case PIO_BYTE:
                        ierr = nc_get_vara_schar(file->fh, vid, start, count, (signed char*)bufptr);
                        break;
                    case PIO_CHAR:
                        ierr = nc_get_vara_text(file->fh, vid, start, count, (char*)bufptr);
                        break;
                    case PIO_SHORT:
                        ierr = nc_get_vara_short(file->fh, vid, start, count, (short*)bufptr);
                        break;
                    case PIO_INT:
                        ierr = nc_get_vara_int(file->fh, vid, start, count, (int*)bufptr);
                        break;
                    case PIO_FLOAT:
                        ierr = nc_get_vara_float(file->fh, vid, start, count, (float*)bufptr);
                        break;
                    case PIO_DOUBLE:
                        ierr = nc_get_vara_double(file->fh, vid, start, count, (double*)bufptr);
                        break;
#ifdef _NETCDF4
                    case PIO_UBYTE:
                        ierr = nc_get_vara_uchar(file->fh, vid, start, count, (unsigned char*)bufptr);
                        break;
                    case PIO_USHORT:
                        ierr = nc_get_vara_ushort(file->fh, vid, start, count, (unsigned short*)bufptr);
                        break;
                    case PIO_UINT:
                        ierr = nc_get_vara_uint(file->fh, vid, start, count, (unsigned int*)bufptr);
                        break;
                    case PIO_INT64:
                        ierr = nc_get_vara_longlong(file->fh, vid, start, count, (long long*)bufptr);
                        break;
                    case PIO_UINT64:
                        ierr = nc_get_vara_ulonglong(file->fh, vid, start, count, (unsigned long long*)bufptr);
                        break;
                    case PIO_STRING:
                        ierr = nc_get_vara_string(file->fh, vid, start, count, (char**)bufptr);
                        break;
#endif /* _NETCDF4 */
                    default:
                        return pio_err(ios, file, PIO_EBADTYPE, __FILE__, __LINE__);
                    }

                    /* Check error code of netCDF call. */
                    if (ierr)
                        return check_netcdf(file, ierr, __FILE__, __LINE__);
                }

                /* The decomposition may not use all of the active io
                 * tasks. rtask here is the io task rank and
                 * ios->num_iotasks is the number of iotasks actually
                 * used in this decomposition. */
                if (rtask < ios->num_iotasks && tmp_bufsize > 0)
                    if ((mpierr = MPI_Send(iobuf, tmp_bufsize, iodesc->mpitype, rtask,
                                           4 * ios->num_iotasks + rtask, ios->io_comm)))
                        return check_mpi(file, mpierr, __FILE__, __LINE__);
            }
        }
    }

#ifdef TIMING
    /* Stop timing this function. */
    GPTLstop("PIO:read_darray_nc_serial");
#endif

    return PIO_NOERR;
}

/**
 * Flush the output buffer. This is only relevant for files opened
 * with pnetcdf.
 *
 * @param file a pointer to the open file descriptor for the file
 * that will be written to
 * @param force true to force the flushing of the buffer
 * @param addsize additional size to add to buffer (in bytes)
 * @return 0 for success, error code otherwise.
 * @ingroup PIO_write_darray
 * @author Jim Edwards, Ed Hartnett
 */
int flush_output_buffer(file_desc_t *file, bool force, PIO_Offset addsize)
{
    int mpierr;  /* Return code from MPI functions. */
    int ierr = PIO_NOERR;

#ifdef _PNETCDF
    var_desc_t *vdesc;
    PIO_Offset usage = 0;

    /* Check inputs. */
    pioassert(file, "invalid input", __FILE__, __LINE__);

    /* Find out the buffer usage. */
    if ((ierr = ncmpi_inq_buffer_usage(file->fh, &usage)))
	/* allow the buffer to be undefined */
	if (ierr != NC_ENULLABUF)
	    return pio_err(NULL, file, PIO_EBADID, __FILE__, __LINE__);

    /* If we are not forcing a flush, spread the usage to all IO
     * tasks. */
    if (!force && file->iosystem->io_comm != MPI_COMM_NULL)
    {
        usage += addsize;
        if ((mpierr = MPI_Allreduce(MPI_IN_PLACE, &usage, 1,  MPI_OFFSET,  MPI_MAX,
                                    file->iosystem->io_comm)))
            return check_mpi(file, mpierr, __FILE__, __LINE__);
    }

    /* Keep track of the maximum usage. */
    if (usage > maxusage)
        maxusage = usage;

    /* If the user forces it, or the buffer has exceeded the size
     * limit, then flush to disk. */
    if (force || usage >= pio_buffer_size_limit)
    {
        int rcnt;
        int  maxreq;
        int reqcnt;
        maxreq = 0;
        reqcnt = 0;
        rcnt = 0;

        for (int i = 0; i < file->nvars; i++)
        {
            if ((ierr = get_var_desc(i, &file->varlist, &vdesc)))
                return pio_err(NULL, file, ierr, __FILE__, __LINE__);
            reqcnt += vdesc->nreqs;
            if (vdesc->nreqs > 0)
                maxreq = i;
        }
        int request[reqcnt];
        int status[reqcnt];

        for (int i = 0; i <= maxreq; i++)
        {
            if ((ierr = get_var_desc(i, &file->varlist, &vdesc)))
                return pio_err(NULL, file, ierr, __FILE__, __LINE__);
#ifdef MPIO_ONESIDED
            /*onesided optimization requires that all of the requests in a wait_all call represent
              a contiguous block of data in the file */
            if (rcnt > 0 && (prev_record != vdesc->record || vdesc->nreqs == 0))
            {
                ierr = ncmpi_wait_all(file->fh, rcnt, request, status);
                rcnt = 0;
            }
            prev_record = vdesc->record;
#endif
            for (reqcnt = 0; reqcnt < vdesc->nreqs; reqcnt++)
                request[rcnt++] = max(vdesc->request[reqcnt], NC_REQ_NULL);

            if (vdesc->request != NULL)
                free(vdesc->request);
            vdesc->request = NULL;
            vdesc->nreqs = 0;

#ifdef FLUSH_EVERY_VAR
            ierr = ncmpi_wait_all(file->fh, rcnt, request, status);
            rcnt = 0;
#endif
        }

        if (rcnt > 0)
            ierr = ncmpi_wait_all(file->fh, rcnt, request, status);

        /* Release resources. */
        if (file->iobuf)
        {
            LOG((3,"freeing variable buffer in flush_output_buffer"));
            brel(file->iobuf);
            file->iobuf = NULL;
        }

        for (int v = 0; v < file->nvars; v++)
        {
            if ((ierr = get_var_desc(v, &file->varlist, &vdesc)))
                return pio_err(NULL, file, ierr, __FILE__, __LINE__);
            if (vdesc->fillbuf)
            {
                brel(vdesc->fillbuf);
                vdesc->fillbuf = NULL;
            }
        }
    }

#endif /* _PNETCDF */
    return ierr;
}

/**
 * Print out info about the buffer for debug purposes. This should
 * only be called when logging is enabled.
 *
 * @param ios pointer to the IO system structure
 * @param collective true if collective report is desired
 * @ingroup PIO_write_darray
 * @author Jim Edwards
 */
void cn_buffer_report(iosystem_desc_t *ios, bool collective)
{
    int mpierr;  /* Return code from MPI functions. */

    LOG((2, "cn_buffer_report ios->iossysid = %d collective = %d CN_bpool = %d",
         ios->iosysid, collective, CN_bpool));
    if (CN_bpool)
    {
        long bget_stats[5];
        long bget_mins[5];
        long bget_maxs[5];

        bstats(bget_stats, bget_stats+1,bget_stats+2,bget_stats+3,bget_stats+4);
        if (collective)
        {
            LOG((3, "cn_buffer_report calling MPI_Reduce ios->comp_comm = %d", ios->comp_comm));
            if ((mpierr = MPI_Reduce(bget_stats, bget_maxs, 5, MPI_LONG, MPI_MAX, 0, ios->comp_comm)))
                check_mpi(NULL, mpierr, __FILE__, __LINE__);
            LOG((3, "cn_buffer_report calling MPI_Reduce"));
            if ((mpierr = MPI_Reduce(bget_stats, bget_mins, 5, MPI_LONG, MPI_MIN, 0, ios->comp_comm)))
                check_mpi(NULL, mpierr, __FILE__, __LINE__);
            if (ios->compmaster == MPI_ROOT)
            {
                LOG((1, "Currently allocated buffer space %ld %ld", bget_mins[0], bget_maxs[0]));
                LOG((1, "Currently available buffer space %ld %ld", bget_mins[1], bget_maxs[1]));
                LOG((1, "Current largest free block %ld %ld", bget_mins[2], bget_maxs[2]));
                LOG((1, "Number of successful bget calls %ld %ld", bget_mins[3], bget_maxs[3]));
                LOG((1, "Number of successful brel calls  %ld %ld", bget_mins[4], bget_maxs[4]));
            }
        }
        else
        {
            LOG((1, "Currently allocated buffer space %ld", bget_stats[0]));
            LOG((1, "Currently available buffer space %ld", bget_stats[1]));
            LOG((1, "Current largest free block %ld", bget_stats[2]));
            LOG((1, "Number of successful bget calls %ld", bget_stats[3]));
            LOG((1, "Number of successful brel calls  %ld", bget_stats[4]));
        }
    }
}

/**
 * Free the buffer pool. If malloc is used (that is, PIO_USE_MALLOC is
 * non zero), this function does nothing.
 *
 * @param ios pointer to the IO system structure.
 * @ingroup PIO_write_darray
 * @author Jim Edwards
 */
void free_cn_buffer_pool(iosystem_desc_t *ios)
{
#if !PIO_USE_MALLOC
    LOG((2, "free_cn_buffer_pool CN_bpool = %d", CN_bpool));
    /* Note: it is possible that CN_bpool has been freed and set to NULL by bpool_free() */
    if (CN_bpool)
    {
        cn_buffer_report(ios, false);
        bpoolrelease(CN_bpool);
        LOG((2, "free_cn_buffer_pool done!"));
        free(CN_bpool);
        CN_bpool = NULL;
    }
#endif /* !PIO_USE_MALLOC */
}

/**
 * Flush the buffer.
 *
 * @param ncid identifies the netCDF file.
 * @param wmb pointer to the wmulti_buffer structure.
 * @param flushtodisk if true, then flush data to disk.
 * @returns 0 for success, error code otherwise.
 * @ingroup PIO_write_darray
 * @author Jim Edwards, Ed Hartnett
 */
int flush_buffer(int ncid, wmulti_buffer *wmb, bool flushtodisk)
{
    file_desc_t *file;
    int ret;

    /* Check input. */
    pioassert(wmb, "invalid input", __FILE__, __LINE__);

    /* Get the file info (to get error handler). */
    if ((ret = pio_get_file(ncid, &file)))
        return pio_err(NULL, NULL, ret, __FILE__, __LINE__);

    LOG((1, "flush_buffer ncid = %d flushtodisk = %d", ncid, flushtodisk));

    /* If there are any variables in this buffer... */
    if (wmb->num_arrays > 0)
    {
        /* Write any data in the buffer. */
        ret = PIOc_write_darray_multi(ncid, wmb->vid,  wmb->ioid, wmb->num_arrays,
                                      wmb->arraylen, wmb->data, wmb->frame,
                                      wmb->fillvalue, flushtodisk);
        LOG((2, "return from PIOc_write_darray_multi ret = %d", ret));

        wmb->num_arrays = 0;

        /* Release the list of variable IDs. */
        brel(wmb->vid);
        wmb->vid = NULL;

        /* Release the data memory. */
        brel(wmb->data);
        wmb->data = NULL;

        /* If there is a fill value, release it. */
        if (wmb->fillvalue)
            brel(wmb->fillvalue);
        wmb->fillvalue = NULL;

        /* Release the record number. */
        if (wmb->frame)
            brel(wmb->frame);
        wmb->frame = NULL;

        if (ret)
            return pio_err(NULL, file, ret, __FILE__, __LINE__);
    }

    return PIO_NOERR;
}

