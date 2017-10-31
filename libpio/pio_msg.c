/** @file
 *
 * PIO async message handling. This file contains the code which
 * runs on the IO nodes when async is in use. This code waits for
 * messages from the computation nodes, and responds to messages by
 * running the appropriate netCDF function.
 *
 * Note that when calling the PIOc_* funtion, the return code should
 * be ignored. It is handled within the function. Only errors in
 * internal pio_msg code should return an error from the handler
 * function.
 *
 * @author Ed Hartnett
 */

#include <config.h>
#include <pio.h>
#include <pio_internal.h>

#ifdef PIO_ENABLE_LOGGING
extern int my_rank;
extern int pio_log_level;
#endif /* PIO_ENABLE_LOGGING */

/**
 * This function is run on the IO tasks to handle nc_inq_type*()
 * functions.
 *
 * @param ios pointer to the iosystem info.
 * @returns 0 for success, PIO_EIO for MPI Bcast errors, or error code
 * from netCDF base function.
 * @internal
 * @author Ed Hartnett
 */
int inq_type_handler(iosystem_desc_t *ios)
{
    int ncid;
    int xtype;
    char name_present, size_present;
    char *namep = NULL, name[NC_MAX_NAME + 1];
    PIO_Offset *sizep = NULL, size;
    int mpierr;

    LOG((1, "inq_type_handler"));
    assert(ios);

    /* Get the parameters for this function that the the comp master
     * task is broadcasting. */
    if ((mpierr = MPI_Bcast(&ncid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&xtype, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&name_present, 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&size_present, 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);

    /* Handle null pointer issues. */
    if (name_present)
        namep = name;
    if (size_present)
        sizep = &size;

    /* Call the function. */
    PIOc_inq_type(ncid, xtype, namep, sizep);

    LOG((1, "inq_type_handler succeeded!"));
    return PIO_NOERR;
}

/**
 * This function is run on the IO tasks to find netCDF file
 * format.
 *
 * @param ios pointer to the iosystem info.
 * @returns 0 for success, PIO_EIO for MPI Bcast errors, or error code
 * from netCDF base function.
 * @internal
 * @author Ed Hartnett
 */
int inq_format_handler(iosystem_desc_t *ios)
{
    int ncid;
    int *formatp = NULL, format;
    char format_present;
    int mpierr;

    LOG((1, "inq_format_handler"));
    assert(ios);

    /* Get the parameters for this function that the the comp master
     * task is broadcasting. */
    if ((mpierr = MPI_Bcast(&ncid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&format_present, 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    LOG((2, "inq_format_handler got parameters ncid = %d format_present = %d",
         ncid, format_present));

    /* Manage NULL pointers. */
    if (format_present)
        formatp = &format;

    /* Call the function. */
    PIOc_inq_format(ncid, formatp);

    LOG((1, "inq_format_handler succeeded!"));

    return PIO_NOERR;
}

/**
 * This function is run on the IO tasks to set the file fill mode.
 *
 * @param ios pointer to the iosystem info.
 * @returns 0 for success, error code otherwise.
 * @internal
 * @author Ed Hartnett
 */
int set_fill_handler(iosystem_desc_t *ios)
{
    int ncid;
    int fillmode;
    int old_modep_present;
    int old_mode, *old_modep = NULL;
    int mpierr;

    LOG((1, "set_fill_handler"));
    assert(ios);

    /* Get the parameters for this function that the the comp master
     * task is broadcasting. */
    if ((mpierr = MPI_Bcast(&ncid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&fillmode, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&old_modep_present, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    LOG((2, "set_fill_handler got parameters ncid = %d fillmode = %d old_modep_present = %d",
         ncid, fillmode, old_modep_present));

    /* Manage NULL pointers. */
    if (old_modep_present)
        old_modep = &old_mode;

    /* Call the function. */
    PIOc_set_fill(ncid, fillmode, old_modep);

    LOG((1, "set_fill_handler succeeded!"));

    return PIO_NOERR;
}

/**
 * This function is run on the IO tasks to create a netCDF file.
 *
 * @param ios pointer to the iosystem info.
 * @returns 0 for success, PIO_EIO for MPI Bcast errors, or error code
 * from netCDF base function.
 * @internal
 * @author Ed Hartnett
 */
int create_file_handler(iosystem_desc_t *ios)
{
    int ncid;
    int len;
    int iotype;
    int mode;
    int mpierr;

    LOG((1, "create_file_handler comproot = %d", ios->comproot));
    assert(ios);

    /* Get the parameters for this function that the he comp master
     * task is broadcasting. */
    if ((mpierr = MPI_Bcast(&len, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);

    /* Get space for the filename. */
    char filename[len + 1];

    if ((mpierr = MPI_Bcast(filename, len + 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&iotype, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&mode, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    LOG((1, "create_file_handler got parameters len = %d filename = %s iotype = %d mode = %d",
         len, filename, iotype, mode));

    /* Call the create file function. */
    PIOc_createfile(ios->iosysid, &ncid, &iotype, filename, mode);
    
    LOG((1, "create_file_handler succeeded!"));
    return PIO_NOERR;
}

/**
 * This function is run on the IO tasks to close a netCDF file. It is
 * only ever run on the IO tasks.
 *
 * @param ios pointer to the iosystem_desc_t.
 * @returns 0 for success, PIO_EIO for MPI Bcast errors, or error code
 * from netCDF base function.
 * @internal
 * @author Ed Hartnett
 */
int close_file_handler(iosystem_desc_t *ios)
{
    int ncid;
    int mpierr;

    LOG((1, "close_file_handler"));
    assert(ios);

    /* Get the parameters for this function that the the comp master
     * task is broadcasting. */
    if ((mpierr = MPI_Bcast(&ncid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    LOG((1, "close_file_handler got parameter ncid = %d", ncid));

    /* Call the close file function. */
    PIOc_closefile(ncid);

    LOG((1, "close_file_handler succeeded!"));
    return PIO_NOERR;
}

/**
 * This function is run on the IO tasks to inq a netCDF file. It is
 * only ever run on the IO tasks.
 *
 * @param ios pointer to the iosystem_desc_t.
 * @returns 0 for success, PIO_EIO for MPI Bcast errors, or error code
 * from netCDF base function.
 * @internal
 * @author Ed Hartnett
 */
int inq_handler(iosystem_desc_t *ios)
{
    int ncid;
    int ndims, nvars, ngatts, unlimdimid;
    int *ndimsp = NULL, *nvarsp = NULL, *ngattsp = NULL, *unlimdimidp = NULL;
    char ndims_present, nvars_present, ngatts_present, unlimdimid_present;
    int mpierr;

    LOG((1, "inq_handler"));
    assert(ios);

    /* Get the parameters for this function that the the comp master
     * task is broadcasting. */
    if ((mpierr = MPI_Bcast(&ncid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&ndims_present, 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&nvars_present, 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&ngatts_present, 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&unlimdimid_present, 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    LOG((1, "inq_handler ndims_present = %d nvars_present = %d ngatts_present = %d unlimdimid_present = %d",
         ndims_present, nvars_present, ngatts_present, unlimdimid_present));

    /* NULLs passed in to any of the pointers in the original call
     * need to be matched with NULLs here. Assign pointers where
     * non-NULL pointers were passed in. */
    if (ndims_present)
        ndimsp = &ndims;
    if (nvars_present)
        nvarsp = &nvars;
    if (ngatts_present)
        ngattsp = &ngatts;
    if (unlimdimid_present)
        unlimdimidp = &unlimdimid;

    /* Call the inq function to get the values. */
    PIOc_inq(ncid, ndimsp, nvarsp, ngattsp, unlimdimidp);
    
    return PIO_NOERR;
}

/**
 * This function is run on the IO tasks to inq unlimited dimension
 * ids of a netCDF file. It is only ever run on the IO tasks.
 *
 * @param ios pointer to the iosystem_desc_t.
 * @returns 0 for success, PIO_EIO for MPI Bcast errors, or error code
 * from netCDF base function.
 * @internal
 * @author Ed Hartnett
 */
int inq_unlimdims_handler(iosystem_desc_t *ios)
{
    int ncid;
    int nunlimdims;
    int unlimdimids;
    int *nunlimdimsp = NULL, *unlimdimidsp = NULL;
    char nunlimdimsp_present, unlimdimidsp_present;
    int mpierr;

    LOG((1, "inq_unlimdims_handler"));
    assert(ios);

    /* Get the parameters for this function that the the comp master
     * task is broadcasting. */
    if ((mpierr = MPI_Bcast(&ncid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&nunlimdimsp_present, 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&unlimdimidsp_present, 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    LOG((1, "inq_unlimdims_handler nunlimdimsp_present = %d unlimdimidsp_present = %d",
         nunlimdimsp_present, unlimdimidsp_present));

    /* NULLs passed in to any of the pointers in the original call
     * need to be matched with NULLs here. Assign pointers where
     * non-NULL pointers were passed in. */
    if (nunlimdimsp_present)
        nunlimdimsp = &nunlimdims;
    if (unlimdimidsp_present)
        unlimdimidsp = &unlimdimids;

    /* Call the inq function to get the values. */
    PIOc_inq_unlimdims(ncid, nunlimdimsp, unlimdimidsp);

    return PIO_NOERR;
}

/** Do an inq_dim on a netCDF dimension. This function is only run on
 * IO tasks.
 *
 * @param ios pointer to the iosystem_desc_t.
 * @param msg the message sent my the comp root task.
 * @returns 0 for success, PIO_EIO for MPI Bcast errors, or error code
 * from netCDF base function.
 * @internal
 * @author Ed Hartnett
 */
int inq_dim_handler(iosystem_desc_t *ios, int msg)
{
    int ncid;
    int dimid;
    char name_present, len_present;
    char *dimnamep = NULL;
    PIO_Offset *dimlenp = NULL;
    char dimname[NC_MAX_NAME + 1];
    PIO_Offset dimlen;
    int mpierr;

    LOG((1, "inq_dim_handler"));
    assert(ios);

    /* Get the parameters for this function that the the comp master
     * task is broadcasting. */
    if ((mpierr = MPI_Bcast(&ncid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&dimid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&name_present, 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&len_present, 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    LOG((2, "inq_handler name_present = %d len_present = %d", name_present,
         len_present));

    /* Set the non-null pointers. */
    if (name_present)
        dimnamep = dimname;
    if (len_present)
        dimlenp = &dimlen;

    /* Call the inq function to get the values. */
    PIOc_inq_dim(ncid, dimid, dimnamep, dimlenp);

    return PIO_NOERR;
}

/** Do an inq_dimid on a netCDF dimension name. This function is only
 * run on IO tasks.
 *
 * @param ios pointer to the iosystem_desc_t.
 * @returns 0 for success, PIO_EIO for MPI Bcast errors, or error code
 * from netCDF base function.
 * @internal
 * @author Ed Hartnett
 */
int inq_dimid_handler(iosystem_desc_t *ios)
{
    int ncid;
    int *dimidp = NULL, dimid;
    int id_present;
    int namelen;
    char name[PIO_MAX_NAME + 1];
    int mpierr;

    LOG((1, "inq_dimid_handler"));
    assert(ios);

    /* Get the parameters for this function that the the comp master
     * task is broadcasting. */
    if ((mpierr = MPI_Bcast(&ncid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&namelen, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(name, namelen + 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&id_present, 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    LOG((1, "inq_dimid_handler ncid = %d namelen = %d name = %s id_present = %d",
         ncid, namelen, name, id_present));

    /* Set non-null pointer. */
    if (id_present)
        dimidp = &dimid;

    /* Call the inq_dimid function. */
    PIOc_inq_dimid(ncid, name, dimidp);

    return PIO_NOERR;
}

/** Handle attribute inquiry operations. This code only runs on IO
 * tasks.
 *
 * @param ios pointer to the iosystem_desc_t.
 * @param msg the message sent my the comp root task.
 * @returns 0 for success, PIO_EIO for MPI Bcast errors, or error code
 * from netCDF base function.
 * @internal
 * @author Ed Hartnett
 */
int inq_att_handler(iosystem_desc_t *ios)
{
    int ncid;
    int varid;
    char name[PIO_MAX_NAME + 1];
    int namelen;
    nc_type xtype, *xtypep = NULL;
    PIO_Offset len, *lenp = NULL;
    char xtype_present, len_present;
    int mpierr;

    LOG((1, "inq_att_handler"));
    assert(ios);

    /* Get the parameters for this function that the the comp master
     * task is broadcasting. */
    if ((mpierr = MPI_Bcast(&ncid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&varid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&namelen, 1, MPI_INT,  ios->compmaster, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(name, namelen + 1, MPI_CHAR, ios->compmaster,
                            ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&xtype_present, 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&len_present, 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);

    /* Match NULLs in collective function call. */
    if (xtype_present)
        xtypep = &xtype;
    if (len_present)
        lenp = &len;

    /* Call the function to learn about the attribute. */
    PIOc_inq_att(ncid, varid, name, xtypep, lenp);

    return PIO_NOERR;
}

/** Handle attribute inquiry operations. This code only runs on IO
 * tasks.
 *
 * @param ios pointer to the iosystem_desc_t.
 * @param msg the message sent my the comp root task.
 * @returns 0 for success, PIO_EIO for MPI Bcast errors, or error code
 * from netCDF base function.
 * @internal
 * @author Ed Hartnett
 */
int inq_attname_handler(iosystem_desc_t *ios)
{
    int ncid;
    int varid;
    int attnum;
    char name[NC_MAX_NAME + 1], *namep = NULL;
    char name_present;
    int mpierr;

    LOG((1, "inq_att_name_handler"));
    assert(ios);

    /* Get the parameters for this function that the the comp master
     * task is broadcasting. */
    if ((mpierr = MPI_Bcast(&ncid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&varid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&attnum, 1, MPI_INT,  ios->compmaster, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&name_present, 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    LOG((2, "inq_attname_handler got ncid = %d varid = %d attnum = %d name_present = %d",
         ncid, varid, attnum, name_present));

    /* Match NULLs in collective function call. */
    if (name_present)
        namep = name;

    /* Call the function to learn about the attribute. */
    PIOc_inq_attname(ncid, varid, attnum, namep);

    return PIO_NOERR;
}

/** Handle attribute inquiry operations. This code only runs on IO
 * tasks.
 *
 * @param ios pointer to the iosystem_desc_t.
 * @param msg the message sent my the comp root task.
 * @returns 0 for success, PIO_EIO for MPI Bcast errors, or error code
 * from netCDF base function.
 * @internal
 * @author Ed Hartnett
 */
int inq_attid_handler(iosystem_desc_t *ios)
{
    int ncid;
    int varid;
    char name[PIO_MAX_NAME + 1];
    int namelen;
    int id, *idp = NULL;
    char id_present;
    int mpierr;

    LOG((1, "inq_attid_handler"));
    assert(ios);

    /* Get the parameters for this function that the the comp master
     * task is broadcasting. */
    if ((mpierr = MPI_Bcast(&ncid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&varid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&namelen, 1, MPI_INT,  ios->compmaster, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(name, namelen + 1, MPI_CHAR,  ios->compmaster, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&id_present, 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    LOG((2, "inq_attid_handler got ncid = %d varid = %d id_present = %d",
         ncid, varid, id_present));

    /* Match NULLs in collective function call. */
    if (id_present)
        idp = &id;

    /* Call the function to learn about the attribute. */
    PIOc_inq_attid(ncid, varid, name, idp);

    return PIO_NOERR;
}

/** Handle attribute operations. This code only runs on IO tasks.
 *
 * @param ios pointer to the iosystem_desc_t.
 * @param msg the message sent my the comp root task.
 * @returns 0 for success, PIO_EIO for MPI Bcast errors, or error code
 * from netCDF base function.
 * @internal
 * @author Ed Hartnett
 */
int att_put_handler(iosystem_desc_t *ios)
{
    int ncid;
    int varid;
    char name[PIO_MAX_NAME + 1];
    int namelen;
    PIO_Offset attlen;  /* Number of elements in att array. */
    nc_type atttype;    /* Type of att in file. */
    PIO_Offset atttype_len; /* Length in bytes of one elementy of type atttype. */
    nc_type memtype;    /* Type of att data in memory. */
    PIO_Offset memtype_len; /* Length of element of memtype. */
    void *op;
    int mpierr;

    LOG((1, "att_put_handler"));
    assert(ios);

    /* Get the parameters for this function that the the comp master
     * task is broadcasting. */
    if ((mpierr = MPI_Bcast(&ncid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&varid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&namelen, 1, MPI_INT,  ios->compmaster, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(name, namelen + 1, MPI_CHAR, ios->compmaster, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&atttype, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&attlen, 1, MPI_OFFSET, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&atttype_len, 1, MPI_OFFSET, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&memtype, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&memtype_len, 1, MPI_OFFSET, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);

    /* Allocate memory for the attribute data. */
    if (!(op = malloc(attlen * memtype_len)))
        return pio_err(ios, NULL, PIO_ENOMEM, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(op, attlen * memtype_len, MPI_BYTE, 0, ios->intercomm)))
    {
        free(op);
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    }
    LOG((1, "att_put_handler ncid = %d varid = %d namelen = %d name = %s"
         "atttype = %d attlen = %d atttype_len = %d memtype = %d memtype_len = 5d",
         ncid, varid, namelen, name, atttype, attlen, atttype_len, memtype, memtype_len));

    /* Call the function to write the attribute. */
    PIOc_put_att_tc(ncid, varid, name, atttype, attlen, memtype, op);

    /* Free resources. */
    free(op);

    LOG((2, "att_put_handler complete!"));
    return PIO_NOERR;
}

/** Handle attribute operations. This code only runs on IO tasks.
 *
 * @param ios pointer to the iosystem_desc_t.
 * @param msg the message sent my the comp root task.
 * @returns 0 for success, PIO_EIO for MPI Bcast errors, or error code
 * from netCDF base function.
 * @internal
 * @author Ed Hartnett
 */
int att_get_handler(iosystem_desc_t *ios)
{
    int ncid;
    int varid;
    int mpierr;
    char name[PIO_MAX_NAME + 1];
    int namelen;
    PIO_Offset attlen;
    nc_type atttype;        /* Type of att in file. */
    PIO_Offset atttype_len; /* Length in bytes of an element of attype. */
    nc_type memtype;        /* Type of att in memory. */
    PIO_Offset memtype_len; /* Length in bytes of an element of memype. */
    int *ip;
    int iotype;

    LOG((1, "att_get_handler"));
    assert(ios);

    /* Get the parameters for this function that the the comp master
     * task is broadcasting. */
    if ((mpierr = MPI_Bcast(&ncid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&varid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&namelen, 1, MPI_INT,  ios->compmaster, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(name, namelen + 1, MPI_CHAR, ios->compmaster, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&iotype, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&atttype, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&attlen, 1, MPI_OFFSET, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&atttype_len, 1, MPI_OFFSET, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&memtype, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&memtype_len, 1, MPI_OFFSET, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    LOG((1, "att_get_handler ncid = %d varid = %d namelen = %d name = %s iotype = %d"
         " atttype = %d attlen = %d atttype_len = %d memtype = %d memtype_len = %d",
         ncid, varid, namelen, name, iotype, atttype, attlen, atttype_len, memtype, memtype_len));

    /* Allocate space for the attribute data. */
    if (!(ip = malloc(attlen * memtype_len)))
        return pio_err(ios, NULL, PIO_ENOMEM, __FILE__, __LINE__);

    /* Call the function to read the attribute. */
    PIOc_get_att_tc(ncid, varid, name, memtype, ip);

    /* Free resources. */
    free(ip);

    return PIO_NOERR;
}

/** Handle var put operations. This code only runs on IO tasks.
 *
 * @param ios pointer to the iosystem_desc_t.
 * @returns 0 for success, PIO_EIO for MPI Bcast errors, or error code
 * from netCDF base function.
 * @internal
 * @author Ed Hartnett
 */
int put_vars_handler(iosystem_desc_t *ios)
{
    int ncid;
    int varid;
    PIO_Offset typelen;  /* Length (in bytes) of this type. */
    nc_type xtype;       /* Type of the data being written. */
    char start_present;  /* Zero if user passed a NULL start. */
    char count_present;  /* Zero if user passed a NULL count. */
    char stride_present; /* Zero if user passed a NULL stride. */
    PIO_Offset *startp = NULL;
    PIO_Offset *countp = NULL;
    PIO_Offset *stridep = NULL;
    int ndims;           /* Number of dimensions. */
    void *buf;           /* Buffer for data storage. */
    PIO_Offset num_elem; /* Number of data elements in the buffer. */
    int mpierr;          /* Error code from MPI function calls. */

    LOG((1, "put_vars_handler"));
    assert(ios);

    /* Get the parameters for this function that the the comp master
     * task is broadcasting. */
    if ((mpierr = MPI_Bcast(&ncid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&varid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&ndims, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);

    /* Now we know how big to make these arrays. */
    PIO_Offset start[ndims], count[ndims], stride[ndims];

    if ((mpierr = MPI_Bcast(&start_present, 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if (start_present)
        if ((mpierr = MPI_Bcast(start, ndims, MPI_OFFSET, 0, ios->intercomm)))
            return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    LOG((1, "put_vars_handler getting start[0] = %d ndims = %d", start[0], ndims));
    if ((mpierr = MPI_Bcast(&count_present, 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if (count_present)
        if ((mpierr = MPI_Bcast(count, ndims, MPI_OFFSET, 0, ios->intercomm)))
            return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&stride_present, 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if (stride_present)
        if ((mpierr = MPI_Bcast(stride, ndims, MPI_OFFSET, 0, ios->intercomm)))
            return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&xtype, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&num_elem, 1, MPI_OFFSET, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&typelen, 1, MPI_OFFSET, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    LOG((1, "put_vars_handler ncid = %d varid = %d ndims = %d "
         "start_present = %d count_present = %d stride_present = %d xtype = %d "
         "num_elem = %d typelen = %d", ncid, varid, ndims, start_present, count_present,
         stride_present, xtype, num_elem, typelen));

    /* Allocate room for our data. */
    if (!(buf = malloc(num_elem * typelen)))
        return pio_err(ios, NULL, PIO_ENOMEM, __FILE__, __LINE__);

    /* Get the data. */
    if ((mpierr = MPI_Bcast(buf, num_elem * typelen, MPI_BYTE, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);

    /* Set the non-NULL pointers. */
    if (start_present)
        startp = start;
    if (count_present)
        countp = count;
    if (stride_present)
        stridep = stride;

    /* Call the function to write the data. No need to check return
     * values, they are bcast to computation tasks inside function. */
    switch(xtype)
    {
    case NC_BYTE:
        PIOc_put_vars_schar(ncid, varid, startp, countp, stridep, buf);
        break;
    case NC_CHAR:
        PIOc_put_vars_text(ncid, varid, startp, countp, stridep, buf);
        break;
    case NC_SHORT:
        PIOc_put_vars_short(ncid, varid, startp, countp, stridep, buf);
        break;
    case NC_INT:
        PIOc_put_vars_int(ncid, varid, startp, countp, stridep, buf);
        break;
    case PIO_LONG_INTERNAL:
        PIOc_put_vars_long(ncid, varid, startp, countp, stridep, buf);
        break;
    case NC_FLOAT:
        PIOc_put_vars_float(ncid, varid, startp, countp, stridep, buf);
        break;
    case NC_DOUBLE:
        PIOc_put_vars_double(ncid, varid, startp, countp, stridep, buf);
        break;
#ifdef _NETCDF4
    case NC_UBYTE:
        PIOc_put_vars_uchar(ncid, varid, startp, countp, stridep, buf);
        break;
    case NC_USHORT:
        PIOc_put_vars_ushort(ncid, varid, startp, countp, stridep, buf);
        break;
    case NC_UINT:
        PIOc_put_vars_uint(ncid, varid, startp, countp, stridep, buf);
        break;
    case NC_INT64:
        PIOc_put_vars_longlong(ncid, varid, startp, countp, stridep, buf);
        break;
    case NC_UINT64:
        PIOc_put_vars_ulonglong(ncid, varid, startp, countp, stridep, buf);
        break;
        /* case NC_STRING: */
        /*      PIOc_put_vars_string(ncid, varid, startp, countp, */
        /*                                stridep, (void *)buf); */
        /*      break; */
        /*    default:*/
        /* PIOc_put_vars(ncid, varid, startp, countp, */
        /*                   stridep, buf); */
#endif /* _NETCDF4 */
    }

    free(buf);

    return PIO_NOERR;
}

/** Handle var put operations. This code only runs on IO tasks.
 *
 * @param ios pointer to the iosystem_desc_t.
 * @returns 0 for success, PIO_EIO for MPI Bcast errors, or error code
 * from netCDF base function.
 * @internal
 * @author Ed Hartnett
 */
int put_vara_handler(iosystem_desc_t *ios)
{
    int ncid;
    int varid;
    PIO_Offset typelen;  /* Length (in bytes) of this type. */
    nc_type xtype;       /* Type of the data being written. */
    char start_present;  /* Zero if user passed a NULL start. */
    char count_present;  /* Zero if user passed a NULL count. */
    PIO_Offset *startp = NULL;
    PIO_Offset *countp = NULL;
    int ndims;           /* Number of dimensions. */
    void *buf;           /* Buffer for data storage. */
    PIO_Offset num_elem; /* Number of data elements in the buffer. */
    int mpierr;          /* Error code from MPI function calls. */

    LOG((1, "put_vara_handler"));
    assert(ios);

    /* Get the parameters for this function that the the comp master
     * task is broadcasting. */
    if ((mpierr = MPI_Bcast(&ncid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&varid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&ndims, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);

    /* Now we know how big to make these arrays. */
    PIO_Offset start[ndims], count[ndims];

    if ((mpierr = MPI_Bcast(&start_present, 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if (start_present)
        if ((mpierr = MPI_Bcast(start, ndims, MPI_OFFSET, 0, ios->intercomm)))
            return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    LOG((1, "put_vars_handler getting start[0] = %d ndims = %d", start[0], ndims));
    if ((mpierr = MPI_Bcast(&count_present, 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if (count_present)
        if ((mpierr = MPI_Bcast(count, ndims, MPI_OFFSET, 0, ios->intercomm)))
            return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&xtype, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&num_elem, 1, MPI_OFFSET, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&typelen, 1, MPI_OFFSET, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    LOG((1, "put_vars_handler ncid %d varid %d ndims %d start_present %d count_present %d "
	 "xtype %d num_elem %d typelen %d", ncid, varid, ndims, start_present,
	 count_present, xtype, num_elem, typelen));

    /* Allocate room for our data. */
    if (!(buf = malloc(num_elem * typelen)))
        return pio_err(ios, NULL, PIO_ENOMEM, __FILE__, __LINE__);

    /* Get the data. */
    if ((mpierr = MPI_Bcast(buf, num_elem * typelen, MPI_BYTE, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);

    /* Set the non-NULL pointers. */
    if (start_present)
        startp = start;
    if (count_present)
        countp = count;

    PIOc_put_vara_tc(ncid, varid, startp, countp, xtype, buf);

    free(buf);

    return PIO_NOERR;
}

/** Handle var get operations. This code only runs on IO tasks.
 *
 * @param ios pointer to the iosystem_desc_t.
 * @returns 0 for success, PIO_EIO for MPI Bcast errors, or error code
 * from netCDF base function.
 * @internal
 * @author Ed Hartnett
 */
int get_vars_handler(iosystem_desc_t *ios)
{
    int ncid;
    int varid;
    int mpierr;
    PIO_Offset typelen; /** Length (in bytes) of this type. */
    nc_type xtype; /**
                    * Type of the data being written. */
    PIO_Offset *start;
    PIO_Offset *count;
    PIO_Offset *stride;
    char start_present;
    char count_present;
    char stride_present;
    PIO_Offset *startp = NULL, *countp = NULL, *stridep = NULL;
    int ndims; /** Number of dimensions. */
    void *buf; /** Buffer for data storage. */
    PIO_Offset num_elem; /** Number of data elements in the buffer. */

    LOG((1, "get_vars_handler"));
    assert(ios);

    /* Get the parameters for this function that the the comp master
     * task is broadcasting. */
    if ((mpierr = MPI_Bcast(&ncid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&varid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&ndims, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&start_present, 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if (start_present)
    {
        if (!(start = malloc(ndims * sizeof(PIO_Offset))))
            return pio_err(ios, NULL, PIO_ENOMEM, __FILE__, __LINE__);
        if ((mpierr = MPI_Bcast(start, ndims, MPI_OFFSET, 0, ios->intercomm)))
            return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    }
    if ((mpierr = MPI_Bcast(&count_present, 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if (count_present)
    {
        if (!(count = malloc(ndims * sizeof(PIO_Offset))))
            return pio_err(ios, NULL, PIO_ENOMEM, __FILE__, __LINE__);
        if ((mpierr = MPI_Bcast(count, ndims, MPI_OFFSET, 0, ios->intercomm)))
            return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    }
    if ((mpierr = MPI_Bcast(&stride_present, 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if (stride_present)
    {
        if (!(stride = malloc(ndims * sizeof(PIO_Offset))))
            return pio_err(ios, NULL, PIO_ENOMEM, __FILE__, __LINE__);
        if ((mpierr = MPI_Bcast(stride, ndims, MPI_OFFSET, 0, ios->intercomm)))
            return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    }
    if ((mpierr = MPI_Bcast(&xtype, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&num_elem, 1, MPI_OFFSET, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&typelen, 1, MPI_OFFSET, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    LOG((1, "get_vars_handler ncid = %d varid = %d ndims = %d "
         "stride_present = %d xtype = %d num_elem = %d typelen = %d",
         ncid, varid, ndims, stride_present, xtype, num_elem, typelen));

    /* Allocate room for our data. */
    if (!(buf = malloc(num_elem * typelen)))
        return pio_err(ios, NULL, PIO_ENOMEM, __FILE__, __LINE__);

    /* Set the non-NULL pointers. */
    if (start_present)
        startp = start;

    if (count_present)
        countp = count;

    if (stride_present)
        stridep = stride;

    /* Call the function to read the data. */
    switch(xtype)
    {
    case NC_BYTE:
        PIOc_get_vars_schar(ncid, varid, startp, countp, stridep, buf);
        break;
    case NC_CHAR:
        PIOc_get_vars_text(ncid, varid, startp, countp, stridep, buf);
        break;
    case NC_SHORT:
        PIOc_get_vars_short(ncid, varid, startp, countp, stridep, buf);
        break;
    case NC_INT:
        PIOc_get_vars_int(ncid, varid, startp, countp, stridep, buf);
        break;
    case PIO_LONG_INTERNAL:
        PIOc_get_vars_long(ncid, varid, startp, countp, stridep, buf);
        break;
    case NC_FLOAT:
        PIOc_get_vars_float(ncid, varid, startp, countp, stridep, buf);
        break;
    case NC_DOUBLE:
        PIOc_get_vars_double(ncid, varid, startp, countp, stridep, buf);
        break;
#ifdef _NETCDF4
    case NC_UBYTE:
        PIOc_get_vars_uchar(ncid, varid, startp, countp, stridep, buf);
        break;
    case NC_USHORT:
        PIOc_get_vars_ushort(ncid, varid, startp, countp, stridep, buf);
        break;
    case NC_UINT:
        PIOc_get_vars_uint(ncid, varid, startp, countp, stridep, buf);
        break;
    case NC_INT64:
        PIOc_get_vars_longlong(ncid, varid, startp, countp, stridep, buf);
        break;
    case NC_UINT64:
        PIOc_get_vars_ulonglong(ncid, varid, startp, countp, stridep, buf);
        break;
        /* case NC_STRING: */
        /*      PIOc_get_vars_string(ncid, varid, startp, countp, */
        /*                                stridep, (void *)buf); */
        /*      break; */
        /*    default:*/
        /* PIOc_get_vars(ncid, varid, startp, countp, */
        /*                   stridep, buf); */
#endif /* _NETCDF4 */
    }

    /* Free resourses. */
    free(buf);
    if (start_present)
        free(start);
    if (count_present)
        free(count);
    if (stride_present)
        free(stride);

    LOG((1, "get_vars_handler succeeded!"));
    return PIO_NOERR;
}

/** Handle var get operations. This code only runs on IO tasks.
 *
 * @param ios pointer to the iosystem_desc_t.
 * @returns 0 for success, PIO_EIO for MPI Bcast errors, or error code
 * from netCDF base function.
 * @internal
 * @author Ed Hartnett
 */
int get_vara_handler(iosystem_desc_t *ios)
{
    int ncid;
    int varid;
    int mpierr;
    PIO_Offset typelen; /* Length (in bytes) of this type. */
    nc_type xtype; /* Type of the data being written. */
    PIO_Offset *start;
    PIO_Offset *count;
    char start_present;
    char count_present;
    PIO_Offset *startp = NULL, *countp = NULL;
    int ndims; /* Number of dimensions. */
    void *buf; /* Buffer for data storage. */
    PIO_Offset num_elem; /* Number of data elements in the buffer. */

    LOG((1, "get_vara_handler"));
    assert(ios);

    /* Get the parameters for this function that the the comp master
     * task is broadcasting. */
    if ((mpierr = MPI_Bcast(&ncid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&varid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&ndims, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&start_present, 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if (start_present)
    {
        if (!(start = malloc(ndims * sizeof(PIO_Offset))))
            return pio_err(ios, NULL, PIO_ENOMEM, __FILE__, __LINE__);
        if ((mpierr = MPI_Bcast(start, ndims, MPI_OFFSET, 0, ios->intercomm)))
            return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    }
    if ((mpierr = MPI_Bcast(&count_present, 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if (count_present)
    {
        if (!(count = malloc(ndims * sizeof(PIO_Offset))))
            return pio_err(ios, NULL, PIO_ENOMEM, __FILE__, __LINE__);
        if ((mpierr = MPI_Bcast(count, ndims, MPI_OFFSET, 0, ios->intercomm)))
            return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    }
    if ((mpierr = MPI_Bcast(&xtype, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&num_elem, 1, MPI_OFFSET, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&typelen, 1, MPI_OFFSET, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    LOG((1, "get_vara_handler ncid %d varid %d ndims %d xtype %d num_elem %d "
	 "typelen %d", ncid, varid, ndims, xtype, num_elem, typelen));

    /* Allocate room for our data. */
    if (!(buf = malloc(num_elem * typelen)))
        return pio_err(ios, NULL, PIO_ENOMEM, __FILE__, __LINE__);

    /* Set the non-NULL pointers. */
    if (start_present)
        startp = start;

    if (count_present)
        countp = count;

    PIOc_get_vara_tc(ncid, varid, startp, countp, xtype, buf);

    /* Free resourses. */
    free(buf);
    if (start_present)
        free(start);
    if (count_present)
        free(count);

    LOG((1, "get_vara_handler succeeded!"));
    return PIO_NOERR;
}

/**
 * Do an inq_var on a netCDF variable. This function is only run on
 * IO tasks.
 *
 * @param ios pointer to the iosystem_desc_t.
 * @returns 0 for success, error code otherwise.
 */
int inq_var_handler(iosystem_desc_t *ios)
{
    int ncid;
    int varid;
    char name_present, xtype_present, ndims_present, dimids_present, natts_present;
    char name[NC_MAX_NAME + 1], *namep = NULL;
    nc_type xtype, *xtypep = NULL;
    int *ndimsp = NULL, *dimidsp = NULL, *nattsp = NULL;
    int ndims, dimids[NC_MAX_DIMS], natts;
    int mpierr;

    LOG((1, "inq_var_handler"));
    assert(ios);

    /* Get the parameters for this function that the the comp master
     * task is broadcasting. */
    if ((mpierr = MPI_Bcast(&ncid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&varid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&name_present, 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&xtype_present, 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&ndims_present, 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&dimids_present, 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&natts_present, 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    LOG((2,"inq_var_handler ncid = %d varid = %d name_present = %d xtype_present = %d ndims_present = %d "
         "dimids_present = %d natts_present = %d",
         ncid, varid, name_present, xtype_present, ndims_present, dimids_present, natts_present));

    /* Set the non-NULL pointers. */
    if (name_present)
        namep = name;
    if (xtype_present)
        xtypep = &xtype;
    if (ndims_present)
        ndimsp = &ndims;
    if (dimids_present)
        dimidsp = dimids;
    if (natts_present)
        nattsp = &natts;

    /* Call the inq function to get the values. */
    PIOc_inq_var(ncid, varid, namep, xtypep, ndimsp, dimidsp, nattsp);

    if (ndims_present)
        LOG((2, "inq_var_handler ndims = %d", ndims));

    return PIO_NOERR;
}

/**
 * Do an inq_var on a netCDF variable. This function is only run on
 * IO tasks.
 *
 * @param ios pointer to the iosystem_desc_t.
 * @returns 0 for success, error code otherwise.
 */
int inq_var_all_handler(iosystem_desc_t *ios)
{
    int ncid;
    int varid;
    char name_present, xtype_present, ndims_present, dimids_present, natts_present;
    char shuffle_present, deflate_present, deflate_level_present, fletcher32_present;
    char contiguous_present, chunksizes_present, no_fill_present, fill_value_present;
    char endianness_present, options_mask_present, pixels_per_block_present;
    char name[NC_MAX_NAME + 1], *namep = NULL;
    nc_type xtype, *xtypep = NULL;
    int *ndimsp = NULL, *dimidsp = NULL, *nattsp = NULL;
    int *shufflep = NULL, *deflatep = NULL, *deflate_levelp = NULL, *fletcher32p = NULL;
    int *contiguousp = NULL, *no_fillp = NULL;
    size_t *chunksizesp = NULL;
    int shuffle, deflate, deflate_level, fletcher32, contiguous, no_fill;
    int endianness, options_mask, pixels_per_block;
    void *fill_valuep = NULL;
    int *endiannessp = NULL, *options_maskp = NULL, *pixels_per_blockp = NULL;
    int ndims, dimids[NC_MAX_DIMS], natts;
    int pio_type_size, ndim1;
    int mpierr;

    LOG((1, "inq_var_handler"));
    assert(ios);

    /* Get the parameters for this function that the the comp master
     * task is broadcasting. */
    if ((mpierr = MPI_Bcast(&ncid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&varid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&name_present, 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&xtype_present, 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&ndims_present, 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&dimids_present, 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&natts_present, 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&shuffle_present, 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&deflate_present, 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&deflate_level_present, 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&fletcher32_present, 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&contiguous_present, 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&chunksizes_present, 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&no_fill_present, 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&fill_value_present, 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&endianness_present, 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&options_mask_present, 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&pixels_per_block_present, 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&pio_type_size, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&ndim1, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    LOG((2, "inq_var_all_handler name_present %d xtype_present %d ndims_present %d "
	 "dimids_present %d, natts_present %d  shuffle_present %d deflate_present %d "
	 "deflate_level_present %d fletcher32_present %d contiguous_present %d "
	 "chunksizes_present %d no_fill_present %d fill_value_present %d "
	 "endianness_present %d options_mask_present %d pixels_per_block_present %d"
	 "pio_type_size %d ndim1 %d",
	 name_present, xtype_present, ndims_present, dimids_present, natts_present,
	 shuffle_present, deflate_present, deflate_level_present, fletcher32_present,
	 contiguous_present, chunksizes_present, no_fill_present, fill_value_present,
	 endianness_present, options_mask_present, pixels_per_block_present, pio_type_size,
	 ndim1));

    /* Set the non-NULL pointers. */
    if (name_present)
        namep = name;
    if (xtype_present)
        xtypep = &xtype;
    if (ndims_present)
        ndimsp = &ndims;
    if (dimids_present)
        dimidsp = dimids;
    if (natts_present)
        nattsp = &natts;
    if (shuffle_present)
	shufflep = &shuffle;
    if (deflate_present)
	deflatep = &deflate;
    if (deflate_level_present)
	deflate_levelp = &deflate_level;
    if (fletcher32_present)
	fletcher32p = &fletcher32;
    if (contiguous_present)
	contiguousp = &contiguous;
    if (chunksizes_present)
	if (!(chunksizesp = malloc(ndim1 * sizeof(size_t))))
            return pio_err(ios, NULL, PIO_ENOMEM, __FILE__, __LINE__);	    
    if (no_fill_present)
	no_fillp = &no_fill;
    if (fill_value_present)
	if (!(fill_valuep = malloc(pio_type_size)))
            return pio_err(ios, NULL, PIO_ENOMEM, __FILE__, __LINE__);	    
    if (endianness_present)
	endiannessp = &endianness;
    if (options_mask_present)
	options_maskp = &options_mask;
    if (pixels_per_block_present)
	pixels_per_blockp = &pixels_per_block;

    /* Call the inq function to get the values. */
    PIOc_inq_var_all(ncid, varid, namep, xtypep, ndimsp, dimidsp, nattsp, shufflep, deflatep,
		     deflate_levelp, fletcher32p, contiguousp, chunksizesp, no_fillp, fill_valuep,
		     endiannessp, options_maskp, pixels_per_blockp);

    /* Release storage. */
    if (fill_value_present)
	free(fill_valuep);

    return PIO_NOERR;
}

/**
 * Do an inq_var_chunking on a netCDF variable. This function is only
 * run on IO tasks.
 *
 * @param ios pointer to the iosystem_desc_t.
 * @returns 0 for success, error code otherwise.
 */
int inq_var_chunking_handler(iosystem_desc_t *ios)
{
    int ncid;
    int varid;
    char storage_present, chunksizes_present;
    int storage, *storagep = NULL;
    PIO_Offset chunksizes[NC_MAX_DIMS], *chunksizesp = NULL;
    int mpierr;

    assert(ios);
    LOG((1, "inq_var_chunking_handler"));

    /* Get the parameters for this function that the the comp master
     * task is broadcasting. */
    if ((mpierr = MPI_Bcast(&ncid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&varid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&storage_present, 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&chunksizes_present, 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    LOG((2,"inq_var_handler ncid = %d varid = %d storage_present = %d chunksizes_present = %d",
         ncid, varid, storage_present, chunksizes_present));

    /* Set the non-NULL pointers. */
    if (storage_present)
        storagep = &storage;
    if (chunksizes_present)
        chunksizesp = chunksizes;

    /* Call the inq function to get the values. */
    PIOc_inq_var_chunking(ncid, varid, storagep, chunksizesp);

    return PIO_NOERR;
}

/**
 * Do an inq_var_fill on a netCDF variable. This function is only
 * run on IO tasks.
 *
 * @param ios pointer to the iosystem_desc_t.
 * @returns 0 for success, error code otherwise.
 */
int inq_var_fill_handler(iosystem_desc_t *ios)
{
    int ncid;
    int varid;
    char fill_mode_present, fill_value_present;
    PIO_Offset type_size;
    int fill_mode, *fill_modep = NULL;
    PIO_Offset *fill_value, *fill_valuep = NULL;
    int mpierr;

    assert(ios);
    LOG((1, "inq_var_fill_handler"));

    /* Get the parameters for this function that the the comp master
     * task is broadcasting. */
    if ((mpierr = MPI_Bcast(&ncid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&varid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&type_size, 1, MPI_OFFSET, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&fill_mode_present, 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&fill_value_present, 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    LOG((2,"inq_var_fill_handler ncid = %d varid = %d type_size = %lld, fill_mode_present = %d fill_value_present = %d",
         ncid, varid, type_size, fill_mode_present, fill_value_present));

    /* If we need to, alocate storage for fill value. */
    if (fill_value_present)
        if (!(fill_value = malloc(type_size)))
            return pio_err(ios, NULL, PIO_ENOMEM, __FILE__, __LINE__);

    /* Set the non-NULL pointers. */
    if (fill_mode_present)
        fill_modep = &fill_mode;
    if (fill_value_present)
        fill_valuep = fill_value;

    /* Call the inq function to get the values. */
    PIOc_inq_var_fill(ncid, varid, fill_modep, fill_valuep);

    /* Free fill value storage if we allocated some. */
    if (fill_value_present)
        free(fill_value);

    return PIO_NOERR;
}

/**
 * Do an inq_var_endian on a netCDF variable. This function is only
 * run on IO tasks.
 *
 * @param ios pointer to the iosystem_desc_t.
 * @returns 0 for success, error code otherwise.
 */
int inq_var_endian_handler(iosystem_desc_t *ios)
{
    int ncid;
    int varid;
    char endian_present;
    int endian, *endianp = NULL;
    int mpierr;

    assert(ios);
    LOG((1, "inq_var_endian_handler"));

    /* Get the parameters for this function that the the comp master
     * task is broadcasting. */
    if ((mpierr = MPI_Bcast(&ncid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&varid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&endian_present, 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    LOG((2,"inq_var_endian_handler ncid = %d varid = %d endian_present = %d", ncid, varid,
         endian_present));

    /* Set the non-NULL pointers. */
    if (endian_present)
        endianp = &endian;

    /* Call the inq function to get the values. */
    PIOc_inq_var_endian(ncid, varid, endianp);

    return PIO_NOERR;
}

/**
 * Do an inq_var_deflate on a netCDF variable. This function is only
 * run on IO tasks.
 *
 * @param ios pointer to the iosystem_desc_t.
 * @returns 0 for success, error code otherwise.
 */
int inq_var_deflate_handler(iosystem_desc_t *ios)
{
    int ncid;
    int varid;
    char shuffle_present;
    char deflate_present;
    char deflate_level_present;
    int shuffle, *shufflep;
    int deflate, *deflatep;
    int deflate_level, *deflate_levelp;
    int mpierr;

    assert(ios);
    LOG((1, "inq_var_deflate_handler"));

    /* Get the parameters for this function that the the comp master
     * task is broadcasting. */
    if ((mpierr = MPI_Bcast(&ncid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&varid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&shuffle_present, 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if (shuffle_present && !mpierr)
        if ((mpierr = MPI_Bcast(&shuffle, 1, MPI_INT, 0, ios->intercomm)))
            return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&deflate_present, 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if (deflate_present && !mpierr)
        if ((mpierr = MPI_Bcast(&deflate, 1, MPI_INT, 0, ios->intercomm)))
            return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&deflate_level_present, 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if (deflate_level_present && !mpierr)
        if ((mpierr = MPI_Bcast(&deflate_level, 1, MPI_INT, 0, ios->intercomm)))
            return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    LOG((2, "inq_var_handler ncid = %d varid = %d shuffle_present = %d deflate_present = %d "
         "deflate_level_present = %d", ncid, varid, shuffle_present, deflate_present,
         deflate_level_present));

    /* Set the non-NULL pointers. */
    if (shuffle_present)
        shufflep = &shuffle;
    if (deflate_present)
        deflatep = &deflate;
    if (deflate_level_present)
        deflate_levelp = &deflate_level;

    /* Call the inq function to get the values. */
    PIOc_inq_var_deflate(ncid, varid, shufflep, deflatep, deflate_levelp);

    return PIO_NOERR;
}

/** Do an inq_varid on a netCDF variable name. This function is only
 * run on IO tasks.
 *
 * @param ios pointer to the iosystem_desc_t.
 * @returns 0 for success, PIO_EIO for MPI Bcast errors, or error code
 * from netCDF base function.
 * @internal
 * @author Ed Hartnett
 */
int inq_varid_handler(iosystem_desc_t *ios)
{
    int ncid;
    int varid;
    int namelen;
    char name[PIO_MAX_NAME + 1];
    int mpierr;

    assert(ios);

    /* Get the parameters for this function that the the comp master
     * task is broadcasting. */
    if ((mpierr = MPI_Bcast(&ncid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&namelen, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(name, namelen + 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);

    /* Call the inq_dimid function. */
    PIOc_inq_varid(ncid, name, &varid);

    return PIO_NOERR;
}

/**
 * This function is run on the IO tasks to sync a netCDF file.
 *
 * @param ios pointer to the iosystem_desc_t.
 * @returns 0 for success, PIO_EIO for MPI Bcast errors, or error code
 * from netCDF base function.
 * @internal
 * @author Ed Hartnett
 */
int sync_file_handler(iosystem_desc_t *ios)
{
    int ncid;
    int mpierr;

    LOG((1, "sync_file_handler"));
    assert(ios);

    /* Get the parameters for this function that the comp master
     * task is broadcasting. */
    if ((mpierr = MPI_Bcast(&ncid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    LOG((1, "sync_file_handler got parameter ncid = %d", ncid));

    /* Call the sync file function. */
    PIOc_sync(ncid);

    LOG((2, "sync_file_handler succeeded!"));
    return PIO_NOERR;
}

/**
 * This function is run on the IO tasks to set the record dimension
 * value for a netCDF variable.
 *
 * @param ios pointer to the iosystem_desc_t.
 * @returns 0 for success, PIO_EIO for MPI Bcast errors, or error code
 * from netCDF base function.
 * @internal
 * @author Ed Hartnett
 */
int setframe_handler(iosystem_desc_t *ios)
{
    int ncid;
    int varid;
    int frame;
    int mpierr;

    LOG((1, "setframe_handler"));
    assert(ios);

    /* Get the parameters for this function that the comp master
     * task is broadcasting. */
    if ((mpierr = MPI_Bcast(&ncid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&varid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&frame, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    LOG((1, "setframe_handler got parameter ncid = %d varid = %d frame = %d",
         ncid, varid, frame));

    /* Call the function. */
    PIOc_setframe(ncid, varid, frame);

    LOG((2, "setframe_handler succeeded!"));
    return PIO_NOERR;
}

/**
 * This function is run on the IO tasks to increment the record
 * dimension value for a netCDF variable.
 *
 * @param ios pointer to the iosystem_desc_t.
 * @returns 0 for success, PIO_EIO for MPI Bcast errors, or error code
 * from netCDF base function.
 * @internal
 * @author Ed Hartnett
 */
int advanceframe_handler(iosystem_desc_t *ios)
{
    int ncid;
    int varid;
    int mpierr;

    LOG((1, "advanceframe_handler"));
    assert(ios);

    /* Get the parameters for this function that the comp master
     * task is broadcasting. */
    if ((mpierr = MPI_Bcast(&ncid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&varid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    LOG((1, "advanceframe_handler got parameter ncid = %d varid = %d",
         ncid, varid));

    /* Call the function. */
    PIOc_advanceframe(ncid, varid);

    LOG((2, "advanceframe_handler succeeded!"));
    return PIO_NOERR;
}

/**
 * This function is run on the IO tasks to enddef a netCDF file.
 *
 * @param ios pointer to the iosystem_desc_t.
 * @returns 0 for success, PIO_EIO for MPI Bcast errors, or error code
 * from netCDF base function.
 * @internal
 * @author Ed Hartnett
 */
int change_def_file_handler(iosystem_desc_t *ios, int msg)
{
    int ncid;
    int mpierr;

    LOG((1, "change_def_file_handler"));
    assert(ios);

    /* Get the parameters for this function that the comp master
     * task is broadcasting. */
    if ((mpierr = MPI_Bcast(&ncid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);

    /* Call the function. */
    if (msg == PIO_MSG_ENDDEF)
        PIOc_enddef(ncid);
    else
        PIOc_redef(ncid);

    LOG((1, "change_def_file_handler succeeded!"));
    return PIO_NOERR;
}

/**
 * This function is run on the IO tasks to define a netCDF
 *  variable.
 *
 * @param ios pointer to the iosystem_desc_t.
 * @returns 0 for success, PIO_EIO for MPI Bcast errors, or error code
 * from netCDF base function.
 * @internal
 * @author Ed Hartnett
 */
int def_var_handler(iosystem_desc_t *ios)
{
    int ncid;
    int namelen;
    char name[PIO_MAX_NAME + 1];
    int varid;
    nc_type xtype;
    int ndims;
    int *dimids;
    int mpierr;

    LOG((1, "def_var_handler comproot = %d", ios->comproot));
    assert(ios);

    /* Get the parameters for this function that the he comp master
     * task is broadcasting. */
    if ((mpierr = MPI_Bcast(&ncid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&namelen, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(name, namelen + 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&xtype, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&ndims, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if (!(dimids = malloc(ndims * sizeof(int))))
        return pio_err(ios, NULL, PIO_ENOMEM, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(dimids, ndims, MPI_INT, 0, ios->intercomm)))
    {
        free(dimids);
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    }
    LOG((1, "def_var_handler got parameters namelen = %d "
         "name = %s ncid = %d", namelen, name, ncid));

    /* Call the function. */
    PIOc_def_var(ncid, name, xtype, ndims, dimids, &varid);

    /* Free resources. */
    free(dimids);

    LOG((1, "def_var_handler succeeded!"));
    return PIO_NOERR;
}

/**
 * This function is run on the IO tasks to define chunking for a
 *  netCDF variable.
 *
 * @param ios pointer to the iosystem_desc_t.
 * @returns 0 for success, error code otherwise.
 */
int def_var_chunking_handler(iosystem_desc_t *ios)
{
    int ncid;
    int varid;
    int ndims;
    int storage;
    char chunksizes_present;
    PIO_Offset chunksizes[NC_MAX_DIMS], *chunksizesp = NULL;
    int mpierr;

    assert(ios);
    LOG((1, "def_var_chunking_handler comproot = %d", ios->comproot));

    /* Get the parameters for this function that the he comp master
     * task is broadcasting. */
    if ((mpierr = MPI_Bcast(&ncid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&varid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&storage, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&ndims, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&chunksizes_present, 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if (chunksizes_present)
        if ((mpierr = MPI_Bcast(chunksizes, ndims, MPI_OFFSET, 0, ios->intercomm)))
            return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    LOG((1, "def_var_chunking_handler got parameters ncid = %d varid = %d storage = %d "
         "ndims = %d chunksizes_present = %d", ncid, varid, storage, ndims, chunksizes_present));

    /* Set the non-NULL pointers. */
    if (chunksizes_present)
        chunksizesp = chunksizes;

    /* Call the function. */
    PIOc_def_var_chunking(ncid, varid, storage, chunksizesp);

    LOG((1, "def_var_chunking_handler succeeded!"));
    return PIO_NOERR;
}

/**
 * This function is run on the IO tasks to define fill mode and fill
 * value.
 *
 * @param ios pointer to the iosystem_desc_t.
 * @returns 0 for success, error code otherwise.
 */
int def_var_fill_handler(iosystem_desc_t *ios)
{
    int ncid;
    int varid;
    int fill_mode;
    char fill_value_present;
    PIO_Offset type_size;
    PIO_Offset *fill_valuep = NULL;
    int mpierr;

    assert(ios);
    LOG((1, "def_var_fill_handler comproot = %d", ios->comproot));

    /* Get the parameters for this function that the he comp master
     * task is broadcasting. */
    if ((mpierr = MPI_Bcast(&ncid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&varid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&fill_mode, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&type_size, 1, MPI_OFFSET, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&fill_value_present, 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if (fill_value_present)
    {
        if (!(fill_valuep = malloc(type_size)))
            return pio_err(ios, NULL, PIO_ENOMEM, __FILE__, __LINE__);
        if ((mpierr = MPI_Bcast(fill_valuep, type_size, MPI_CHAR, 0, ios->intercomm)))
        {
            free(fill_valuep);
            return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
        }
    }
    LOG((1, "def_var_fill_handler got parameters ncid = %d varid = %d fill_mode = %d "
         "type_size = %lld fill_value_present = %d", ncid, varid, fill_mode, type_size, fill_value_present));

    /* Call the function. */
    PIOc_def_var_fill(ncid, varid, fill_mode, fill_valuep);

    /* Free memory allocated for the fill value. */
    if (fill_valuep)
        free(fill_valuep);

    LOG((1, "def_var_fill_handler succeeded!"));
    return PIO_NOERR;
}

/**
 * This function is run on the IO tasks to define endianness for a
 * netCDF variable.
 *
 * @param ios pointer to the iosystem_desc_t.
 * @returns 0 for success, error code otherwise.
 */
int def_var_endian_handler(iosystem_desc_t *ios)
{
    int ncid;
    int varid;
    int endian;
    int mpierr;

    assert(ios);
    LOG((1, "def_var_endian_handler comproot = %d", ios->comproot));

    /* Get the parameters for this function that the he comp master
     * task is broadcasting. */
    if ((mpierr = MPI_Bcast(&ncid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&varid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&endian, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    LOG((1, "def_var_endian_handler got parameters ncid = %d varid = %d endain = %d ",
         ncid, varid, endian));

    /* Call the function. */
    PIOc_def_var_endian(ncid, varid, endian);

    LOG((1, "def_var_chunking_handler succeeded!"));
    return PIO_NOERR;
}

/**
 * This function is run on the IO tasks to define deflate settings for
 * a netCDF variable.
 *
 * @param ios pointer to the iosystem_desc_t.
 * @returns 0 for success, error code otherwise.
 */
int def_var_deflate_handler(iosystem_desc_t *ios)
{
    int ncid;
    int varid;
    int shuffle;
    int deflate;
    int deflate_level;
    int mpierr;

    assert(ios);
    LOG((1, "def_var_deflate_handler comproot = %d", ios->comproot));

    /* Get the parameters for this function that the he comp master
     * task is broadcasting. */
    if ((mpierr = MPI_Bcast(&ncid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&varid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&shuffle, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&deflate, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&deflate_level, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    LOG((1, "def_var_deflate_handler got parameters ncid = %d varid = %d shuffle = %d ",
         "deflate = %d deflate_level = %d", ncid, varid, shuffle, deflate, deflate_level));

    /* Call the function. */
    PIOc_def_var_deflate(ncid, varid, shuffle, deflate, deflate_level);

    LOG((1, "def_var_deflate_handler succeeded!"));
    return PIO_NOERR;
}

/**
 * This function is run on the IO tasks to define chunk cache settings
 * for a netCDF variable.
 *
 * @param ios pointer to the iosystem_desc_t.
 * @returns 0 for success, error code otherwise.
 */
int set_var_chunk_cache_handler(iosystem_desc_t *ios)
{
    int ncid;
    int varid;
    PIO_Offset size;
    PIO_Offset nelems;
    float preemption;
    int mpierr = MPI_SUCCESS;  /* Return code from MPI function codes. */

    assert(ios);
    LOG((1, "set_var_chunk_cache_handler comproot = %d", ios->comproot));

    /* Get the parameters for this function that the he comp master
     * task is broadcasting. */
    if ((mpierr = MPI_Bcast(&ncid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&varid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&size, 1, MPI_OFFSET, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&nelems, 1, MPI_OFFSET, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&preemption, 1, MPI_FLOAT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    LOG((1, "set_var_chunk_cache_handler got params ncid = %d varid = %d size = %d "
         "nelems = %d preemption = %g", ncid, varid, size, nelems, preemption));

    /* Call the function. */
    PIOc_set_var_chunk_cache(ncid, varid, size, nelems, preemption);

    LOG((1, "def_var_chunk_cache_handler succeeded!"));
    return PIO_NOERR;
}

/**
 * This function is run on the IO tasks to define a netCDF
 * dimension.
 *
 * @param ios pointer to the iosystem_desc_t.
 * @returns 0 for success, PIO_EIO for MPI Bcast errors, or error code
 * from netCDF base function.
 * @internal
 * @author Ed Hartnett
 */
int def_dim_handler(iosystem_desc_t *ios)
{
    int ncid;
    int len, namelen;
    char name[PIO_MAX_NAME + 1];
    int dimid;
    int mpierr;

    LOG((1, "def_dim_handler comproot = %d", ios->comproot));
    assert(ios);

    /* Get the parameters for this function that the he comp master
     * task is broadcasting. */
    if ((mpierr = MPI_Bcast(&ncid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&namelen, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(name, namelen + 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&len, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    LOG((2, "def_dim_handler got parameters namelen = %d "
         "name = %s len = %d ncid = %d", namelen, name, len, ncid));

    /* Call the function. */
    PIOc_def_dim(ncid, name, len, &dimid);

    LOG((1, "def_dim_handler succeeded!"));
    return PIO_NOERR;
}

/**
 * This function is run on the IO tasks to rename a netCDF
 * dimension.
 *
 * @param ios pointer to the iosystem_desc_t.
 * @returns 0 for success, PIO_EIO for MPI Bcast errors, or error code
 * from netCDF base function.
 * @internal
 * @author Ed Hartnett
 */
int rename_dim_handler(iosystem_desc_t *ios)
{
    int ncid;
    int namelen;
    char name[PIO_MAX_NAME + 1];
    int dimid;
    int mpierr;

    LOG((1, "rename_dim_handler"));
    assert(ios);

    /* Get the parameters for this function that the he comp master
     * task is broadcasting. */
    if ((mpierr = MPI_Bcast(&ncid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&dimid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&namelen, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(name, namelen + 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    LOG((2, "rename_dim_handler got parameters namelen = %d "
         "name = %s ncid = %d dimid = %d", namelen, name, ncid, dimid));

    /* Call the function. */
    PIOc_rename_dim(ncid, dimid, name);

    LOG((1, "rename_dim_handler succeeded!"));
    return PIO_NOERR;
}

/**
 * This function is run on the IO tasks to rename a netCDF
 * dimension.
 *
 * @param ios pointer to the iosystem_desc_t.
 * @returns 0 for success, PIO_EIO for MPI Bcast errors, or error code
 * from netCDF base function.
 * @internal
 * @author Ed Hartnett
 */
int rename_var_handler(iosystem_desc_t *ios)
{
    int ncid;
    int namelen;
    char name[PIO_MAX_NAME + 1];
    int varid;
    int mpierr;

    LOG((1, "rename_var_handler"));
    assert(ios);

    /* Get the parameters for this function that the he comp master
     * task is broadcasting. */
    if ((mpierr = MPI_Bcast(&ncid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&varid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&namelen, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(name, namelen + 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    LOG((2, "rename_var_handler got parameters namelen = %d "
         "name = %s ncid = %d varid = %d", namelen, name, ncid, varid));

    /* Call the function. */
    PIOc_rename_var(ncid, varid, name);

    LOG((1, "rename_var_handler succeeded!"));
    return PIO_NOERR;
}

/**
 * This function is run on the IO tasks to rename a netCDF
 * attribute.
 *
 * @param ios pointer to the iosystem_desc_t.
 * @returns 0 for success, PIO_EIO for MPI Bcast errors, or error code
 * from netCDF base function.
 * @internal
 * @author Ed Hartnett
 */
int rename_att_handler(iosystem_desc_t *ios)
{
    int ncid;
    int varid;
    int namelen, newnamelen;
    char name[PIO_MAX_NAME + 1], newname[PIO_MAX_NAME + 1];
    int mpierr;

    LOG((1, "rename_att_handler"));
    assert(ios);

    /* Get the parameters for this function that the he comp master
     * task is broadcasting. */
    if ((mpierr = MPI_Bcast(&ncid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&varid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&namelen, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(name, namelen + 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&newnamelen, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(newname, newnamelen + 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    LOG((2, "rename_att_handler got parameters namelen = %d name = %s ncid = %d varid = %d "
         "newnamelen = %d newname = %s", namelen, name, ncid, varid, newnamelen, newname));

    /* Call the function. */
    PIOc_rename_att(ncid, varid, name, newname);

    LOG((1, "rename_att_handler succeeded!"));
    return PIO_NOERR;
}

/**
 * This function is run on the IO tasks to delete a netCDF
 * attribute.
 *
 * @param ios pointer to the iosystem_desc_t.
 * @returns 0 for success, PIO_EIO for MPI Bcast errors, or error code
 * from netCDF base function.
 * @internal
 * @author Ed Hartnett
 */
int delete_att_handler(iosystem_desc_t *ios)
{
    int ncid;
    int varid;
    int namelen;
    char name[PIO_MAX_NAME + 1];
    int mpierr;

    LOG((1, "delete_att_handler"));
    assert(ios);

    /* Get the parameters for this function that the he comp master
     * task is broadcasting. */
    if ((mpierr = MPI_Bcast(&ncid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&varid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&namelen, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(name, namelen + 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    LOG((2, "delete_att_handler namelen = %d name = %s ncid = %d varid = %d ",
         namelen, name, ncid, varid));

    /* Call the function. */
    PIOc_del_att(ncid, varid, name);

    LOG((1, "delete_att_handler succeeded!"));
    return PIO_NOERR;
}

/**
 * This function is run on the IO tasks to open a netCDF file.
 *
 *
 * @param ios pointer to the iosystem_desc_t.
 * @returns 0 for success, PIO_EIO for MPI Bcast errors, or error code
 * from netCDF base function.
 * @internal
 * @author Ed Hartnett
 */
int open_file_handler(iosystem_desc_t *ios)
{
    int ncid;
    int len;
    int iotype;
    int mode;
    int mpierr;

    LOG((1, "open_file_handler comproot = %d", ios->comproot));
    assert(ios);

    /* Get the parameters for this function that the he comp master
     * task is broadcasting. */
    if ((mpierr = MPI_Bcast(&len, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    LOG((2, "open_file_handler got parameter len = %d", len));

    /* Get space for the filename. */
    char filename[len + 1];

    if ((mpierr = MPI_Bcast(filename, len + 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&iotype, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&mode, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);

    LOG((2, "open_file_handler got parameters len = %d filename = %s iotype = %d mode = %d",
         len, filename, iotype, mode));

    /* Call the open file function. Errors are handling within
     * function, so return code can be ignored. */
    PIOc_openfile_retry(ios->iosysid, &ncid, &iotype, filename, mode, 0);

    return PIO_NOERR;
}

/**
 * This function is run on the IO tasks to delete a netCDF file.
 *
 * @param ios pointer to the iosystem_desc_t data.
 *
 * @returns 0 for success, PIO_EIO for MPI Bcast errors, or error code
 * from netCDF base function.
 * @internal
 * @author Ed Hartnett
 */
int delete_file_handler(iosystem_desc_t *ios)
{
    int len;
    int mpierr;

    LOG((1, "delete_file_handler comproot = %d", ios->comproot));
    assert(ios);

    /* Get the parameters for this function that the he comp master
     * task is broadcasting. */
    if ((mpierr = MPI_Bcast(&len, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);

    /* Get space for the filename. */
    char filename[len + 1];

    if ((mpierr = MPI_Bcast(filename, len + 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    LOG((1, "delete_file_handler got parameters len = %d filename = %s",
         len, filename));

    /* Call the delete file function. */
    PIOc_deletefile(ios->iosysid, filename);

    LOG((1, "delete_file_handler succeeded!"));
    return PIO_NOERR;
}

/**
 * This function is run on the IO tasks to initialize a decomposition.
 *
 * @param ios pointer to the iosystem_desc_t data.
 * @returns 0 for success, PIO_EIO for MPI Bcast errors, or error code
 * from netCDF base function.
 */
int initdecomp_dof_handler(iosystem_desc_t *ios)
{
    int iosysid;
    int pio_type;
    int ndims;
    int maplen;
    int ioid;
    char rearranger_present;
    int rearranger;
    int *rearrangerp = NULL;
    char iostart_present;
    PIO_Offset *iostartp = NULL;
    char iocount_present;
    PIO_Offset *iocountp = NULL;
    int mpierr = MPI_SUCCESS;  /* Return code from MPI function codes. */
    int ret; /* Return code. */

    LOG((1, "initdecomp_dof_handler called"));
    assert(ios);

    /* Get the parameters for this function that the the comp master
     * task is broadcasting. */
    if ((mpierr = MPI_Bcast(&iosysid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&pio_type, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&ndims, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);

    /* Now we know the size of these arrays. */
    int dims[ndims];
    PIO_Offset iostart[ndims];
    PIO_Offset iocount[ndims];

    if ((mpierr = MPI_Bcast(dims, ndims, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&maplen, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);

    PIO_Offset compmap[maplen];

    if ((mpierr = MPI_Bcast(compmap, maplen, MPI_OFFSET, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);

    if ((mpierr = MPI_Bcast(&rearranger_present, 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);

    if (rearranger_present)
        if ((mpierr = MPI_Bcast(&rearranger, 1, MPI_INT, 0, ios->intercomm)))
            return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);

    if ((mpierr = MPI_Bcast(&iostart_present, 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);

    if (iostart_present)
        if ((mpierr = MPI_Bcast(iostart, ndims, MPI_OFFSET, 0, ios->intercomm)))
            return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);

    if ((mpierr = MPI_Bcast(&iocount_present, 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);

    if (iocount_present)
        if ((mpierr = MPI_Bcast(iocount, ndims, MPI_OFFSET, 0, ios->intercomm)))
            return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);

    LOG((2, "initdecomp_dof_handler iosysid = %d pio_type = %d ndims = %d maplen = %d "
         "rearranger_present = %d iostart_present = %d iocount_present = %d ",
         iosysid, pio_type, ndims, maplen, rearranger_present, iostart_present, iocount_present));

    if (rearranger_present)
        rearrangerp = &rearranger;
    if (iostart_present)
        iostartp = iostart;
    if (iocount_present)
        iocountp = iocount;

    /* Call the function. */
    ret = PIOc_InitDecomp(iosysid, pio_type, ndims, dims, maplen, compmap, &ioid, rearrangerp,
                          iostartp, iocountp);

    LOG((1, "PIOc_InitDecomp returned %d", ret));
    return PIO_NOERR;
}

/**
 * This function is run on the IO tasks to do darray writes.
 *
 * @param ios pointer to the iosystem_desc_t data.
 *
 * @returns 0 for success, PIO_EIO for MPI Bcast errors, or error code
 * from netCDF base function.
 * @internal
 * @author Ed Hartnett
 */
int write_darray_multi_handler(iosystem_desc_t *ios)
{
    int ncid;
    file_desc_t *file;     /* Pointer to file information. */
    int nvars;
    int ioid;
    io_desc_t *iodesc;     /* The IO description. */
    char frame_present;
    int *framep = NULL;
    int *frame;
    PIO_Offset arraylen;
    void *array;
    char fillvalue_present;
    void *fillvaluep = NULL;
    void *fillvalue;
    int flushtodisk;
    int mpierr;
    int ret;

    LOG((1, "write_darray_multi_handler"));
    assert(ios);

    /* Get the parameters for this function that the the comp master
     * task is broadcasting. */
    if ((mpierr = MPI_Bcast(&ncid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&nvars, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    int varids[nvars];
    if ((mpierr = MPI_Bcast(varids, nvars, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&ioid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);

    /* Get decomposition information. */
    if (!(iodesc = pio_get_iodesc_from_id(ioid)))
        return pio_err(ios, file, PIO_EBADID, __FILE__, __LINE__);

    if ((mpierr = MPI_Bcast(&arraylen, 1, MPI_OFFSET, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if (!(array = malloc(arraylen * iodesc->piotype_size)))
        return pio_err(NULL, NULL, PIO_ENOMEM, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(array, arraylen * iodesc->piotype_size, MPI_CHAR, 0,
                            ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&frame_present, 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if (frame_present)
    {
        if (!(frame = malloc(nvars * sizeof(int))))
            return pio_err(ios, NULL, PIO_ENOMEM, __FILE__, __LINE__);
        if ((mpierr = MPI_Bcast(frame, nvars, MPI_INT, 0, ios->intercomm)))
            return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    }
    if ((mpierr = MPI_Bcast(&fillvalue_present, 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if (fillvalue_present)
    {
        if (!(fillvalue = malloc(nvars * iodesc->piotype_size)))
            return pio_err(ios, NULL, PIO_ENOMEM, __FILE__, __LINE__);
        if ((mpierr = MPI_Bcast(fillvalue, nvars * iodesc->piotype_size, MPI_CHAR, 0, ios->intercomm)))
            return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    }
    if ((mpierr = MPI_Bcast(&flushtodisk, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    LOG((1, "write_darray_multi_handler ncid = %d nvars = %d ioid = %d arraylen = %d "
         "frame_present = %d fillvalue_present flushtodisk = %d", ncid, nvars,
         ioid, arraylen, frame_present, fillvalue_present, flushtodisk));

    /* Get file info based on ncid. */
    if ((ret = pio_get_file(ncid, &file)))
        return pio_err(NULL, NULL, ret, __FILE__, __LINE__);

    /* Get decomposition information. */
    if (!(iodesc = pio_get_iodesc_from_id(ioid)))
        return pio_err(ios, file, PIO_EBADID, __FILE__, __LINE__);

    /* Was a frame array provided? */
    if (frame_present)
        framep = frame;

    /* Was a fillvalue array provided? */
    if (fillvalue_present)
        fillvaluep = fillvalue;

    /* Call the function from IO tasks. Errors are handled within
     * function. */
    PIOc_write_darray_multi(ncid, varids, ioid, nvars, arraylen, array, framep,
                            fillvaluep, flushtodisk);

    /* Free resources. */
    if (frame_present)
        free(frame);
    if (fillvalue_present)
        free(fillvalue);
    free(array);

    LOG((1, "write_darray_multi_handler succeeded!"));
    return PIO_NOERR;
}

/**
 * This function is run on the IO tasks to...
 * NOTE: not yet implemented
 *
 * @param ios pointer to the iosystem_desc_t data.
 *
 * @returns 0 for success, PIO_EIO for MPI Bcast errors, or error code
 * from netCDF base function.
 * @internal
 * @author Ed Hartnett
 */
int readdarray_handler(iosystem_desc_t *ios)
{
    assert(ios);
    return PIO_NOERR;
}

/**
 * This function is run on the IO tasks to set the error handler.
 *
 * @param ios pointer to the iosystem_desc_t data.
 *
 * @returns 0 for success, PIO_EIO for MPI Bcast errors, or error code
 * from netCDF base function.
 * @internal
 * @author Ed Hartnett
 */
int seterrorhandling_handler(iosystem_desc_t *ios)
{
    int method;
    int old_method_present;
    int old_method;
    int *old_methodp = NULL;
    int mpierr;

    LOG((1, "seterrorhandling_handler comproot = %d", ios->comproot));
    assert(ios);

    /* Get the parameters for this function that the he comp master
     * task is broadcasting. */
    if ((mpierr = MPI_Bcast(&method, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&old_method_present, 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);

    LOG((1, "seterrorhandling_handler got parameters method = %d old_method_present = %d",
         method, old_method_present));

    if (old_method_present)
        old_methodp = &old_method;

    /* Call the function. */
    PIOc_set_iosystem_error_handling(ios->iosysid, method, old_methodp);

    LOG((1, "seterrorhandling_handler succeeded!"));
    return PIO_NOERR;
}

/**
 * This function is run on the IO tasks to set the chunk cache
 * parameters for netCDF-4.
 *
 * @param ios pointer to the iosystem_desc_t data.
 * @returns 0 for success, PIO_EIO for MPI Bcast errors, or error code
 * from netCDF base function.
 * @author Ed Hartnett
 */
int set_chunk_cache_handler(iosystem_desc_t *ios)
{
    int iosysid;
    int iotype;
    PIO_Offset size;
    PIO_Offset nelems;
    float preemption;
    int mpierr = MPI_SUCCESS;  /* Return code from MPI function codes. */

    LOG((1, "set_chunk_cache_handler called"));
    assert(ios);

    /* Get the parameters for this function that the the comp master
     * task is broadcasting. */
    if ((mpierr = MPI_Bcast(&iosysid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&iotype, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&size, 1, MPI_OFFSET, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&nelems, 1, MPI_OFFSET, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&preemption, 1, MPI_FLOAT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    LOG((1, "set_chunk_cache_handler got params iosysid = %d iotype = %d size = %d "
         "nelems = %d preemption = %g", iosysid, iotype, size, nelems, preemption));

    /* Call the function. */
    PIOc_set_chunk_cache(iosysid, iotype, size, nelems, preemption);

    LOG((1, "set_chunk_cache_handler succeeded!"));
    return PIO_NOERR;
}

/**
 * This function is run on the IO tasks to get the chunk cache
 * parameters for netCDF-4.
 *
 * @param ios pointer to the iosystem_desc_t data.
 * @returns 0 for success, PIO_EIO for MPI Bcast errors, or error code
 * from netCDF base function.
 * @author Ed Hartnett
 */
int get_chunk_cache_handler(iosystem_desc_t *ios)
{
    int iosysid;
    int iotype;
    char size_present, nelems_present, preemption_present;
    PIO_Offset size, *sizep;
    PIO_Offset nelems, *nelemsp;
    float preemption, *preemptionp;
    int mpierr = MPI_SUCCESS;  /* Return code from MPI function codes. */

    LOG((1, "get_chunk_cache_handler called"));
    assert(ios);

    /* Get the parameters for this function that the the comp master
     * task is broadcasting. */
    if ((mpierr = MPI_Bcast(&iosysid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&iotype, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&size_present, 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&nelems_present, 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&preemption_present, 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    LOG((1, "get_chunk_cache_handler got params iosysid = %d iotype = %d size_present = %d "
         "nelems_present = %d preemption_present = %g", iosysid, iotype, size_present,
         nelems_present, preemption_present));

    /* Set the non-NULL pointers. */
    if (size_present)
        sizep = &size;
    if (nelems_present)
        nelemsp = &nelems;
    if (preemption_present)
        preemptionp = &preemption;

    /* Call the function. */
    PIOc_get_chunk_cache(iosysid, iotype, sizep, nelemsp, preemptionp);

    LOG((1, "get_chunk_cache_handler succeeded!"));
    return PIO_NOERR;
}

/**
 * This function is run on the IO tasks to get the variable chunk
 * cache parameters for netCDF-4.
 *
 * @param ios pointer to the iosystem_desc_t data.
 * @returns 0 for success, PIO_EIO for MPI Bcast errors, or error code
 * from netCDF base function.
 * @author Ed Hartnett
 */
int get_var_chunk_cache_handler(iosystem_desc_t *ios)
{
    int ncid;
    int varid;
    char size_present, nelems_present, preemption_present;
    PIO_Offset size, *sizep;
    PIO_Offset nelems, *nelemsp;
    float preemption, *preemptionp;
    int mpierr = MPI_SUCCESS;  /* Return code from MPI function codes. */

    LOG((1, "get_var_chunk_cache_handler called"));
    assert(ios);

    /* Get the parameters for this function that the the comp master
     * task is broadcasting. */
    if ((mpierr = MPI_Bcast(&ncid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&varid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&size_present, 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&nelems_present, 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&preemption_present, 1, MPI_CHAR, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    LOG((1, "get_var_chunk_cache_handler got params ncid = %d varid = %d size_present = %d "
         "nelems_present = %d preemption_present = %g", ncid, varid, size_present,
         nelems_present, preemption_present));

    /* Set the non-NULL pointers. */
    if (size_present)
        sizep = &size;
    if (nelems_present)
        nelemsp = &nelems;
    if (preemption_present)
        preemptionp = &preemption;

    /* Call the function. */
    PIOc_get_var_chunk_cache(ncid, varid, sizep, nelemsp, preemptionp);

    LOG((1, "get_var_chunk_cache_handler succeeded!"));
    return PIO_NOERR;
}

/**
 * This function is run on the IO tasks to free the decomp hanlder.
 *
 * @param ios pointer to the iosystem_desc_t data.
 * @returns 0 for success, PIO_EIO for MPI Bcast errors, or error code
 * from netCDF base function.
 * @author Ed Hartnett
 */
int freedecomp_handler(iosystem_desc_t *ios)
{
    int iosysid;
    int ioid;
    int mpierr = MPI_SUCCESS;  /* Return code from MPI function codes. */
    int ret; /* Return code. */

    LOG((1, "freedecomp_handler called"));
    assert(ios);

    /* Get the parameters for this function that the the comp master
     * task is broadcasting. */
    if ((mpierr = MPI_Bcast(&iosysid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    if ((mpierr = MPI_Bcast(&ioid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    LOG((2, "freedecomp_handler iosysid = %d ioid = %d", iosysid, ioid));

    /* Call the function. */
    ret = PIOc_freedecomp(iosysid, ioid);

    LOG((1, "PIOc_freedecomp returned %d", ret));
    return PIO_NOERR;
}

/**
 * Handle the finalize call.
 *
 * @param ios pointer to the iosystem info
 * @param index
 * @returns 0 for success, PIO_EIO for MPI Bcast errors, or error code
 * from netCDF base function.
 * @internal
 * @author Ed Hartnett
 */
int finalize_handler(iosystem_desc_t *ios, int index)
{
    int iosysid;
    int mpierr;

    LOG((1, "finalize_handler called index = %d", index));
    assert(ios);

    /* Get the parameters for this function that the the comp master
     * task is broadcasting. */
    if ((mpierr = MPI_Bcast(&iosysid, 1, MPI_INT, 0, ios->intercomm)))
        return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
    LOG((1, "finalize_handler got parameter iosysid = %d", iosysid));

    /* Call the function. */
    PIOc_finalize(iosysid);

    LOG((1, "finalize_handler succeeded!"));
    return PIO_NOERR;
}

/**
 * This function is called by the IO tasks.  This function will not
 * return, unless there is an error.
 *
 * @param io_rank
 * @param component_count number of computation components
 * @param iosys pointer to pointer to iosystem info
 * @param io_comm MPI communicator for IO
 * @returns 0 for success, error code otherwise.
 * @author Ed Hartnett
 */
int pio_msg_handler2(int io_rank, int component_count, iosystem_desc_t **iosys,
                     MPI_Comm io_comm)
{
    iosystem_desc_t *my_iosys;
    int msg = 0;
    MPI_Request req[component_count];
    MPI_Status status;
    int index;
    int open_components = component_count;
    int finalize = 0;
    int mpierr;
    int ret = PIO_NOERR;

    LOG((1, "pio_msg_handler2 called"));
    assert(iosys);

    /* Have IO comm rank 0 (the ioroot) register to receive
     * (non-blocking) for a message from each of the comproots. */
    if (!io_rank)
    {
        for (int cmp = 0; cmp < component_count; cmp++)
        {
            my_iosys = iosys[cmp];
            LOG((1, "about to call MPI_Irecv union_comm = %d", my_iosys->union_comm));
            if ((mpierr = MPI_Irecv(&msg, 1, MPI_INT, my_iosys->comproot, MPI_ANY_TAG,
                                    my_iosys->union_comm, &req[cmp])))
                return check_mpi(NULL, mpierr, __FILE__, __LINE__);
            LOG((1, "MPI_Irecv req[%d] = %d", cmp, req[cmp]));
        }
    }

    /* Keep processing messages until loop is broken. */
    while (1)
    {
        LOG((3, "pio_msg_handler2 at top of loop"));

        /* Wait until any one of the requests are complete. Once it
         * returns, the Waitany function automatically sets the
         * appropriate member of the req array to MPI_REQUEST_NULL. */
        if (!io_rank)
        {
            LOG((1, "about to call MPI_Waitany req[0] = %d MPI_REQUEST_NULL = %d",
                 req[0], MPI_REQUEST_NULL));
            for (int c = 0; c < component_count; c++)
                LOG((3, "req[%d] = %d", c, req[c]));
            if ((mpierr = MPI_Waitany(component_count, req, &index, &status)))
                return check_mpi(NULL, mpierr, __FILE__, __LINE__);
            LOG((3, "Waitany returned index = %d req[%d] = %d", index, index, req[index]));
            for (int c = 0; c < component_count; c++)
                LOG((3, "req[%d] = %d", c, req[c]));
        }

        /* Broadcast the index of the computational component that
         * originated the request to the rest of the IO tasks. */
        LOG((3, "About to do Bcast of index = %d io_comm = %d", index, io_comm));
        if ((mpierr = MPI_Bcast(&index, 1, MPI_INT, 0, io_comm)))
            return check_mpi(NULL, mpierr, __FILE__, __LINE__);
        LOG((3, "index MPI_Bcast complete index = %d", index));

        /* Set the correct iosys depending on the index. */
        my_iosys = iosys[index];

        /* Broadcast the msg value to the rest of the IO tasks. */
        LOG((3, "about to call msg MPI_Bcast io_comm = %d", io_comm));
        if ((mpierr = MPI_Bcast(&msg, 1, MPI_INT, 0, io_comm)))
            return check_mpi(NULL, mpierr, __FILE__, __LINE__);
        LOG((1, "pio_msg_handler2 msg MPI_Bcast complete msg = %d", msg));

        /* Handle the message. This code is run on all IO tasks. */
        switch (msg)
        {
        case PIO_MSG_INQ_TYPE:
            ret = inq_type_handler(my_iosys);
            break;
        case PIO_MSG_INQ_FORMAT:
            ret = inq_format_handler(my_iosys);
            break;
        case PIO_MSG_CREATE_FILE:
            ret = create_file_handler(my_iosys);
            break;
        case PIO_MSG_SYNC:
            ret = sync_file_handler(my_iosys);
            break;
        case PIO_MSG_ENDDEF:
        case PIO_MSG_REDEF:
            ret = change_def_file_handler(my_iosys, msg);
            break;
        case PIO_MSG_OPEN_FILE:
            ret = open_file_handler(my_iosys);
            break;
        case PIO_MSG_CLOSE_FILE:
            ret = close_file_handler(my_iosys);
            break;
        case PIO_MSG_DELETE_FILE:
            ret = delete_file_handler(my_iosys);
            break;
        case PIO_MSG_RENAME_DIM:
            ret = rename_dim_handler(my_iosys);
            break;
        case PIO_MSG_RENAME_VAR:
            ret = rename_var_handler(my_iosys);
            break;
        case PIO_MSG_RENAME_ATT:
            ret = rename_att_handler(my_iosys);
            break;
        case PIO_MSG_DEL_ATT:
            ret = delete_att_handler(my_iosys);
            break;
        case PIO_MSG_DEF_DIM:
            ret = def_dim_handler(my_iosys);
            break;
        case PIO_MSG_DEF_VAR:
            ret = def_var_handler(my_iosys);
            break;
        case PIO_MSG_DEF_VAR_CHUNKING:
            ret = def_var_chunking_handler(my_iosys);
            break;
        case PIO_MSG_DEF_VAR_FILL:
            ret = def_var_fill_handler(my_iosys);
            break;
        case PIO_MSG_DEF_VAR_ENDIAN:
            ret = def_var_endian_handler(my_iosys);
            break;
        case PIO_MSG_DEF_VAR_DEFLATE:
            ret = def_var_deflate_handler(my_iosys);
            break;
        case PIO_MSG_INQ_VAR_ENDIAN:
            ret = inq_var_endian_handler(my_iosys);
            break;
        case PIO_MSG_SET_VAR_CHUNK_CACHE:
            ret = set_var_chunk_cache_handler(my_iosys);
            break;
        case PIO_MSG_GET_VAR_CHUNK_CACHE:
            ret = get_var_chunk_cache_handler(my_iosys);
            break;
        case PIO_MSG_INQ:
            ret = inq_handler(my_iosys);
            break;
        case PIO_MSG_INQ_UNLIMDIMS:
            ret = inq_unlimdims_handler(my_iosys);
            break;
        case PIO_MSG_INQ_DIM:
            ret = inq_dim_handler(my_iosys, msg);
            break;
        case PIO_MSG_INQ_DIMID:
            ret = inq_dimid_handler(my_iosys);
            break;
        case PIO_MSG_INQ_VAR:
            ret = inq_var_handler(my_iosys);
            break;
        case PIO_MSG_INQ_VAR_ALL:
            ret = inq_var_all_handler(my_iosys);
            break;
        case PIO_MSG_INQ_VAR_CHUNKING:
            ret = inq_var_chunking_handler(my_iosys);
            break;
        case PIO_MSG_INQ_VAR_FILL:
            ret = inq_var_fill_handler(my_iosys);
            break;
        case PIO_MSG_INQ_VAR_DEFLATE:
            ret = inq_var_deflate_handler(my_iosys);
            break;
        case PIO_MSG_GET_ATT:
            ret = att_get_handler(my_iosys);
            break;
        case PIO_MSG_PUT_ATT:
            ret = att_put_handler(my_iosys);
            break;
        case PIO_MSG_INQ_VARID:
            ret = inq_varid_handler(my_iosys);
            break;
        case PIO_MSG_INQ_ATT:
            ret = inq_att_handler(my_iosys);
            break;
        case PIO_MSG_INQ_ATTNAME:
            ret = inq_attname_handler(my_iosys);
            break;
        case PIO_MSG_INQ_ATTID:
            ret = inq_attid_handler(my_iosys);
            break;
        case PIO_MSG_GET_VARS:
            ret = get_vars_handler(my_iosys);
            break;
        case PIO_MSG_GET_VARA:
            ret = get_vara_handler(my_iosys);
            break;
        case PIO_MSG_PUT_VARA:
            ret = put_vara_handler(my_iosys);
            break;
        case PIO_MSG_PUT_VARS:
            ret = put_vars_handler(my_iosys);
            break;
        case PIO_MSG_INITDECOMP_DOF:
            ret = initdecomp_dof_handler(my_iosys);
            break;
        case PIO_MSG_WRITEDARRAYMULTI:
            ret = write_darray_multi_handler(my_iosys);
            break;
        case PIO_MSG_SETFRAME:
            ret = setframe_handler(my_iosys);
            break;
        case PIO_MSG_ADVANCEFRAME:
            ret = advanceframe_handler(my_iosys);
            break;
        case PIO_MSG_READDARRAY:
            ret = readdarray_handler(my_iosys);
            break;
        case PIO_MSG_SETERRORHANDLING:
            ret = seterrorhandling_handler(my_iosys);
            break;
        case PIO_MSG_SET_CHUNK_CACHE:
            ret = set_chunk_cache_handler(my_iosys);
            break;
        case PIO_MSG_GET_CHUNK_CACHE:
            ret = get_chunk_cache_handler(my_iosys);
            break;
        case PIO_MSG_FREEDECOMP:
            ret = freedecomp_handler(my_iosys);
            break;
        case PIO_MSG_SET_FILL:
            ret = set_fill_handler(my_iosys);
            break;
        case PIO_MSG_EXIT:
            finalize++;
            ret = finalize_handler(my_iosys, index);
            break;
        default:
            LOG((0, "unknown message received %d", msg));
            return PIO_EINVAL;
        }

        /* If an error was returned by the handler, exit. */
        LOG((3, "pio_msg_handler2 ret %d msg %d index %d io_rank %d", ret, msg, index, io_rank));
        if (ret)
            return pio_err(my_iosys, NULL, ret, __FILE__, __LINE__);            

        /* Listen for another msg from the component whose message we
         * just handled. */
        if (!io_rank && !finalize)
        {
            my_iosys = iosys[index];
            LOG((3, "pio_msg_handler2 about to Irecv index = %d comproot = %d union_comm = %d",
                 index, my_iosys->comproot, my_iosys->union_comm));
            if ((mpierr = MPI_Irecv(&msg, 1, MPI_INT, my_iosys->comproot, MPI_ANY_TAG, my_iosys->union_comm,
                                    &req[index])))
                return check_mpi(NULL, mpierr, __FILE__, __LINE__);
            LOG((3, "pio_msg_handler2 called MPI_Irecv req[%d] = %d", index, req[index]));
        }

        LOG((3, "pio_msg_handler2 done msg = %d open_components = %d",
             msg, open_components));

        /* If there are no more open components, exit. */
        if (finalize)
        {
            if (--open_components)
                finalize = 0;
            else
                break;
        }
    }

    LOG((3, "returning from pio_msg_handler2"));
    return PIO_NOERR;
}
