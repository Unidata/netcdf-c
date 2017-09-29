/**
 * @file
 * PIO interfaces to
 * [NetCDF](http://www.unidata.ucar.edu/software/netcdf/docs/modules.html)
 * support functions
 *
 *  This file provides an interface to the
 *  [NetCDF](http://www.unidata.ucar.edu/software/netcdf/docs/modules.html)
 *  support functions.  Each subroutine calls the underlying netcdf or
 *  pnetcdf or netcdf4 functions from the appropriate subset of mpi
 *  tasks (io_comm). Each routine must be called collectively from
 *  union_comm.
 *
 * @author Jim Edwards (jedwards@ucar.edu), Ed Hartnett
 * @date     Feburary 2014, April 2016
 */
#include <config.h>
#include <pio.h>
#include <pio_internal.h>

/**
 * @ingroup PIO_inq
 * The PIO-C interface for the NetCDF function nc_inq.
 *
 * This routine is called collectively by all tasks in the
 * communicator ios.union_comm. For more information on the underlying
 * NetCDF commmand please read about this function in the NetCDF
 * documentation at:
 * http://www.unidata.ucar.edu/software/netcdf/docs/group__datasets.html
 *
 * @param ncid the ncid of the open file, obtained from
 * PIOc_openfile() or PIOc_createfile().
 *
 * @return PIO_NOERR for success, error code otherwise. See
 * PIOc_Set_File_Error_Handling
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_inq(int ncid, int *ndimsp, int *nvarsp, int *ngattsp, int *unlimdimidp)
{
    iosystem_desc_t *ios;  /* Pointer to io system information. */
    file_desc_t *file;     /* Pointer to file information. */
    int ierr;              /* Return code from function calls. */
    int mpierr = MPI_SUCCESS, mpierr2;  /* Return code from MPI function calls. */

    LOG((1, "PIOc_inq ncid = %d", ncid));

    /* Find the info about this file. */
    if ((ierr = pio_get_file(ncid, &file)))
        return pio_err(NULL, NULL, ierr, __FILE__, __LINE__);
    ios = file->iosystem;

    /* If async is in use, and this is not an IO task, bcast the parameters. */
    if (ios->async)
    {
        if (!ios->ioproc)
        {
            int msg = PIO_MSG_INQ; /* Message for async notification. */
            char ndims_present = ndimsp ? true : false;
            char nvars_present = nvarsp ? true : false;
            char ngatts_present = ngattsp ? true : false;
            char unlimdimid_present = unlimdimidp ? true : false;

            if (ios->compmaster == MPI_ROOT)
                mpierr = MPI_Send(&msg, 1, MPI_INT, ios->ioroot, 1, ios->union_comm);

            if (!mpierr)
                mpierr = MPI_Bcast(&ncid, 1, MPI_INT, ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast(&ndims_present, 1, MPI_CHAR, ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast(&nvars_present, 1, MPI_CHAR, ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast(&ngatts_present, 1, MPI_CHAR, ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast(&unlimdimid_present, 1, MPI_CHAR, ios->compmaster, ios->intercomm);
            LOG((2, "PIOc_inq ncid = %d ndims_present = %d nvars_present = %d ngatts_present = %d unlimdimid_present = %d",
                 ncid, ndims_present, nvars_present, ngatts_present, unlimdimid_present));
        }

        /* Handle MPI errors. */
        if ((mpierr2 = MPI_Bcast(&mpierr, 1, MPI_INT, ios->comproot, ios->my_comm)))
            return check_mpi(file, mpierr2, __FILE__, __LINE__);
        if (mpierr)
            return check_mpi(file, mpierr, __FILE__, __LINE__);
    }

    /* If this is an IO task, then call the netCDF function. */
    if (ios->ioproc)
    {
#ifdef _PNETCDF
        if (file->iotype == PIO_IOTYPE_PNETCDF)
        {
            ierr = ncmpi_inq(file->fh, ndimsp, nvarsp, ngattsp, unlimdimidp);
            if (unlimdimidp)
                LOG((2, "PIOc_inq returned from ncmpi_inq unlimdimid = %d", *unlimdimidp));
        }
#endif /* _PNETCDF */
        if (file->iotype == PIO_IOTYPE_NETCDF && file->do_io)
        {
            LOG((2, "PIOc_inq calling classic nc_inq"));
            /* Should not be necessary to do this - nc_inq should
             * handle null pointers. This has been reported as a bug
             * to netCDF developers. */
            int tmp_ndims, tmp_nvars, tmp_ngatts, tmp_unlimdimid;
            LOG((2, "PIOc_inq calling classic nc_inq"));
            ierr = nc_inq(file->fh, &tmp_ndims, &tmp_nvars, &tmp_ngatts, &tmp_unlimdimid);
            LOG((2, "PIOc_inq calling classic nc_inq"));
            if (unlimdimidp)
                LOG((2, "classic tmp_unlimdimid = %d", tmp_unlimdimid));
            if (ndimsp)
                *ndimsp = tmp_ndims;
            if (nvarsp)
                *nvarsp = tmp_nvars;
            if (ngattsp)
                *ngattsp = tmp_ngatts;
            if (unlimdimidp)
                *unlimdimidp = tmp_unlimdimid;
            if (unlimdimidp)
                LOG((2, "classic unlimdimid = %d", *unlimdimidp));
        }
        else if (file->iotype != PIO_IOTYPE_PNETCDF && file->do_io)
        {
            LOG((2, "PIOc_inq calling netcdf-4 nc_inq"));
            ierr = nc_inq(file->fh, ndimsp, nvarsp, ngattsp, unlimdimidp);
        }

        LOG((2, "PIOc_inq netcdf call returned %d", ierr));
    }

    /* Broadcast and check the return code. */
    if ((mpierr = MPI_Bcast(&ierr, 1, MPI_INT, ios->ioroot, ios->my_comm)))
        return check_mpi(file, mpierr, __FILE__, __LINE__);
    if (ierr)
        return check_netcdf(file, ierr, __FILE__, __LINE__);

    /* Broadcast results to all tasks. Ignore NULL parameters. */
    if (ndimsp)
        if ((mpierr = MPI_Bcast(ndimsp, 1, MPI_INT, ios->ioroot, ios->my_comm)))
            return check_mpi(file, mpierr, __FILE__, __LINE__);

    if (nvarsp)
        if ((mpierr = MPI_Bcast(nvarsp, 1, MPI_INT, ios->ioroot, ios->my_comm)))
            return check_mpi(file, mpierr, __FILE__, __LINE__);

    if (ngattsp)
        if ((mpierr = MPI_Bcast(ngattsp, 1, MPI_INT, ios->ioroot, ios->my_comm)))
            return check_mpi(file, mpierr, __FILE__, __LINE__);

    if (unlimdimidp)
        if ((mpierr = MPI_Bcast(unlimdimidp, 1, MPI_INT, ios->ioroot, ios->my_comm)))
            return check_mpi(file, mpierr, __FILE__, __LINE__);

    return PIO_NOERR;
}

/**
 * @ingroup PIO_inq_ndims
 * Find out how many dimensions are defined in the file.
 *
 * @param ncid the ncid of the open file.
 * @param ndimsp a pointer that will get the number of dimensions.
 * @returns 0 for success, error code otherwise.
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_inq_ndims(int ncid, int *ndimsp)
{
    LOG((1, "PIOc_inq_ndims"));
    return PIOc_inq(ncid, ndimsp, NULL, NULL, NULL);
}

/**
 * @ingroup PIO_inq_nvars
 * Find out how many variables are defined in a file.
 *
 * @param ncid the ncid of the open file.
 * @param nvarsp a pointer that will get the number of variables.
 * @returns 0 for success, error code otherwise.
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_inq_nvars(int ncid, int *nvarsp)
{
    return PIOc_inq(ncid, NULL, nvarsp, NULL, NULL);
}

/**
 * @ingroup PIO_inq_natts
 * Find out how many global attributes are defined in a file.
 *
 * @param ncid the ncid of the open file.
 * @param nattsp a pointer that will get the number of attributes.
 * @returns 0 for success, error code otherwise.
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_inq_natts(int ncid, int *ngattsp)
{
    return PIOc_inq(ncid, NULL, NULL, ngattsp, NULL);
}

/**
 * @ingroup PIO_inq_unlimdim
 * Find out the dimension ids of the unlimited dimension.
 *
 * @param ncid the ncid of the open file.
 * @param unlimdimidp a pointer that will the ID of the unlimited
 * dimension, or -1 if there is no unlimited dimension.
 * @returns 0 for success, error code otherwise.
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_inq_unlimdim(int ncid, int *unlimdimidp)
{
    LOG((1, "PIOc_inq_unlimdim ncid = %d", ncid));
    return PIOc_inq(ncid, NULL, NULL, NULL, unlimdimidp);
}

/**
 * Find out the dimension ids of all unlimited dimensions. Note that
 * only netCDF-4 files can have more than 1 unlimited dimension.
 *
 * @param ncid the ncid of the open file.
 * @param nunlimdimsp a pointer that gets the number of unlimited
 * dimensions. Ignored if NULL.
 * @param unlimdimidsp a pointer that will get an array of unlimited
 * dimension IDs.
 * @returns 0 for success, error code otherwise.
 * @ingroup PIO_inq_unlimdim
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_inq_unlimdims(int ncid, int *nunlimdimsp, int *unlimdimidsp)
{
    iosystem_desc_t *ios;  /* Pointer to io system information. */
    file_desc_t *file;     /* Pointer to file information. */
    int tmp_nunlimdims;    /* The number of unlimited dims. */
    int ierr;              /* Return code from function calls. */
    int mpierr = MPI_SUCCESS, mpierr2;  /* Return code from MPI function calls. */

    LOG((1, "PIOc_inq_unlimdims ncid = %d", ncid));

    /* Find the info about this file. */
    if ((ierr = pio_get_file(ncid, &file)))
        return pio_err(NULL, NULL, ierr, __FILE__, __LINE__);
    ios = file->iosystem;

    /* If async is in use, and this is not an IO task, bcast the parameters. */
    if (ios->async)
    {
        if (!ios->ioproc)
        {
            int msg = PIO_MSG_INQ_UNLIMDIMS; /* Message for async notification. */
            char nunlimdimsp_present = nunlimdimsp ? true : false;
            char unlimdimidsp_present = unlimdimidsp ? true : false;

            if (ios->compmaster == MPI_ROOT)
                mpierr = MPI_Send(&msg, 1, MPI_INT, ios->ioroot, 1, ios->union_comm);

            if (!mpierr)
                mpierr = MPI_Bcast(&ncid, 1, MPI_INT, ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast(&nunlimdimsp_present, 1, MPI_CHAR, ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast(&unlimdimidsp_present, 1, MPI_CHAR, ios->compmaster, ios->intercomm);
            LOG((2, "PIOc_inq_unlimdims ncid = %d nunlimdimsp_present = %d unlimdimidsp_present = %d",
                 ncid, nunlimdimsp_present, unlimdimidsp_present));
        }

        /* Handle MPI errors. */
        if ((mpierr2 = MPI_Bcast(&mpierr, 1, MPI_INT, ios->comproot, ios->my_comm)))
            return check_mpi(file, mpierr2, __FILE__, __LINE__);
        if (mpierr)
            return check_mpi(file, mpierr, __FILE__, __LINE__);
    }

    LOG((2, "file->iotype = %d", file->iotype));
    /* If this is an IO task, then call the netCDF function. */
    if (ios->ioproc)
    {
        if (file->iotype == PIO_IOTYPE_NETCDF && file->do_io)
        {
            LOG((2, "netcdf"));
            int tmp_unlimdimid;
            ierr = nc_inq_unlimdim(file->fh, &tmp_unlimdimid);
            LOG((2, "classic tmp_unlimdimid = %d", tmp_unlimdimid));
            tmp_nunlimdims = tmp_unlimdimid >= 0 ? 1 : 0;
            if (nunlimdimsp)
                *nunlimdimsp = tmp_unlimdimid >= 0 ? 1 : 0;
            if (unlimdimidsp)
                *unlimdimidsp = tmp_unlimdimid;
        }
#ifdef _PNETCDF
        else if (file->iotype == PIO_IOTYPE_PNETCDF)
        {
            LOG((2, "pnetcdf"));
            int tmp_unlimdimid;
            ierr = ncmpi_inq_unlimdim(file->fh, &tmp_unlimdimid);
            LOG((2, "pnetcdf tmp_unlimdimid = %d", tmp_unlimdimid));
            tmp_nunlimdims = tmp_unlimdimid >= 0 ? 1 : 0;
            if (nunlimdimsp)
                *nunlimdimsp = tmp_nunlimdims;
            if (unlimdimidsp)
                *unlimdimidsp = tmp_unlimdimid;
        }
#endif /* _PNETCDF */
#ifdef _NETCDF4
        else if ((file->iotype == PIO_IOTYPE_NETCDF4C || file->iotype == PIO_IOTYPE_NETCDF4P) &&
                 file->do_io)
        {
            LOG((2, "PIOc_inq calling netcdf-4 nc_inq_unlimdims"));
            int *tmp_unlimdimids;
            ierr = nc_inq_unlimdims(file->fh, &tmp_nunlimdims, NULL);
            if (!ierr)
            {
                if (nunlimdimsp)
                    *nunlimdimsp = tmp_nunlimdims;
                LOG((3, "tmp_nunlimdims = %d", tmp_nunlimdims));
                if (!(tmp_unlimdimids = malloc(tmp_nunlimdims * sizeof(int))))
                    ierr = PIO_ENOMEM;
                if (!ierr)
                    ierr = nc_inq_unlimdims(file->fh, &tmp_nunlimdims, tmp_unlimdimids);
                if (unlimdimidsp)
                    for (int d = 0; d < tmp_nunlimdims; d++)
                    {
                        LOG((3, "tmp_unlimdimids[%d] = %d", d, tmp_unlimdimids[d]));
                        unlimdimidsp[d] = tmp_unlimdimids[d];
                    }
                free(tmp_unlimdimids);
            }
        }
#endif /* _NETCDF4 */

        LOG((2, "PIOc_inq_unlimdims netcdf call returned %d", ierr));
    }

    /* Broadcast and check the return code. */
    if ((mpierr = MPI_Bcast(&ierr, 1, MPI_INT, ios->ioroot, ios->my_comm)))
        return check_mpi(file, mpierr, __FILE__, __LINE__);
    if (ierr)
        return check_netcdf(file, ierr, __FILE__, __LINE__);

    /* Broadcast results to all tasks. Ignore NULL parameters. */
    if ((mpierr = MPI_Bcast(&tmp_nunlimdims, 1, MPI_INT, ios->ioroot, ios->my_comm)))
        return check_mpi(file, mpierr, __FILE__, __LINE__);

    if (nunlimdimsp)
        if ((mpierr = MPI_Bcast(nunlimdimsp, 1, MPI_INT, ios->ioroot, ios->my_comm)))
            return check_mpi(file, mpierr, __FILE__, __LINE__);

    if (unlimdimidsp)
        if ((mpierr = MPI_Bcast(unlimdimidsp, tmp_nunlimdims, MPI_INT, ios->ioroot, ios->my_comm)))
            return check_mpi(file, mpierr, __FILE__, __LINE__);

    return PIO_NOERR;
}

/**
 * @ingroup PIO_typelen
 * Learn the name and size of a type.
 *
 * @param ncid the ncid of the open file.
 * @param xtype the type to learn about
 * @param name pointer that will get the name of the type.
 * @param sizep pointer that will get the size of the type in bytes.
 * @returns 0 for success, error code otherwise.
 * @author Ed Hartnett
 */
int PIOc_inq_type(int ncid, nc_type xtype, char *name, PIO_Offset *sizep)
{
    iosystem_desc_t *ios;  /* Pointer to io system information. */
    file_desc_t *file;     /* Pointer to file information. */
    int ierr;              /* Return code from function calls. */
    int mpierr = MPI_SUCCESS, mpierr2;  /* Return code from MPI function codes. */

    LOG((1, "PIOc_inq_type ncid = %d xtype = %d", ncid, xtype));

    /* Find the info about this file. */
    if ((ierr = pio_get_file(ncid, &file)))
        return pio_err(NULL, NULL, ierr, __FILE__, __LINE__);
    ios = file->iosystem;

    /* If async is in use, and this is not an IO task, bcast the parameters. */
    if (ios->async)
    {
        if (!ios->ioproc)
        {
            int msg = PIO_MSG_INQ_TYPE; /* Message for async notification. */
            char name_present = name ? true : false;
            char size_present = sizep ? true : false;

            if (ios->compmaster == MPI_ROOT)
                mpierr = MPI_Send(&msg, 1,MPI_INT, ios->ioroot, 1, ios->union_comm);

            if (!mpierr)
                mpierr = MPI_Bcast(&ncid, 1, MPI_INT, ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast(&xtype, 1, MPI_INT, ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast(&name_present, 1, MPI_CHAR, ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast(&size_present, 1, MPI_CHAR, ios->compmaster, ios->intercomm);
        }

        /* Handle MPI errors. */
        if ((mpierr2 = MPI_Bcast(&mpierr, 1, MPI_INT, ios->comproot, ios->my_comm)))
            return check_mpi(file, mpierr2, __FILE__, __LINE__);
        if (mpierr)
            return check_mpi(file, mpierr, __FILE__, __LINE__);
    }

    /* If this is an IO task, then call the netCDF function. */
    if (ios->ioproc)
    {
#ifdef _PNETCDF
        if (file->iotype == PIO_IOTYPE_PNETCDF)
            ierr = pioc_pnetcdf_inq_type(ncid, xtype, name, sizep);
#endif /* _PNETCDF */

        if (file->iotype != PIO_IOTYPE_PNETCDF && file->do_io)
            ierr = nc_inq_type(file->fh, xtype, name, (size_t *)sizep);
        LOG((2, "PIOc_inq_type netcdf call returned %d", ierr));
    }

    /* Broadcast and check the return code. */
    if ((mpierr = MPI_Bcast(&ierr, 1, MPI_INT, ios->ioroot, ios->my_comm)))
        return check_mpi(file, mpierr, __FILE__, __LINE__);
    if (ierr)
        return check_netcdf(file, ierr, __FILE__, __LINE__);

    /* Broadcast results to all tasks. Ignore NULL parameters. */
    if (name)
    {
        int slen;
        if (ios->iomaster == MPI_ROOT)
            slen = strlen(name);
        if ((mpierr = MPI_Bcast(&slen, 1, MPI_INT, ios->ioroot, ios->my_comm)))
            return check_mpi(file, mpierr, __FILE__, __LINE__);
        if (!mpierr)
            if ((mpierr = MPI_Bcast((void *)name, slen + 1, MPI_CHAR, ios->ioroot, ios->my_comm)))
                return check_mpi(file, mpierr, __FILE__, __LINE__);
    }
    if (sizep)
        if ((mpierr = MPI_Bcast(sizep , 1, MPI_OFFSET, ios->ioroot, ios->my_comm)))
            return check_mpi(file, mpierr, __FILE__, __LINE__);

    return PIO_NOERR;
}

/**
 * @ingroup PIO_inq_format
 * Learn the netCDF format of an open file.
 *
 * @param ncid the ncid of an open file.
 * @param formatp a pointer that will get the format.
 * @returns 0 for success, error code otherwise.
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_inq_format(int ncid, int *formatp)
{
    iosystem_desc_t *ios;  /* Pointer to io system information. */
    file_desc_t *file;     /* Pointer to file information. */
    int ierr;              /* Return code from function calls. */
    int mpierr = MPI_SUCCESS, mpierr2;  /* Return code from MPI function codes. */

    LOG((1, "PIOc_inq ncid = %d", ncid));

    /* Find the info about this file. */
    if ((ierr = pio_get_file(ncid, &file)))
        return pio_err(NULL, NULL, ierr, __FILE__, __LINE__);
    ios = file->iosystem;

    /* If async is in use, and this is not an IO task, bcast the parameters. */
    if (ios->async)
    {
        if (!ios->ioproc)
        {
            int msg = PIO_MSG_INQ_FORMAT;
            char format_present = formatp ? true : false;

            if (ios->compmaster == MPI_ROOT)
                mpierr = MPI_Send(&msg, 1,MPI_INT, ios->ioroot, 1, ios->union_comm);

            if (!mpierr)
                mpierr = MPI_Bcast(&ncid, 1, MPI_INT, ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast(&format_present, 1, MPI_CHAR, ios->compmaster, ios->intercomm);
        }

        /* Handle MPI errors. */
        if ((mpierr2 = MPI_Bcast(&mpierr, 1, MPI_INT, ios->comproot, ios->my_comm)))
            return check_mpi(file, mpierr2, __FILE__, __LINE__);
        if (mpierr)
            return check_mpi(file, mpierr, __FILE__, __LINE__);
    }

    /* If this is an IO task, then call the netCDF function. */
    if (ios->ioproc)
    {
#ifdef _PNETCDF
        if (file->iotype == PIO_IOTYPE_PNETCDF)
            ierr = ncmpi_inq_format(file->fh, formatp);
#endif /* _PNETCDF */

        if (file->iotype != PIO_IOTYPE_PNETCDF && file->do_io)
            ierr = nc_inq_format(file->fh, formatp);
        LOG((2, "PIOc_inq netcdf call returned %d", ierr));
    }

    /* Broadcast and check the return code. */
    if ((mpierr = MPI_Bcast(&ierr, 1, MPI_INT, ios->ioroot, ios->my_comm)))
        return check_mpi(file, mpierr, __FILE__, __LINE__);
    if (ierr)
        return check_netcdf(file, ierr, __FILE__, __LINE__);

    /* Broadcast results to all tasks. Ignore NULL parameters. */
    if (formatp)
        if ((mpierr = MPI_Bcast(formatp , 1, MPI_INT, ios->ioroot, ios->my_comm)))
            return check_mpi(file, mpierr, __FILE__, __LINE__);

    return PIO_NOERR;
}

/**
 * @ingroup PIO_inq_dim
 * The PIO-C interface for the NetCDF function nc_inq_dim.
 *
 * This routine is called collectively by all tasks in the communicator
 * ios.union_comm. For more information on the underlying NetCDF commmand
 * please read about this function in the NetCDF documentation at:
 * http://www.unidata.ucar.edu/software/netcdf/docs/group__dimensions.html
 *
 * @param ncid the ncid of the open file, obtained from
 * PIOc_openfile() or PIOc_createfile().
 * @param lenp a pointer that will get the number of values
 * @return PIO_NOERR for success, error code otherwise.  See PIOc_Set_File_Error_Handling
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_inq_dim(int ncid, int dimid, char *name, PIO_Offset *lenp)
{
    iosystem_desc_t *ios;  /* Pointer to io system information. */
    file_desc_t *file;     /* Pointer to file information. */
    int ierr;              /* Return code from function calls. */
    int mpierr = MPI_SUCCESS, mpierr2;  /* Return code from MPI function codes. */

    LOG((1, "PIOc_inq_dim ncid = %d dimid = %d", ncid, dimid));

    /* Get the file info, based on the ncid. */
    if ((ierr = pio_get_file(ncid, &file)))
        return pio_err(NULL, NULL, ierr, __FILE__, __LINE__);
    ios = file->iosystem;

    /* If async is in use, and this is not an IO task, bcast the parameters. */
    if (ios->async)
    {
        if (!ios->ioproc)
        {
            int msg = PIO_MSG_INQ_DIM;
            char name_present = name ? true : false;
            char len_present = lenp ? true : false;

            if (ios->compmaster == MPI_ROOT)
                mpierr = MPI_Send(&msg, 1,MPI_INT, ios->ioroot, 1, ios->union_comm);

            if (!mpierr)
                mpierr = MPI_Bcast(&ncid, 1, MPI_INT, ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast(&dimid, 1, MPI_INT, ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast(&name_present, 1, MPI_CHAR, ios->compmaster, ios->intercomm);
            LOG((2, "PIOc_inq netcdf Bcast name_present = %d", name_present));
            if (!mpierr)
                mpierr = MPI_Bcast(&len_present, 1, MPI_CHAR, ios->compmaster, ios->intercomm);
            LOG((2, "PIOc_inq netcdf Bcast len_present = %d", len_present));
        }

        /* Handle MPI errors. */
        if ((mpierr2 = MPI_Bcast(&mpierr, 1, MPI_INT, ios->comproot, ios->my_comm)))
            return check_mpi(file, mpierr2, __FILE__, __LINE__);
        if (mpierr)
            return check_mpi(file, mpierr, __FILE__, __LINE__);
    }

    /* If this is an IO task, then call the netCDF function. */
    if (ios->ioproc)
    {
#ifdef _PNETCDF
        if (file->iotype == PIO_IOTYPE_PNETCDF)
        {
            LOG((2, "calling ncmpi_inq_dim"));
            ierr = ncmpi_inq_dim(file->fh, dimid, name, lenp);;
        }
#endif /* _PNETCDF */

        if (file->iotype != PIO_IOTYPE_PNETCDF && file->do_io)
        {
            LOG((2, "calling nc_inq_dim"));
            ierr = nc_inq_dim(file->fh, dimid, name, (size_t *)lenp);;
        }
        LOG((2, "ierr = %d", ierr));
    }

    /* Broadcast and check the return code. */
    if ((mpierr = MPI_Bcast(&ierr, 1, MPI_INT, ios->ioroot, ios->my_comm)))
        return check_mpi(file, mpierr, __FILE__, __LINE__);
    if (ierr)
        return check_netcdf(file, ierr, __FILE__, __LINE__);

    /* Broadcast results to all tasks. Ignore NULL parameters. */
    if (name)
    {
        int slen;
        LOG((2, "bcasting results my_comm = %d", ios->my_comm));
        if (ios->iomaster == MPI_ROOT)
            slen = strlen(name);
        if ((mpierr = MPI_Bcast(&slen, 1, MPI_INT, ios->ioroot, ios->my_comm)))
            return check_mpi(file, mpierr, __FILE__, __LINE__);
        if ((mpierr = MPI_Bcast((void *)name, slen + 1, MPI_CHAR, ios->ioroot, ios->my_comm)))
            return check_mpi(file, mpierr, __FILE__, __LINE__);
    }

    if (lenp)
        if ((mpierr = MPI_Bcast(lenp , 1, MPI_OFFSET, ios->ioroot, ios->my_comm)))
            return check_mpi(file, mpierr, __FILE__, __LINE__);

    LOG((2, "done with PIOc_inq_dim"));
    return PIO_NOERR;
}

/**
 * @ingroup PIO_inq_dimname
 * Find the name of a dimension.
 *
 * @param ncid the ncid of an open file.
 * @param dimid the dimension ID.
 * @param name a pointer that gets the name of the dimension. Igorned
 * if NULL.
 * @returns 0 for success, error code otherwise.
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_inq_dimname(int ncid, int dimid, char *name)
{
    LOG((1, "PIOc_inq_dimname ncid = %d dimid = %d", ncid, dimid));
    return PIOc_inq_dim(ncid, dimid, name, NULL);
}

/**
 * @ingroup PIO_inq_dimlen
 * Find the length of a dimension.
 *
 * @param ncid the ncid of an open file.
 * @param dimid the dimension ID.
 * @param lenp a pointer that gets the length of the dimension. Igorned
 * if NULL.
 * @returns 0 for success, error code otherwise.
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_inq_dimlen(int ncid, int dimid, PIO_Offset *lenp)
{
    return PIOc_inq_dim(ncid, dimid, NULL, lenp);
}

/**
 * @ingroup PIO_inq_dimid
 * The PIO-C interface for the NetCDF function nc_inq_dimid.
 *
 * This routine is called collectively by all tasks in the communicator
 * ios.union_comm. For more information on the underlying NetCDF commmand
 * please read about this function in the NetCDF documentation at:
 * http://www.unidata.ucar.edu/software/netcdf/docs/group__dimensions.html
 *
 * @param ncid the ncid of the open file, obtained from
 * PIOc_openfile() or PIOc_createfile().
 * @param idp a pointer that will get the id of the variable or attribute.
 * @return PIO_NOERR for success, error code otherwise.  See PIOc_Set_File_Error_Handling
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_inq_dimid(int ncid, const char *name, int *idp)
{
    iosystem_desc_t *ios;
    file_desc_t *file;
    int ierr;
    int mpierr = MPI_SUCCESS, mpierr2;  /* Return code from MPI function codes. */

    /* Get the file info, based on the ncid. */
    if ((ierr = pio_get_file(ncid, &file)))
        return pio_err(NULL, NULL, ierr, __FILE__, __LINE__);
    ios = file->iosystem;
    LOG((2, "iosysid = %d", ios->iosysid));

    /* User must provide name shorter than NC_MAX_NAME +1. */
    if (!name || strlen(name) > NC_MAX_NAME)
        return pio_err(ios, file, PIO_EINVAL, __FILE__, __LINE__);

    LOG((1, "PIOc_inq_dimid ncid = %d name = %s", ncid, name));

    /* If using async, and not an IO task, then send parameters. */
    if (ios->async)
    {
        if (!ios->ioproc)
        {
            int msg = PIO_MSG_INQ_DIMID;
            char id_present = idp ? true : false;

            if (ios->compmaster == MPI_ROOT)
                mpierr = MPI_Send(&msg, 1,MPI_INT, ios->ioroot, 1, ios->union_comm);

            if (!mpierr)
                mpierr = MPI_Bcast(&ncid, 1, MPI_INT, ios->compmaster, ios->intercomm);
            int namelen = strlen(name);
            if (!mpierr)
                mpierr = MPI_Bcast(&namelen, 1, MPI_INT, ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast((void *)name, namelen + 1, MPI_CHAR, ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast(&id_present, 1, MPI_CHAR, ios->compmaster, ios->intercomm);
        }

        /* Handle MPI errors. */
        if ((mpierr2 = MPI_Bcast(&mpierr, 1, MPI_INT, ios->comproot, ios->my_comm)))
            return check_mpi(file, mpierr2, __FILE__, __LINE__);
        if (mpierr)
            return check_mpi(file, mpierr, __FILE__, __LINE__);
    }

    /* IO tasks call the netCDF functions. */
    if (ios->ioproc)
    {
#ifdef _PNETCDF
        if (file->iotype == PIO_IOTYPE_PNETCDF)
            ierr = ncmpi_inq_dimid(file->fh, name, idp);
#endif /* _PNETCDF */

        if (file->iotype != PIO_IOTYPE_PNETCDF && file->do_io)
            ierr = nc_inq_dimid(file->fh, name, idp);
    }
    LOG((3, "nc_inq_dimid call complete ierr = %d", ierr));

    /* Broadcast and check the return code. */
    if ((mpierr = MPI_Bcast(&ierr, 1, MPI_INT, ios->ioroot, ios->my_comm)))
        return check_mpi(file, mpierr, __FILE__, __LINE__);
    if (ierr)
        return check_netcdf(file, ierr, __FILE__, __LINE__);

    /* Broadcast results. */
    if (idp)
        if ((mpierr = MPI_Bcast(idp, 1, MPI_INT, ios->ioroot, ios->my_comm)))
            return check_mpi(file, mpierr, __FILE__, __LINE__);

    return PIO_NOERR;
}

/**
 * @ingroup PIO_inq_var
 * The PIO-C interface for the NetCDF function nc_inq_var.
 *
 * This routine is called collectively by all tasks in the communicator
 * ios.union_comm. For more information on the underlying NetCDF commmand
 * please read about this function in the NetCDF documentation at:
 * http://www.unidata.ucar.edu/software/netcdf/docs/group__variables.html
 *
 * @param ncid the ncid of the open file, obtained from
 * PIOc_openfile() or PIOc_createfile().
 * @param varid the variable ID.
 * @param xtypep a pointer that will get the type of the attribute.
 * @param nattsp a pointer that will get the number of attributes
 * @return PIO_NOERR for success, error code otherwise.
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_inq_var(int ncid, int varid, char *name, nc_type *xtypep, int *ndimsp,
                 int *dimidsp, int *nattsp)
{
    iosystem_desc_t *ios;
    file_desc_t *file;
    int ndims = 0;    /* The number of dimensions for this variable. */
    int ierr;
    int mpierr = MPI_SUCCESS, mpierr2;  /* Return code from MPI function codes. */

    LOG((1, "PIOc_inq_var ncid = %d varid = %d", ncid, varid));

    /* Get the file info, based on the ncid. */
    if ((ierr = pio_get_file(ncid, &file)))
        return pio_err(NULL, NULL, ierr, __FILE__, __LINE__);
    ios = file->iosystem;

    /* If async is in use, and this is not an IO task, bcast the parameters. */
    if (ios->async)
    {
        if (!ios->ioproc)
        {
            int msg = PIO_MSG_INQ_VAR;
            char name_present = name ? true : false;
            char xtype_present = xtypep ? true : false;
            char ndims_present = ndimsp ? true : false;
            char dimids_present = dimidsp ? true : false;
            char natts_present = nattsp ? true : false;

            if (ios->compmaster == MPI_ROOT)
                mpierr = MPI_Send(&msg, 1,MPI_INT, ios->ioroot, 1, ios->union_comm);

            if (!mpierr)
                mpierr = MPI_Bcast(&ncid, 1, MPI_INT, ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast(&varid, 1, MPI_INT, ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast(&name_present, 1, MPI_CHAR, ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast(&xtype_present, 1, MPI_CHAR, ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast(&ndims_present, 1, MPI_CHAR, ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast(&dimids_present, 1, MPI_CHAR, ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast(&natts_present, 1, MPI_CHAR, ios->compmaster, ios->intercomm);
            LOG((2, "PIOc_inq_var name_present = %d xtype_present = %d ndims_present = %d "
                 "dimids_present = %d, natts_present = %d nattsp = %d",
                 name_present, xtype_present, ndims_present, dimids_present, natts_present, nattsp));
        }

        /* Handle MPI errors. */
        if ((mpierr2 = MPI_Bcast(&mpierr, 1, MPI_INT, ios->comproot, ios->my_comm)))
            return check_mpi(file, mpierr2, __FILE__, __LINE__);
        if (mpierr)
            return check_mpi(file, mpierr, __FILE__, __LINE__);
    }

    /* Call the netCDF layer. */
    if (ios->ioproc)
    {
        LOG((2, "Calling the netCDF layer"));
#ifdef _PNETCDF
        if (file->iotype == PIO_IOTYPE_PNETCDF)
        {
            ierr = ncmpi_inq_varndims(file->fh, varid, &ndims);
            LOG((2, "from pnetcdf ndims = %d", ndims));
            if (!ierr)
                ierr = ncmpi_inq_var(file->fh, varid, name, xtypep, ndimsp, dimidsp, nattsp);
        }
#endif /* _PNETCDF */

        if (file->iotype != PIO_IOTYPE_PNETCDF && file->do_io)
        {
            ierr = nc_inq_varndims(file->fh, varid, &ndims);
            LOG((3, "nc_inq_varndims called ndims = %d", ndims));
            if (!ierr)
            {
                char my_name[NC_MAX_NAME + 1];
                nc_type my_xtype;
                int my_ndims = 0, my_dimids[ndims], my_natts = 0;
                ierr = nc_inq_var(file->fh, varid, my_name, &my_xtype, &my_ndims, my_dimids, &my_natts);
                LOG((3, "my_name = %s my_xtype = %d my_ndims = %d my_natts = %d",  my_name, my_xtype, my_ndims, my_natts));
                if (!ierr)
                {
                    if (name)
                        strcpy(name, my_name);
                    if (xtypep)
                        *xtypep = my_xtype;
                    if (ndimsp)
                        *ndimsp = my_ndims;
                    if (dimidsp)
                    {
                        for (int d = 0; d < ndims; d++)
                            dimidsp[d] = my_dimids[d];
                    }
                    if (nattsp)
                        *nattsp = my_natts;
                }
            }
        }
        if (ndimsp)
            LOG((2, "PIOc_inq_var ndims = %d ierr = %d", *ndimsp, ierr));
    }

    /* Broadcast and check the return code. */
    if ((mpierr = MPI_Bcast(&ierr, 1, MPI_INT, ios->ioroot, ios->my_comm)))
        return check_mpi(file, mpierr, __FILE__, __LINE__);
    if (ierr)
        return check_netcdf(file, ierr, __FILE__, __LINE__);

    /* Broadcast the results for non-null pointers. */
    if (name)
    {
        int slen;
        if (ios->iomaster == MPI_ROOT)
            slen = strlen(name);
        if ((mpierr = MPI_Bcast(&slen, 1, MPI_INT, ios->ioroot, ios->my_comm)))
            return check_mpi(file, mpierr, __FILE__, __LINE__);
        if ((mpierr = MPI_Bcast((void *)name, slen + 1, MPI_CHAR, ios->ioroot, ios->my_comm)))
            return check_mpi(file, mpierr, __FILE__, __LINE__);
    }
    if (xtypep)
        if ((mpierr = MPI_Bcast(xtypep, 1, MPI_INT, ios->ioroot, ios->my_comm)))
            return check_mpi(file, mpierr, __FILE__, __LINE__);

    if (ndimsp)
    {
        LOG((2, "PIOc_inq_var about to Bcast ndims = %d ios->ioroot = %d ios->my_comm = %d",
             *ndimsp, ios->ioroot, ios->my_comm));
        if ((mpierr = MPI_Bcast(ndimsp, 1, MPI_INT, ios->ioroot, ios->my_comm)))
            return check_mpi(file, mpierr, __FILE__, __LINE__);
        LOG((2, "PIOc_inq_var Bcast ndims = %d", *ndimsp));
    }
    if (dimidsp)
    {
        if ((mpierr = MPI_Bcast(&ndims, 1, MPI_INT, ios->ioroot, ios->my_comm)))
            return check_mpi(file, mpierr, __FILE__, __LINE__);
        if ((mpierr = MPI_Bcast(dimidsp, ndims, MPI_INT, ios->ioroot, ios->my_comm)))
            return check_mpi(file, mpierr, __FILE__, __LINE__);
    }
    if (nattsp)
        if ((mpierr = MPI_Bcast(nattsp, 1, MPI_INT, ios->ioroot, ios->my_comm)))
            return check_mpi(file, mpierr, __FILE__, __LINE__);

    return PIO_NOERR;
}

/**
 * @ingroup PIO_inq_varname
 * Get the name of a variable.
 *
 * @param ncid the ncid of the open file.
 * @param varid the variable ID.
 * @param name a pointer that will get the variable name.
 * @return PIO_NOERR for success, error code otherwise.
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_inq_varname(int ncid, int varid, char *name)
{
    return PIOc_inq_var(ncid, varid, name, NULL, NULL, NULL, NULL);
}

/**
 * @ingroup PIO_inq_vartype
 * Find the type of a variable.
 *
 * @param ncid the ncid of the open file.
 * @param varid the variable ID.
 * @param xtypep a pointer that will get the type of the
 * attribute. Ignored if NULL.
 * @return PIO_NOERR for success, error code otherwise.
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_inq_vartype(int ncid, int varid, nc_type *xtypep)
{
    return PIOc_inq_var(ncid, varid, NULL, xtypep, NULL, NULL, NULL);
}

/**
 * @ingroup PIO_inq_varndims
 * Find the number of dimensions of a variable.
 *
 * @param ncid the ncid of the open file.
 * @param varid the variable ID.
 * @param ndimsp a pointer that will get the number of
 * dimensions. Ignored if NULL.
 * @return PIO_NOERR for success, error code otherwise.
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_inq_varndims(int ncid, int varid, int *ndimsp)
{
    return PIOc_inq_var(ncid, varid, NULL, NULL, ndimsp, NULL, NULL);
}

/**
 * @ingroup PIO_inq_vardimid
 * Find the dimension IDs associated with a variable.
 *
 * @param ncid the ncid of the open file.
 * @param varid the variable ID.
 * @param dimidsp a pointer that will get an array of dimids. Ignored
 * if NULL.
 * @return PIO_NOERR for success, error code otherwise.
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_inq_vardimid(int ncid, int varid, int *dimidsp)
{
    return PIOc_inq_var(ncid, varid, NULL, NULL, NULL, dimidsp, NULL);
}

/**
 * @ingroup PIO_inq_varnatts
 * Find the number of attributes associated with a variable.
 *
 * @param ncid the ncid of the open file.
 * @param varid the variable ID.
 * @param nattsp a pointer that will get the number of attriburtes. Ignored
 * if NULL.
 * @return PIO_NOERR for success, error code otherwise.
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_inq_varnatts(int ncid, int varid, int *nattsp)
{
    return PIOc_inq_var(ncid, varid, NULL, NULL, NULL, NULL, nattsp);
}

/**
 * @ingroup PIO_inq_varid
 * The PIO-C interface for the NetCDF function nc_inq_varid.
 *
 * This routine is called collectively by all tasks in the communicator
 * ios.union_comm. For more information on the underlying NetCDF commmand
 * please read about this function in the NetCDF documentation at:
 * http://www.unidata.ucar.edu/software/netcdf/docs/group__variables.html
 *
 * @param ncid the ncid of the open file, obtained from
 * PIOc_openfile() or PIOc_createfile().
 * @param varid the variable ID.
 * @param varidp a pointer that will get the variable id
 * @return PIO_NOERR for success, error code otherwise.  See PIOc_Set_File_Error_Handling
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_inq_varid(int ncid, const char *name, int *varidp)
{
    iosystem_desc_t *ios;  /* Pointer to io system information. */
    file_desc_t *file;     /* Pointer to file information. */
    int ierr;              /* Return code from function calls. */
    int mpierr = MPI_SUCCESS, mpierr2;  /* Return code from MPI function codes. */

    /* Get file info based on ncid. */
    if ((ierr = pio_get_file(ncid, &file)))
        return pio_err(NULL, NULL, ierr, __FILE__, __LINE__);
    ios = file->iosystem;

    /* Caller must provide name. */
    if (!name || strlen(name) > NC_MAX_NAME)
        return pio_err(ios, file, PIO_EINVAL, __FILE__, __LINE__);

    LOG((1, "PIOc_inq_varid ncid = %d name = %s", ncid, name));

    if (ios->async)
    {
        if (!ios->ioproc)
        {
            int msg = PIO_MSG_INQ_VARID;

            if (ios->compmaster == MPI_ROOT)
                mpierr = MPI_Send(&msg, 1,MPI_INT, ios->ioroot, 1, ios->union_comm);

            if (!mpierr)
                mpierr = MPI_Bcast(&ncid, 1, MPI_INT, ios->compmaster, ios->intercomm);
            int namelen;
            namelen = strlen(name);
            if (!mpierr)
                mpierr = MPI_Bcast(&namelen, 1, MPI_INT, ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast((void *)name, namelen + 1, MPI_CHAR, ios->compmaster, ios->intercomm);
        }

        /* Handle MPI errors. */
        if ((mpierr2 = MPI_Bcast(&mpierr, 1, MPI_INT, ios->comproot, ios->my_comm)))
            check_mpi(file, mpierr2, __FILE__, __LINE__);
        if (mpierr)
            return check_mpi(file, mpierr, __FILE__, __LINE__);
    }

    /* If this is an IO task, then call the netCDF function. */
    if (ios->ioproc)
    {
#ifdef _PNETCDF
        if (file->iotype == PIO_IOTYPE_PNETCDF)
            ierr = ncmpi_inq_varid(file->fh, name, varidp);
#endif /* _PNETCDF */

        if (file->iotype != PIO_IOTYPE_PNETCDF && file->do_io)
            ierr = nc_inq_varid(file->fh, name, varidp);
    }

    /* Broadcast and check the return code. */
    if ((mpierr = MPI_Bcast(&ierr, 1, MPI_INT, ios->ioroot, ios->my_comm)))
        return check_mpi(file, mpierr, __FILE__, __LINE__);
    if (ierr)
        return check_netcdf(file, ierr, __FILE__, __LINE__);

    /* Broadcast results to all tasks. Ignore NULL parameters. */
    if (varidp)
        if ((mpierr = MPI_Bcast(varidp, 1, MPI_INT, ios->ioroot, ios->my_comm)))
            check_mpi(file, mpierr, __FILE__, __LINE__);

    return PIO_NOERR;
}

/**
 * @ingroup PIO_inq_att
 * The PIO-C interface for the NetCDF function nc_inq_att.
 *
 * This routine is called collectively by all tasks in the communicator
 * ios.union_comm. For more information on the underlying NetCDF commmand
 * please read about this function in the NetCDF documentation at:
 * http://www.unidata.ucar.edu/software/netcdf/docs/group__attributes.html
 *
 * @param ncid the ncid of the open file, obtained from
 * PIOc_openfile() or PIOc_createfile().
 * @param varid the variable ID.
 * @param xtypep a pointer that will get the type of the attribute.
 * @param lenp a pointer that will get the number of values
 * @return PIO_NOERR for success, error code otherwise.
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_inq_att(int ncid, int varid, const char *name, nc_type *xtypep,
                 PIO_Offset *lenp)
{
    int msg = PIO_MSG_INQ_ATT;
    iosystem_desc_t *ios;
    file_desc_t *file;
    int mpierr = MPI_SUCCESS, mpierr2;  /* Return code from MPI function codes. */
    int ierr;

    /* Find file based on ncid. */
    if ((ierr = pio_get_file(ncid, &file)))
        return pio_err(NULL, NULL, ierr, __FILE__, __LINE__);
    ios = file->iosystem;

    /* User must provide name shorter than NC_MAX_NAME +1. */
    if (!name || strlen(name) > NC_MAX_NAME)
        return pio_err(ios, file, PIO_EINVAL, __FILE__, __LINE__);

    LOG((1, "PIOc_inq_att ncid = %d varid = %d", ncid, varid));

    /* If async is in use, and this is not an IO task, bcast the parameters. */
    if (ios->async)
    {
        if (!ios->ioproc)
        {
            char xtype_present = xtypep ? true : false;
            char len_present = lenp ? true : false;
            int namelen = strlen(name);

            if (ios->compmaster == MPI_ROOT)
                mpierr = MPI_Send(&msg, 1,MPI_INT, ios->ioroot, 1, ios->union_comm);

            if (!mpierr)
                mpierr = MPI_Bcast(&ncid, 1, MPI_INT, ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast(&varid, 1, MPI_INT, ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast(&namelen, 1, MPI_INT,  ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast((void *)name, namelen + 1, MPI_CHAR, ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast(&xtype_present, 1, MPI_CHAR, ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast(&len_present, 1, MPI_CHAR, ios->compmaster, ios->intercomm);
        }

        /* Handle MPI errors. */
        if ((mpierr2 = MPI_Bcast(&mpierr, 1, MPI_INT, ios->comproot, ios->my_comm)))
            check_mpi(file, mpierr2, __FILE__, __LINE__);
        if (mpierr)
            return check_mpi(file, mpierr, __FILE__, __LINE__);
    }

    /* If this is an IO task, then call the netCDF function. */
    if (ios->ioproc)
    {
#ifdef _PNETCDF
        if (file->iotype == PIO_IOTYPE_PNETCDF)
            ierr = ncmpi_inq_att(file->fh, varid, name, xtypep, lenp);
#endif /* _PNETCDF */

        if (file->iotype != PIO_IOTYPE_PNETCDF && file->do_io)
            ierr = nc_inq_att(file->fh, varid, name, xtypep, (size_t *)lenp);
        LOG((2, "PIOc_inq netcdf call returned %d", ierr));
    }

    /* Broadcast and check the return code. */
    if ((mpierr = MPI_Bcast(&ierr, 1, MPI_INT, ios->ioroot, ios->my_comm)))
        return check_mpi(file, mpierr, __FILE__, __LINE__);
    if (ierr)
        return check_netcdf(file, ierr, __FILE__, __LINE__);

    /* Broadcast results. */
    if (xtypep)
        if ((mpierr = MPI_Bcast(xtypep, 1, MPI_INT, ios->ioroot, ios->my_comm)))
            check_mpi(file, mpierr, __FILE__, __LINE__);
    if (lenp)
        if ((mpierr = MPI_Bcast(lenp, 1, MPI_OFFSET, ios->ioroot, ios->my_comm)))
            check_mpi(file, mpierr, __FILE__, __LINE__);

    return PIO_NOERR;
}

/**
 * @ingroup PIO_inq_attlen
 * Get the length of an attribute.
 *
 * @param ncid the ID of an open file.
 * @param varid the variable ID, or NC_GLOBAL for global attributes.
 * @param name the name of the attribute.
 * @param lenp a pointer that gets the lenght of the attribute
 * array. Ignored if NULL.
 * @return PIO_NOERR for success, error code otherwise.
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_inq_attlen(int ncid, int varid, const char *name, PIO_Offset *lenp)
{
    return PIOc_inq_att(ncid, varid, name, NULL, lenp);
}

/**
 * @ingroup PIO_inq_atttype
 * Get the type of an attribute.
 *
 * @param ncid the ID of an open file.
 * @param varid the variable ID, or NC_GLOBAL for global attributes.
 * @param name the name of the attribute.
 * @param xtypep a pointer that gets the type of the
 * attribute. Ignored if NULL.
 * @return PIO_NOERR for success, error code otherwise.
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_inq_atttype(int ncid, int varid, const char *name, nc_type *xtypep)
{
    return PIOc_inq_att(ncid, varid, name, xtypep, NULL);
}

/**
 * @ingroup PIO_inq_attname
 * The PIO-C interface for the NetCDF function nc_inq_attname.
 *
 * This routine is called collectively by all tasks in the communicator
 * ios.union_comm. For more information on the underlying NetCDF commmand
 * please read about this function in the NetCDF documentation at:
 * http://www.unidata.ucar.edu/software/netcdf/docs/group__attributes.html
 *
 * @param ncid the ncid of the open file, obtained from
 * PIOc_openfile() or PIOc_createfile().
 * @param varid the variable ID.
 * @param attnum the attribute ID.
 * @return PIO_NOERR for success, error code otherwise.  See PIOc_Set_File_Error_Handling
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_inq_attname(int ncid, int varid, int attnum, char *name)
{
    iosystem_desc_t *ios;  /* Pointer to io system information. */
    file_desc_t *file;     /* Pointer to file information. */
    int ierr;              /* Return code from function calls. */
    int mpierr = MPI_SUCCESS, mpierr2;  /* Return code from MPI function codes. */

    LOG((1, "PIOc_inq_attname ncid = %d varid = %d attnum = %d", ncid, varid,
         attnum));

    /* Find the info about this file. */
    if ((ierr = pio_get_file(ncid, &file)))
        return pio_err(NULL, NULL, ierr, __FILE__, __LINE__);
    ios = file->iosystem;

    /* If async is in use, and this is not an IO task, bcast the parameters. */
    if (ios->async)
    {
        if (!ios->ioproc)
        {
            int msg = PIO_MSG_INQ_ATTNAME;
            char name_present = name ? true : false;

            if (ios->compmaster == MPI_ROOT)
                mpierr = MPI_Send(&msg, 1,MPI_INT, ios->ioroot, 1, ios->union_comm);

            if (!mpierr)
                mpierr = MPI_Bcast(&ncid, 1, MPI_INT, ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast(&varid, 1, MPI_INT, ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast(&attnum, 1, MPI_INT, ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast(&name_present, 1, MPI_CHAR, ios->compmaster, ios->intercomm);
        }

        /* Handle MPI errors. */
        if ((mpierr2 = MPI_Bcast(&mpierr, 1, MPI_INT, ios->comproot, ios->my_comm)))
            check_mpi(file, mpierr2, __FILE__, __LINE__);
        if (mpierr)
            return check_mpi(file, mpierr, __FILE__, __LINE__);
    }

    /* If this is an IO task, then call the netCDF function. */
    if (ios->ioproc)
    {
#ifdef _PNETCDF
        if (file->iotype == PIO_IOTYPE_PNETCDF)
            ierr = ncmpi_inq_attname(file->fh, varid, attnum, name);
#endif /* _PNETCDF */

        if (file->iotype != PIO_IOTYPE_PNETCDF && file->do_io)
            ierr = nc_inq_attname(file->fh, varid, attnum, name);
        LOG((2, "PIOc_inq_attname netcdf call returned %d", ierr));
    }

    /* Broadcast and check the return code. */
    if ((mpierr = MPI_Bcast(&ierr, 1, MPI_INT, ios->ioroot, ios->my_comm)))
        return check_mpi(file, mpierr, __FILE__, __LINE__);
    if (ierr)
        return check_netcdf(file, ierr, __FILE__, __LINE__);

    /* Broadcast results to all tasks. Ignore NULL parameters. */
    if (name)
    {
        int namelen = strlen(name);
        if ((mpierr = MPI_Bcast(&namelen, 1, MPI_INT, ios->ioroot, ios->my_comm)))
            check_mpi(file, mpierr, __FILE__, __LINE__);
        /* Casting to void to avoid warnings on some compilers. */
        if ((mpierr = MPI_Bcast((void *)name, namelen + 1, MPI_CHAR, ios->ioroot, ios->my_comm)))
            check_mpi(file, mpierr, __FILE__, __LINE__);
    }

    return PIO_NOERR;
}

/**
 * @ingroup PIO_inq_attid
 * The PIO-C interface for the NetCDF function nc_inq_attid.
 *
 * This routine is called collectively by all tasks in the communicator
 * ios.union_comm. For more information on the underlying NetCDF commmand
 * please read about this function in the NetCDF documentation at:
 * http://www.unidata.ucar.edu/software/netcdf/docs/group__attributes.html
 *
 * @param ncid the ncid of the open file, obtained from
 * PIOc_openfile() or PIOc_createfile().
 * @param varid the variable ID.
 * @param idp a pointer that will get the id of the variable or attribute.
 * @return PIO_NOERR for success, error code otherwise.  See PIOc_Set_File_Error_Handling
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_inq_attid(int ncid, int varid, const char *name, int *idp)
{
    iosystem_desc_t *ios;  /* Pointer to io system information. */
    file_desc_t *file;     /* Pointer to file information. */
    int ierr;              /* Return code from function calls. */
    int mpierr = MPI_SUCCESS, mpierr2;  /* Return code from MPI function codes. */

    /* Find the info about this file. */
    if ((ierr = pio_get_file(ncid, &file)))
        return pio_err(NULL, NULL, ierr, __FILE__, __LINE__);
    ios = file->iosystem;

    /* User must provide name shorter than NC_MAX_NAME +1. */
    if (!name || strlen(name) > NC_MAX_NAME)
        return pio_err(ios, file, PIO_EINVAL, __FILE__, __LINE__);

    LOG((1, "PIOc_inq_attid ncid = %d varid = %d name = %s", ncid, varid, name));

    /* If async is in use, and this is not an IO task, bcast the parameters. */
    if (ios->async)
    {
        if (!ios->ioproc)
        {
            int msg = PIO_MSG_INQ_ATTID;
            int namelen = strlen(name);
            char id_present = idp ? true : false;

            if (ios->compmaster == MPI_ROOT)
                mpierr = MPI_Send(&msg, 1,MPI_INT, ios->ioroot, 1, ios->union_comm);

            if (!mpierr)
                mpierr = MPI_Bcast(&ncid, 1, MPI_INT, ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast(&varid, 1, MPI_INT, ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast(&namelen, 1, MPI_INT, ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast((char *)name, namelen + 1, MPI_CHAR, ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast(&id_present, 1, MPI_CHAR, ios->compmaster, ios->intercomm);
        }

        /* Handle MPI errors. */
        if ((mpierr2 = MPI_Bcast(&mpierr, 1, MPI_INT, ios->comproot, ios->my_comm)))
            check_mpi(file, mpierr2, __FILE__, __LINE__);
        if (mpierr)
            return check_mpi(file, mpierr, __FILE__, __LINE__);
    }

    /* If this is an IO task, then call the netCDF function. */
    if (ios->ioproc)
    {
#ifdef _PNETCDF
        if (file->iotype == PIO_IOTYPE_PNETCDF)
            ierr = ncmpi_inq_attid(file->fh, varid, name, idp);
#endif /* _PNETCDF */

        if (file->iotype != PIO_IOTYPE_PNETCDF && file->do_io)
            ierr = nc_inq_attid(file->fh, varid, name, idp);
        LOG((2, "PIOc_inq_attname netcdf call returned %d", ierr));
    }

    /* Broadcast and check the return code. */
    if ((mpierr = MPI_Bcast(&ierr, 1, MPI_INT, ios->ioroot, ios->my_comm)))
        return check_mpi(file, mpierr, __FILE__, __LINE__);
    if (ierr)
        return check_netcdf(file, ierr, __FILE__, __LINE__);

    /* Broadcast results. */
    if (idp)
        if ((mpierr = MPI_Bcast(idp, 1, MPI_INT, ios->ioroot, ios->my_comm)))
            check_mpi(file, mpierr, __FILE__, __LINE__);

    return PIO_NOERR;
}

/**
 * @ingroup PIO_rename_dim
 * The PIO-C interface for the NetCDF function nc_rename_dim.
 *
 * This routine is called collectively by all tasks in the communicator
 * ios.union_comm. For more information on the underlying NetCDF commmand
 * please read about this function in the NetCDF documentation at:
 * http://www.unidata.ucar.edu/software/netcdf/docs/group__dimensions.html
 *
 * @param ncid the ncid of the open file, obtained from
 * PIOc_openfile() or PIOc_createfile().
 * @return PIO_NOERR for success, error code otherwise.  See PIOc_Set_File_Error_Handling
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_rename_dim(int ncid, int dimid, const char *name)
{
    iosystem_desc_t *ios;  /* Pointer to io system information. */
    file_desc_t *file;     /* Pointer to file information. */
    int ierr;              /* Return code from function calls. */
    int mpierr = MPI_SUCCESS, mpierr2;  /* Return code from MPI function codes. */

    /* Find the info about this file. */
    if ((ierr = pio_get_file(ncid, &file)))
        return pio_err(NULL, NULL, ierr, __FILE__, __LINE__);
    ios = file->iosystem;

    /* User must provide name shorter than NC_MAX_NAME +1. */
    if (!name || strlen(name) > NC_MAX_NAME)
        return pio_err(ios, file, PIO_EINVAL, __FILE__, __LINE__);

    LOG((1, "PIOc_rename_dim ncid = %d dimid = %d name = %s", ncid, dimid, name));

    /* If async is in use, and this is not an IO task, bcast the parameters. */
    if (ios->async)
    {
        if (!ios->ioproc)
        {
            int msg = PIO_MSG_RENAME_DIM; /* Message for async notification. */
            int namelen = strlen(name);

            if (ios->compmaster == MPI_ROOT)
                mpierr = MPI_Send(&msg, 1,MPI_INT, ios->ioroot, 1, ios->union_comm);

            if (!mpierr)
                mpierr = MPI_Bcast(&ncid, 1, MPI_INT, ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast(&dimid, 1, MPI_INT, ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast(&namelen, 1, MPI_INT,  ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast((void *)name, namelen + 1, MPI_CHAR, ios->compmaster, ios->intercomm);
            LOG((2, "PIOc_rename_dim Bcast file->fh = %d dimid = %d namelen = %d name = %s",
                 file->fh, dimid, namelen, name));
        }

        /* Handle MPI errors. */
        if ((mpierr2 = MPI_Bcast(&mpierr, 1, MPI_INT, ios->comproot, ios->my_comm)))
            check_mpi(file, mpierr2, __FILE__, __LINE__);
        if (mpierr)
            return check_mpi(file, mpierr, __FILE__, __LINE__);
    }


    /* If this is an IO task, then call the netCDF function. */
    if (ios->ioproc)
    {
#ifdef _PNETCDF
        if (file->iotype == PIO_IOTYPE_PNETCDF)
            ierr = ncmpi_rename_dim(file->fh, dimid, name);
#endif /* _PNETCDF */

        if (file->iotype != PIO_IOTYPE_PNETCDF && file->do_io)
            ierr = nc_rename_dim(file->fh, dimid, name);
        LOG((2, "PIOc_inq netcdf call returned %d", ierr));
    }

    /* Broadcast and check the return code. */
    if ((mpierr = MPI_Bcast(&ierr, 1, MPI_INT, ios->ioroot, ios->my_comm)))
        return check_mpi(file, mpierr, __FILE__, __LINE__);
    if (ierr)
        return check_netcdf(file, ierr, __FILE__, __LINE__);

    return PIO_NOERR;
}

/**
 * @ingroup PIO_rename_var
 * The PIO-C interface for the NetCDF function nc_rename_var.
 *
 * This routine is called collectively by all tasks in the communicator
 * ios.union_comm. For more information on the underlying NetCDF commmand
 * please read about this function in the NetCDF documentation at:
 * http://www.unidata.ucar.edu/software/netcdf/docs/group__variables.html
 *
 * @param ncid the ncid of the open file, obtained from
 * PIOc_openfile() or PIOc_createfile().
 * @param varid the variable ID.
 * @return PIO_NOERR for success, error code otherwise.  See PIOc_Set_File_Error_Handling
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_rename_var(int ncid, int varid, const char *name)
{
    iosystem_desc_t *ios;  /* Pointer to io system information. */
    file_desc_t *file;     /* Pointer to file information. */
    int ierr;              /* Return code from function calls. */
    int mpierr = MPI_SUCCESS, mpierr2;  /* Return code from MPI function codes. */

    /* Find the info about this file. */
    if ((ierr = pio_get_file(ncid, &file)))
        return pio_err(NULL, NULL, ierr, __FILE__, __LINE__);
    ios = file->iosystem;

    /* User must provide name shorter than NC_MAX_NAME +1. */
    if (!name || strlen(name) > NC_MAX_NAME)
        return pio_err(ios, file, PIO_EINVAL, __FILE__, __LINE__);

    LOG((1, "PIOc_rename_var ncid = %d varid = %d name = %s", ncid, varid, name));

    /* If async is in use, and this is not an IO task, bcast the parameters. */
    if (ios->async)
    {
        if (!ios->ioproc)
        {
            int msg = PIO_MSG_RENAME_VAR; /* Message for async notification. */
            int namelen = strlen(name);

            if (ios->compmaster == MPI_ROOT)
                mpierr = MPI_Send(&msg, 1,MPI_INT, ios->ioroot, 1, ios->union_comm);

            if (!mpierr)
                mpierr = MPI_Bcast(&ncid, 1, MPI_INT, ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast(&varid, 1, MPI_INT, ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast(&namelen, 1, MPI_INT,  ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast((void *)name, namelen + 1, MPI_CHAR, ios->compmaster, ios->intercomm);
            LOG((2, "PIOc_rename_var Bcast file->fh = %d varid = %d namelen = %d name = %s",
                 file->fh, varid, namelen, name));
        }

        /* Handle MPI errors. */
        if ((mpierr2 = MPI_Bcast(&mpierr, 1, MPI_INT, ios->comproot, ios->my_comm)))
            check_mpi(file, mpierr2, __FILE__, __LINE__);
        if (mpierr)
            return check_mpi(file, mpierr, __FILE__, __LINE__);
    }


    /* If this is an IO task, then call the netCDF function. */
    if (ios->ioproc)
    {
#ifdef _PNETCDF
        if (file->iotype == PIO_IOTYPE_PNETCDF)
            ierr = ncmpi_rename_var(file->fh, varid, name);
#endif /* _PNETCDF */

        if (file->iotype != PIO_IOTYPE_PNETCDF && file->do_io)
            ierr = nc_rename_var(file->fh, varid, name);
        LOG((2, "PIOc_inq netcdf call returned %d", ierr));
    }

    /* Broadcast and check the return code. */
    if ((mpierr = MPI_Bcast(&ierr, 1, MPI_INT, ios->ioroot, ios->my_comm)))
        return check_mpi(file, mpierr, __FILE__, __LINE__);
    if (ierr)
        return check_netcdf(file, ierr, __FILE__, __LINE__);

    return PIO_NOERR;
}

/**
 * @ingroup PIO_rename_att
 * The PIO-C interface for the NetCDF function nc_rename_att.
 *
 * This routine is called collectively by all tasks in the communicator
 * ios.union_comm. For more information on the underlying NetCDF commmand
 * please read about this function in the NetCDF documentation at:
 * http://www.unidata.ucar.edu/software/netcdf/docs/group__attributes.html
 *
 * @param ncid the ncid of the open file, obtained from
 * PIOc_openfile() or PIOc_createfile().
 * @param varid the variable ID.
 * @return PIO_NOERR for success, error code otherwise.  See
 * PIOc_Set_File_Error_Handling
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_rename_att(int ncid, int varid, const char *name,
                    const char *newname)
{
    iosystem_desc_t *ios;  /* Pointer to io system information. */
    file_desc_t *file;     /* Pointer to file information. */
    int ierr;              /* Return code from function calls. */
    int mpierr = MPI_SUCCESS, mpierr2;  /* Return code from MPI functions. */

    /* Find the info about this file. */
    if ((ierr = pio_get_file(ncid, &file)))
        return pio_err(NULL, NULL, ierr, __FILE__, __LINE__);
    ios = file->iosystem;

    /* User must provide names of correct length. */
    if (!name || strlen(name) > NC_MAX_NAME ||
        !newname || strlen(newname) > NC_MAX_NAME)
        return pio_err(ios, file, PIO_EINVAL, __FILE__, __LINE__);

    LOG((1, "PIOc_rename_att ncid = %d varid = %d name = %s newname = %s",
         ncid, varid, name, newname));

    /* If async is in use, and this is not an IO task, bcast the parameters. */
    if (ios->async)
    {
        if (!ios->ioproc)
        {
            int msg = PIO_MSG_RENAME_ATT; /* Message for async notification. */
            int namelen = strlen(name);
            int newnamelen = strlen(newname);

            if (ios->compmaster == MPI_ROOT)
                mpierr = MPI_Send(&msg, 1, MPI_INT, ios->ioroot, 1, ios->union_comm);

            if (!mpierr)
                mpierr = MPI_Bcast(&ncid, 1, MPI_INT, ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast(&varid, 1, MPI_INT, ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast(&namelen, 1, MPI_INT,  ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast((char *)name, namelen + 1, MPI_CHAR, ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast(&newnamelen, 1, MPI_INT,  ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast((char *)newname, newnamelen + 1, MPI_CHAR, ios->compmaster, ios->intercomm);
        }

        /* Handle MPI errors. */
        if ((mpierr2 = MPI_Bcast(&mpierr, 1, MPI_INT, ios->comproot, ios->my_comm)))
            check_mpi(file, mpierr2, __FILE__, __LINE__);
        if (mpierr)
            return check_mpi(file, mpierr, __FILE__, __LINE__);
    }

    /* If this is an IO task, then call the netCDF function. */
    if (ios->ioproc)
    {
#ifdef _PNETCDF
        if (file->iotype == PIO_IOTYPE_PNETCDF)
            ierr = ncmpi_rename_att(file->fh, varid, name, newname);
#endif /* _PNETCDF */

        if (file->iotype != PIO_IOTYPE_PNETCDF && file->do_io)
            ierr = nc_rename_att(file->fh, varid, name, newname);
    }

    /* Broadcast and check the return code. */
    if ((mpierr = MPI_Bcast(&ierr, 1, MPI_INT, ios->ioroot, ios->my_comm)))
        return check_mpi(file, mpierr, __FILE__, __LINE__);
    if (ierr)
        return check_netcdf(file, ierr, __FILE__, __LINE__);

    LOG((2, "PIOc_rename_att succeeded"));
    return PIO_NOERR;
}

/**
 * @ingroup PIO_del_att
 * The PIO-C interface for the NetCDF function nc_del_att.
 *
 * This routine is called collectively by all tasks in the communicator
 * ios.union_comm. For more information on the underlying NetCDF commmand
 * please read about this function in the NetCDF documentation at:
 * http://www.unidata.ucar.edu/software/netcdf/docs/group__attributes.html
 *
 * @param ncid the ncid of the open file, obtained from
 * PIOc_openfile() or PIOc_createfile().
 * @param varid the variable ID.
 * @param name of the attribute to delete.
 * @return PIO_NOERR for success, error code otherwise.
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_del_att(int ncid, int varid, const char *name)
{
    iosystem_desc_t *ios;  /* Pointer to io system information. */
    file_desc_t *file;     /* Pointer to file information. */
    int ierr;              /* Return code from function calls. */
    int mpierr = MPI_SUCCESS, mpierr2;  /* Return code from MPI functions. */

    /* Find the info about this file. */
    if ((ierr = pio_get_file(ncid, &file)))
        return pio_err(NULL, NULL, ierr, __FILE__, __LINE__);
    ios = file->iosystem;

    /* User must provide name shorter than NC_MAX_NAME +1. */
    if (!name || strlen(name) > NC_MAX_NAME)
        return pio_err(ios, file, PIO_EINVAL, __FILE__, __LINE__);

    LOG((1, "PIOc_del_att ncid = %d varid = %d name = %s", ncid, varid, name));

    /* If async is in use, and this is not an IO task, bcast the parameters. */
    if (ios->async)
    {
        if (!ios->ioproc)
        {
            int msg = PIO_MSG_DEL_ATT;
            int namelen = strlen(name); /* Length of name string. */

            if (ios->compmaster == MPI_ROOT)
                mpierr = MPI_Send(&msg, 1,MPI_INT, ios->ioroot, 1, ios->union_comm);

            if (!mpierr)
                mpierr = MPI_Bcast(&ncid, 1, MPI_INT, ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast(&varid, 1, MPI_INT, ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast(&namelen, 1, MPI_INT,  ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast((char *)name, namelen + 1, MPI_CHAR, ios->compmaster, ios->intercomm);
        }

        /* Handle MPI errors. */
        if ((mpierr2 = MPI_Bcast(&mpierr, 1, MPI_INT, ios->comproot, ios->my_comm)))
            check_mpi(file, mpierr2, __FILE__, __LINE__);
        if (mpierr)
            return check_mpi(file, mpierr, __FILE__, __LINE__);
    }

    /* If this is an IO task, then call the netCDF function. */
    if (ios->ioproc)
    {
#ifdef _PNETCDF
        if (file->iotype == PIO_IOTYPE_PNETCDF)
            ierr = ncmpi_del_att(file->fh, varid, name);
#endif /* _PNETCDF */

        if (file->iotype != PIO_IOTYPE_PNETCDF && file->do_io)
            ierr = nc_del_att(file->fh, varid, name);
    }

    /* Broadcast and check the return code. */
    if ((mpierr = MPI_Bcast(&ierr, 1, MPI_INT, ios->ioroot, ios->my_comm)))
        return check_mpi(file, mpierr, __FILE__, __LINE__);
    if (ierr)
        return check_netcdf(file, ierr, __FILE__, __LINE__);

    return PIO_NOERR;
}

/**
 * The PIO-C interface for the NetCDF function nc_set_fill.
 *
 * This routine is called collectively by all tasks in the communicator
 * ios.union_comm. For more information on the underlying NetCDF commmand
 * please read about this function in the NetCDF documentation at:
 * http://www.unidata.ucar.edu/software/netcdf/docs/group__datasets.html
 *
 * @param ncid the ncid of the open file, obtained from
 * PIOc_openfile() or PIOc_createfile().
 * @param fillmode either NC_FILL or NC_NOFILL.
 * @param old_modep a pointer to an int that gets the old setting.
 * @return PIO_NOERR for success, error code otherwise.
 * @ingroup PIO_set_fill
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_set_fill(int ncid, int fillmode, int *old_modep)
{
    iosystem_desc_t *ios;  /* Pointer to io system information. */
    file_desc_t *file;     /* Pointer to file information. */
    int ierr;              /* Return code from function calls. */
    int mpierr = MPI_SUCCESS, mpierr2;  /* Return code from MPI functions. */

    LOG((1, "PIOc_set_fill ncid = %d fillmode = %d", ncid, fillmode));

    /* Find the info about this file. */
    if ((ierr = pio_get_file(ncid, &file)))
        return pio_err(NULL, NULL, ierr, __FILE__, __LINE__);
    ios = file->iosystem;

    /* If async is in use, and this is not an IO task, bcast the parameters. */
    if (ios->async)
    {
        if (!ios->ioproc)
        {
            int msg = PIO_MSG_SET_FILL;
            int old_modep_present = old_modep ? 1 : 0;

            LOG((3, "PIOc_set_fill about to send msg %d", msg));
            if (ios->compmaster == MPI_ROOT)
                mpierr = MPI_Send(&msg, 1,MPI_INT, ios->ioroot, 1, ios->union_comm);

            if (!mpierr)
                mpierr = MPI_Bcast(&ncid, 1, MPI_INT, ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast(&fillmode, 1, MPI_INT, ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast(&old_modep_present, 1, MPI_INT, ios->compmaster, ios->intercomm);
            LOG((2, "PIOc_set_fill sent ncid = %d fillmode = %d old_modep_present = %d", ncid, fillmode,
                 old_modep_present));
        }

        /* Handle MPI errors. */
        if ((mpierr2 = MPI_Bcast(&mpierr, 1, MPI_INT, ios->comproot, ios->my_comm)))
            check_mpi(file, mpierr2, __FILE__, __LINE__);
        if (mpierr)
            return check_mpi(file, mpierr, __FILE__, __LINE__);
    }

    /* If this is an IO task, then call the netCDF function. */
    if (ios->ioproc)
    {
#ifdef _PNETCDF
        if (file->iotype == PIO_IOTYPE_PNETCDF)
        {
            LOG((3, "about to call ncmpi_set_fill() fillmode = %d", fillmode));
            ierr = ncmpi_set_fill(file->fh, fillmode, old_modep);
        }
#endif /* _PNETCDF */

        if (file->iotype != PIO_IOTYPE_PNETCDF && file->do_io)
            ierr = nc_set_fill(file->fh, fillmode, old_modep);
    }

    /* Broadcast and check the return code. */
    if ((mpierr = MPI_Bcast(&ierr, 1, MPI_INT, ios->ioroot, ios->my_comm)))
        return check_mpi(file, mpierr, __FILE__, __LINE__);
    if (ierr)
        return check_netcdf(file, ierr, __FILE__, __LINE__);

    /* Broadcast results. */
    if (old_modep)
    {
        LOG((2, "old_mode = %d", *old_modep));
        if ((mpierr = MPI_Bcast(old_modep, 1, MPI_INT, ios->ioroot, ios->my_comm)))
            check_mpi(file, mpierr, __FILE__, __LINE__);
    }

    LOG((2, "PIOc_set_fill succeeded"));
    return PIO_NOERR;
}

/**
 * @ingroup PIO_enddef
 * The PIO-C interface for the NetCDF function nc_enddef.
 *
 * This routine is called collectively by all tasks in the communicator
 * ios.union_comm. For more information on the underlying NetCDF commmand
 * please read about this function in the NetCDF documentation at:
 * http://www.unidata.ucar.edu/software/netcdf/docs/group__datasets.html
 *
 * @param ncid the ncid of the open file, obtained from
 * PIOc_openfile() or PIOc_createfile().
 * @return PIO_NOERR for success, error code otherwise.
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_enddef(int ncid)
{
    return pioc_change_def(ncid, 1);
}

/**
 * @ingroup PIO_redef
 * The PIO-C interface for the NetCDF function nc_redef.
 *
 * This routine is called collectively by all tasks in the communicator
 * ios.union_comm. For more information on the underlying NetCDF commmand
 * please read about this function in the NetCDF documentation at:
 * http://www.unidata.ucar.edu/software/netcdf/docs/group__datasets.html
 *
 * @param ncid the ncid of the open file, obtained from
 * PIOc_openfile() or PIOc_createfile().
 * @return PIO_NOERR for success, error code otherwise.
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_redef(int ncid)
{
    return pioc_change_def(ncid, 0);
}

/**
 * @ingroup PIO_def_dim
 * The PIO-C interface for the NetCDF function nc_def_dim.
 *
 * This routine is called collectively by all tasks in the communicator
 * ios.union_comm. For more information on the underlying NetCDF commmand
 * please read about this function in the NetCDF documentation at:
 * http://www.unidata.ucar.edu/software/netcdf/docs/group__dimensions.html
 *
 * @param ncid the ncid of the open file, obtained from
 * PIOc_openfile() or PIOc_createfile().
 * @param idp a pointer that will get the id of the variable or attribute.
 * @return PIO_NOERR for success, error code otherwise.
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_def_dim(int ncid, const char *name, PIO_Offset len, int *idp)
{
    iosystem_desc_t *ios;  /* Pointer to io system information. */
    file_desc_t *file;     /* Pointer to file information. */
    int ierr;              /* Return code from function calls. */
    int mpierr = MPI_SUCCESS, mpierr2;  /* Return code from MPI function codes. */

    /* Find the info about this file. */
    if ((ierr = pio_get_file(ncid, &file)))
        return pio_err(NULL, NULL, ierr, __FILE__, __LINE__);
    ios = file->iosystem;

    /* User must provide name shorter than NC_MAX_NAME +1. */
    if (!name || strlen(name) > NC_MAX_NAME)
        return pio_err(ios, file, PIO_EINVAL, __FILE__, __LINE__);

    LOG((1, "PIOc_def_dim ncid = %d name = %s len = %d", ncid, name, len));

    /* If async is in use, and this is not an IO task, bcast the parameters. */
    if (ios->async)
    {
        if (!ios->ioproc)
        {
            int msg = PIO_MSG_DEF_DIM;
            int namelen = strlen(name);

            if (ios->compmaster == MPI_ROOT)
                mpierr = MPI_Send(&msg, 1,MPI_INT, ios->ioroot, 1, ios->union_comm);

            if (!mpierr)
                mpierr = MPI_Bcast(&ncid, 1, MPI_INT, ios->compmaster, ios->intercomm);

            if (!mpierr)
                mpierr = MPI_Bcast(&namelen, 1, MPI_INT,  ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast((void *)name, namelen + 1, MPI_CHAR, ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast(&len, 1, MPI_INT,  ios->compmaster, ios->intercomm);
        }


        /* Handle MPI errors. */
        if ((mpierr2 = MPI_Bcast(&mpierr, 1, MPI_INT, ios->comproot, ios->my_comm)))
            check_mpi(file, mpierr2, __FILE__, __LINE__);
        if (mpierr)
            return check_mpi(file, mpierr, __FILE__, __LINE__);
    }

    /* If this is an IO task, then call the netCDF function. */
    if (ios->ioproc)
    {
#ifdef _PNETCDF
        if (file->iotype == PIO_IOTYPE_PNETCDF)
            ierr = ncmpi_def_dim(file->fh, name, len, idp);
#endif /* _PNETCDF */

        if (file->iotype != PIO_IOTYPE_PNETCDF && file->do_io)
            ierr = nc_def_dim(file->fh, name, (size_t)len, idp);
    }

    /* Broadcast and check the return code. */
    if ((mpierr = MPI_Bcast(&ierr, 1, MPI_INT, ios->ioroot, ios->my_comm)))
        return check_mpi(file, mpierr, __FILE__, __LINE__);
    if (ierr)
        return check_netcdf(file, ierr, __FILE__, __LINE__);

    /* Broadcast results to all tasks. Ignore NULL parameters. */
    if (idp)
        if ((mpierr = MPI_Bcast(idp , 1, MPI_INT, ios->ioroot, ios->my_comm)))
            check_mpi(file, mpierr, __FILE__, __LINE__);

    LOG((2, "def_dim ierr = %d", ierr));
    return PIO_NOERR;
}

/**
 * The PIO-C interface for the NetCDF function nc_def_var.
 *
 * This routine is called collectively by all tasks in the communicator
 * ios.union_comm. For more information on the underlying NetCDF commmand
 * please read about this function in the NetCDF documentation at:
 * http://www.unidata.ucar.edu/software/netcdf/docs/group__variables.html
 *
 * @param ncid the ncid of the open file, obtained from
 * PIOc_openfile() or PIOc_createfile().
 * @param varid the variable ID.
 * @param varidp a pointer that will get the variable id
 * @return PIO_NOERR for success, error code otherwise.
 * @ingroup PIO_def_var
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_def_var(int ncid, const char *name, nc_type xtype, int ndims,
                 const int *dimidsp, int *varidp)
{
    iosystem_desc_t *ios;      /* Pointer to io system information. */
    file_desc_t *file;         /* Pointer to file information. */
    int invalid_unlim_dim = 0; /* True invalid dims are used. */
    int varid;                 /* The varid of the created var. */
    int rec_var = 0;           /* Non-zero if this var uses unlimited dim. */
    PIO_Offset pio_type_size;  /* Size of pio type in bytes. */
    MPI_Datatype mpi_type;     /* The correspoding MPI type. */
    int mpi_type_size;         /* Size of mpi type. */
    int mpierr = MPI_SUCCESS, mpierr2;  /* Return code from MPI function codes. */
    int ierr;                  /* Return code from function calls. */

    /* Get the file information. */
    if ((ierr = pio_get_file(ncid, &file)))
        return pio_err(NULL, NULL, ierr, __FILE__, __LINE__);
    ios = file->iosystem;

    /* User must provide name. */
    if (!name || strlen(name) > NC_MAX_NAME)
        return pio_err(ios, file, PIO_EINVAL, __FILE__, __LINE__);

    LOG((1, "PIOc_def_var ncid = %d name = %s xtype = %d ndims = %d", ncid, name,
         xtype, ndims));

    /* Run this on all tasks if async is not in use, but only on
     * non-IO tasks if async is in use. Learn whether each dimension
     * is unlimited. */
    if (!ios->async || !ios->ioproc)
    {
        int nunlimdims;

        /* Get size of type. */
        if ((ierr = PIOc_inq_type(ncid, xtype, NULL, &pio_type_size)))
            return check_netcdf(file, ierr, __FILE__, __LINE__);

        /* Get the MPI type corresponding with the PIO type. */
        if ((ierr = find_mpi_type(xtype, &mpi_type, NULL)))
            return pio_err(ios, NULL, ierr, __FILE__, __LINE__);
        
        /* Get the size of the MPI type. */
        if ((mpierr = MPI_Type_size(mpi_type, &mpi_type_size)))
            return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);

        /* How many unlimited dims are present in the file? */
        if ((ierr = PIOc_inq_unlimdims(ncid, &nunlimdims, NULL)))
            return check_netcdf(file, ierr, __FILE__, __LINE__);

        if (nunlimdims)
        {
            int unlimdimids[nunlimdims];

            /* Find the IDs of the unlimited dimension(s). */
            if ((ierr = PIOc_inq_unlimdims(ncid, NULL, unlimdimids)))
                return check_netcdf(file, ierr, __FILE__, __LINE__);

            /* Check each dimid for this variable to see it it is an
             * unlimited dimension. */
            for (int d = 0; d < ndims; d++)
            {
                int unlim_found = 0;

                /* Check against each unlimited dimid. */
                for (int ud = 0; ud < nunlimdims; ud++)
                {
                    if (dimidsp[d] == unlimdimids[ud])
                    {
                        unlim_found++;
                        break;
                    }
                }

                /* Only first dim may be unlimited, for PIO. */
                if (unlim_found)
                {
                    if (d == 0)
                        rec_var++;
                    else
                        invalid_unlim_dim++;
                }
            }
        }
    }

    /* If using async, and not an IO task, then send parameters. */
    if (ios->async)
    {
        if (!ios->ioproc)
        {
            int msg = PIO_MSG_DEF_VAR;
            int namelen = strlen(name);

            if (ios->compmaster == MPI_ROOT)
                mpierr = MPI_Send(&msg, 1, MPI_INT, ios->ioroot, 1, ios->union_comm);

            if (!mpierr)
                mpierr = MPI_Bcast(&(ncid), 1, MPI_INT, ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast(&namelen, 1, MPI_INT,  ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast((void *)name, namelen + 1, MPI_CHAR, ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast(&xtype, 1, MPI_INT, ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast(&ndims, 1, MPI_INT, ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast((void *)dimidsp, ndims, MPI_INT, ios->compmaster, ios->intercomm);
        }

        /* Handle MPI errors. */
        if ((mpierr2 = MPI_Bcast(&mpierr, 1, MPI_INT, ios->comproot, ios->my_comm)))
            check_mpi(file, mpierr2, __FILE__, __LINE__);
        if (mpierr)
            return check_mpi(file, mpierr, __FILE__, __LINE__);

        /* Broadcast values currently only known on computation tasks to IO tasks. */
        if ((mpierr = MPI_Bcast(&rec_var, 1, MPI_INT, ios->comproot, ios->my_comm)))
            check_mpi(file, mpierr, __FILE__, __LINE__);
        if ((mpierr = MPI_Bcast(&invalid_unlim_dim, 1, MPI_INT, ios->comproot, ios->my_comm)))
            check_mpi(file, mpierr, __FILE__, __LINE__);
        if ((mpierr = MPI_Bcast(&pio_type_size, 1, MPI_OFFSET, ios->comproot, ios->my_comm)))
            check_mpi(file, mpierr, __FILE__, __LINE__);
        if ((mpierr = MPI_Bcast(&mpi_type, 1, MPI_INT, ios->comproot, ios->my_comm)))
            check_mpi(file, mpierr, __FILE__, __LINE__);
        if ((mpierr = MPI_Bcast(&mpi_type_size, 1, MPI_INT, ios->comproot, ios->my_comm)))
            check_mpi(file, mpierr, __FILE__, __LINE__);
    }

    /* Check that only one unlimited dim is specified, and that it is
     * first. */
    if (invalid_unlim_dim)
        return PIO_EINVAL;

    /* If this is an IO task, then call the netCDF function. */
    if (ios->ioproc)
    {
#ifdef _PNETCDF
        if (file->iotype == PIO_IOTYPE_PNETCDF)
            ierr = ncmpi_def_var(file->fh, name, xtype, ndims, dimidsp, &varid);
#endif /* _PNETCDF */

        if (file->iotype != PIO_IOTYPE_PNETCDF && file->do_io)
            ierr = nc_def_var(file->fh, name, xtype, ndims, dimidsp, &varid);
#ifdef _NETCDF4
        /* For netCDF-4 serial files, turn on compression for this variable. */
        if (!ierr && file->iotype == PIO_IOTYPE_NETCDF4C)
            ierr = nc_def_var_deflate(file->fh, varid, 0, 1, 1);

        /* For netCDF-4 parallel files, set parallel access to collective. */
        if (!ierr && file->iotype == PIO_IOTYPE_NETCDF4P)
            ierr = nc_var_par_access(file->fh, varid, NC_COLLECTIVE);
#endif /* _NETCDF4 */
    }

    /* Broadcast and check the return code. */
    if ((mpierr = MPI_Bcast(&ierr, 1, MPI_INT, ios->ioroot, ios->my_comm)))
        return check_mpi(file, mpierr, __FILE__, __LINE__);
    if (ierr)
        return check_netcdf(file, ierr, __FILE__, __LINE__);

    /* Broadcast results. */
    if ((mpierr = MPI_Bcast(&varid, 1, MPI_INT, ios->ioroot, ios->my_comm)))
        check_mpi(file, mpierr, __FILE__, __LINE__);
    if (varidp)
        *varidp = varid;

    /* Add to the list of var_desc_t structs for this file. */
    if ((ierr = add_to_varlist(varid, rec_var, xtype, (int)pio_type_size, mpi_type,
                               mpi_type_size, &file->varlist)))
        return pio_err(ios, NULL, ierr, __FILE__, __LINE__);
    file->nvars++;

    return PIO_NOERR;
}

/**
 * Set the fill value for a variable.
 *
 * See the <a
 * href="http://www.unidata.ucar.edu/software/netcdf/docs/group__variables.html">netCDF
 * variable documentation</a> for details about the operation of this
 * function.
 *
 * When the fill mode for the file is NC_FILL, then fill values are
 * used for missing data. This function sets the fill value to be used
 * for a variable. If no specific fill value is set (as a _FillValue
 * attribute), then the default fill values from netcdf.h are used.
 *
 * NetCDF-4 and pnetcdf files allow setting fill_mode (to NC_FILL or
 * NC_NOFILL) on a per-variable basis. NetCDF classic only allows the
 * fill_mode setting to be set for the whole file. For this function,
 * the fill_mode parameter is ignored for classic files. Set the
 * file-level fill mode with PIOc_set_fill().
 *
 * @param ncid the ncid of the open file.
 * @param varid the ID of the variable to set chunksizes for.
 * @param fill_mode fill mode for this variable (NC_FILL or NC_NOFILL)
 * @param fill_value pointer to the fill value to be used if fill_mode is set to NC_FILL.
 * @return PIO_NOERR for success, otherwise an error code.
 * @ingroup PIO_def_var
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_def_var_fill(int ncid, int varid, int fill_mode, const void *fill_valuep)
{
    iosystem_desc_t *ios;  /* Pointer to io system information. */
    file_desc_t *file;     /* Pointer to file information. */
    nc_type xtype;         /* The type of the variable (and fill value att). */
    PIO_Offset type_size;  /* Size in bytes of this variable's type. */
    int ierr;              /* Return code from function calls. */
    int mpierr = MPI_SUCCESS, mpierr2;  /* Return code from MPI function codes. */

    LOG((1, "PIOc_def_var_fill ncid = %d varid = %d fill_mode = %d\n", ncid, varid,
         fill_mode));

    /* Get the file info. */
    if ((ierr = pio_get_file(ncid, &file)))
        return pio_err(NULL, NULL, ierr, __FILE__, __LINE__);
    ios = file->iosystem;

    /* Caller must provide correct values. */
    if ((fill_mode != NC_FILL && fill_mode != NC_NOFILL) ||
        (fill_mode == NC_FILL && !fill_valuep))
        return pio_err(ios, file, PIO_EINVAL, __FILE__, __LINE__);

    /* Run this on all tasks if async is not in use, but only on
     * non-IO tasks if async is in use. Get the size of this vars
     * type. */
    if (!ios->async || !ios->ioproc)
    {
        if ((ierr = PIOc_inq_vartype(ncid, varid, &xtype)))
            return check_netcdf(file, ierr, __FILE__, __LINE__);
        if ((ierr = PIOc_inq_type(ncid, xtype, NULL, &type_size)))
            return check_netcdf(file, ierr, __FILE__, __LINE__);
    }
    LOG((2, "PIOc_def_var_fill type_size = %d", type_size));

    /* If async is in use, and this is not an IO task, bcast the parameters. */
    if (ios->async)
    {
        if (!ios->ioproc)
        {
            int msg = PIO_MSG_DEF_VAR_FILL;
            char fill_value_present = fill_valuep ? true : false;

            if (ios->compmaster == MPI_ROOT)
                mpierr = MPI_Send(&msg, 1,MPI_INT, ios->ioroot, 1, ios->union_comm);

            if (!mpierr)
                mpierr = MPI_Bcast(&ncid, 1, MPI_INT, ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast(&varid, 1, MPI_INT, ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast(&fill_mode, 1, MPI_INT, ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast(&type_size, 1, MPI_OFFSET, ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast(&fill_value_present, 1, MPI_CHAR, ios->compmaster, ios->intercomm);
            if (!mpierr && fill_value_present)
                mpierr = MPI_Bcast((PIO_Offset *)fill_valuep, type_size, MPI_CHAR, ios->compmaster,
                                   ios->intercomm);
            LOG((2, "PIOc_def_var_fill ncid = %d varid = %d fill_mode = %d type_size = %d fill_value_present = %d",
                 ncid, varid, fill_mode, type_size, fill_value_present));
        }

        /* Handle MPI errors. */
        if ((mpierr2 = MPI_Bcast(&mpierr, 1, MPI_INT, ios->comproot, ios->my_comm)))
            return check_mpi(file, mpierr2, __FILE__, __LINE__);
        if (mpierr)
            return check_mpi(file, mpierr, __FILE__, __LINE__);

        /* Broadcast values currently only known on computation tasks to IO tasks. */
        if ((mpierr = MPI_Bcast(&xtype, 1, MPI_INT, ios->comproot, ios->my_comm)))
            check_mpi(file, mpierr, __FILE__, __LINE__);
        if ((mpierr = MPI_Bcast(&type_size, 1, MPI_OFFSET, ios->comproot, ios->my_comm)))
            check_mpi(file, mpierr, __FILE__, __LINE__);
    }

    if (ios->ioproc)
    {
        if (file->iotype == PIO_IOTYPE_PNETCDF)
        {
#ifdef _PNETCDF
            ierr = ncmpi_def_var_fill(file->fh, varid, fill_mode, (void *)fill_valuep);
#endif /* _PNETCDF */
        }
        else if (file->iotype == PIO_IOTYPE_NETCDF)
        {
            LOG((2, "defining fill value attribute for netCDF classic file"));
            if (file->do_io)
                ierr = nc_put_att(file->fh, varid, _FillValue, xtype, 1, fill_valuep);
        }
        else
        {
#ifdef _NETCDF4
            if (file->do_io)
                ierr = nc_def_var_fill(file->fh, varid, fill_mode, fill_valuep);
#endif
        }
        LOG((2, "after def_var_fill ierr = %d", ierr));
    }

    /* Broadcast and check the return code. */
    if ((mpierr = MPI_Bcast(&ierr, 1, MPI_INT, ios->ioroot, ios->my_comm)))
        return check_mpi(file, mpierr, __FILE__, __LINE__);
    if (ierr)
        return check_netcdf(file, ierr, __FILE__, __LINE__);

    return PIO_NOERR;
}

/**
 * The PIO-C interface for the NetCDF function nc_inq_var_fill.
 *
 * This routine is called collectively by all tasks in the communicator
 * ios.union_comm. For more information on the underlying NetCDF commmand
 * please read about this function in the NetCDF documentation at:
 * http://www.unidata.ucar.edu/software/netcdf/docs/group__variables.html
 *
 * @param ncid the ncid of the open file, obtained from
 * PIOc_openfile() or PIOc_createfile().
 * @param varid the variable ID.
 * @param no_fill a pointer to int that will get the fill
 * mode. Ignored if NULL (except with pnetcdf, which seg-faults with
 * NULL.)
 * @param fill_valuep pointer to space that gets the fill value for
 * this variable. Ignored if NULL.
 * @return PIO_NOERR for success, error code otherwise.
 * @ingroup PIO_inq_var_fill
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_inq_var_fill(int ncid, int varid, int *no_fill, void *fill_valuep)
{
    iosystem_desc_t *ios;  /* Pointer to io system information. */
    file_desc_t *file;     /* Pointer to file information. */
    nc_type xtype;         /* Type of variable and its _FillValue attribute. */
    PIO_Offset type_size;  /* Size in bytes of this variable's type. */
    int mpierr = MPI_SUCCESS, mpierr2;  /* Return code from MPI function codes. */
    int ierr = PIO_NOERR;  /* Return code from function calls. */

    LOG((1, "PIOc_inq_var_fill ncid = %d varid = %d", ncid, varid));

    /* Find the info about this file. */
    if ((ierr = pio_get_file(ncid, &file)))
        return pio_err(NULL, NULL, ierr, __FILE__, __LINE__);
    ios = file->iosystem;
    LOG((2, "found file"));

    /* Run this on all tasks if async is not in use, but only on
     * non-IO tasks if async is in use. Get the size of this vars
     * type. */
    if (!ios->async || !ios->ioproc)
    {
        if ((ierr = PIOc_inq_vartype(ncid, varid, &xtype)))
            return check_netcdf(file, ierr, __FILE__, __LINE__);
        if ((ierr = PIOc_inq_type(ncid, xtype, NULL, &type_size)))
            return check_netcdf(file, ierr, __FILE__, __LINE__);
        LOG((2, "PIOc_inq_var_fill type_size = %d", type_size));
    }

    /* If async is in use, and this is not an IO task, bcast the parameters. */
    if (ios->async)
    {
        if (!ios->ioproc)
        {
            int msg = PIO_MSG_INQ_VAR_FILL;
            char no_fill_present = no_fill ? true : false;
            char fill_value_present = fill_valuep ? true : false;

            LOG((2, "sending msg type_size = %d", type_size));
            if (ios->compmaster == MPI_ROOT)
                mpierr = MPI_Send(&msg, 1,MPI_INT, ios->ioroot, 1, ios->union_comm);

            if (!mpierr)
                mpierr = MPI_Bcast(&ncid, 1, MPI_INT, ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast(&varid, 1, MPI_INT, ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast(&type_size, 1, MPI_OFFSET, ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast(&no_fill_present, 1, MPI_CHAR, ios->compmaster, ios->intercomm);
            if (!mpierr)
                mpierr = MPI_Bcast(&fill_value_present, 1, MPI_CHAR, ios->compmaster, ios->intercomm);
            LOG((2, "PIOc_inq_var_fill ncid = %d varid = %d type_size = %lld no_fill_present = %d fill_value_present = %d",
                 ncid, varid, type_size, no_fill_present, fill_value_present));
        }

        /* Handle MPI errors. */
        if ((mpierr2 = MPI_Bcast(&mpierr, 1, MPI_INT, ios->comproot, ios->my_comm)))
            check_mpi(file, mpierr2, __FILE__, __LINE__);
        if (mpierr)
            return check_mpi(file, mpierr, __FILE__, __LINE__);

        /* Broadcast values currently only known on computation tasks to IO tasks. */
        if ((mpierr = MPI_Bcast(&xtype, 1, MPI_INT, ios->comproot, ios->my_comm)))
            check_mpi(file, mpierr, __FILE__, __LINE__);
        if ((mpierr = MPI_Bcast(&type_size, 1, MPI_OFFSET, ios->comproot, ios->my_comm)))
            check_mpi(file, mpierr, __FILE__, __LINE__);
    }

    /* If this is an IO task, then call the netCDF function. */
    if (ios->ioproc)
    {
        LOG((2, "calling inq_var_fill file->iotype = %d file->fh = %d varid = %d",
             file->iotype, file->fh, varid));
        if (file->iotype == PIO_IOTYPE_PNETCDF)
        {
#ifdef _PNETCDF
            ierr = ncmpi_inq_var_fill(file->fh, varid, no_fill, fill_valuep);
#endif /* _PNETCDF */
        }
        else if (file->iotype == PIO_IOTYPE_NETCDF && file->do_io)
        {
            /* Get the file-level fill mode. */
            if (no_fill)
            {
                if (file->writable)
                {
                    ierr = nc_set_fill(file->fh, NC_NOFILL, no_fill);
                    if (!ierr)
                        ierr = nc_set_fill(file->fh, *no_fill, NULL);
                }
                else
                {
                    /* pnetcdf and netCDF-4 return PIO_FILL for read-only
                     * files. */
                    *no_fill = PIO_FILL;
                }
            }

            if (!ierr && fill_valuep)
            {
                ierr = nc_get_att(file->fh, varid, _FillValue, fill_valuep);
                if (ierr == NC_ENOTATT)
                {
                    char char_fill_value = NC_FILL_CHAR;
                    signed char byte_fill_value = NC_FILL_BYTE;
                    short short_fill_value = NC_FILL_SHORT;
                    int int_fill_value = NC_FILL_INT;
                    float float_fill_value = NC_FILL_FLOAT;
                    double double_fill_value = NC_FILL_DOUBLE;
                    switch (xtype)
                    {
                    case NC_BYTE:
                        memcpy(fill_valuep, &byte_fill_value, sizeof(signed char));
                        break;
                    case NC_CHAR:
                        memcpy(fill_valuep, &char_fill_value, sizeof(char));
                        break;
                    case NC_SHORT:
                        memcpy(fill_valuep, &short_fill_value, sizeof(short));
                        break;
                    case NC_INT:
                        memcpy(fill_valuep, &int_fill_value, sizeof(int));
                        break;
                    case NC_FLOAT:
                        memcpy(fill_valuep, &float_fill_value, sizeof(float));
                        break;
                    case NC_DOUBLE:
                        memcpy(fill_valuep, &double_fill_value, sizeof(double));
                        break;
                    default:
                        return pio_err(ios, file, NC_EBADTYPE, __FILE__, __LINE__);
                    }
                    ierr = PIO_NOERR;
                }
            }
        }
        else
        {
#ifdef _NETCDF4
            /* The inq_var_fill is not supported in classic-only builds. */
            if (file->do_io)
                ierr = nc_inq_var_fill(file->fh, varid, no_fill, fill_valuep);
#endif /* _NETCDF */
        }
        LOG((2, "after call to inq_var_fill, ierr = %d", ierr));
    }

    /* Broadcast and check the return code. */
    if ((mpierr = MPI_Bcast(&ierr, 1, MPI_INT, ios->ioroot, ios->my_comm)))
        return check_mpi(file, mpierr, __FILE__, __LINE__);
    if (ierr)
        return check_netcdf(file, ierr, __FILE__, __LINE__);

    /* Broadcast results to all tasks. Ignore NULL parameters. */
    if (no_fill)
        if ((mpierr = MPI_Bcast(no_fill, 1, MPI_INT, ios->ioroot, ios->my_comm)))
            check_mpi(file, mpierr, __FILE__, __LINE__);
    if (fill_valuep)
        if ((mpierr = MPI_Bcast(fill_valuep, type_size, MPI_CHAR, ios->ioroot, ios->my_comm)))
            check_mpi(file, mpierr, __FILE__, __LINE__);

    return PIO_NOERR;
}

/**
 * Get the value of an attribute of any type, with no type conversion.
 *
 * This routine is called collectively by all tasks in the communicator
 * ios.union_comm.
 *
 * @param ncid the ncid of the open file, obtained from
 * PIOc_openfile() or PIOc_createfile().
 * @param varid the variable ID.
 * @param name the name of the attribute to get
 * @param ip a pointer that will get the attribute value.
 * @return PIO_NOERR for success, error code otherwise.
 * @ingroup PIO_get_att
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_get_att(int ncid, int varid, const char *name, void *ip)
{
    iosystem_desc_t *ios;  /* Pointer to io system information. */
    file_desc_t *file;     /* Pointer to file information. */
    int ierr;              /* Return code from function calls. */
    nc_type atttype;       /* The type of the attribute. */

    /* Find the info about this file. */
    if ((ierr = pio_get_file(ncid, &file)))
        return pio_err(NULL, NULL, ierr, __FILE__, __LINE__);
    ios = file->iosystem;

    /* User must provide a name and destination pointer. */
    if (!name || !ip || strlen(name) > NC_MAX_NAME)
        return pio_err(ios, file, PIO_EINVAL, __FILE__, __LINE__);

    LOG((1, "PIOc_get_att ncid %d varid %d name %s", ncid, varid, name));

    /* Get the type of the attribute. */
    if ((ierr = PIOc_inq_att(ncid, varid, name, &atttype, NULL)))
        return check_netcdf(file, ierr, __FILE__, __LINE__);
    LOG((2, "atttype = %d", atttype));

    return PIOc_get_att_tc(ncid, varid, name, atttype, ip);
}

/**
 * @ingroup PIO_put_att
 * Write a netCDF attribute of any type.
 *
 * This routine is called collectively by all tasks in the communicator
 * ios.union_comm.
 *
 * @param ncid the ncid of the open file, obtained from
 * PIOc_openfile() or PIOc_createfile().
 * @param varid the variable ID.
 * @param name the name of the attribute.
 * @param xtype the nc_type of the attribute.
 * @param len the length of the attribute array.
 * @param op a pointer with the attribute data.
 * @return PIO_NOERR for success, error code otherwise.
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_put_att(int ncid, int varid, const char *name, nc_type xtype,
                 PIO_Offset len, const void *op)
{
    return PIOc_put_att_tc(ncid, varid, name, xtype, len, xtype, op);
}

/**
 * @ingroup PIO_get_att
 * Get the value of an 64-bit floating point array attribute.
 *
 * This routine is called collectively by all tasks in the communicator
 * ios.union_comm.
 *
 * @param ncid the ncid of the open file, obtained from
 * PIOc_openfile() or PIOc_createfile().
 * @param varid the variable ID.
 * @param name the name of the attribute to get
 * @param ip a pointer that will get the attribute value.
 * @return PIO_NOERR for success, error code otherwise.
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_get_att_double(int ncid, int varid, const char *name, double *ip)
{
    return PIOc_get_att_tc(ncid, varid, name, PIO_DOUBLE, (void *)ip);
}

/**
 * @ingroup PIO_get_att
 * Get the value of an 8-bit unsigned char array attribute.
 *
 * This routine is called collectively by all tasks in the communicator
 * ios.union_comm.
 *
 * @param ncid the ncid of the open file, obtained from
 * PIOc_openfile() or PIOc_createfile().
 * @param varid the variable ID.
 * @param name the name of the attribute to get
 * @param ip a pointer that will get the attribute value.
 * @return PIO_NOERR for success, error code otherwise.
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_get_att_uchar(int ncid, int varid, const char *name, unsigned char *ip)
{
    return PIOc_get_att_tc(ncid, varid, name, PIO_UBYTE, (void *)ip);
}

/**
 * @ingroup PIO_get_att
 * Get the value of an 16-bit unsigned integer array attribute.
 *
 * This routine is called collectively by all tasks in the communicator
 * ios.union_comm.
 *
 * @param ncid the ncid of the open file, obtained from
 * PIOc_openfile() or PIOc_createfile().
 * @param varid the variable ID.
 * @param name the name of the attribute to get
 * @param ip a pointer that will get the attribute value.
 * @return PIO_NOERR for success, error code otherwise.
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_get_att_ushort(int ncid, int varid, const char *name, unsigned short *ip)
{
    return PIOc_get_att_tc(ncid, varid, name, PIO_USHORT, (void *)ip);
}

/**
 * Get the value of an 32-bit unsigned integer array attribute.
 *
 * This routine is called collectively by all tasks in the communicator
 * ios.union_comm.
 *
 * @param ncid the ncid of the open file, obtained from
 * PIOc_openfile() or PIOc_createfile().
 * @param varid the variable ID.
 * @param name the name of the attribute to get
 * @param ip a pointer that will get the attribute value.
 * @return PIO_NOERR for success, error code otherwise.
 * @ingroup PIO_get_att
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_get_att_uint(int ncid, int varid, const char *name, unsigned int *ip)
{
    return PIOc_get_att_tc(ncid, varid, name, PIO_UINT, (void *)ip);
}

/**
 * @ingroup PIO_get_att
 * Get the value of an 32-bit ingeger array attribute.
 *
 * This routine is called collectively by all tasks in the communicator
 * ios.union_comm.
 *
 * @param ncid the ncid of the open file, obtained from
 * PIOc_openfile() or PIOc_createfile().
 * @param varid the variable ID.
 * @param name the name of the attribute to get
 * @param ip a pointer that will get the attribute value.
 * @return PIO_NOERR for success, error code otherwise.
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_get_att_long(int ncid, int varid, const char *name, long *ip)
{
    return PIOc_get_att_tc(ncid, varid, name, PIO_LONG_INTERNAL, (void *)ip);
}

/**
 * Get the value of an text attribute. There is no type conversion
 * with this call. If the attribute is not of type NC_CHAR, then an
 * error will be returned.
 *
 * This routine is called collectively by all tasks in the communicator
 * ios.union_comm.
 *
 * @param ncid the ncid of the open file, obtained from
 * PIOc_openfile() or PIOc_createfile().
 * @param varid the variable ID.
 * @param name the name of the attribute to get
 * @param ip a pointer that will get the attribute value.
 * @return PIO_NOERR for success, error code otherwise.
 * @ingroup PIO_get_att
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_get_att_text(int ncid, int varid, const char *name, char *ip)
{
    return PIOc_get_att_tc(ncid, varid, name, PIO_CHAR, (void *)ip);
}

/**
 * @ingroup PIO_get_att
 * Get the value of an 8-bit signed char array attribute.
 *
 * This routine is called collectively by all tasks in the communicator
 * ios.union_comm.
 *
 * @param ncid the ncid of the open file, obtained from
 * PIOc_openfile() or PIOc_createfile().
 * @param varid the variable ID.
 * @param name the name of the attribute to get
 * @param ip a pointer that will get the attribute value.
 * @return PIO_NOERR for success, error code otherwise.
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_get_att_schar(int ncid, int varid, const char *name, signed char *ip)
{
    return PIOc_get_att_tc(ncid, varid, name, PIO_BYTE, (void *)ip);
}

/**
 * @ingroup PIO_get_att
 * Get the value of an 64-bit unsigned integer array attribute.
 *
 * This routine is called collectively by all tasks in the communicator
 * ios.union_comm.
 *
 * @param ncid the ncid of the open file, obtained from
 * PIOc_openfile() or PIOc_createfile().
 * @param varid the variable ID.
 * @param name the name of the attribute to get
 * @param ip a pointer that will get the attribute value.
 * @return PIO_NOERR for success, error code otherwise.
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_get_att_ulonglong(int ncid, int varid, const char *name, unsigned long long *ip)
{
    return PIOc_get_att_tc(ncid, varid, name, PIO_UINT64, (void *)ip);
}

/**
 * @ingroup PIO_get_att
 * Get the value of an 16-bit integer array attribute.
 *
 * This routine is called collectively by all tasks in the communicator
 * ios.union_comm.
 *
 * @param ncid the ncid of the open file, obtained from
 * PIOc_openfile() or PIOc_createfile().
 * @param varid the variable ID.
 * @param name the name of the attribute to get
 * @param ip a pointer that will get the attribute value.
 * @return PIO_NOERR for success, error code otherwise.
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_get_att_short(int ncid, int varid, const char *name, short *ip)
{
    return PIOc_get_att_tc(ncid, varid, name, PIO_SHORT, (void *)ip);
}

/**
 * @ingroup PIO_get_att
 * Get the value of an 32-bit integer array attribute.
 *
 * This routine is called collectively by all tasks in the communicator
 * ios.union_comm.
 *
 * @param ncid the ncid of the open file, obtained from
 * PIOc_openfile() or PIOc_createfile().
 * @param varid the variable ID.
 * @param name the name of the attribute to get
 * @param ip a pointer that will get the attribute value.
 * @return PIO_NOERR for success, error code otherwise.
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_get_att_int(int ncid, int varid, const char *name, int *ip)
{
    return PIOc_get_att_tc(ncid, varid, name, PIO_INT, (void *)ip);
}

/**
 * @ingroup PIO_get_att
 * Get the value of an 64-bit integer array attribute.
 *
 * This routine is called collectively by all tasks in the communicator
 * ios.union_comm.
 *
 * @param ncid the ncid of the open file, obtained from
 * PIOc_openfile() or PIOc_createfile().
 * @param varid the variable ID.
 * @param name the name of the attribute to get
 * @param ip a pointer that will get the attribute value.
 * @return PIO_NOERR for success, error code otherwise.
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_get_att_longlong(int ncid, int varid, const char *name, long long *ip)
{
    return PIOc_get_att_tc(ncid, varid, name, PIO_INT64, (void *)ip);
}

/**
 * @ingroup PIO_get_att
 * Get the value of an 32-bit floating point array attribute.
 *
 * This routine is called collectively by all tasks in the communicator
 * ios.union_comm.
 *
 * @param ncid the ncid of the open file, obtained from
 * PIOc_openfile() or PIOc_createfile().
 * @param varid the variable ID.
 * @param name the name of the attribute to get
 * @param ip a pointer that will get the attribute value.
 * @return PIO_NOERR for success, error code otherwise.
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_get_att_float(int ncid, int varid, const char *name, float *ip)
{
    return PIOc_get_att_tc(ncid, varid, name, PIO_FLOAT, (void *)ip);
}

/**
 * @ingroup PIO_put_att
 * Write a netCDF attribute array of 8-bit signed chars.
 *
 * This routine is called collectively by all tasks in the communicator
 * ios.union_comm.
 *
 * @param ncid the ncid of the open file, obtained from
 * PIOc_openfile() or PIOc_createfile().
 * @param varid the variable ID.
 * @param name the name of the attribute.
 * @param xtype the nc_type of the attribute.
 * @param len the length of the attribute array.
 * @param op a pointer with the attribute data.
 * @return PIO_NOERR for success, error code otherwise.
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_put_att_schar(int ncid, int varid, const char *name, nc_type xtype,
                       PIO_Offset len, const signed char *op)
{
    return PIOc_put_att_tc(ncid, varid, name, xtype, len, PIO_BYTE, op);
}

/**
 * @ingroup PIO_put_att
 * Write a netCDF attribute array of 32-bit signed integers.
 *
 * This routine is called collectively by all tasks in the communicator
 * ios.union_comm.
 *
 * @param ncid the ncid of the open file, obtained from
 * PIOc_openfile() or PIOc_createfile().
 * @param varid the variable ID.
 * @param name the name of the attribute.
 * @param xtype the nc_type of the attribute.
 * @param len the length of the attribute array.
 * @param op a pointer with the attribute data.
 * @return PIO_NOERR for success, error code otherwise.
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_put_att_long(int ncid, int varid, const char *name, nc_type xtype,
                      PIO_Offset len, const long *op)
{
    return PIOc_put_att_tc(ncid, varid, name, xtype, len, PIO_LONG_INTERNAL, op);
}

/**
 * @ingroup PIO_put_att
 * Write a netCDF attribute array of 32-bit signed integers.
 *
 * This routine is called collectively by all tasks in the communicator
 * ios.union_comm.
 *
 * @param ncid the ncid of the open file, obtained from
 * PIOc_openfile() or PIOc_createfile().
 * @param varid the variable ID.
 * @param name the name of the attribute.
 * @param xtype the nc_type of the attribute.
 * @param len the length of the attribute array.
 * @param op a pointer with the attribute data.
 * @return PIO_NOERR for success, error code otherwise.
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_put_att_int(int ncid, int varid, const char *name, nc_type xtype,
                     PIO_Offset len, const int *op)
{
    return PIOc_put_att_tc(ncid, varid, name, xtype, len, PIO_INT, op);
}

/**
 * @ingroup PIO_put_att
 * Write a netCDF attribute array of 8-bit unsigned chars.
 *
 * This routine is called collectively by all tasks in the communicator
 * ios.union_comm.
 *
 * @param ncid the ncid of the open file, obtained from
 * PIOc_openfile() or PIOc_createfile().
 * @param varid the variable ID.
 * @param name the name of the attribute.
 * @param xtype the nc_type of the attribute.
 * @param len the length of the attribute array.
 * @param op a pointer with the attribute data.
 * @return PIO_NOERR for success, error code otherwise.
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_put_att_uchar(int ncid, int varid, const char *name, nc_type xtype,
                       PIO_Offset len, const unsigned char *op)
{
    return PIOc_put_att_tc(ncid, varid, name, xtype, len, PIO_UBYTE, op);
}

/**
 * @ingroup PIO_put_att
 * Write a netCDF attribute array of 64-bit signed integers.
 *
 * This routine is called collectively by all tasks in the communicator
 * ios.union_comm.
 *
 * @param ncid the ncid of the open file, obtained from
 * PIOc_openfile() or PIOc_createfile().
 * @param varid the variable ID.
 * @param name the name of the attribute.
 * @param xtype the nc_type of the attribute.
 * @param len the length of the attribute array.
 * @param op a pointer with the attribute data.
 * @return PIO_NOERR for success, error code otherwise.
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_put_att_longlong(int ncid, int varid, const char *name, nc_type xtype,
                          PIO_Offset len, const long long *op)
{
    return PIOc_put_att_tc(ncid, varid, name, xtype, len, PIO_INT64, op);
}

/**
 * @ingroup PIO_put_att
 * Write a netCDF attribute array of 32-bit unsigned integers.
 *
 * This routine is called collectively by all tasks in the communicator
 * ios.union_comm.
 *
 * @param ncid the ncid of the open file, obtained from
 * PIOc_openfile() or PIOc_createfile().
 * @param varid the variable ID.
 * @param name the name of the attribute.
 * @param xtype the nc_type of the attribute.
 * @param len the length of the attribute array.
 * @param op a pointer with the attribute data.
 * @return PIO_NOERR for success, error code otherwise.
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_put_att_uint(int ncid, int varid, const char *name, nc_type xtype,
                      PIO_Offset len, const unsigned int *op)
{
    return PIOc_put_att_tc(ncid, varid, name, xtype, len, PIO_UINT, op);
}

/**
 * @ingroup PIO_put_att
 * Write a netCDF attribute array of 32-bit floating points.
 *
 * This routine is called collectively by all tasks in the communicator
 * ios.union_comm.
 *
 * @param ncid the ncid of the open file, obtained from
 * PIOc_openfile() or PIOc_createfile().
 * @param varid the variable ID.
 * @param name the name of the attribute.
 * @param xtype the nc_type of the attribute.
 * @param len the length of the attribute array.
 * @param op a pointer with the attribute data.
 * @return PIO_NOERR for success, error code otherwise.
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_put_att_float(int ncid, int varid, const char *name, nc_type xtype,
                       PIO_Offset len, const float *op)
{
    return PIOc_put_att_tc(ncid, varid, name, xtype, len, PIO_FLOAT, op);
}

/**
 * @ingroup PIO_put_att
 * Write a netCDF attribute array of 64-bit unsigned integers.
 *
 * This routine is called collectively by all tasks in the communicator
 * ios.union_comm.
 *
 * @param ncid the ncid of the open file, obtained from
 * PIOc_openfile() or PIOc_createfile().
 * @param varid the variable ID.
 * @param name the name of the attribute.
 * @param xtype the nc_type of the attribute.
 * @param len the length of the attribute array.
 * @param op a pointer with the attribute data.
 * @return PIO_NOERR for success, error code otherwise.
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_put_att_ulonglong(int ncid, int varid, const char *name, nc_type xtype,
                           PIO_Offset len, const unsigned long long *op)
{
    return PIOc_put_att_tc(ncid, varid, name, xtype, len, PIO_UINT64, op);
}

/**
 * @ingroup PIO_put_att
 * Write a netCDF attribute array of 16-bit unsigned integers.
 *
 * This routine is called collectively by all tasks in the communicator
 * ios.union_comm.
 *
 * @param ncid the ncid of the open file, obtained from
 * PIOc_openfile() or PIOc_createfile().
 * @param varid the variable ID.
 * @param name the name of the attribute.
 * @param xtype the nc_type of the attribute.
 * @param len the length of the attribute array.
 * @param op a pointer with the attribute data.
 * @return PIO_NOERR for success, error code otherwise.
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_put_att_ushort(int ncid, int varid, const char *name, nc_type xtype,
                        PIO_Offset len, const unsigned short *op)
{
    return PIOc_put_att_tc(ncid, varid, name, xtype, len, PIO_USHORT, op);
}

/**
 * @ingroup PIO_put_att
 * Write a netCDF text attribute.
 *
 * This routine is called collectively by all tasks in the communicator
 * ios.union_comm.
 *
 * @param ncid the ncid of the open file, obtained from
 * PIOc_openfile() or PIOc_createfile().
 * @param varid the variable ID.
 * @param name the name of the attribute.
 * @param xtype the nc_type of the attribute.
 * @param len the length of the attribute array.
 * @param op a pointer with the attribute data.
 * @return PIO_NOERR for success, error code otherwise.
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_put_att_text(int ncid, int varid, const char *name,
                      PIO_Offset len, const char *op)
{
    return PIOc_put_att_tc(ncid, varid, name, NC_CHAR, len, NC_CHAR, op);
}

/**
 * @ingroup PIO_put_att
 * Write a netCDF attribute array of 16-bit integers.
 *
 * This routine is called collectively by all tasks in the communicator
 * ios.union_comm.
 *
 * @param ncid the ncid of the open file, obtained from
 * PIOc_openfile() or PIOc_createfile().
 * @param varid the variable ID.
 * @param name the name of the attribute.
 * @param xtype the nc_type of the attribute.
 * @param len the length of the attribute array.
 * @param op a pointer with the attribute data.
 * @return PIO_NOERR for success, error code otherwise.
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_put_att_short(int ncid, int varid, const char *name, nc_type xtype,
                       PIO_Offset len, const short *op)
{
    return PIOc_put_att_tc(ncid, varid, name, xtype, len, PIO_SHORT, op);
}

/**
 * @ingroup PIO_put_att
 * Write a netCDF attribute array of 64-bit floating points.
 *
 * This routine is called collectively by all tasks in the communicator
 * ios.union_comm.
 *
 * @param ncid the ncid of the open file, obtained from
 * PIOc_openfile() or PIOc_createfile().
 * @param varid the variable ID.
 * @param name the name of the attribute.
 * @param xtype the nc_type of the attribute.
 * @param len the length of the attribute array.
 * @param op a pointer with the attribute data.
 * @return PIO_NOERR for success, error code otherwise.
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_put_att_double(int ncid, int varid, const char *name, nc_type xtype,
                        PIO_Offset len, const double *op)
{
    return PIOc_put_att_tc(ncid, varid, name, xtype, len, PIO_DOUBLE, op);
}
