/**
 * @file
 * Some initialization and support functions.
 * @author Jim Edwards
 * @date  2014
 *
 * @see http://code.google.com/p/parallelio/
 */

#include <config.h>
#include <pio.h>
#include <pio_internal.h>

/** The default error handler used when iosystem cannot be located. */
int default_error_handler = PIO_INTERNAL_ERROR;

/** The target blocksize for each io task when the box rearranger is
 * used (see pio_sc.c). */
extern int blocksize;

/* Used when assiging decomposition IDs. */
int pio_next_ioid = 512;

/**
 * Check to see if PIO has been initialized.
 *
 * @param iosysid the IO system ID
 * @param active pointer that gets true if IO system is active, false
 * otherwise.
 * @returns 0 on success, error code otherwise
 * @author Jim Edwards
 */
int PIOc_iosystem_is_active(int iosysid, bool *active)
{
    iosystem_desc_t *ios;

    /* Get the ios if there is one. */
    ios = pio_get_iosystem_from_id(iosysid);

    if (active)
    {
        if (!ios || (ios->comp_comm == MPI_COMM_NULL && ios->io_comm == MPI_COMM_NULL))
            *active = false;
        else
            *active = true;
    }

    return PIO_NOERR;
}

/**
 * Check to see if PIO file is open.
 *
 * @param ncid the ncid of an open file
 * @returns 1 if file is open, 0 otherwise.
 * @author Jim Edwards
 */
int PIOc_File_is_Open(int ncid)
{
    file_desc_t *file;

    /* If get file returns non-zero, then this file is not open. */
    if (pio_get_file(ncid, &file))
        return 0;
    else
        return 1;
}

/**
 * Set the error handling method to be used for subsequent pio library
 * calls, returns the previous method setting. Note that this changes
 * error handling for the IO system that was used when this file was
 * opened. Other files opened with the same IO system will also he
 * affected by this call. This function is supported but
 * deprecated. New code should use PIOc_set_iosystem_error_handling().
 * This method has no way to return an error, so any failure will
 * result in MPI_Abort.
 *
 * @param ncid the ncid of an open file
 * @param method the error handling method
 * @returns old error handler
 * @ingroup PIO_error_method
 * @author Jim Edwards
 */
int PIOc_Set_File_Error_Handling(int ncid, int method)
{
    file_desc_t *file;
    int oldmethod;

    /* Get the file info. */
    if (pio_get_file(ncid, &file))
        piodie("Could not find file", __FILE__, __LINE__);

    /* Check that valid error handler was provided. */
    if (method != PIO_INTERNAL_ERROR && method != PIO_BCAST_ERROR &&
        method != PIO_RETURN_ERROR)
        piodie("Invalid error hanlder method", __FILE__, __LINE__);

    /* Get the old method. */
    oldmethod = file->iosystem->error_handler;

    /* Set the error hanlder. */
    file->iosystem->error_handler = method;

    return oldmethod;
}

/**
 * Increment the unlimited dimension of the given variable.
 *
 * @param ncid the ncid of the open file
 * @param varid the variable ID
 * @returns 0 on success, error code otherwise
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_advanceframe(int ncid, int varid)
{
    iosystem_desc_t *ios;     /* Pointer to io system information. */
    file_desc_t *file;        /* Pointer to file information. */
    var_desc_t *vdesc;        /* Info about the var. */
    int mpierr = MPI_SUCCESS, mpierr2;  /* Return code from MPI function codes. */
    int ret;

    LOG((1, "PIOc_advanceframe ncid = %d varid = %d"));

    /* Get the file info. */
    if ((ret = pio_get_file(ncid, &file)))
        return pio_err(NULL, NULL, ret, __FILE__, __LINE__);
    ios = file->iosystem;

    /* Get info about variable. */
    if ((ret = get_var_desc(varid, &file->varlist, &vdesc)))
        return pio_err(ios, file, ret, __FILE__, __LINE__);

    /* If using async, and not an IO task, then send parameters. */
    if (ios->async)
    {
        if (!ios->ioproc)
        {
            int msg = PIO_MSG_ADVANCEFRAME;

            if (ios->compmaster == MPI_ROOT)
                mpierr = MPI_Send(&msg, 1, MPI_INT, ios->ioroot, 1, ios->union_comm);

            if (!mpierr)
                mpierr = MPI_Bcast(&ncid, 1, MPI_INT, ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast(&varid, 1, MPI_INT, ios->compmaster, ios->intercomm);
        }

        /* Handle MPI errors. */
        if ((mpierr2 = MPI_Bcast(&mpierr, 1, MPI_INT, ios->comproot, ios->my_comm)))
            check_mpi2(ios, NULL, mpierr2, __FILE__, __LINE__);
        if (mpierr)
            return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    }

    /* Increment the record number. */
    /* file->varlist[varid].record++; */
    vdesc->record++;

    return PIO_NOERR;
}

/**
 * Set the unlimited dimension of the given variable
 *
 * @param ncid the ncid of the file.
 * @param varid the varid of the variable
 * @param frame the value of the unlimited dimension.  In c 0 for the
 * first record, 1 for the second
 * @return PIO_NOERR for no error, or error code.
 * @ingroup PIO_setframe
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_setframe(int ncid, int varid, int frame)
{
    iosystem_desc_t *ios;     /* Pointer to io system information. */
    file_desc_t *file;        /* Pointer to file information. */
    var_desc_t *vdesc;        /* Info about the var. */
    int mpierr = MPI_SUCCESS, mpierr2;  /* Return code from MPI function codes. */
    int ret;

    LOG((1, "PIOc_setframe ncid = %d varid = %d frame = %d", ncid,
         varid, frame));

    /* Get file info. */
    if ((ret = pio_get_file(ncid, &file)))
        return pio_err(NULL, NULL, ret, __FILE__, __LINE__);
    ios = file->iosystem;

    /* Get info about variable. */
    if ((ret = get_var_desc(varid, &file->varlist, &vdesc)))
        return pio_err(ios, file, ret, __FILE__, __LINE__);

    /* If using async, and not an IO task, then send parameters. */
    if (ios->async)
    {
        if (!ios->ioproc)
        {
            int msg = PIO_MSG_SETFRAME;

            if (ios->compmaster == MPI_ROOT)
                mpierr = MPI_Send(&msg, 1, MPI_INT, ios->ioroot, 1, ios->union_comm);

            if (!mpierr)
                mpierr = MPI_Bcast(&ncid, 1, MPI_INT, ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast(&varid, 1, MPI_INT, ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast(&frame, 1, MPI_INT, ios->compmaster, ios->intercomm);
        }

        /* Handle MPI errors. */
        if ((mpierr2 = MPI_Bcast(&mpierr, 1, MPI_INT, ios->comproot, ios->my_comm)))
            check_mpi2(ios, NULL, mpierr2, __FILE__, __LINE__);
        if (mpierr)
            return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    }

    /* Set the record dimension value for this variable. This will be
     * used by the write_darray functions. */
    /* file->varlist[varid].record = frame; */
    vdesc->record = frame;

    return PIO_NOERR;
}

/**
 * Get the number of IO tasks set.
 *
 * @param iosysid the IO system ID
 * @param numiotasks a pointer taht gets the number of IO
 * tasks. Ignored if NULL.
 * @returns 0 on success, error code otherwise
 * @author Ed Hartnett
 */
int PIOc_get_numiotasks(int iosysid, int *numiotasks)
{
    iosystem_desc_t *ios;

    if (!(ios = pio_get_iosystem_from_id(iosysid)))
        return pio_err(NULL, NULL, PIO_EBADID, __FILE__, __LINE__);

    if (numiotasks)
        *numiotasks = ios->num_iotasks;

    return PIO_NOERR;
}

/**
 * Get the local size of the variable.
 *
 * @param ioid IO descrption ID.
 * @returns the size of the array.
 * @author Jim Edwards
 */
int PIOc_get_local_array_size(int ioid)
{
    io_desc_t *iodesc;

    if (!(iodesc = pio_get_iodesc_from_id(ioid)))
        piodie("Could not get iodesc", __FILE__, __LINE__);

    return iodesc->ndof;
}

/**
 * Set the error handling method used for subsequent calls. This
 * function is deprecated. New code should use
 * PIOc_set_iosystem_error_handling(). This method has no way to
 * return an error, so any failure will result in MPI_Abort.
 *
 * @param iosysid the IO system ID
 * @param method the error handling method
 * @returns old error handler
 * @ingroup PIO_error_method
 * @author Jim Edwards
 */
int PIOc_Set_IOSystem_Error_Handling(int iosysid, int method)
{
    iosystem_desc_t *ios;
    int oldmethod;

    /* Get the iosystem info. */
    if (iosysid != PIO_DEFAULT)
        if (!(ios = pio_get_iosystem_from_id(iosysid)))
            piodie("Could not find IO system.", __FILE__, __LINE__);

    /* Set the error handler. */
    if (PIOc_set_iosystem_error_handling(iosysid, method, &oldmethod))
        piodie("Could not set the IOSystem error hanlder", __FILE__, __LINE__);

    return oldmethod;
}

/**
 * Set the error handling method used for subsequent calls for this IO
 * system.
 *
 * @param iosysid the IO system ID. Passing PIO_DEFAULT instead
 * changes the default error handling for the library.
 * @param method the error handling method
 * @param old_method pointer to int that will get old method. Ignored
 * if NULL.
 * @returns 0 for success, error code otherwise.
 * @ingroup PIO_error_method
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_set_iosystem_error_handling(int iosysid, int method, int *old_method)
{
    iosystem_desc_t *ios = NULL;
    int mpierr = MPI_SUCCESS, mpierr2;  /* Return code from MPI function codes. */

    LOG((1, "PIOc_set_iosystem_error_handling iosysid = %d method = %d", iosysid,
         method));

    /* Find info about this iosystem. */
    if (iosysid != PIO_DEFAULT)
        if (!(ios = pio_get_iosystem_from_id(iosysid)))
            return pio_err(NULL, NULL, PIO_EBADID, __FILE__, __LINE__);

    /* Check that valid error handler was provided. */
    if (method != PIO_INTERNAL_ERROR && method != PIO_BCAST_ERROR &&
        method != PIO_RETURN_ERROR)
        return pio_err(ios, NULL, PIO_EINVAL, __FILE__, __LINE__);

    /* If using async, and not an IO task, then send parameters. */
    if (iosysid != PIO_DEFAULT)
        if (ios->async)
        {
            if (!ios->ioproc)
            {
                int msg = PIO_MSG_SETERRORHANDLING;
                char old_method_present = old_method ? true : false;

                if (ios->compmaster == MPI_ROOT)
                    mpierr = MPI_Send(&msg, 1, MPI_INT, ios->ioroot, 1, ios->union_comm);

                if (!mpierr)
                    mpierr = MPI_Bcast(&method, 1, MPI_INT, ios->compmaster, ios->intercomm);
                if (!mpierr)
                    mpierr = MPI_Bcast(&old_method_present, 1, MPI_CHAR, ios->compmaster, ios->intercomm);
            }

            /* Handle MPI errors. */
            if ((mpierr2 = MPI_Bcast(&mpierr, 1, MPI_INT, ios->comproot, ios->my_comm)))
                check_mpi2(ios, NULL, mpierr2, __FILE__, __LINE__);
            if (mpierr)
                return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
        }

    /* Return the current handler. */
    if (old_method)
        *old_method = iosysid == PIO_DEFAULT ? default_error_handler : ios->error_handler;

    /* Set new error handler. */
    if (iosysid == PIO_DEFAULT)
        default_error_handler = method;
    else
        ios->error_handler = method;

    return PIO_NOERR;
}

/**
 * Initialize the decomposition used with distributed arrays. The
 * decomposition describes how the data will be distributed between
 * tasks.
 *
 * Internally, this function will:
 * <ul>
 * <li>Allocate and initialize an iodesc struct for this
 * decomposition. (This also allocates an io_region struct for the
 * first region.)
 * <li>(Box rearranger only) If iostart or iocount are NULL, call
 * CalcStartandCount() to determine starts/counts. Then call
 * compute_maxIObuffersize() to compute the max IO buffer size needed.
 * <li>Create the rearranger.
 * <li>Assign an ioid and add this decomposition to the list of open
 * decompositions.
 * </ul>
 *
 * @param iosysid the IO system ID.
 * @param pio_type the basic PIO data type used.
 * @param ndims the number of dimensions in the variable, not
 * including the unlimited dimension.
 * @param gdimlen an array length ndims with the sizes of the global
 * dimensions.
 * @param maplen the local length of the compmap array.
 * @param compmap a 1 based array of offsets into the array record on
 * file. A 0 in this array indicates a value which should not be
 * transfered.
 * @param ioidp pointer that will get the io description ID. Ignored
 * if NULL.
 * @param rearranger pointer to the rearranger to be used for this
 * decomp or NULL to use the default.
 * @param iostart An array of start values for block cyclic
 * decompositions for the SUBSET rearranger. Ignored if block
 * rearranger is used. If NULL and SUBSET rearranger is used, the
 * iostarts are generated.
 * @param iocount An array of count values for block cyclic
 * decompositions for the SUBSET rearranger. Ignored if block
 * rearranger is used. If NULL and SUBSET rearranger is used, the
 * iostarts are generated.
 * @returns 0 on success, error code otherwise
 * @ingroup PIO_initdecomp
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_InitDecomp(int iosysid, int pio_type, int ndims, const int *gdimlen, int maplen,
                    const PIO_Offset *compmap, int *ioidp, const int *rearranger,
                    const PIO_Offset *iostart, const PIO_Offset *iocount)
{
    iosystem_desc_t *ios;  /* Pointer to io system information. */
    io_desc_t *iodesc;     /* The IO description. */
    int mpierr = MPI_SUCCESS, mpierr2;  /* Return code from MPI function calls. */
    int ierr;              /* Return code. */

    LOG((1, "PIOc_InitDecomp iosysid = %d pio_type = %d ndims = %d maplen = %d",
         iosysid, pio_type, ndims, maplen));

    /* Get IO system info. */
    if (!(ios = pio_get_iosystem_from_id(iosysid)))
        return pio_err(NULL, NULL, PIO_EBADID, __FILE__, __LINE__);

    /* Caller must provide these. */
    if (!gdimlen || !compmap || !ioidp)
        return pio_err(ios, NULL, PIO_EINVAL, __FILE__, __LINE__);

    /* Check the dim lengths. */
    for (int i = 0; i < ndims; i++)
        if (gdimlen[i] <= 0)
            return pio_err(ios, NULL, PIO_EINVAL, __FILE__, __LINE__);

    /* If async is in use, and this is not an IO task, bcast the parameters. */
    if (ios->async)
    {
        if (!ios->ioproc)
        {
            int msg = PIO_MSG_INITDECOMP_DOF; /* Message for async notification. */
            char rearranger_present = rearranger ? true : false;
            char iostart_present = iostart ? true : false;
            char iocount_present = iocount ? true : false;

            if (ios->compmaster == MPI_ROOT)
                mpierr = MPI_Send(&msg, 1, MPI_INT, ios->ioroot, 1, ios->union_comm);

            if (!mpierr)
                mpierr = MPI_Bcast(&iosysid, 1, MPI_INT, ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast(&pio_type, 1, MPI_INT, ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast(&ndims, 1, MPI_INT, ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast((int *)gdimlen, ndims, MPI_INT, ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast(&maplen, 1, MPI_INT, ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast((PIO_Offset *)compmap, maplen, MPI_OFFSET, ios->compmaster, ios->intercomm);

            if (!mpierr)
                mpierr = MPI_Bcast(&rearranger_present, 1, MPI_CHAR, ios->compmaster, ios->intercomm);
            if (rearranger_present && !mpierr)
                mpierr = MPI_Bcast((int *)rearranger, 1, MPI_INT, ios->compmaster, ios->intercomm);

            if (!mpierr)
                mpierr = MPI_Bcast(&iostart_present, 1, MPI_CHAR, ios->compmaster, ios->intercomm);
            if (iostart_present && !mpierr)
                mpierr = MPI_Bcast((PIO_Offset *)iostart, ndims, MPI_OFFSET, ios->compmaster, ios->intercomm);

            if (!mpierr)
                mpierr = MPI_Bcast(&iocount_present, 1, MPI_CHAR, ios->compmaster, ios->intercomm);
            if (iocount_present && !mpierr)
                mpierr = MPI_Bcast((PIO_Offset *)iocount, ndims, MPI_OFFSET, ios->compmaster, ios->intercomm);
            LOG((2, "PIOc_InitDecomp iosysid = %d pio_type = %d ndims = %d maplen = %d rearranger_present = %d iostart_present = %d "
                 "iocount_present = %d ", iosysid, pio_type, ndims, maplen, rearranger_present, iostart_present, iocount_present));
        }

        /* Handle MPI errors. */
        if ((mpierr2 = MPI_Bcast(&mpierr, 1, MPI_INT, ios->comproot, ios->my_comm)))
            return check_mpi2(ios, NULL, mpierr2, __FILE__, __LINE__);
        if (mpierr)
            return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    }

    /* Allocate space for the iodesc info. This also allocates the
     * first region and copies the rearranger opts into this
     * iodesc. */
    LOG((2, "allocating iodesc pio_type %d ndims %d", pio_type, ndims));
    if ((ierr = malloc_iodesc(ios, pio_type, ndims, &iodesc)))
        return pio_err(ios, NULL, ierr, __FILE__, __LINE__);

    /* Remember the maplen. */
    iodesc->maplen = maplen;

    /* Remember the map. */
    if (!(iodesc->map = malloc(sizeof(PIO_Offset) * maplen)))
        return pio_err(ios, NULL, PIO_ENOMEM, __FILE__, __LINE__);
    for (int m = 0; m < maplen; m++)
        iodesc->map[m] = compmap[m];

    /* Remember the dim sizes. */
    if (!(iodesc->dimlen = malloc(sizeof(int) * ndims)))
        return pio_err(ios, NULL, PIO_ENOMEM, __FILE__, __LINE__);
    for (int d = 0; d < ndims; d++)
        iodesc->dimlen[d] = gdimlen[d];

    /* Set the rearranger. */
    if (!rearranger)
        iodesc->rearranger = ios->default_rearranger;
    else
        iodesc->rearranger = *rearranger;
    LOG((2, "iodesc->rearranger = %d", iodesc->rearranger));

    /* Is this the subset rearranger? */
    if (iodesc->rearranger == PIO_REARR_SUBSET)
    {
        iodesc->num_aiotasks = ios->num_iotasks;
        LOG((2, "creating subset rearranger iodesc->num_aiotasks = %d",
             iodesc->num_aiotasks));
        if ((ierr = subset_rearrange_create(ios, maplen, (PIO_Offset *)compmap, gdimlen,
                                            ndims, iodesc)))
            return pio_err(ios, NULL, ierr, __FILE__, __LINE__);
    }
    else /* box rearranger */
    {
        if (ios->ioproc)
        {
            /*  Unless the user specifies the start and count for each
             *  IO task compute it. */
            if (iostart && iocount)
            {
                LOG((3, "iostart and iocount provided"));
                for (int i = 0; i < ndims; i++)
                {
                    iodesc->firstregion->start[i] = iostart[i];
                    iodesc->firstregion->count[i] = iocount[i];
                }
                iodesc->num_aiotasks = ios->num_iotasks;
            }
            else
            {
                /* Compute start and count values for each io task. */
                LOG((2, "about to call CalcStartandCount pio_type = %d ndims = %d", pio_type, ndims));
                if ((ierr = CalcStartandCount(pio_type, ndims, gdimlen, ios->num_iotasks,
                                              ios->io_rank, iodesc->firstregion->start,
                                              iodesc->firstregion->count, &iodesc->num_aiotasks)))
                    return pio_err(ios, NULL, ierr, __FILE__, __LINE__);
            }

            /* Compute the max io buffer size needed for an iodesc. */
            if ((ierr = compute_maxIObuffersize(ios->io_comm, iodesc)))
                return pio_err(ios, NULL, ierr, __FILE__, __LINE__);
            LOG((3, "compute_maxIObuffersize called iodesc->maxiobuflen = %d",
                 iodesc->maxiobuflen));
        }

        /* Depending on array size and io-blocksize the actual number
         * of io tasks used may vary. */
        if ((mpierr = MPI_Bcast(&(iodesc->num_aiotasks), 1, MPI_INT, ios->ioroot,
                                ios->my_comm)))
            return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
        LOG((3, "iodesc->num_aiotasks = %d", iodesc->num_aiotasks));

        /* Compute the communications pattern for this decomposition. */
        if (iodesc->rearranger == PIO_REARR_BOX)
            if ((ierr = box_rearrange_create(ios, maplen, compmap, gdimlen, ndims, iodesc)))
                return pio_err(ios, NULL, ierr, __FILE__, __LINE__);
    }

    /* Broadcast next ioid to all tasks from io root.*/
    if (ios->async)
    {
        LOG((3, "createfile bcasting pio_next_ioid %d", pio_next_ioid));
        if ((mpierr = MPI_Bcast(&pio_next_ioid, 1, MPI_INT, ios->ioroot, ios->my_comm)))
            return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
        LOG((3, "createfile bcast pio_next_ioid %d", pio_next_ioid));
    }

    /* Set the decomposition ID. */
    iodesc->ioid = pio_next_ioid++;
    if (ioidp)
        *ioidp = iodesc->ioid;

    /* Add this IO description to the list. */
    if ((ierr = pio_add_to_iodesc_list(iodesc)))
        return pio_err(ios, NULL, ierr, __FILE__, __LINE__);

#if PIO_ENABLE_LOGGING
    /* Log results. */
    LOG((2, "iodesc ioid = %d nrecvs = %d ndof = %d ndims = %d num_aiotasks = %d "
         "rearranger = %d maxregions = %d needsfill = %d llen = %d maxiobuflen  = %d",
         iodesc->ioid, iodesc->nrecvs, iodesc->ndof, iodesc->ndims, iodesc->num_aiotasks,
         iodesc->rearranger, iodesc->maxregions, iodesc->needsfill, iodesc->llen,
         iodesc->maxiobuflen));
    for (int j = 0; j < iodesc->llen; j++)
        LOG((3, "rindex[%d] = %lld", j, iodesc->rindex[j]));
#endif /* PIO_ENABLE_LOGGING */

    /* This function only does something if pre-processor macro
     * PERFTUNE is set. */
    performance_tune_rearranger(ios, iodesc);

    return PIO_NOERR;
}

/**
 * Initialize the decomposition used with distributed arrays. The
 * decomposition describes how the data will be distributed between
 * tasks.
 *
 * @param iosysid the IO system ID.
 * @param pio_type the basic PIO data type used.
 * @param ndims the number of dimensions in the variable, not
 * including the unlimited dimension.
 * @param gdimlen an array length ndims with the sizes of the global
 * dimensions.
 * @param maplen the local length of the compmap array.
 * @param compmap a 0 based array of offsets into the array record on
 * file. A -1 in this array indicates a value which should not be
 * transfered.
 * @param ioidp pointer that will get the io description ID.
 * @param rearranger the rearranger to be used for this decomp or 0 to
 * use the default. Valid rearrangers are PIO_REARR_BOX and
 * PIO_REARR_SUBSET.
 * @param iostart An array of start values for block cyclic
 * decompositions. If NULL ???
 * @param iocount An array of count values for block cyclic
 * decompositions. If NULL ???
 * @returns 0 on success, error code otherwise
 * @ingroup PIO_initdecomp
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_init_decomp(int iosysid, int pio_type, int ndims, const int *gdimlen, int maplen,
                     const PIO_Offset *compmap, int *ioidp, int rearranger,
                     const PIO_Offset *iostart, const PIO_Offset *iocount)
{
    PIO_Offset compmap_1_based[maplen];
    int *rearrangerp = NULL;

    LOG((1, "PIOc_init_decomp iosysid = %d pio_type = %d ndims = %d maplen = %d",
         iosysid, pio_type, ndims, maplen));

    /* If the user specified a non-default rearranger, use it. */
    if (rearranger)
        rearrangerp = &rearranger;

    /* Add 1 to all elements in compmap. */
    for (int e = 0; e < maplen; e++)
    {
        LOG((3, "zero-based compmap[%d] = %d", e, compmap[e]));
        compmap_1_based[e] = compmap[e] + 1;
    }

    /* Call the legacy version of the function. */
    return PIOc_InitDecomp(iosysid, pio_type, ndims, gdimlen, maplen, compmap_1_based,
                           ioidp, rearrangerp, iostart, iocount);
}

/**
 * This is a simplified initdecomp which can be used if the memory
 * order of the data can be expressed in terms of start and count on
 * the file. In this case we compute the compdof.
 *
 * @param iosysid the IO system ID
 * @param pio_type
 * @param ndims the number of dimensions
 * @param gdimlen an array length ndims with the sizes of the global
 * dimensions.
 * @param start start array
 * @param count count array
 * @param pointer that gets the IO ID.
 * @returns 0 for success, error code otherwise
 * @ingroup PIO_initdecomp
 * @author Jim Edwards
 */
int PIOc_InitDecomp_bc(int iosysid, int pio_type, int ndims, const int *gdimlen,
                       const long int *start, const long int *count, int *ioidp)

{
    iosystem_desc_t *ios;
    int n, i, maplen = 1;
    PIO_Offset prod[ndims], loc[ndims];
    int rearr = PIO_REARR_SUBSET;

    LOG((1, "PIOc_InitDecomp_bc iosysid = %d pio_type = %d ndims = %d"));

    /* Get the info about the io system. */
    if (!(ios = pio_get_iosystem_from_id(iosysid)))
        return pio_err(NULL, NULL, PIO_EBADID, __FILE__, __LINE__);

    /* Check for required inputs. */
    if (!gdimlen || !start || !count || !ioidp)
        return pio_err(ios, NULL, PIO_EINVAL, __FILE__, __LINE__);

    /* Check that dim, start, and count values are not obviously
     * incorrect. */
    for (int i = 0; i < ndims; i++)
        if (gdimlen[i] <= 0 || start[i] < 0 || count[i] < 0 || (start[i] + count[i]) > gdimlen[i])
            return pio_err(ios, NULL, PIO_EINVAL, __FILE__, __LINE__);

    /* Find the maplen. */
    for (i = 0; i < ndims; i++)
        maplen *= count[i];

    /* Get storage for the compmap. */
    PIO_Offset compmap[maplen];

    /* Find the compmap. */
    prod[ndims - 1] = 1;
    loc[ndims - 1] = 0;
    for (n = ndims - 2; n >= 0; n--)
    {
        prod[n] = prod[n + 1] * gdimlen[n + 1];
        loc[n] = 0;
    }
    for (i = 0; i < maplen; i++)
    {
        compmap[i] = 0;
        for (n = ndims - 1; n >= 0; n--)
            compmap[i] += (start[n] + loc[n]) * prod[n];

        n = ndims - 1;
        loc[n] = (loc[n] + 1) % count[n];
        while (loc[n] == 0 && n > 0)
        {
            n--;
            loc[n] = (loc[n] + 1) % count[n];
        }
    }

    return PIOc_InitDecomp(iosysid, pio_type, ndims, gdimlen, maplen, compmap, ioidp,
                           &rearr, NULL, NULL);
}

/**
 * Library initialization used when IO tasks are a subset of compute
 * tasks.
 *
 * This function creates an MPI intracommunicator between a set of IO
 * tasks and one or more sets of computational tasks.
 *
 * The caller must create all comp_comm and the io_comm MPI
 * communicators before calling this function.
 *
 * Internally, this function does the following:
 *
 * <ul>
 * <li>Initialize logging system (if PIO_ENABLE_LOGGING is set).
 * <li>Allocates and initializes the iosystem_desc_t struct (ios).
 * <li>MPI duplicated user comp_comm to ios->comp_comm and
 * ios->union_comm.
 * <li>Set ios->my_comm to be ios->comp_comm. (Not an MPI
 * duplication.)
 * <li>Find MPI rank in comp_comm, determine ranks of IO tasks,
 * determine whether this task is one of the IO tasks.
 * <li>Identify the root IO tasks.
 * <li>Create MPI groups for IO tasks, and for computation tasks.
 * <li>On IO tasks, create an IO communicator (ios->io_comm).
 * <li>Assign an iosystemid, and put this iosystem_desc_t into the
 * list of open iosystems.
 * <li>Initialize the bget buffer, unless PIO_USE_MALLOC was used.
 * </ul>
 *
 * When complete, there are three MPI communicators (ios->comp_comm,
 * ios->union_comm, and ios->io_comm) that must be freed by MPI.
 *
 * @param comp_comm the MPI_Comm of the compute tasks.
 * @param num_iotasks the number of io tasks to use.
 * @param stride the offset between io tasks in the comp_comm.
 * @param base the comp_comm index of the first io task.
 * @param rearr the rearranger to use by default, this may be
 * overriden in the PIO_init_decomp(). The rearranger is not used
 * until the decomposition is initialized.
 * @param iosysidp index of the defined system descriptor.
 * @return 0 on success, otherwise a PIO error code.
 * @ingroup PIO_init
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_Init_Intracomm(MPI_Comm comp_comm, int num_iotasks, int stride, int base,
                        int rearr, int *iosysidp)
{
    iosystem_desc_t *ios;
    int ustride;
    MPI_Group compgroup;  /* Contains tasks involved in computation. */
    MPI_Group iogroup;    /* Contains the processors involved in I/O. */
    int num_comptasks; /* The size of the comp_comm. */
    int mpierr;        /* Return value for MPI calls. */
    int ret;           /* Return code for function calls. */

    /* Turn on the logging system. */
    pio_init_logging();

    /* Find the number of computation tasks. */
    if ((mpierr = MPI_Comm_size(comp_comm, &num_comptasks)))
        return check_mpi2(NULL, NULL, mpierr, __FILE__, __LINE__);

    /* Check the inputs. */
    if (!iosysidp || num_iotasks < 1 || num_iotasks * stride > num_comptasks)
        return pio_err(NULL, NULL, PIO_EINVAL, __FILE__, __LINE__);

    LOG((1, "PIOc_Init_Intracomm comp_comm = %d num_iotasks = %d stride = %d base = %d "
         "rearr = %d", comp_comm, num_iotasks, stride, base, rearr));

    /* Allocate memory for the iosystem info. */
    if (!(ios = calloc(1, sizeof(iosystem_desc_t))))
        return pio_err(NULL, NULL, PIO_ENOMEM, __FILE__, __LINE__);

    ios->io_comm = MPI_COMM_NULL;
    ios->intercomm = MPI_COMM_NULL;
    ios->error_handler = default_error_handler;
    ios->default_rearranger = rearr;
    ios->num_iotasks = num_iotasks;
    ios->num_comptasks = num_comptasks;

    /* For non-async, the IO tasks are a subset of the comptasks. */
    ios->num_uniontasks = num_comptasks;

    /* Initialize the rearranger options. */
    ios->rearr_opts.comm_type = PIO_REARR_COMM_COLL;
    ios->rearr_opts.fcd = PIO_REARR_COMM_FC_2D_DISABLE;

    /* Copy the computation communicator into union_comm. */
    if ((mpierr = MPI_Comm_dup(comp_comm, &ios->union_comm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);

    /* Copy the computation communicator into comp_comm. */
    if ((mpierr = MPI_Comm_dup(comp_comm, &ios->comp_comm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    LOG((2, "union_comm = %d comp_comm = %d", ios->union_comm, ios->comp_comm));

    ios->my_comm = ios->comp_comm;
    ustride = stride;

    /* Find MPI rank in comp_comm communicator. */
    if ((mpierr = MPI_Comm_rank(ios->comp_comm, &ios->comp_rank)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);

    /* With non-async, all tasks are part of computation component. */
    ios->compproc = true;

    /* Create an array that holds the ranks of the tasks to be used
     * for computation. */
    if (!(ios->compranks = calloc(ios->num_comptasks, sizeof(int))))
        return pio_err(ios, NULL, PIO_ENOMEM, __FILE__, __LINE__);
    for (int i = 0; i < ios->num_comptasks; i++)
        ios->compranks[i] = i;

    /* Is this the comp master? */
    if (ios->comp_rank == 0)
        ios->compmaster = MPI_ROOT;
    LOG((2, "comp_rank = %d num_comptasks = %d", ios->comp_rank, ios->num_comptasks));

    /* Create an array that holds the ranks of the tasks to be used
     * for IO. */
    if (!(ios->ioranks = calloc(ios->num_iotasks, sizeof(int))))
        return pio_err(ios, NULL, PIO_ENOMEM, __FILE__, __LINE__);
    for (int i = 0; i < ios->num_iotasks; i++)
    {
        ios->ioranks[i] = (base + i * ustride) % ios->num_comptasks;
        if (ios->ioranks[i] == ios->comp_rank)
            ios->ioproc = true;
        LOG((3, "ios->ioranks[%d] = %d", i, ios->ioranks[i]));
    }
    ios->ioroot = ios->ioranks[0];

    /* We are not providing an info object. */
    ios->info = MPI_INFO_NULL;

    /* Identify the task that will be the root of the IO communicator. */
    if (ios->comp_rank == ios->ioranks[0])
        ios->iomaster = MPI_ROOT;

    /* Create a group for the computation tasks. */
    if ((mpierr = MPI_Comm_group(ios->comp_comm, &compgroup)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);

    /* Create a group for the IO tasks. */
    if ((mpierr = MPI_Group_incl(compgroup, ios->num_iotasks, ios->ioranks,
                                 &iogroup)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);    

    /* Create an MPI communicator for the IO tasks. */
    if ((mpierr = MPI_Comm_create(ios->comp_comm, iogroup, &ios->io_comm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);

    /* Free the MPI groups. */
    if (compgroup != MPI_GROUP_NULL)
        MPI_Group_free(&compgroup);

    if (iogroup != MPI_GROUP_NULL)
        MPI_Group_free(&iogroup);

    /* For the tasks that are doing IO, get their rank within the IO
     * communicator. If they are not doing IO, set their io_rank to
     * -1. */
    if (ios->ioproc)
    {
        if ((mpierr = MPI_Comm_rank(ios->io_comm, &ios->io_rank)))
            return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    }
    else
        ios->io_rank = -1;
    LOG((3, "ios->io_comm = %d ios->io_rank = %d", ios->io_comm, ios->io_rank));

    /* Rank in the union comm is the same as rank in the comp comm. */
    ios->union_rank = ios->comp_rank;

    /* Add this ios struct to the list in the PIO library. */
    *iosysidp = pio_add_to_iosystem_list(ios);

    /* Allocate buffer space for compute nodes. */
    if ((ret = compute_buffer_init(ios)))
        return ret;

    LOG((2, "Init_Intracomm complete iosysid = %d", *iosysidp));

    return PIO_NOERR;
}

/**
 * Interface to call from pio_init from fortran.
 *
 * @param f90_comp_comm
 * @param num_iotasks the number of IO tasks
 * @param stride the stride to use assigning tasks
 * @param base the starting point when assigning tasks
 * @param rearr the rearranger
 * @param rearr_opts the rearranger options
 * @param iosysidp a pointer that gets the IO system ID
 * @returns 0 for success, error code otherwise
 * @author Jim Edwards
 */
int PIOc_Init_Intracomm_from_F90(int f90_comp_comm,
                                 const int num_iotasks, const int stride,
                                 const int base, const int rearr,
                                 rearr_opt_t *rearr_opts, int *iosysidp)
{
    int ret = PIO_NOERR;
    ret = PIOc_Init_Intracomm(MPI_Comm_f2c(f90_comp_comm), num_iotasks,
                              stride, base, rearr,
                              iosysidp);
    if (ret != PIO_NOERR)
    {
        LOG((1, "PIOc_Init_Intracomm failed"));
        return ret;
    }

    if (rearr_opts)
    {
        LOG((1, "Setting rearranger options, iosys=%d", *iosysidp));
        return PIOc_set_rearr_opts(*iosysidp, rearr_opts->comm_type,
                                   rearr_opts->fcd,
                                   rearr_opts->comp2io.hs,
                                   rearr_opts->comp2io.isend,
                                   rearr_opts->comp2io.max_pend_req,
                                   rearr_opts->io2comp.hs,
                                   rearr_opts->io2comp.isend,
                                   rearr_opts->io2comp.max_pend_req);
    }
    return ret;
}

/**
 * Send a hint to the MPI-IO library.
 *
 * @param iosysid the IO system ID
 * @param hint the hint for MPI
 * @param hintval the value of the hint
 * @returns 0 for success, or PIO_BADID if iosysid can't be found.
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_set_hint(int iosysid, const char *hint, const char *hintval)
{
    iosystem_desc_t *ios;
    int mpierr; /* Return value for MPI calls. */

    /* Get the iosysid. */
    if (!(ios = pio_get_iosystem_from_id(iosysid)))
        return pio_err(NULL, NULL, PIO_EBADID, __FILE__, __LINE__);

    /* User must provide these. */
    if (!hint || !hintval)
        return pio_err(ios, NULL, PIO_EINVAL, __FILE__, __LINE__);

    LOG((1, "PIOc_set_hint hint = %s hintval = %s", hint, hintval));

    /* Make sure we have an info object. */
    if (ios->info == MPI_INFO_NULL)
        if ((mpierr = MPI_Info_create(&ios->info)))
            return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);

    /* Set the MPI hint. */
    if (ios->ioproc)
        if ((mpierr = MPI_Info_set(ios->info, hint, hintval)))
            return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);

    return PIO_NOERR;
}

/**
 * Clean up internal data structures, free MPI resources, and exit the
 * pio library.
 *
 * @param iosysid: the io system ID provided by PIOc_Init_Intracomm().
 * @returns 0 for success or non-zero for error.
 * @ingroup PIO_finalize
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_finalize(int iosysid)
{
    iosystem_desc_t *ios;
    int niosysid;          /* The number of currently open IO systems. */
    int mpierr = MPI_SUCCESS, mpierr2;  /* Return code from MPI function codes. */
    int ierr = PIO_NOERR;

    LOG((1, "PIOc_finalize iosysid = %d MPI_COMM_NULL = %d", iosysid,
         MPI_COMM_NULL));

    /* Find the IO system information. */
    if (!(ios = pio_get_iosystem_from_id(iosysid)))
        return pio_err(NULL, NULL, PIO_EBADID, __FILE__, __LINE__);

    /* If asynch IO is in use, send the PIO_MSG_EXIT message from the
     * comp master to the IO processes. This may be called by
     * componets for other components iosysid. So don't send unless
     * there is a valid union_comm. */
    if (ios->async && ios->union_comm != MPI_COMM_NULL)
    {
        int msg = PIO_MSG_EXIT;

        LOG((3, "found iosystem info comproot = %d union_comm = %d comp_idx = %d",
             ios->comproot, ios->union_comm, ios->comp_idx));
        if (!ios->ioproc)
        {
            LOG((2, "sending msg = %d ioroot = %d union_comm = %d", msg,
                 ios->ioroot, ios->union_comm));

            /* Send the message to the message handler. */
            if (ios->compmaster == MPI_ROOT)
                mpierr = MPI_Send(&msg, 1, MPI_INT, ios->ioroot, 1, ios->union_comm);

            /* Send the parameters of the function call. */
            if (!mpierr)
                mpierr = MPI_Bcast((int *)&iosysid, 1, MPI_INT, ios->compmaster, ios->intercomm);
        }

        /* Handle MPI errors. */
        LOG((3, "handling async errors mpierr = %d my_comm = %d", mpierr, ios->my_comm));
        if ((mpierr2 = MPI_Bcast(&mpierr, 1, MPI_INT, ios->comproot, ios->my_comm)))
            return check_mpi2(ios, NULL, mpierr2, __FILE__, __LINE__);
        if (mpierr)
            return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
        LOG((3, "async errors bcast"));
    }

    /* Free this memory that was allocated in init_intracomm. */
    if (ios->ioranks)
        free(ios->ioranks);
    LOG((3, "Freed ioranks."));
    if (ios->compranks)
        free(ios->compranks);
    LOG((3, "Freed compranks."));

    /* Learn the number of open IO systems. */
    if ((ierr = pio_num_iosystem(&niosysid)))
        return pio_err(ios, NULL, ierr, __FILE__, __LINE__);
    LOG((2, "%d iosystems are still open.", niosysid));

    /* Only free the buffer pool if this is the last open iosysid. */
    if (niosysid == 1)
    {
        free_cn_buffer_pool(ios);
        LOG((2, "Freed buffer pool."));
    }

    /* Free the MPI communicators. my_comm is just a copy (but not an
     * MPI copy), so does not have to have an MPI_Comm_free()
     * call. comp_comm and io_comm are MPI duplicates of the comms
     * handed into init_intercomm. So they need to be freed by MPI. */
    if (ios->intercomm != MPI_COMM_NULL)
        MPI_Comm_free(&ios->intercomm);
    if (ios->union_comm != MPI_COMM_NULL)
        MPI_Comm_free(&ios->union_comm);
    if (ios->io_comm != MPI_COMM_NULL)
        MPI_Comm_free(&ios->io_comm);
    if (ios->comp_comm != MPI_COMM_NULL)
        MPI_Comm_free(&ios->comp_comm);
    if (ios->my_comm != MPI_COMM_NULL)
        ios->my_comm = MPI_COMM_NULL;

    /* Free the MPI Info object. */
    if (ios->info != MPI_INFO_NULL)
        MPI_Info_free(&ios->info);

    /* Delete the iosystem_desc_t data associated with this id. */
    LOG((2, "About to delete iosysid %d.", iosysid));
    if ((ierr = pio_delete_iosystem_from_list(iosysid)))
        return pio_err(NULL, NULL, ierr, __FILE__, __LINE__);

    if (niosysid == 1)
    {
        LOG((1, "about to finalize logging"));
        pio_finalize_logging();
    }

    LOG((2, "PIOc_finalize completed successfully"));
    return PIO_NOERR;
}

/**
 * Return a logical indicating whether this task is an IO task.
 *
 * @param iosysid the io system ID
 * @param ioproc a pointer that gets 1 if task is an IO task, 0
 * otherwise. Ignored if NULL.
 * @returns 0 for success, or PIO_BADID if iosysid can't be found.
 * @author Jim Edwards
 */
int PIOc_iam_iotask(int iosysid, bool *ioproc)
{
    iosystem_desc_t *ios;

    if (!(ios = pio_get_iosystem_from_id(iosysid)))
        return pio_err(NULL, NULL, PIO_EBADID, __FILE__, __LINE__);

    if (ioproc)
        *ioproc = ios->ioproc;

    return PIO_NOERR;
}

/**
 * Return the rank of this task in the IO communicator or -1 if this
 * task is not in the communicator.
 *
 * @param iosysid the io system ID
 * @param iorank a pointer that gets the io rank, or -1 if task is not
 * in the IO communicator. Ignored if NULL.
 * @returns 0 for success, or PIO_BADID if iosysid can't be found.
 * @author Jim Edwards
 */
int PIOc_iotask_rank(int iosysid, int *iorank)
{
    iosystem_desc_t *ios;

    if (!(ios = pio_get_iosystem_from_id(iosysid)))
        return pio_err(NULL, NULL, PIO_EBADID, __FILE__, __LINE__);

    if (iorank)
        *iorank = ios->io_rank;

    return PIO_NOERR;
}

/**
 * Return true if this iotype is supported in the build, 0 otherwise.
 *
 * @param iotype the io type to check
 * @returns 1 if iotype is in build, 0 if not.
 * @author Jim Edwards
 */
int PIOc_iotype_available(int iotype)
{
    switch(iotype)
    {
#ifdef _NETCDF4
    case PIO_IOTYPE_NETCDF4P:
    case PIO_IOTYPE_NETCDF4C:
        return 1;
#endif
    case PIO_IOTYPE_NETCDF:
        return 1;
#ifdef _PNETCDF
    case PIO_IOTYPE_PNETCDF:
        return 1;
        break;
#endif
    default:
        return 0;
    }
}

/**
 * Library initialization used when IO tasks are distinct from compute
 * tasks.
 *
 * This is a collective call.  Input parameters are read on
 * comp_rank=0 values on other tasks are ignored.  This variation of
 * PIO_init sets up a distinct set of tasks to handle IO, these tasks
 * do not return from this call.  Instead they go to an internal loop
 * and wait to receive further instructions from the computational
 * tasks.
 *
 * Sequence of Events to do Asynch I/O
 * -----------------------------------
 *
 * Here is the sequence of events that needs to occur when an IO
 * operation is called from the collection of compute tasks.  I'm
 * going to use pio_put_var because write_darray has some special
 * characteristics that make it a bit more complicated...
 *
 * Compute tasks call pio_put_var with an integer argument
 *
 * The MPI_Send sends a message from comp_rank=0 to io_rank=0 on
 * union_comm (a comm defined as the union of io and compute tasks)
 * msg is an integer which indicates the function being called, in
 * this case the msg is PIO_MSG_PUT_VAR_INT
 *
 * The iotasks now know what additional arguments they should expect
 * to receive from the compute tasks, in this case a file handle, a
 * variable id, the length of the array and the array itself.
 *
 * The iotasks now have the information they need to complete the
 * operation and they call the pio_put_var routine.  (In pio1 this bit
 * of code is in pio_get_put_callbacks.F90.in)
 *
 * After the netcdf operation is completed (in the case of an inq or
 * get operation) the result is communicated back to the compute
 * tasks.
 *
 * @param world the communicator containing all the available tasks.
 *
 * @param num_io_procs the number of processes for the IO component.
 *
 * @param io_proc_list an array of lenth num_io_procs with the
 * processor number for each IO processor. If NULL then the IO
 * processes are assigned starting at processes 0.
 *
 * @param component_count number of computational components
 *
 * @param num_procs_per_comp an array of int, of length
 * component_count, with the number of processors in each computation
 * component.
 *
 * @param proc_list an array of arrays containing the processor
 * numbers for each computation component. If NULL then the
 * computation components are assigned processors sequentially
 * starting with processor num_io_procs.
 *
 * @param user_io_comm pointer to an MPI_Comm. If not NULL, it will
 * get an MPI duplicate of the IO communicator. (It is a full
 * duplicate and later must be freed with MPI_Free() by the caller.)
 *
 * @param user_comp_comm pointer to an array of pointers to MPI_Comm;
 * the array is of length component_count. If not NULL, it will get an
 * MPI duplicate of each computation communicator. (These are full
 * duplicates and each must later be freed with MPI_Free() by the
 * caller.)
 *
 * @param rearranger the default rearranger to use for decompositions
 * in this IO system. Only PIO_REARR_BOX is supported for
 * async. Support for PIO_REARR_SUBSET will be provided in a future
 * version.
 *
 * @param iosysidp pointer to array of length component_count that
 * gets the iosysid for each component.
 *
 * @return PIO_NOERR on success, error code otherwise.
 * @ingroup PIO_init
 * @author Ed Hartnett
 */
int PIOc_init_async(MPI_Comm world, int num_io_procs, int *io_proc_list,
                    int component_count, int *num_procs_per_comp, int **proc_list,
                    MPI_Comm *user_io_comm, MPI_Comm *user_comp_comm, int rearranger,
                    int *iosysidp)
{
    int my_rank;          /* Rank of this task. */
    int *my_proc_list[component_count];   /* Array of arrays of procs for comp components. */
    int my_io_proc_list[num_io_procs]; /* List of processors in IO component. */
    int mpierr;           /* Return code from MPI functions. */
    int ret;              /* Return code. */

    /* Check input parameters. Only allow box rearranger for now. */
    if (num_io_procs < 1 || component_count < 1 || !num_procs_per_comp || !iosysidp ||
        (rearranger != PIO_REARR_BOX))
        return pio_err(NULL, NULL, PIO_EINVAL, __FILE__, __LINE__);

    /* Turn on the logging system for PIO. */
    pio_init_logging();
    LOG((1, "PIOc_init_async num_io_procs = %d component_count = %d", num_io_procs,
         component_count));

    /* Determine which tasks to use for IO. */
    for (int p = 0; p < num_io_procs; p++)
        my_io_proc_list[p] = io_proc_list ? io_proc_list[p] : p;

    /* Determine which tasks to use for each computational component. */
    if ((ret = determine_procs(num_io_procs, component_count, num_procs_per_comp,
                               proc_list, my_proc_list)))
        return pio_err(NULL, NULL, ret, __FILE__, __LINE__);        

    /* Get rank of this task in world. */
    if ((ret = MPI_Comm_rank(world, &my_rank)))
        return check_mpi(NULL, ret, __FILE__, __LINE__);

    /* Is this process in the IO component? */
    int pidx;
    for (pidx = 0; pidx < num_io_procs; pidx++)
        if (my_rank == my_io_proc_list[pidx])
            break;
    int in_io = (pidx == num_io_procs) ? 0 : 1;
    LOG((3, "in_io = %d", in_io));

    /* Allocate struct to hold io system info for each computation component. */
    iosystem_desc_t *iosys[component_count], *my_iosys;
    for (int cmp1 = 0; cmp1 < component_count; cmp1++)
        if (!(iosys[cmp1] = (iosystem_desc_t *)calloc(1, sizeof(iosystem_desc_t))))
            return pio_err(NULL, NULL, PIO_ENOMEM, __FILE__, __LINE__);

    /* Create group for world. */
    MPI_Group world_group;
    if ((ret = MPI_Comm_group(world, &world_group)))
        return check_mpi(NULL, ret, __FILE__, __LINE__);
    LOG((3, "world group created"));

    /* We will create a group for the IO component. */
    MPI_Group io_group;

    /* The shared IO communicator. */
    MPI_Comm io_comm;

    /* Rank of current process in IO communicator. */
    int io_rank = -1;

    /* Set to MPI_ROOT on master process, MPI_PROC_NULL on other
     * processes. */
    int iomaster;

    /* Create a group for the IO component. */
    if ((ret = MPI_Group_incl(world_group, num_io_procs, my_io_proc_list, &io_group)))
        return check_mpi(NULL, ret, __FILE__, __LINE__);
    LOG((3, "created IO group - io_group = %d MPI_GROUP_EMPTY = %d", io_group, MPI_GROUP_EMPTY));

    /* There is one shared IO comm. Create it. */
    if ((ret = MPI_Comm_create(world, io_group, &io_comm)))
        return check_mpi(NULL, ret, __FILE__, __LINE__);
    LOG((3, "created io comm io_comm = %d", io_comm));

    /* Does the user want a copy of the IO communicator? */
    if (user_io_comm)
    {
        *user_io_comm = MPI_COMM_NULL;
        if (in_io)
            if ((mpierr = MPI_Comm_dup(io_comm, user_io_comm)))
                return check_mpi(NULL, mpierr, __FILE__, __LINE__);
    }

    /* For processes in the IO component, get their rank within the IO
     * communicator. */
    if (in_io)
    {
        LOG((3, "about to get io rank"));
        if ((ret = MPI_Comm_rank(io_comm, &io_rank)))
            return check_mpi(NULL, ret, __FILE__, __LINE__);
        iomaster = !io_rank ? MPI_ROOT : MPI_PROC_NULL;
        LOG((3, "intracomm created for io_comm = %d io_rank = %d IO %s",
             io_comm, io_rank, iomaster == MPI_ROOT ? "MASTER" : "SERVANT"));
    }

    /* We will create a group for each computational component. */
    MPI_Group group[component_count];

    /* We will also create a group for each component and the IO
     * component processes (i.e. a union of computation and IO
     * processes. */
    MPI_Group union_group[component_count];

    /* For each computation component. */
    for (int cmp = 0; cmp < component_count; cmp++)
    {
        LOG((3, "processing component %d", cmp));

        /* Get pointer to current iosys. */
        my_iosys = iosys[cmp];

        /* Initialize some values. */
        my_iosys->io_comm = MPI_COMM_NULL;
        my_iosys->comp_comm = MPI_COMM_NULL;
        my_iosys->union_comm = MPI_COMM_NULL;
        my_iosys->intercomm = MPI_COMM_NULL;
        my_iosys->my_comm = MPI_COMM_NULL;
        my_iosys->async = 1;
        my_iosys->error_handler = default_error_handler;
        my_iosys->num_comptasks = num_procs_per_comp[cmp];
        my_iosys->num_iotasks = num_io_procs;
        my_iosys->num_uniontasks = my_iosys->num_comptasks + my_iosys->num_iotasks;
        my_iosys->default_rearranger = rearranger;

        /* Initialize the rearranger options. */
        my_iosys->rearr_opts.comm_type = PIO_REARR_COMM_COLL;
        my_iosys->rearr_opts.fcd = PIO_REARR_COMM_FC_2D_DISABLE;

        /* The rank of the computation leader in the union comm. */
        my_iosys->comproot = num_io_procs;
        LOG((3, "my_iosys->comproot = %d", my_iosys->comproot));

        /* We are not providing an info object. */
        my_iosys->info = MPI_INFO_NULL;

        /* Create a group for this component. */
        if ((ret = MPI_Group_incl(world_group, num_procs_per_comp[cmp], my_proc_list[cmp],
                                  &group[cmp])))
            return check_mpi(NULL, ret, __FILE__, __LINE__);
        LOG((3, "created component MPI group - group[%d] = %d", cmp, group[cmp]));

        /* For all the computation components create a union group
         * with their processors and the processors of the (shared) IO
         * component. */

        /* How many processors in the union comm? */
        int nprocs_union = num_io_procs + num_procs_per_comp[cmp];

        /* This will hold proc numbers from both computation and IO
         * components. */
        int proc_list_union[nprocs_union];

        /* Add proc numbers from IO. */
        for (int p = 0; p < num_io_procs; p++)
            proc_list_union[p] = my_io_proc_list[p];

        /* Add proc numbers from computation component. */
        for (int p = 0; p < num_procs_per_comp[cmp]; p++)
        {
            proc_list_union[p + num_io_procs] = my_proc_list[cmp][p];
            LOG((3, "p %d num_io_procs %d proc_list_union[p + num_io_procs] %d ",
                 p, num_io_procs, proc_list_union[p + num_io_procs]));
        }

        /* Allocate space for computation task ranks. */
        if (!(my_iosys->compranks = calloc(my_iosys->num_comptasks, sizeof(int))))
            return pio_err(NULL, NULL, PIO_ENOMEM, __FILE__, __LINE__);

        /* Remember computation task ranks. We need the ranks within
         * the union_comm. */
        for (int p = 0; p < num_procs_per_comp[cmp]; p++)
            my_iosys->compranks[p] = num_io_procs + p;

        /* Remember whether this process is in the IO component. */
        my_iosys->ioproc = in_io;

        /* With async, tasks are either in a computation component or
         * the IO component. */
        my_iosys->compproc = !in_io;

        /* Is this process in this computation component? */
        int in_cmp = 0;
        for (pidx = 0; pidx < num_procs_per_comp[cmp]; pidx++)
            if (my_rank == my_proc_list[cmp][pidx])
                break;
        in_cmp = (pidx == num_procs_per_comp[cmp]) ? 0 : 1;
        LOG((3, "pidx = %d num_procs_per_comp[%d] = %d in_cmp = %d",
             pidx, cmp, num_procs_per_comp[cmp], in_cmp));

        /* Create the union group. */
        if ((ret = MPI_Group_incl(world_group, nprocs_union, proc_list_union, &union_group[cmp])))
            return check_mpi(NULL, ret, __FILE__, __LINE__);
        LOG((3, "created union MPI_group - union_group[%d] = %d with %d procs", cmp,
             union_group[cmp], nprocs_union));

        /* Create an intracomm for this component. Only processes in
         * the component need to participate in the intracomm create
         * call. */
        LOG((3, "creating intracomm cmp = %d from group[%d] = %d", cmp, cmp, group[cmp]));
        if ((ret = MPI_Comm_create(world, group[cmp], &my_iosys->comp_comm)))
            return check_mpi(NULL, ret, __FILE__, __LINE__);

        if (in_cmp)
        {
            /* Does the user want a copy? */
            if (user_comp_comm)
                if ((mpierr = MPI_Comm_dup(my_iosys->comp_comm, &user_comp_comm[cmp])))
                    return check_mpi(NULL, mpierr, __FILE__, __LINE__);

            /* Get the rank in this comp comm. */
            if ((ret = MPI_Comm_rank(my_iosys->comp_comm, &my_iosys->comp_rank)))
                return check_mpi(NULL, ret, __FILE__, __LINE__);

            /* Set comp_rank 0 to be the compmaster. It will have a
             * setting of MPI_ROOT, all other tasks will have a
             * setting of MPI_PROC_NULL. */
            my_iosys->compmaster = my_iosys->comp_rank ? MPI_PROC_NULL : MPI_ROOT;

            LOG((3, "intracomm created for cmp = %d comp_comm = %d comp_rank = %d comp %s",
                 cmp, my_iosys->comp_comm, my_iosys->comp_rank,
                 my_iosys->compmaster == MPI_ROOT ? "MASTER" : "SERVANT"));
        }

        /* If this is the IO component, make a copy of the IO comm for
         * each computational component. */
        if (in_io)
        {
            LOG((3, "making a dup of io_comm = %d io_rank = %d", io_comm, io_rank));
            if ((ret = MPI_Comm_dup(io_comm, &my_iosys->io_comm)))
                return check_mpi(NULL, ret, __FILE__, __LINE__);
            LOG((3, "dup of io_comm = %d io_rank = %d", my_iosys->io_comm, io_rank));
            my_iosys->iomaster = iomaster;
            my_iosys->io_rank = io_rank;
            my_iosys->ioroot = 0;
            my_iosys->comp_idx = cmp;
        }

        /* Create an array that holds the ranks of the tasks to be used
         * for IO. */
        if (!(my_iosys->ioranks = calloc(my_iosys->num_iotasks, sizeof(int))))
            return pio_err(NULL, NULL, PIO_ENOMEM, __FILE__, __LINE__);
        for (int i = 0; i < my_iosys->num_iotasks; i++)
            my_iosys->ioranks[i] = my_io_proc_list[i];
        my_iosys->ioroot = my_iosys->ioranks[0];

        /* All the processes in this component, and the IO component,
         * are part of the union_comm. */
        LOG((3, "before creating union_comm my_iosys->io_comm = %d group = %d", my_iosys->io_comm, union_group[cmp]));
        if ((ret = MPI_Comm_create(world, union_group[cmp], &my_iosys->union_comm)))
            return check_mpi(NULL, ret, __FILE__, __LINE__);
        LOG((3, "created union comm for cmp %d my_iosys->union_comm %d", cmp, my_iosys->union_comm));

        if (in_io || in_cmp)
        {
            if ((ret = MPI_Comm_rank(my_iosys->union_comm, &my_iosys->union_rank)))
                return check_mpi(NULL, ret, __FILE__, __LINE__);
            LOG((3, "my_iosys->union_rank %d", my_iosys->union_rank));
            
            /* Set my_comm to union_comm for async. */
            my_iosys->my_comm = my_iosys->union_comm;
            LOG((3, "intracomm created for union cmp = %d union_rank = %d union_comm = %d",
                 cmp, my_iosys->union_rank, my_iosys->union_comm));

            if (in_io)
            {
                LOG((3, "my_iosys->io_comm = %d", my_iosys->io_comm));
                /* Create the intercomm from IO to computation component. */
                LOG((3, "about to create intercomm for IO component to cmp = %d "
                     "my_iosys->io_comm = %d", cmp, my_iosys->io_comm));
                if ((ret = MPI_Intercomm_create(my_iosys->io_comm, 0, my_iosys->union_comm,
                                                my_iosys->num_iotasks, cmp, &my_iosys->intercomm)))
                    return check_mpi(NULL, ret, __FILE__, __LINE__);
            }
            else
            {
                /* Create the intercomm from computation component to IO component. */
                LOG((3, "about to create intercomm for cmp = %d my_iosys->comp_comm = %d", cmp,
                     my_iosys->comp_comm));
                if ((ret = MPI_Intercomm_create(my_iosys->comp_comm, 0, my_iosys->union_comm,
                                                0, cmp, &my_iosys->intercomm)))
                    return check_mpi(NULL, ret, __FILE__, __LINE__);
            }
            LOG((3, "intercomm created for cmp = %d", cmp));
        }

        /* Add this id to the list of PIO iosystem ids. */
        iosysidp[cmp] = pio_add_to_iosystem_list(my_iosys);
        LOG((2, "new iosys ID added to iosystem_list iosysidp[%d] = %d", cmp, iosysidp[cmp]));
    } /* next computational component */

    /* Now call the function from which the IO tasks will not return
     * until the PIO_MSG_EXIT message is sent. This will handle all
     * components. */
    if (in_io)
    {
        LOG((2, "Starting message handler io_rank = %d component_count = %d",
             io_rank, component_count));
        if ((ret = pio_msg_handler2(io_rank, component_count, iosys, io_comm)))
            return pio_err(NULL, NULL, ret, __FILE__, __LINE__);
        LOG((2, "Returned from pio_msg_handler2() ret = %d", ret));
    }

    /* Free resources if needed. */
    if (in_io)
        if ((mpierr = MPI_Comm_free(&io_comm)))
            return check_mpi(NULL, ret, __FILE__, __LINE__);

    /* Free the arrays of processor numbers. */
    for (int cmp = 0; cmp < component_count; cmp++)
        free(my_proc_list[cmp]);

    /* Free MPI groups. */
    if ((ret = MPI_Group_free(&io_group)))
        return check_mpi(NULL, ret, __FILE__, __LINE__);

    for (int cmp = 0; cmp < component_count; cmp++)
    {
        if ((ret = MPI_Group_free(&group[cmp])))
            return check_mpi(NULL, ret, __FILE__, __LINE__);
        if ((ret = MPI_Group_free(&union_group[cmp])))
            return check_mpi(NULL, ret, __FILE__, __LINE__);
    }

    if ((ret = MPI_Group_free(&world_group)))
        return check_mpi(NULL, ret, __FILE__, __LINE__);

    LOG((2, "successfully done with PIO_Init_Async"));
    return PIO_NOERR;
}

/**
 * Set the target blocksize for the box rearranger.
 *
 * @param newblocksize the new blocksize.
 * @returns 0 for success.
 * @ingroup PIO_set_blocksize
 * @author Jim Edwards
 */
int PIOc_set_blocksize(int newblocksize)
{
    if (newblocksize > 0)
        blocksize = newblocksize;
    return PIO_NOERR;
}
