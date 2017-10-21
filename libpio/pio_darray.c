/** @file
 *
 * Public functions that read and write distributed arrays in PIO.
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
PIO_Offset pio_buffer_size_limit = PIO_BUFFER_SIZE;

/* Global buffer pool pointer. */
void *CN_bpool = NULL;

/* Maximum buffer usage. */
PIO_Offset maxusage = 0;

/* For write_darray_multi_serial() and write_darray_multi_par() to
 * indicate whether fill or data are being written. */
#define DARRAY_FILL 1
#define DARRAY_DATA 0

/**
 * Set the PIO IO node data buffer size limit.
 *
 * The pio_buffer_size_limit will only apply to files opened after
 * the setting is changed.
 *
 * @param limit the size of the buffer on the IO nodes
 * @return The previous limit setting.
 * @author Jim Edwards
 */
PIO_Offset PIOc_set_buffer_size_limit(PIO_Offset limit)
{
    PIO_Offset oldsize = pio_buffer_size_limit;

    /* If the user passed a valid size, use it. */
    if (limit > 0)
        pio_buffer_size_limit = limit;

    return oldsize;
}

/**
 * Write one or more arrays with the same IO decomposition to the
 * file.
 *
 * This funciton is similar to PIOc_write_darray(), but allows the
 * caller to use their own data buffering (instead of using the
 * buffering implemented in PIOc_write_darray()).
 *
 * When the user calls PIOc_write_darray() one or more times, then
 * PIO_write_darray_multi() will be called when the buffer is flushed.
 *
 * Internally, this function will:
 * <ul>
 * <li>Find info about file, decomposition, and variable.
 * <li>Do a special flush for pnetcdf if needed.
 * <li>Allocates a buffer big enough to hold all the data in the
 * multi-buffer, for all tasks.
 * <li>Calls rearrange_comp2io() to move data from compute to IO
 * tasks.
 * <li>For parallel iotypes (pnetcdf and netCDF-4 parallel) call
 * pio_write_darray_multi_nc().
 * <li>For serial iotypes (netcdf classic and netCDF-4 serial) call
 * write_darray_multi_serial().
 * <li>For subset rearranger, create holegrid to write missing
 * data. Then call pio_write_darray_multi_nc() or
 * write_darray_multi_serial() to write the holegrid.
 * <li>Special buffer flush for pnetcdf.
 * </ul>
 *
 * @param ncid identifies the netCDF file.
 * @param varids an array of length nvars containing the variable ids to
 * be written.
 * @param ioid the I/O description ID as passed back by
 * PIOc_InitDecomp().
 * @param nvars the number of variables to be written with this
 * call.
 * @param arraylen the length of the array to be written. This is the
 * length of the distrubited array. That is, the length of the portion
 * of the data that is on the processor. The same arraylen is used for
 * all variables in the call.
 * @param array pointer to the data to be written. This is a pointer
 * to an array of arrays with the distributed portion of the array
 * that is on this processor. There are nvars arrays of data, and each
 * array of data contains one record worth of data for that variable.
 * @param frame an array of length nvars with the frame or record
 * dimension for each of the nvars variables in IOBUF. NULL if this
 * iodesc contains non-record vars.
 * @param fillvalue pointer an array (of length nvars) of pointers to
 * the fill value to be used for missing data.
 * @param flushtodisk non-zero to cause buffers to be flushed to disk.
 * @return 0 for success, error code otherwise.
 * @ingroup PIO_write_darray
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_write_darray_multi(int ncid, const int *varids, int ioid, int nvars,
                            PIO_Offset arraylen, void *array, const int *frame,
                            void **fillvalue, bool flushtodisk)
{
    iosystem_desc_t *ios;  /* Pointer to io system information. */
    file_desc_t *file;     /* Pointer to file information. */
    io_desc_t *iodesc;     /* Pointer to IO description information. */
    int rlen;              /* Total data buffer size. */
    var_desc_t *vdesc0;    /* First entry in array of var_desc structure for each var. */
    int fndims;            /* Number of dims in the var in the file. */
    int mpierr = MPI_SUCCESS, mpierr2;  /* Return code from MPI function calls. */
    int ierr;              /* Return code. */

    /* Get the file info. */
    if ((ierr = pio_get_file(ncid, &file)))
        return pio_err(NULL, NULL, PIO_EBADID, __FILE__, __LINE__);
    ios = file->iosystem;

    /* Check inputs. */
    if (nvars <= 0 || !varids)
        return pio_err(ios, file, PIO_EINVAL, __FILE__, __LINE__);

    LOG((1, "PIOc_write_darray_multi ncid = %d ioid = %d nvars = %d arraylen = %ld "
         "flushtodisk = %d",
         ncid, ioid, nvars, arraylen, flushtodisk));

    /* Check that we can write to this file. */
    if (!file->writable)
        return pio_err(ios, file, PIO_EPERM, __FILE__, __LINE__);

    /* Get iodesc. */
    if (!(iodesc = pio_get_iodesc_from_id(ioid)))
        return pio_err(ios, file, PIO_EBADID, __FILE__, __LINE__);
    pioassert(iodesc->rearranger == PIO_REARR_BOX || iodesc->rearranger == PIO_REARR_SUBSET,
              "unknown rearranger", __FILE__, __LINE__);

    /* Check the types of all the vars. They must match the type of
     * the decomposition. */
    for (int v = 0; v < nvars; v++)
    {
        var_desc_t *vdesc;
        if ((ierr = get_var_desc(varids[v], &file->varlist, &vdesc)))
            return pio_err(ios, file, ierr, __FILE__, __LINE__);
        if (vdesc->pio_type != iodesc->piotype)
            return pio_err(ios, file, PIO_EINVAL, __FILE__, __LINE__);
    }

    /* Get a pointer to the variable info for the first variable. */
    if ((ierr = get_var_desc(varids[0], &file->varlist, &vdesc0)))
        return pio_err(ios, file, ierr, __FILE__, __LINE__);

    /* Run these on all tasks if async is not in use, but only on
     * non-IO tasks if async is in use. */
    if (!ios->async || !ios->ioproc)
    {
        /* Get the number of dims for this var. */
        LOG((3, "about to call PIOc_inq_varndims varids[0] = %d", varids[0]));
        if ((ierr = PIOc_inq_varndims(file->pio_ncid, varids[0], &fndims)))
            return check_netcdf(file, ierr, __FILE__, __LINE__);
        LOG((3, "called PIOc_inq_varndims varids[0] = %d fndims = %d", varids[0], fndims));
    }

    /* If async is in use, and this is not an IO task, bcast the parameters. */
    if (ios->async)
    {
        if (!ios->ioproc)
        {
            int msg = PIO_MSG_WRITEDARRAYMULTI;
            char frame_present = frame ? true : false;         /* Is frame non-NULL? */
            char fillvalue_present = fillvalue ? true : false; /* Is fillvalue non-NULL? */
            int flushtodisk_int = flushtodisk; /* Need this to be int not boolean. */

            if (ios->compmaster == MPI_ROOT)
                mpierr = MPI_Send(&msg, 1, MPI_INT, ios->ioroot, 1, ios->union_comm);

            /* Send the function parameters and associated informaiton
             * to the msg handler. */
            if (!mpierr)
                mpierr = MPI_Bcast(&ncid, 1, MPI_INT, ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast(&nvars, 1, MPI_INT, ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast((void *)varids, nvars, MPI_INT, ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast(&ioid, 1, MPI_INT, ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast(&arraylen, 1, MPI_OFFSET, ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast(array, arraylen * iodesc->piotype_size, MPI_CHAR, ios->compmaster,
                                   ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast(&frame_present, 1, MPI_CHAR, ios->compmaster, ios->intercomm);
            if (!mpierr && frame_present)
                mpierr = MPI_Bcast((void *)frame, nvars, MPI_INT, ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast(&fillvalue_present, 1, MPI_CHAR, ios->compmaster, ios->intercomm);
            if (!mpierr && fillvalue_present)
                mpierr = MPI_Bcast((void *)fillvalue, nvars * iodesc->piotype_size, MPI_CHAR,
                                   ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast(&flushtodisk_int, 1, MPI_INT, ios->compmaster, ios->intercomm);
            LOG((2, "PIOc_write_darray_multi file->pio_ncid = %d nvars = %d ioid = %d arraylen = %d "
                 "frame_present = %d fillvalue_present = %d flushtodisk = %d", file->pio_ncid, nvars,
                 ioid, arraylen, frame_present, fillvalue_present, flushtodisk));
        }

        /* Handle MPI errors. */
        if ((mpierr2 = MPI_Bcast(&mpierr, 1, MPI_INT, ios->comproot, ios->my_comm)))
            return check_mpi(file, mpierr2, __FILE__, __LINE__);
        if (mpierr)
            return check_mpi(file, mpierr, __FILE__, __LINE__);

        /* Share results known only on computation tasks with IO tasks. */
        if ((mpierr = MPI_Bcast(&fndims, 1, MPI_INT, ios->comproot, ios->my_comm)))
            check_mpi(file, mpierr, __FILE__, __LINE__);
        LOG((3, "shared fndims = %d", fndims));
    }

    /* if the buffer is already in use in pnetcdf we need to flush first */
    if (file->iotype == PIO_IOTYPE_PNETCDF && file->iobuf)
        if ((ierr = flush_output_buffer(file, 1, 0)))
            return pio_err(ios, file, ierr, __FILE__, __LINE__);

    pioassert(!file->iobuf, "buffer overwrite",__FILE__, __LINE__);

    /* Determine total size of aggregated data (all vars/records).
     * For netcdf serial writes we collect the data on io nodes and
     * then move that data one node at a time to the io master node
     * and write (or read). The buffer size on io task 0 must be as
     * large as the largest used to accommodate this serial io
     * method.  */
    rlen = 0;
    if (iodesc->llen > 0)
        rlen = iodesc->maxiobuflen * nvars;

    /* Allocate iobuf. */
    if (rlen > 0)
    {
        /* Allocate memory for the buffer for all vars/records. */
        if (!(file->iobuf = bget(iodesc->mpitype_size * (size_t)rlen)))
            return pio_err(ios, file, PIO_ENOMEM, __FILE__, __LINE__);
        LOG((3, "allocated %lld bytes for variable buffer", (size_t)rlen * iodesc->mpitype_size));

        /* If fill values are desired, and we're using the BOX
         * rearranger, insert fill values. */
        if (iodesc->needsfill && iodesc->rearranger == PIO_REARR_BOX)
        {
            LOG((3, "inerting fill values iodesc->maxiobuflen = %d", iodesc->maxiobuflen));
            for (int nv = 0; nv < nvars; nv++)
                for (int i = 0; i < iodesc->maxiobuflen; i++)
                    memcpy(&((char *)file->iobuf)[iodesc->mpitype_size * (i + nv * iodesc->maxiobuflen)],
                           &((char *)fillvalue)[nv * iodesc->mpitype_size], iodesc->mpitype_size);
        }
    }
    else if (file->iotype == PIO_IOTYPE_PNETCDF && ios->ioproc)
    {
	/* this assures that iobuf is allocated on all iotasks thus
           assuring that the flush_output_buffer call above is called
           collectively (from all iotasks) */
        if (!(file->iobuf = bget(1)))
            return pio_err(ios, file, PIO_ENOMEM, __FILE__, __LINE__);
        LOG((3, "allocated token for variable buffer"));
    }

    /* Move data from compute to IO tasks. */
    if ((ierr = rearrange_comp2io(ios, iodesc, array, file->iobuf, nvars)))
        return pio_err(ios, file, ierr, __FILE__, __LINE__);

    /* Write the darray based on the iotype. */
    LOG((2, "about to write darray for iotype = %d", file->iotype));
    switch (file->iotype)
    {
    case PIO_IOTYPE_NETCDF4P:
    case PIO_IOTYPE_PNETCDF:
        if ((ierr = write_darray_multi_par(file, nvars, fndims, varids, iodesc,
                                           DARRAY_DATA, frame)))
            return pio_err(ios, file, ierr, __FILE__, __LINE__);
        break;
    case PIO_IOTYPE_NETCDF4C:
    case PIO_IOTYPE_NETCDF:
        if ((ierr = write_darray_multi_serial(file, nvars, fndims, varids, iodesc,
                                              DARRAY_DATA, frame)))
            return pio_err(ios, file, ierr, __FILE__, __LINE__);

        break;
    default:
        return pio_err(NULL, NULL, PIO_EBADIOTYPE, __FILE__, __LINE__);
    }

    /* For PNETCDF the iobuf is freed in flush_output_buffer() */
    if (file->iotype != PIO_IOTYPE_PNETCDF)
    {
        /* Release resources. */
        if (file->iobuf)
        {
	    LOG((3,"freeing variable buffer in pio_darray"));
            brel(file->iobuf);
            file->iobuf = NULL;
        }
    }

    /* The box rearranger will always have data (it could be fill
     * data) to fill the entire array - that is the aggregate start
     * and count values will completely describe one unlimited
     * dimension unit of the array. For the subset method this is not
     * necessarily the case, areas of missing data may never be
     * written. In order to make sure that these areas are given the
     * missing value a 'holegrid' is used to describe the missing
     * points. This is generally faster than the netcdf method of
     * filling the entire array with missing values before overwriting
     * those values later. */
    if (iodesc->rearranger == PIO_REARR_SUBSET && iodesc->needsfill)
    {
        LOG((2, "nvars = %d holegridsize = %ld iodesc->needsfill = %d\n", nvars,
             iodesc->holegridsize, iodesc->needsfill));

	pioassert(!vdesc0->fillbuf, "buffer overwrite",__FILE__, __LINE__);

        /* Get a buffer. */
	if (ios->io_rank == 0)
	    vdesc0->fillbuf = bget(iodesc->maxholegridsize * iodesc->mpitype_size * nvars);
	else if (iodesc->holegridsize > 0)
	    vdesc0->fillbuf = bget(iodesc->holegridsize * iodesc->mpitype_size * nvars);

        /* copying the fill value into the data buffer for the box
         * rearranger. This will be overwritten with data where
         * provided. */
        for (int nv = 0; nv < nvars; nv++)
            for (int i = 0; i < iodesc->holegridsize; i++)
                memcpy(&((char *)vdesc0->fillbuf)[iodesc->mpitype_size * (i + nv * iodesc->holegridsize)],
                       &((char *)fillvalue)[iodesc->mpitype_size * nv], iodesc->mpitype_size);

        /* Write the darray based on the iotype. */
        switch (file->iotype)
        {
        case PIO_IOTYPE_PNETCDF:
        case PIO_IOTYPE_NETCDF4P:
            if ((ierr = write_darray_multi_par(file, nvars, fndims, varids, iodesc,
                                               DARRAY_FILL, frame)))
                return pio_err(ios, file, ierr, __FILE__, __LINE__);
            break;
        case PIO_IOTYPE_NETCDF4C:
        case PIO_IOTYPE_NETCDF:
            if ((ierr = write_darray_multi_serial(file, nvars, fndims, varids, iodesc,
                                                  DARRAY_FILL, frame)))
                return pio_err(ios, file, ierr, __FILE__, __LINE__);
            break;
        default:
            return pio_err(ios, file, PIO_EBADIOTYPE, __FILE__, __LINE__);
        }

        /* For PNETCDF fillbuf is freed in flush_output_buffer() */
        if (file->iotype != PIO_IOTYPE_PNETCDF)
        {
            /* Free resources. */
            if (vdesc0->fillbuf)
            {
                brel(vdesc0->fillbuf);
                vdesc0->fillbuf = NULL;
            }
        }
    }

    /* Flush data to disk for pnetcdf. */
    if (ios->ioproc && file->iotype == PIO_IOTYPE_PNETCDF)
        if ((ierr = flush_output_buffer(file, flushtodisk, 0)))
            return pio_err(ios, file, ierr, __FILE__, __LINE__);

    return PIO_NOERR;
}

/**
 * Find the fillvalue that should be used for a variable.
 *
 * @param file Info about file we are writing to.
 * @param varid the variable ID.
 * @param vdesc pointer to var_desc_t info for this var.
 * @returns 0 for success, non-zero error code for failure.
 * @ingroup PIO_write_darray
 * @author Ed Hartnett
 */
int find_var_fillvalue(file_desc_t *file, int varid, var_desc_t *vdesc)
{
    iosystem_desc_t *ios;  /* Pointer to io system information. */
    int pio_type;
    PIO_Offset type_size;
    int no_fill;
    int ierr;

    /* Check inputs. */
    pioassert(file && file->iosystem && vdesc, "invalid input", __FILE__, __LINE__);
    ios = file->iosystem;

    LOG((3, "find_var_fillvalue file->pio_ncid = %d varid = %d", file->pio_ncid, varid));

    /* Find out PIO data type of var. */
    if ((ierr = PIOc_inq_vartype(file->pio_ncid, varid, &pio_type)))
        return pio_err(ios, NULL, ierr, __FILE__, __LINE__);
    LOG((3, "pio_type %d", pio_type));

    /* Find out length of type. */
    if ((ierr = PIOc_inq_type(file->pio_ncid, pio_type, NULL, &type_size)))
        return pio_err(ios, NULL, ierr, __FILE__, __LINE__);
    LOG((3, "getting fill value for varid = %d pio_type = %d type_size = %d",
         varid, pio_type, type_size));

    /* Allocate storage for the fill value. */
    if (!(vdesc->fillvalue = malloc(type_size)))
        return pio_err(ios, NULL, PIO_ENOMEM, __FILE__, __LINE__);

    /* Get the fill value. */
    if ((ierr = PIOc_inq_var_fill(file->pio_ncid, varid, &no_fill, vdesc->fillvalue)))
        return pio_err(ios, NULL, ierr, __FILE__, __LINE__);
    vdesc->use_fill = no_fill ? 0 : 1;
    LOG((3, "vdesc->use_fill = %d", vdesc->use_fill));

    return PIO_NOERR;
}

/**
 * Write a distributed array to the output file.
 *
 * This routine aggregates output on the compute nodes and only sends
 * it to the IO nodes when the compute buffer is full or when a flush
 * is triggered.
 *
 * Internally, this function will:
 * <ul>
 * <li>Locate info about this file, decomposition, and variable.
 * <li>If we don't have a fillvalue for this variable, determine one
 * and remember it for future calls.
 * <li>Initialize or find the multi_buffer for this record/var.
 * <li>Find out how much free space is available in the multi buffer
 * and flush if needed.
 * <li>Store the new user data in the mutli buffer.
 * <li>If needed (only for subset rearranger), fill in gaps in data
 * with fillvalue.
 * <li>Remember the frame value (i.e. record number) of this data if
 * there is one.
 * </ul>
 *
 * NOTE: The write multi buffer wmulti_buffer is the cache on compute
 * nodes that will collect and store multiple variables before sending
 * them to the io nodes. Aggregating variables in this way leads to a
 * considerable savings in communication expense. Variables in the wmb
 * array must have the same decomposition and base data size and we
 * also need to keep track of whether each is a recordvar (has an
 * unlimited dimension) or not.
 *
 * @param ncid the ncid of the open netCDF file.
 * @param varid the ID of the variable that these data will be written
 * to.
 * @param ioid the I/O description ID as passed back by
 * PIOc_InitDecomp().
 * @param arraylen the length of the array to be written. This should
 * be at least the length of the local component of the distrubited
 * array. (Any values beyond length of the local component will be
 * ignored.)
 * @param array pointer to an array of length arraylen with the data
 * to be written. This is a pointer to the distributed portion of the
 * array that is on this task.
 * @param fillvalue pointer to the fill value to be used for missing
 * data.
 * @returns 0 for success, non-zero error code for failure.
 * @ingroup PIO_write_darray
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_write_darray(int ncid, int varid, int ioid, PIO_Offset arraylen, void *array,
                      void *fillvalue)
{
    iosystem_desc_t *ios;  /* Pointer to io system information. */
    file_desc_t *file;     /* Info about file we are writing to. */
    io_desc_t *iodesc;     /* The IO description. */
    var_desc_t *vdesc;     /* Info about the var being written. */
    void *bufptr;          /* A data buffer. */
    wmulti_buffer *wmb;    /* The write multi buffer for one or more vars. */
    int needsflush = 0;    /* True if we need to flush buffer. */
#if PIO_USE_MALLOC
    void *realloc_data = NULL;
#else
    bufsize totfree;       /* Amount of free space in the buffer. */
    bufsize maxfree;       /* Max amount of free space in buffer. */
#endif
    int mpierr = MPI_SUCCESS;  /* Return code from MPI functions. */
    int ierr = PIO_NOERR;      /* Return code. */

   LOG((1, "PIOc_write_darray ncid = %d varid = %d ioid = %d arraylen = %d", 
         ncid, varid, ioid, arraylen));

    /* Get the file info. */
    if ((ierr = pio_get_file(ncid, &file)))
        return pio_err(NULL, NULL, PIO_EBADID, __FILE__, __LINE__);
    ios = file->iosystem;

    /* Can we write to this file? */
    if (!file->writable)
        return pio_err(ios, file, PIO_EPERM, __FILE__, __LINE__);

    /* Get decomposition information. */
    if (!(iodesc = pio_get_iodesc_from_id(ioid)))
        return pio_err(ios, file, PIO_EBADID, __FILE__, __LINE__);

    /* Check that the local size of the variable passed in matches the
     * size expected by the io descriptor. Fail if arraylen is too
     * small, just put a warning in the log if it is too big (the
     * excess values will be ignored.) */
    if (arraylen < iodesc->ndof)
        return pio_err(ios, file, PIO_EINVAL, __FILE__, __LINE__);
    LOG((2, "%s arraylen = %d iodesc->ndof = %d",
         (iodesc->ndof != arraylen) ? "WARNING: iodesc->ndof != arraylen" : "",
         arraylen, iodesc->ndof));

    /* Get var description. */
    if ((ierr = get_var_desc(varid, &file->varlist, &vdesc)))
        return pio_err(ios, file, ierr, __FILE__, __LINE__);

    /* If the type of the var doesn't match the type of the
     * decomposition, return an error. */
    if (iodesc->piotype != vdesc->pio_type)
        return pio_err(ios, file, PIO_EINVAL, __FILE__, __LINE__);
    pioassert(iodesc->mpitype_size == vdesc->mpi_type_size, "wrong mpi info",
              __FILE__, __LINE__);

    /* If we don't know the fill value for this var, get it. */
    if (!vdesc->fillvalue)
        if ((ierr = find_var_fillvalue(file, varid, vdesc)))
            return pio_err(ios, file, ierr, __FILE__, __LINE__);
    LOG((3, "found fill value"));

    /* Check that if the user passed a fill value, it is correct. */
    if (fillvalue)
        if (memcmp(fillvalue, vdesc->fillvalue, vdesc->pio_type_size))
            return pio_err(ios, file, PIO_EINVAL, __FILE__, __LINE__);
    
    /* Move to end of list or the entry that matches this ioid. */
    for (wmb = &file->buffer; wmb->next; wmb = wmb->next)
        if (wmb->ioid == ioid && wmb->recordvar == vdesc->rec_var)
            break;
    LOG((3, "wmb->ioid = %d wmb->recordvar = %d", wmb->ioid, wmb->recordvar));

    /* If we did not find an existing wmb entry, create a new wmb. */
    if (wmb->ioid != ioid || wmb->recordvar != vdesc->rec_var)
    {
	/* Allocate a buffer. */
	LOG((3, "about to allocate multibuffer, size %d", sizeof(wmulti_buffer)));
        if (!(wmb->next = bget((bufsize)sizeof(wmulti_buffer))))
            return pio_err(ios, file, PIO_ENOMEM, __FILE__, __LINE__);
        /* if (!(wmb->next = malloc(sizeof(wmulti_buffer)))) */
        /*     return pio_err(ios, file, PIO_ENOMEM, __FILE__, __LINE__); */
	LOG((3, "allocated multibuffer, size %d", sizeof(wmulti_buffer)));

        /* Set pointer to newly allocated buffer and initialize.*/
        wmb = wmb->next;
        wmb->recordvar = vdesc->rec_var;
        wmb->next = NULL;
        wmb->ioid = ioid;
        wmb->num_arrays = 0;
        wmb->arraylen = arraylen;
        wmb->vid = NULL;
        wmb->data = NULL;
        wmb->frame = NULL;
        wmb->fillvalue = NULL;
    }
    LOG((2, "wmb->num_arrays = %d arraylen = %d vdesc->mpi_type_size = %d\n",
         wmb->num_arrays, arraylen, vdesc->mpi_type_size));

#if PIO_USE_MALLOC
    /* Try realloc first and call flush if realloc fails. */
    if (arraylen > 0)
    {
        size_t data_size = (1 + wmb->num_arrays) * arraylen * vdesc->mpi_type_size;
        
        if ((realloc_data = realloc(wmb->data, data_size)))
        {
            needsflush = 0;
            wmb->data = realloc_data;
        }
        else /* Failed to realloc, but wmb->data is still valid for a flush. */
        {
            needsflush = 1;
        }
        LOG((2, "realloc attempted to get %ld bytes for data, needsflush %d", data_size,
             needsflush));
    }
#else
    /* Find out how much free, contiguous space is available. */
    bfreespace(&totfree, &maxfree);

    /* maxfree is the available memory. If that is < 10% greater than
     * the size of the current request needsflush is true. */
    if (needsflush == 0)
        needsflush = (maxfree <= 1.1 * (1 + wmb->num_arrays) * arraylen * vdesc->mpi_type_size);
#endif

    /* Tell all tasks on the computation communicator whether we need
     * to flush data. */
    if ((mpierr = MPI_Allreduce(MPI_IN_PLACE, &needsflush, 1,  MPI_INT,  MPI_MAX,
                                ios->comp_comm)))
        return check_mpi(file, mpierr, __FILE__, __LINE__);
    LOG((2, "needsflush = %d", needsflush));

    /* Flush data if needed. */
    if (needsflush > 0)
    {
#if !PIO_USE_MALLOC
#ifdef PIO_ENABLE_LOGGING
        /* Collect a debug report about buffer. */
        cn_buffer_report(ios, true);
        LOG((2, "maxfree = %ld wmb->num_arrays = %d (1 + wmb->num_arrays) *"
             " arraylen * vdesc->mpi_type_size = %ld totfree = %ld\n", maxfree, wmb->num_arrays,
             (1 + wmb->num_arrays) * arraylen * vdesc->mpi_type_size, totfree));
#endif /* PIO_ENABLE_LOGGING */
#endif /* !PIO_USE_MALLOC */

        /* If needsflush == 2 flush to disk otherwise just flush to io
         * node. This will cause PIOc_write_darray_multi() to be
         * called. */
        if ((ierr = flush_buffer(ncid, wmb, needsflush == 2)))
            return pio_err(ios, file, ierr, __FILE__, __LINE__);
    }

#if PIO_USE_MALLOC
    /* Try realloc again if there is a flush. */
    if (arraylen > 0 && needsflush > 0)
    {
        if (!(wmb->data = realloc(wmb->data, (1 + wmb->num_arrays) * arraylen * vdesc->mpi_type_size)))
            return pio_err(ios, file, PIO_ENOMEM, __FILE__, __LINE__);
        LOG((2, "after a flush, realloc got %ld bytes for data", (1 + wmb->num_arrays) * arraylen * vdesc->mpi_type_size));
    }
#else
    /* Get memory for data. */
    if (arraylen > 0)
    {
        if (!(wmb->data = bgetr(wmb->data, (1 + wmb->num_arrays) * arraylen * vdesc->mpi_type_size)))
            return pio_err(ios, file, PIO_ENOMEM, __FILE__, __LINE__);
        LOG((2, "got %ld bytes for data", (1 + wmb->num_arrays) * arraylen * vdesc->mpi_type_size));
    }
#endif

    /* vid is an array of variable ids in the wmb list, grow the list
     * and add the new entry. */
    if (!(wmb->vid = bgetr(wmb->vid, sizeof(int) * (1 + wmb->num_arrays))))
        return pio_err(ios, file, PIO_ENOMEM, __FILE__, __LINE__);

    /* wmb->frame is the record number, we assume that the variables
     * in the wmb list may not all have the same unlimited dimension
     * value although they usually do. */
    if (vdesc->record >= 0)
        if (!(wmb->frame = bgetr(wmb->frame, sizeof(int) * (1 + wmb->num_arrays))))
            return pio_err(ios, file, PIO_ENOMEM, __FILE__, __LINE__);

    /* If we need a fill value, get it. If we are using the subset
     * rearranger and not using the netcdf fill mode then we need to
     * do an extra write to fill in the holes with the fill value. */
    if (iodesc->needsfill)
    {
        /* Get memory to hold fill value. */
        if (!(wmb->fillvalue = bgetr(wmb->fillvalue, vdesc->mpi_type_size * (1 + wmb->num_arrays))))
            return pio_err(ios, file, PIO_ENOMEM, __FILE__, __LINE__);

        memcpy((char *)wmb->fillvalue + vdesc->mpi_type_size * wmb->num_arrays,
               vdesc->fillvalue, vdesc->mpi_type_size);
    }

    /* Tell the buffer about the data it is getting. */
    wmb->arraylen = arraylen;
    wmb->vid[wmb->num_arrays] = varid;
    LOG((3, "wmb->num_arrays = %d wmb->vid[wmb->num_arrays] = %d", wmb->num_arrays,
         wmb->vid[wmb->num_arrays]));

    /* Copy the user-provided data to the buffer. */
    bufptr = (void *)((char *)wmb->data + arraylen * vdesc->mpi_type_size * wmb->num_arrays);
    if (arraylen > 0)
    {
        memcpy(bufptr, array, arraylen * vdesc->mpi_type_size);
        LOG((3, "copied %ld bytes of user data", arraylen * vdesc->mpi_type_size));
    }

    /* Add the unlimited dimension value of this variable to the frame
     * array in wmb. */
    if (wmb->frame)
        wmb->frame[wmb->num_arrays] = vdesc->record;
    wmb->num_arrays++;

    LOG((2, "wmb->num_arrays = %d iodesc->maxbytes / vdesc->mpi_type_size = %d "
         "iodesc->ndof = %d iodesc->llen = %d", wmb->num_arrays,
         iodesc->maxbytes / vdesc->mpi_type_size, iodesc->ndof, iodesc->llen));

    return PIO_NOERR;
}

/**
 * Read a field from a file to the IO library.
 *
 * @param ncid identifies the netCDF file
 * @param varid the variable ID to be read
 * @param ioid: the I/O description ID as passed back by
 * PIOc_InitDecomp().
 * @param arraylen: the length of the array to be read. This
 * is the length of the distrubited array. That is, the length of
 * the portion of the data that is on the processor.
 * @param array: pointer to the data to be read. This is a
 * pointer to the distributed portion of the array that is on this
 * processor.
 * @return 0 for success, error code otherwise.
 * @ingroup PIO_read_darray
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_read_darray(int ncid, int varid, int ioid, PIO_Offset arraylen,
                     void *array)
{
    iosystem_desc_t *ios;  /* Pointer to io system information. */
    file_desc_t *file;     /* Pointer to file information. */
    io_desc_t *iodesc;     /* Pointer to IO description information. */
    void *iobuf = NULL;    /* holds the data as read on the io node. */
    size_t rlen = 0;       /* the length of data in iobuf. */
    int ierr;           /* Return code. */

    /* Get the file info. */
    if ((ierr = pio_get_file(ncid, &file)))
        return pio_err(NULL, NULL, PIO_EBADID, __FILE__, __LINE__);
    ios = file->iosystem;

    /* Get the iodesc. */
    if (!(iodesc = pio_get_iodesc_from_id(ioid)))
        return pio_err(ios, file, PIO_EBADID, __FILE__, __LINE__);
    pioassert(iodesc->rearranger == PIO_REARR_BOX || iodesc->rearranger == PIO_REARR_SUBSET,
              "unknown rearranger", __FILE__, __LINE__);

    /* ??? */
    if (ios->iomaster == MPI_ROOT)
        rlen = iodesc->maxiobuflen;
    else
        rlen = iodesc->llen;

    /* Allocate a buffer for one record. */
    if (ios->ioproc && rlen > 0)
        if (!(iobuf = bget(iodesc->mpitype_size * rlen)))
            return pio_err(ios, file, PIO_ENOMEM, __FILE__, __LINE__);

    /* Call the correct darray read function based on iotype. */
    switch (file->iotype)
    {
    case PIO_IOTYPE_NETCDF:
    case PIO_IOTYPE_NETCDF4C:
        if ((ierr = pio_read_darray_nc_serial(file, iodesc, varid, iobuf)))
            return pio_err(ios, file, ierr, __FILE__, __LINE__);
        break;
    case PIO_IOTYPE_PNETCDF:
    case PIO_IOTYPE_NETCDF4P:
        if ((ierr = pio_read_darray_nc(file, iodesc, varid, iobuf)))
            return pio_err(ios, file, ierr, __FILE__, __LINE__);
        break;
    default:
        return pio_err(NULL, NULL, PIO_EBADIOTYPE, __FILE__, __LINE__);
    }

    /* Rearrange the data. */
    if ((ierr = rearrange_io2comp(ios, iodesc, iobuf, array)))
        return pio_err(ios, file, ierr, __FILE__, __LINE__);

    /* Free the buffer. */
    if (rlen > 0)
        brel(iobuf);

    return PIO_NOERR;
}
