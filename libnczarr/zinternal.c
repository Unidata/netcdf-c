/* Copyright 2003-2018, University Corporation for Atmospheric
 * Research. See the COPYRIGHT file for copying and redistribution
 * conditions.
 */

/**
 * @file @internal Internal netcdf-4 functions.
 *
 * This file contains functions internal to the netcdf4 library. None of
 * the functions in this file are exposed in the exetnal API. These
 * functions all relate to the manipulation of netcdf-4's in-memory
 * buffer of metadata information, i.e. the linked list of NC
 * structs.
 *
 * @author Dennis Heimbigner, Ed Hartnett
 */

#include "zincludes.h"
#include "zfilter.h"
#include "zfill.h"

/* Forward */

#ifdef LOGGING
/* This is the severity level of messages which will be logged. Use
   severity 0 for errors, 1 for important log messages, 2 for less
   important, etc. */
extern int nc_log_level;
#endif /* LOGGING */

const NCproplist* NCplistzarrv2 = NULL;
const NCproplist* NCplistzarrv3 = NULL;

#ifdef LOOK
/**
 * @internal Provide a wrapper for H5Eset_auto
 * @param func Pointer to func.
 * @param client_data Client data.
 *
 * @return 0 for success
 */
static herr_t
set_auto(void* func, void *client_data)
{
#ifdef DEBUGH5
    return H5Eset_auto2(H5E_DEFAULT,(H5E_auto2_t)h5catch,client_data);
#else
    return H5Eset_auto2(H5E_DEFAULT,(H5E_auto2_t)func,client_data);
#endif
}
#endif

/**
 * @internal Provide a function to do any necessary initialization of
 * the ZARR library.
 */
int
NCZ_initialize_internal(void)
{
    int stat = NC_NOERR;
    char* dimsep = NULL;
    NCglobalstate* ngs = NULL;

    ncz_initialized = 1;
    ngs = NC_getglobalstate();
    if(ngs != NULL) {
        /* Defaults */
	ngs->zarr.default_zarrformat = DFALTZARRFORMAT;
	/* Allow .rc file override */
        dimsep = NC_rclookup("ZARR.DIMENSION_SEPARATOR",NULL,NULL);
        if(dimsep != NULL) {
            /* Verify its value */
	    if(dimsep != NULL && strlen(dimsep) == 1 && islegaldimsep(dimsep[0]))
		ngs->zarr.dimension_separator = dimsep[0];
        }    
	/* Allow environment variable override */
        if(getenv(NCZARRDEFAULTFORMAT) != NULL) { 
	    int dfalt = 0;
	    sscanf(getenv(NCZARRDEFAULTFORMAT),"%d",&dfalt);
	    if(dfalt == 2 || dfalt == 3)
	        ngs->zarr.default_zarrformat = dfalt;
	}
	if(ngs->zarr.default_zarrformat == 3)
	    ngs->zarr.dimension_separator = DFALT_DIM_SEPARATOR_V3;
	else
	    ngs->zarr.dimension_separator = DFALT_DIM_SEPARATOR_V2;
    }
    /* Build some common proplists */
    NCplistzarrv2 = ncproplistnew();
    ncproplistadd((NCproplist*)NCplistzarrv2,"zarrformat",(uintptr_t)2);
    NCplistzarrv3 = ncproplistnew();
    ncproplistadd((NCproplist*)NCplistzarrv3,"zarrformat",(uintptr_t)3);

    return stat;
}

/**
 * @internal Provide a function to do any necessary finalization of
 * the ZARR library.
 */
int
NCZ_finalize_internal(void)
{
    /* Reclaim global resources */
    ncz_initialized = 0;
#ifdef NETCDF_ENABLE_NCZARR_FILTERS
    NCZ_filter_finalize();
#endif
#ifdef NETCDF_ENABLE_S3
    NCZ_s3finalize();
#endif

    /* Cleanup common proplists */
    ncproplistfree((NCproplist*)NCplistzarrv2); NCplistzarrv2 = NULL;
    ncproplistfree((NCproplist*)NCplistzarrv3); NCplistzarrv3 = NULL;

    return NC_NOERR;
}

/**
 * @internal Given a varid, return the maximum length of a dimension
 * using dimid.
 *
 * @param grp Pointer to group info struct.
 * @param varid Variable ID.
 * @param dimid Dimension ID.
 * @param maxlen Pointer that gets the max length.
 *
 * @return ::NC_NOERR No error.
 * @author Dennis Heimbigner, Ed Hartnett
 */
static int
find_var_dim_max_length(NC_GRP_INFO_T *grp, int varid, int dimid, size_t *maxlen)
{
    NC_VAR_INFO_T *var;
    int retval = NC_NOERR;

    NC_UNUSED(dimid);

    *maxlen = 0;

    /* Find this var. */
    var = (NC_VAR_INFO_T*)ncindexith(grp->vars,(size_t)varid);
    if (!var) return NC_ENOTVAR;
    assert(var->hdr.id == varid);

    /* If the var hasn't been created yet, its size is 0. */
    if (!var->created)
    {
        *maxlen = 0;
    }
    else
    {
        /* Get the number of records in the dataset. */
    }
    return retval;
}

/**
 * @internal Find the actual length of a dim by checking the length of
 * that dim in all variables that use it, in grp or children. **len
 * must be initialized to zero before this function is called.
 *
 * @param grp Pointer to group info struct.
 * @param dimid Dimension ID.
 * @param len Pointer to pointer that gets length.
 *
 * @return ::NC_NOERR No error.
 * @author Dennis Heimbigner, Ed Hartnett
 */
int
ncz_find_dim_len(NC_GRP_INFO_T *grp, int dimid, size_t **len)
{
    NC_VAR_INFO_T *var;
    int retval;
    size_t i;

    assert(grp && len);
    LOG((3, "%s: grp->name %s dimid %d", __func__, grp->hdr.name, dimid));

    /* If there are any groups, call this function recursively on
     * them. */
    for (i = 0; i < ncindexsize(grp->children); i++) {
        if ((retval = ncz_find_dim_len((NC_GRP_INFO_T*)ncindexith(grp->children, i),
                                       dimid, len)))
            return retval;
    }
    /* For all variables in this group, find the ones that use this
     * dimension, and remember the max length. */
    for (i = 0; i < ncindexsize(grp->vars); i++)
    {
        size_t mylen;
        var = (NC_VAR_INFO_T *)ncindexith(grp->vars, i);
        assert(var);

        /* Find max length of dim in this variable... */
        if ((retval = find_var_dim_max_length(grp, var->hdr.id, dimid, &mylen)))
            return retval;

        **len = **len > mylen ? **len : mylen;
    }

    return NC_NOERR;
}


/**
 * @internal Given an ncid, varid, and attribute name, return
 * normalized name and (optionally) pointers to the file, group,
 * var, and att info structs.
 * Lazy reads of attributes and variable metadata are done as needed.
 *
 * @param ncid File/group ID.
 * @param varid Variable ID.
 * @param name Name to of attribute.
 * @param attnum Number of attribute.
 * @param use_name If true, use the name to get the
 * attribute. Otherwise use the attnum.
 * @param norm_name Pointer to storage of size NC_MAX_NAME + 1,
 * which will get the normalized name, if use_name is true. Ignored if
 * NULL.
 * @param h5 Pointer to pointer that gets file info struct. Ignored if
 * NULL.
 * @param grp Pointer to pointer that gets group info struct. Ignored
 * if NULL.
 * @param h5 Pointer to pointer that gets variable info
 * struct. Ignored if NULL.
 * @param att Pointer to pointer that gets attribute info
 * struct. Ignored if NULL.
 *
 * @return ::NC_NOERR No error.
 * @return ::NC_EBADID Bad ncid.
 * @return ::NC_ENOTVAR Variable not found.
 * @return ::NC_ENOTATT Attribute not found.
 * @author Dennis Heimbigner, Ed Hartnett
 */
int
ncz_find_grp_var_att(int ncid, int varid, const char *name, int attnum,
                          int use_name, char *norm_name, NC_FILE_INFO_T **h5,
                          NC_GRP_INFO_T **grp, NC_VAR_INFO_T **var,
                          NC_ATT_INFO_T **att)
{
    NC_FILE_INFO_T *my_h5;
    NC_GRP_INFO_T *my_grp;
    NC_VAR_INFO_T *my_var = NULL;
    NC_ATT_INFO_T *my_att;
    char my_norm_name[NC_MAX_NAME + 1] = "";
    NCindex *attlist = NULL;
    int retval;

    LOG((4, "%s: ncid %d varid %d attnum %d use_name %d", __func__, ncid, varid,
         attnum, use_name));

    /* Don't need to provide name unless getting att pointer and using
     * use_name. */
    assert(!att || ((use_name && name) || !use_name));

    /* Find info for this file, group, and h5 info. */
    if ((retval = nc4_find_nc_grp_h5(ncid, NULL, &my_grp, &my_h5)))
        return retval;
    assert(my_grp && my_h5);

    /* Read the attributes for this var, if any */
    switch (retval = ncz_getattlist(my_grp, varid, &my_var, &attlist)) {
    case NC_NOERR: assert(attlist); break;
    case NC_ENOOBJECT: retval = NC_NOERR; attlist = NULL; break; /* variable has no attributes */
    default: return retval; /* significant error */
    }

    /* Need a name if use_name is true. */
    if (use_name && !name)
        return NC_EBADNAME;

    /* Normalize the name. */
    if (use_name)
        if ((retval = nc4_normalize_name(name, my_norm_name)))
            return retval;

    /* Now find the attribute by name or number. */
    if (att)
    {
        my_att = use_name ? (NC_ATT_INFO_T *)ncindexlookup(attlist, my_norm_name) :
            (NC_ATT_INFO_T *)ncindexith(attlist, (size_t)attnum);
        if (!my_att)
            return NC_ENOTATT;
    }

    /* Give the people what they want. */
    if (norm_name)
        strncpy(norm_name, my_norm_name, NC_MAX_NAME);
    if (h5)
        *h5 = my_h5;
    if (grp)
        *grp = my_grp;
    if (var)
        *var = my_var;
    if (att)
        *att = my_att;

    return retval;
}

/**
 * @internal Ensure that either var->no_fill == NC_NOFILL || var->fill_value != NULL.
 * Side effects: set as default if necessary and build _FillValue attribute.
 *
 * @param h5 Pointer to file info struct.
 * @param var Pointer to variable info struct.
 *
 * @returns NC_NOERR No error.
 * @returns NC_ENOMEM Out of memory.
 * @author Ed Hartnett, Dennis Heimbigner
 */
int
NCZ_ensure_fill_value(NC_VAR_INFO_T *var)
{
    int stat = NC_NOERR;

    if(var->no_fill == NC_NOFILL)
        return NC_NOERR;

    /* If the user has set a fill_value for this var, use, otherwise find the default fill value. */
    if(var->fill_value == NULL) {
	NC_FILE_INFO_T* file = var->container->nc4_info;
	nc_type vartid = var->type_info->hdr.id;
	void* dfalt = NCZ_getdfaltfillvalue(vartid);
	if((stat = NCZ_set_dual_obj_data(file,(NC_OBJ*)var,NC_FillValue,DA_FILLVALUE,1,dfalt))) goto done;
	/* synchronize to attribute */
	if((stat = NCZ_sync_dual_att(file,(NC_OBJ*)var,NC_FillValue,DA_FILLVALUE,FIXATT))) goto done;
    }
    assert(var->fill_value != NULL);

    LOG((4, "Found a fill value for var %s", var->hdr.name));
done:
    return THROW(stat);
}

#ifdef LOGGING
/* We will need to check against nc log level from nc4internal.c. */
extern int nc_log_level;

/**
 * @internal This is like nc_set_log_level(), but will also turn on
 * ZARR internal logging, in addition to netCDF logging. This should
 * never be called by the user. It is called in open/create when the
 * nc logging level has changed.
 *
 * @return ::NC_NOERR No error.
 * @author Dennis Heimbigner, Ed Hartnett
 */
int
NCZ_set_log_level()
{
    /* If the user wants to completely turn off logging, turn off NCZ
       logging too. Now I truely can't think of what to do if this
       fails, so just ignore the return code. */
    if (nc_log_level == NC_TURN_OFF_LOGGING)
    {
#ifdef LOOK
        set_auto(NULL, NULL);
#endif
        LOG((1, "NCZ error messages turned off!"));
    }
    else
    {
#ifdef LOOK
        if (set_auto((H5E_auto_t)&H5Eprint1, stderr) < 0)
            LOG((0, "H5Eset_auto failed!"));
#endif
        LOG((1, "NCZ error messages turned on."));
    }

    return NC_NOERR;
}
#endif /* LOGGING */

/**
 * @internal Get the format (i.e. NC_FORMAT_NETCDF4 pr
 * NC_FORMAT_NETCDF4_CLASSIC) of an open netCDF-4 file.
 *
 * @param ncid File ID (ignored).
 * @param formatp Pointer that gets the constant indicating format.

 * @return ::NC_NOERR No error.
 * @return ::NC_EBADID Bad ncid.
 * @author Ed Hartnett
 */
int
NCZ_inq_format(int ncid, int *formatp)
{
    int stat = NC_NOERR;

    ZTRACE(0,"ncid=%d formatp=%p",ncid,formatp);
    stat = NC4_inq_format(ncid,formatp);
    return ZUNTRACEX(stat,"formatp=%d",(formatp?-1:*formatp));
}

/**
 * @internal Return the extended format (i.e. the dispatch model),
 * plus the mode associated with an open file.
 *
 * @param ncid File ID (ignored).
 * @param formatp a pointer that gets the extended format. Note that
 * this is not the same as the format provided by nc_inq_format(). The
 * extended foramt indicates the dispatch layer model. NetCDF-4 files
 * will always get NC_FORMATX_NC4.
 * @param modep a pointer that gets the open/create mode associated with
 * this file. Ignored if NULL.

 * @return ::NC_NOERR No error.
 * @return ::NC_EBADID Bad ncid.
 * @author Dennis Heimbigner
 */
int
NCZ_inq_format_extended(int ncid, int *formatp, int *modep)
{
    NC *nc;
    int retval;

    LOG((2, "%s: ncid 0x%x", __func__, ncid));

    if ((retval = nc4_find_nc_grp_h5(ncid, &nc, NULL, NULL)))
        return NC_EBADID;

    if(modep)
        *modep = nc->mode|NC_NETCDF4;

    if (formatp)
        *formatp = NC_FORMATX_NCZARR;

    return NC_NOERR;
}


void
zdfaltstrlen(size_t* p, size_t strlen)
{
    *p = strlen;
}

void
zmaxstrlen(size_t* p, size_t strlen)
{
    *p = strlen;
}


void
zsetmaxstrlen(size_t maxstrlen, NC_VAR_INFO_T* var)
{
    NCZ_VAR_INFO_T* zvar = (NCZ_VAR_INFO_T*)var->format_var_info;
    zvar->maxstrlen = maxstrlen;
    if(zvar->maxstrlen == 0) zvar->maxstrlen = NCZ_get_maxstrlen((NC_OBJ*)var);
}

void
zsetdfaltstrlen(size_t dfaltstrlen, NC_FILE_INFO_T* file)
{
    NCZ_FILE_INFO_T* zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    zfile->default_maxstrlen = dfaltstrlen;
    if(zfile->default_maxstrlen == 0) zfile->default_maxstrlen = NCZ_MAXSTR_DFALT;
}

void
zsetdimsep(char sep, NC_VAR_INFO_T* var)
{
    NCZ_VAR_INFO_T* zvar = (NCZ_VAR_INFO_T*)var->format_var_info;
    zvar->dimension_separator = sep;
    if(zvar->dimension_separator == '\0') zvar->dimension_separator = NCZ_get_dimsep(var);
}

void
zsetdfaltdimsep(char dimsep, NC_FILE_INFO_T* file)
{
    NCglobalstate* gs = NC_getglobalstate();
    NC_UNUSED(file);
    assert(gs != NULL);
    assert(gs->zarr.dimension_separator != '\0');
    gs->zarr.dimension_separator = dimsep;
    assert(gs->zarr.dimension_separator != '\0');
}
