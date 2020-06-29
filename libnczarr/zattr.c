/* Copyright 2003-2018, University Corporation for Atmospheric
 * Research. See COPYRIGHT file for copying and redistribution
 * conditions. */

/**
 * @file
 * @internal This file handles ZARR attributes.
 *
 * @author Dennis Heimbigner, Ed Hartnett
 */

#include "zincludes.h"

#undef ADEBUG

/**
 * @internal Get the attribute list for either a varid or NC_GLOBAL
 *
 * @param grp Group
 * @param varid Variable ID | NC_BLOGAL
 * @param varp Pointer that gets pointer to NC_VAR_INFO_T
 * instance. Ignored if NULL.
 * @param attlist Pointer that gets pointer to attribute list.
 *
 * @return NC_NOERR No error.
 * @author Dennis Heimbigner, Ed Hartnett
 * [Candidate for moving to libsrc4]
 */
int
ncz_getattlist(NC_GRP_INFO_T *grp, int varid, NC_VAR_INFO_T **varp, NCindex **attlist)
{
    int retval;
    NC_FILE_INFO_T* file = grp->nc4_info;
    NCZ_FILE_INFO_T* zinfo = file->format_file_info;

    assert(grp && attlist && file && zinfo);

    if (varid == NC_GLOBAL)
    {
        /* Do we need to read the atts? */
        if (!grp->atts_read)
            if ((retval = ncz_read_atts(file, (NC_OBJ*)grp)))
                return retval;

        if (varp)
            *varp = NULL;
        *attlist = grp->att;
    }
    else
    {
        NC_VAR_INFO_T *var;

        if (!(var = (NC_VAR_INFO_T *)ncindexith(grp->vars, varid)))
            return NC_ENOTVAR;
        assert(var->hdr.id == varid);

        /* Do we need to read the atts? */
        if (!var->atts_read)
            if ((retval = ncz_read_atts(file, (NC_OBJ*)var)))
                return retval;

        if (varp)
            *varp = var;
        *attlist = var->att;
    }
    return NC_NOERR;
}

/**
 * @internal Get one of three special attributes, NCPROPS,
 * ISNETCDF4ATT, and SUPERBLOCKATT. These atts are not all really in
 * the file, they are constructed on the fly.
 *
 * @param h5 Pointer to ZARR file info struct.
 * @param name Name of attribute.
 * @param filetypep Pointer that gets type of the attribute data in
 * file.
 * @param mem_type Type of attribute data in memory.
 * @param lenp Pointer that gets length of attribute array.
 * @param attnump Pointer that gets the attribute number.
 * @param data Attribute data.
 *
 * @return ::NC_NOERR No error.
 * @return ::NC_EBADID Bad ncid.
 * @return ::NC_ERANGE Data conversion out of range.
 * @author Dennis Heimbigner
 */
int
ncz_get_att_special(NC_FILE_INFO_T* h5, const char* name,
                    nc_type* filetypep, nc_type mem_type, size_t* lenp,
                    int* attnump, void* data)
{
    /* Fail if asking for att id */
    if(attnump)
        return NC_EATTMETA;

    if(strcmp(name,NCPROPS)==0) {
        int len;
        if(h5->provenance.ncproperties == NULL)
            return NC_ENOTATT;
        if(mem_type == NC_NAT) mem_type = NC_CHAR;
        if(mem_type != NC_CHAR)
            return NC_ECHAR;
        if(filetypep) *filetypep = NC_CHAR;
	len = strlen(h5->provenance.ncproperties);
        if(lenp) *lenp = len;
        if(data) strncpy((char*)data,h5->provenance.ncproperties,len+1);
    } else if(strcmp(name,ISNETCDF4ATT)==0
              || strcmp(name,SUPERBLOCKATT)==0) {
        unsigned long long iv = 0;
        if(filetypep) *filetypep = NC_INT;
        if(lenp) *lenp = 1;
        if(strcmp(name,SUPERBLOCKATT)==0)
            iv = (unsigned long long)h5->provenance.superblockversion;
        else /* strcmp(name,ISNETCDF4ATT)==0 */
            iv = NCZ_isnetcdf4(h5);
        if(mem_type == NC_NAT) mem_type = NC_INT;
        if(data)
            switch (mem_type) {
            case NC_BYTE: *((char*)data) = (char)iv; break;
            case NC_SHORT: *((short*)data) = (short)iv; break;
            case NC_INT: *((int*)data) = (int)iv; break;
            case NC_UBYTE: *((unsigned char*)data) = (unsigned char)iv; break;
            case NC_USHORT: *((unsigned short*)data) = (unsigned short)iv; break;
            case NC_UINT: *((unsigned int*)data) = (unsigned int)iv; break;
            case NC_INT64: *((long long*)data) = (long long)iv; break;
            case NC_UINT64: *((unsigned long long*)data) = (unsigned long long)iv; break;
            default:
                return NC_ERANGE;
            }
    }
    return NC_NOERR;
}

/**
 * @internal I think all atts should be named the exact same thing, to
 * avoid confusion!
 *
 * @param ncid File and group ID.
 * @param varid Variable ID.
 * @param name Name of attribute.
 * @param newname New name for attribute.
 *
 * @return ::NC_NOERR No error.
 * @return ::NC_EBADID Bad ncid.
 * @return ::NC_EMAXNAME New name too long.
 * @return ::NC_EPERM File is read-only.
 * @return ::NC_ENAMEINUSE New name already in use.
 * @return ::NC_ENOTINDEFINE Classic model file not in define mode.
 * @return ::NC_EHDFERR HDF error.
 * @return ::NC_ENOMEM Out of memory.
 * @return ::NC_EINTERNAL Could not rebuild list.
 * @author Dennis Heimbigner, Ed Hartnett
 */
int
NCZ_rename_att(int ncid, int varid, const char *name, const char *newname)
{
    NC_GRP_INFO_T *grp;
    NC_FILE_INFO_T *h5;
    NC_VAR_INFO_T *var = NULL;
    NC_ATT_INFO_T *att;
    NCindex *list;
    char norm_newname[NC_MAX_NAME + 1], norm_name[NC_MAX_NAME + 1];
    int retval = NC_NOERR;

    if (!name || !newname)
        return NC_EINVAL;

    LOG((2, "nc_rename_att: ncid 0x%x varid %d name %s newname %s",
         ncid, varid, name, newname));

    /* If the new name is too long, that's an error. */
    if (strlen(newname) > NC_MAX_NAME)
        return NC_EMAXNAME;

    /* Find info for this file, group, and h5 info. */
    if ((retval = nc4_find_grp_h5(ncid, &grp, &h5)))
        return retval;
    assert(h5 && grp);

    /* If the file is read-only, return an error. */
    if (h5->no_write)
        return NC_EPERM;

    /* Check and normalize the name. */
    if ((retval = nc4_check_name(newname, norm_newname)))
        return retval;

    /* Get the list of attributes. */
    if ((retval = ncz_getattlist(grp, varid, &var, &list)))
        return retval;

    /* Is new name in use? */
    att = (NC_ATT_INFO_T*)ncindexlookup(list,norm_newname);
    if(att != NULL)
        return NC_ENAMEINUSE;

    /* Normalize name and find the attribute. */
    if ((retval = nc4_normalize_name(name, norm_name)))
        return retval;

    att = (NC_ATT_INFO_T*)ncindexlookup(list,norm_name);
    if (!att)
        return NC_ENOTATT;

    /* If we're not in define mode, new name must be of equal or
       less size, if complying with strict NC3 rules. */
    if (!(h5->flags & NC_INDEF) && strlen(norm_newname) > strlen(att->hdr.name) &&
        (h5->cmode & NC_CLASSIC_MODEL))
        return NC_ENOTINDEFINE;

    /* Copy the new name into our metadata. */
    if(att->hdr.name) free(att->hdr.name);
    if (!(att->hdr.name = strdup(norm_newname)))
        return NC_ENOMEM;
    att->hdr.hashkey = NC_hashmapkey(att->hdr.name,strlen(att->hdr.name)); /* Fix hash key */

    att->dirty = NC_TRUE;

    /* Rehash the attribute list so that the new name is used */
    if(!ncindexrebuild(list))
        return NC_EINTERNAL;

    /* Mark attributes on variable dirty, so they get written */
    if(var)
        var->attr_dirty = NC_TRUE;
    return retval;
}

/**
 * @internal Delete an att. Rub it out. Push the button on
 * it. Liquidate it. Bump it off. Take it for a one-way
 * ride. Terminate it.
 *
 * @param ncid File and group ID.
 * @param varid Variable ID.
 * @param name Name of attribute to delete.
 *
 * @return ::NC_NOERR No error.
 * @return ::NC_EBADID Bad ncid.
 * @return ::NC_ENOTATT Attribute not found.
 * @return ::NC_EINVAL No name provided.
 * @return ::NC_EPERM File is read only.
 * @return ::NC_ENOTINDEFINE Classic model not in define mode.
 * @return ::NC_EINTERNAL Could not rebuild list.
 * @author Dennis Heimbigner, Ed Hartnett
 */
int
NCZ_del_att(int ncid, int varid, const char *name)
{
    NC_GRP_INFO_T *grp;
    NC_VAR_INFO_T *var;
    NC_FILE_INFO_T *h5;
    NC_ATT_INFO_T *att;
    NCindex* attlist = NULL;
    int i;
    size_t deletedid;
    int retval;

    /* Name must be provided. */
    if (!name)
        return NC_EINVAL;

    LOG((2, "nc_del_att: ncid 0x%x varid %d name %s", ncid, varid, name));

    /* Find info for this file, group, and h5 info. */
    if ((retval = nc4_find_grp_h5(ncid, &grp, &h5)))
        return retval;
    assert(h5 && grp);

    /* If the file is read-only, return an error. */
    if (h5->no_write)
        return NC_EPERM;

    /* If file is not in define mode, return error for classic model
     * files, otherwise switch to define mode. */
    if (!(h5->flags & NC_INDEF))
    {
        if (h5->cmode & NC_CLASSIC_MODEL)
            return NC_ENOTINDEFINE;
        if ((retval = NCZ_redef(ncid)))
            return retval;
    }

    /* Get either the global or a variable attribute list. */
    if ((retval = ncz_getattlist(grp, varid, &var, &attlist)))
        return retval;

#ifdef LOOK
    /* Determine the location id in the ZARR file. */
    if (varid == NC_GLOBAL)
        locid = ((NCZ_GRP_INFO_T *)(grp->format_grp_info))->hdf_grpid;
    else if (var->created)
        locid = ((NCZ_VAR_INFO_T *)(var->format_var_info))->hdf_datasetid;
#endif

    /* Now find the attribute by name. */
    if (!(att = (NC_ATT_INFO_T*)ncindexlookup(attlist, name)))
        return NC_ENOTATT;

    /* Delete it from the ZARR file, if it's been created. */
    if (att->created)
    {
#ifdef LOOK
        assert(locid);
        if (H5Adelete(locid, att->hdr.name) < 0)
            return NC_EATTMETA;
#endif
    }

    deletedid = att->hdr.id;

    /* Remove this attribute in this list */
    if ((retval = nc4_att_list_del(attlist, att)))
        return retval;

    /* Renumber all attributes with higher indices. */
    for (i = 0; i < ncindexsize(attlist); i++)
    {
        NC_ATT_INFO_T *a;
        if (!(a = (NC_ATT_INFO_T *)ncindexith(attlist, i)))
            continue;
        if (a->hdr.id > deletedid)
            a->hdr.id--;
    }

    /* Rebuild the index. */
    if (!ncindexrebuild(attlist))
        return NC_EINTERNAL;

    return NC_NOERR;
}

/**
 * @internal This will return the length of a netcdf atomic data type
 * in bytes.
 *
 * @param type A netcdf atomic type.
 *
 * @return Type size in bytes, or -1 if type not found.
 * @author Dennis Heimbigner, Ed Hartnett
 */
static int
nc4typelen(nc_type type)
{
    switch(type){
    case NC_BYTE:
    case NC_CHAR:
    case NC_UBYTE:
        return 1;
    case NC_USHORT:
    case NC_SHORT:
        return 2;
    case NC_FLOAT:
    case NC_INT:
    case NC_UINT:
        return 4;
    case NC_DOUBLE:
    case NC_INT64:
    case NC_UINT64:
        return 8;
    }
    return -1;
}

/**
 * @internal
 * Write an attribute to a netCDF-4/NCZ file, converting
 * data type if necessary.
 *
 * @param ncid File and group ID.
 * @param varid Variable ID.
 * @param name Name of attribute.
 * @param file_type Type of the attribute data in file.
 * @param len Number of elements in attribute array.
 * @param data Attribute data.
 * @param mem_type Type of data in memory.
 * @param force write even if the attribute is special
 *
 * @return ::NC_NOERR No error.
 * @return ::NC_EINVAL Invalid parameters.
 * @return ::NC_EBADID Bad ncid.
 * @return ::NC_ENOTVAR Variable not found.
 * @return ::NC_EBADNAME Name contains illegal characters.
 * @return ::NC_ENAMEINUSE Name already in use.
 * @author Dennis Heimbigner, Ed Hartnett
 */
int
ncz_put_att(NC_GRP_INFO_T* grp, int varid, const char *name, nc_type file_type,
            size_t len, const void *data, nc_type mem_type, int force)
{
    NC* nc;
    NC_FILE_INFO_T *h5;
    NC_VAR_INFO_T *var = NULL;
    NCindex* attlist = NULL;
    NC_ATT_INFO_T* att;
    char norm_name[NC_MAX_NAME + 1];
    nc_bool_t new_att = NC_FALSE;
    int retval = NC_NOERR, range_error = 0;
    size_t type_size;
    int i;
    int ret;
    int ncid;

    h5 = grp->nc4_info;
    nc = h5->controller;
    assert(nc && grp && h5);

    ncid = nc->ext_ncid | grp->hdr.id;

    /* Find att, if it exists. (Must check varid first or nc_test will
     * break.) This also does lazy att reads if needed. */
    if ((ret = ncz_getattlist(grp, varid, &var, &attlist)))
        return ret;

    /* The length needs to be positive (cast needed for braindead
       systems with signed size_t). */
    if((unsigned long) len > X_INT_MAX)
        return NC_EINVAL;

    /* Check name before LOG statement. */
    if (!name || strlen(name) > NC_MAX_NAME)
        return NC_EBADNAME;

    LOG((1, "%s: ncid 0x%x varid %d name %s file_type %d mem_type %d len %d",
         __func__,ncid, varid, name, file_type, mem_type, len));

    /* If len is not zero, then there must be some data. */
    if (len && !data)
        return NC_EINVAL;

    /* If the file is read-only, return an error. */
    if (h5->no_write)
        return NC_EPERM;

    /* Check and normalize the name. */
    if ((retval = nc4_check_name(name, norm_name)))
        return retval;

    /* Check that a reserved att name is not being used improperly */
    const NC_reservedatt* ra = NC_findreserved(name);
    if(ra != NULL && !force) {
        /* case 1: grp=root, varid==NC_GLOBAL, flags & READONLYFLAG */
        if (nc->ext_ncid == ncid && varid == NC_GLOBAL && grp->parent == NULL
            && (ra->flags & READONLYFLAG))
            return NC_ENAMEINUSE;
        /* case 2: grp=NA, varid!=NC_GLOBAL, flags & DIMSCALEFLAG */
        if (varid != NC_GLOBAL && (ra->flags & DIMSCALEFLAG))
            return NC_ENAMEINUSE;
    }

    /* See if there is already an attribute with this name. */
    att = (NC_ATT_INFO_T*)ncindexlookup(attlist,norm_name);

    if (!att)
    {
        /* If this is a new att, require define mode. */
        if (!(h5->flags & NC_INDEF))
        {

            if (h5->cmode & NC_CLASSIC_MODEL)
                return NC_ENOTINDEFINE;
            if ((retval = NCZ_redef(ncid)))
                BAIL(retval);
        }
        new_att = NC_TRUE;
    }
    else
    {
        /* For an existing att, if we're not in define mode, the len
           must not be greater than the existing len for classic model. */
        if (!(h5->flags & NC_INDEF) &&
            len * nc4typelen(file_type) > (size_t)att->len * nc4typelen(att->nc_typeid))
        {
            if (h5->cmode & NC_CLASSIC_MODEL)
                return NC_ENOTINDEFINE;
            if ((retval = NCZ_redef(ncid)))
                BAIL(retval);
        }
    }

    /* We must have two valid types to continue. */
    if (file_type == NC_NAT || mem_type == NC_NAT)
        return NC_EBADTYPE;

    /* Get information about this type. */
    if ((retval = nc4_get_typelen_mem(h5, file_type, &type_size)))
        return retval;

    /* No character conversions are allowed. */
    if (file_type != mem_type &&
        (file_type == NC_CHAR || mem_type == NC_CHAR ||
         file_type == NC_STRING || mem_type == NC_STRING))
        return NC_ECHAR;

    /* For classic mode file, only allow atts with classic types to be
     * created. */
    if (h5->cmode & NC_CLASSIC_MODEL && file_type > NC_DOUBLE)
        return NC_ESTRICTNC3;

    /* Add to the end of the attribute list, if this att doesn't
       already exist. */
    if (new_att)
    {
        LOG((3, "adding attribute %s to the list...", norm_name));
        if ((ret = nc4_att_list_add(attlist, norm_name, &att)))
            BAIL(ret);

        /* Allocate storage for the ZARR specific att info. */
        if (!(att->format_att_info = calloc(1, sizeof(NCZ_ATT_INFO_T))))
            BAIL(NC_ENOMEM);
    }

    /* Now fill in the metadata. */
    att->dirty = NC_TRUE;
    att->nc_typeid = file_type;

    /* If this att has vlen or string data, release it before we lose the length value. */
    if (att->stdata)
    {
        for (i = 0; i < att->len; i++)
            if(att->stdata[i])
                free(att->stdata[i]);
        free(att->stdata);
        att->stdata = NULL;
    }
    if (att->vldata)
    {
        for (i = 0; i < att->len; i++)
            nc_free_vlen(&att->vldata[i]); /* FIX: see warning of nc_free_vlen */
        free(att->vldata);
        att->vldata = NULL;
    }

    att->len = len;

    /* If this is the _FillValue attribute, then we will also have to
     * copy the value to the fill_vlue pointer of the NC_VAR_INFO_T
     * struct for this var. (But ignore a global _FillValue
     * attribute). */
    if (!strcmp(att->hdr.name, _FillValue) && varid != NC_GLOBAL)
    {
        int size;

        /* Fill value must be same type and have exactly one value */
        if (att->nc_typeid != var->type_info->hdr.id)
            return NC_EBADTYPE;
        if (att->len != 1)
            return NC_EINVAL;

        /* If we already wrote to the dataset, then return an error. */
        if (var->written_to)
            return NC_ELATEFILL;

        /* Get the length of the veriable data type. */
        if ((retval = nc4_get_typelen_mem(grp->nc4_info, var->type_info->hdr.id,
                                          &type_size)))
            return retval;

        /* Already set a fill value? Now I'll have to free the old
         * one. Make up your damn mind, would you? */
        if (var->fill_value)
        {
            if (var->type_info->nc_type_class == NC_VLEN)
            {
                if ((retval = nc_free_vlen(var->fill_value)))
                    return retval;
            }
            else if (var->type_info->nc_type_class == NC_STRING)
            {
                if (*(char **)var->fill_value)
                    free(*(char **)var->fill_value);
            }
            free(var->fill_value);
        }

#ifdef LOOK
        /* Determine the size of the fill value in bytes. */
        if (var->type_info->nc_type_class == NC_VLEN)
            size = sizeof(hvl_t);
        else
#endif
	if (var->type_info->nc_type_class == NC_STRING)
            size = sizeof(char *);
        else
            size = type_size;

        /* Allocate space for the fill value. */
        if (!(var->fill_value = calloc(1, size)))
            return NC_ENOMEM;

        /* Copy the fill_value. */
        LOG((4, "Copying fill value into metadata for variable %s", var->hdr.name));
        if (var->type_info->nc_type_class == NC_VLEN)
        {
            nc_vlen_t *in_vlen = (nc_vlen_t *)data, *fv_vlen = (nc_vlen_t *)(var->fill_value);
            NC_TYPE_INFO_T* basetype;
            size_t basetypesize = 0;

            /* get the basetype and its size */
            basetype = var->type_info;
            if ((retval = nc4_get_typelen_mem(grp->nc4_info, basetype->hdr.id, &basetypesize)))
                return retval;
            /* shallow clone the content of the vlen; shallow because it has only a temporary existence */
            fv_vlen->len = in_vlen->len;
            if (!(fv_vlen->p = malloc(basetypesize * in_vlen->len)))
                return NC_ENOMEM;
            memcpy(fv_vlen->p, in_vlen->p, in_vlen->len * basetypesize);
        }
        else if (var->type_info->nc_type_class == NC_STRING)
        {
            if (*(char **)data)
            {
                if (!(*(char **)(var->fill_value) = malloc(strlen(*(char **)data) + 1)))
                    return NC_ENOMEM;
                strcpy(*(char **)var->fill_value, *(char **)data);
            }
            else
                *(char **)var->fill_value = NULL;
        }
        else
            memcpy(var->fill_value, data, type_size);

        /* Indicate that the fill value was changed, if the variable has already
         * been created in the file, so the dataset gets deleted and re-created. */
        if (var->created)
            var->fill_val_changed = NC_TRUE;
    }

    /* Copy the attribute data, if there is any. VLENs and string
     * arrays have to be handled specially. */
    if (att->len)
    {
        nc_type type_class;    /* Class of attribute's type */

        /* Get class for this type. */
        if ((retval = nc4_get_typeclass(h5, file_type, &type_class)))
            return retval;

        assert(data);
#ifdef LOOK
        if (type_class == NC_VLEN)
        {
            const hvl_t *vldata1;
            NC_TYPE_INFO_T *vltype;
            size_t base_typelen;

            /* Get the type object for the attribute's type */
            if ((retval = nc4_find_type(h5, file_type, &vltype)))
                BAIL(retval);

            /* Retrieve the size of the base type */
            if ((retval = nc4_get_typelen_mem(h5, vltype->u.v.base_nc_typeid, &base_typelen)))
                BAIL(retval);

            vldata1 = data;
            if (!(att->vldata = (nc_vlen_t*)malloc(att->len * sizeof(hvl_t))))
                BAIL(NC_ENOMEM);
            for (i = 0; i < att->len; i++)
            {
                att->vldata[i].len = vldata1[i].len;
                /* Warning, this only works for cases described for nc_free_vlen() */
                if (!(att->vldata[i].p = malloc(base_typelen * att->vldata[i].len)))
                    BAIL(NC_ENOMEM);
                memcpy(att->vldata[i].p, vldata1[i].p, base_typelen * att->vldata[i].len);
            }
        }
        else
#endif
	if (type_class == NC_STRING)
        {
            LOG((4, "copying array of NC_STRING"));
            if (!(att->stdata = malloc(sizeof(char *) * att->len))) {
                BAIL(NC_ENOMEM);
            }

            /* If we are overwriting an existing attribute,
               specifically an NC_CHAR, we need to clean up
               the pre-existing att->data. */
            if (!new_att && att->data) {
                free(att->data);
                att->data = NULL;
            }

            for (i = 0; i < att->len; i++)
            {
                if(NULL != ((char **)data)[i]) {
                    LOG((5, "copying string %d of size %d", i, strlen(((char **)data)[i]) + 1));
                    if (!(att->stdata[i] = strdup(((char **)data)[i])))
                        BAIL(NC_ENOMEM);
                }
                else
                    att->stdata[i] = ((char **)data)[i];
            }
        }
        else
        {
            /* [Re]allocate memory for the attribute data */
            if (!new_att)
                free (att->data);
	    if (!(att->data = malloc(att->len * type_size)))
                BAIL(NC_ENOMEM);
#ifdef ADEBUG
	fprintf(stderr,"new attr: %s: len=%d alloc=%d\n",att->hdr.name,(int)att->len,(int)(att->len*type_size));
#endif

            /* Just copy the data, for non-atomic types */
            if (type_class == NC_OPAQUE || type_class == NC_COMPOUND || type_class == NC_ENUM)
                memcpy(att->data, data, len * type_size);
            else
            {
                /* Data types are like religions, in that one can convert.  */
                if ((retval = nc4_convert_type(data, att->data, mem_type, file_type,
                                               len, &range_error, NULL,
                                               (h5->cmode & NC_CLASSIC_MODEL))))
                    BAIL(retval);
            }
        }
    }
    att->dirty = NC_TRUE;
    att->created = NC_FALSE;

    /* Mark attributes on variable dirty, so they get written */
    if(var)
        var->attr_dirty = NC_TRUE;

exit:
    /* If there was an error return it, otherwise return any potential
       range error value. If none, return NC_NOERR as usual.*/
    if (retval)
        return retval;
    if (range_error)
        return NC_ERANGE;
    return NC_NOERR;
}

/**
 * @internal Write an attribute to a netCDF-4/NCZ file, converting
 * data type if necessary.
 *
 * @param ncid File and group ID.
 * @param varid Variable ID.
 * @param name Name of attribute.
 * @param file_type Type of the attribute data in file.
 * @param len Number of elements in attribute array.
 * @param data Attribute data.
 * @param mem_type Type of data in memory.
 *
 * @return ::NC_NOERR No error.
 * @return ::NC_EINVAL Invalid parameters.
 * @return ::NC_EBADID Bad ncid.
 * @return ::NC_ENOTVAR Variable not found.
 * @return ::NC_EBADNAME Name contains illegal characters.
 * @return ::NC_ENAMEINUSE Name already in use.
 * @author Dennis Heimbigner, Ed Hartnett
 */
int
NCZ_put_att(int ncid, int varid, const char *name, nc_type file_type,
                 size_t len, const void *data, nc_type mem_type)
{
    NC_FILE_INFO_T *h5;
    NC_GRP_INFO_T *grp;
    int ret;

    /* Find info for this file, group, and h5 info. */
    if ((ret = nc4_find_grp_h5(ncid, &grp, &h5)))
        return ret;
    assert(grp && h5);

    return ncz_put_att(grp, varid, name, file_type, len, data, mem_type, 0);
}

/**
 * @internal Learn about an att. All the nc4 nc_inq_ functions just
 * call ncz_get_att to get the metadata on an attribute.
 *
 * @param ncid File and group ID.
 * @param varid Variable ID.
 * @param name Name of attribute.
 * @param xtypep Pointer that gets type of attribute.
 * @param lenp Pointer that gets length of attribute data array.
 *
 * @return ::NC_NOERR No error.
 * @return ::NC_EBADID Bad ncid.
 * @author Dennis Heimbigner, Ed Hartnett
 */
int
NCZ_inq_att(int ncid, int varid, const char *name, nc_type *xtypep,
                 size_t *lenp)
{
    NC_FILE_INFO_T *h5;
    NC_GRP_INFO_T *grp;
    NC_VAR_INFO_T *var = NULL;
    char norm_name[NC_MAX_NAME + 1];
    int retval;

    LOG((2, "%s: ncid 0x%x varid %d", __func__, ncid, varid));

    /* Find the file, group, and var info, and do lazy att read if
     * needed. */
    if ((retval = ncz_find_grp_var_att(ncid, varid, name, 0, 1, norm_name,
                                            &h5, &grp, &var, NULL)))
        return retval;

    /* If this is one of the reserved atts, use nc_get_att_special. */
    if (!var)
    {
        const NC_reservedatt *ra = NC_findreserved(norm_name);
        if (ra  && ra->flags & NAMEONLYFLAG)
            return ncz_get_att_special(h5, norm_name, xtypep, NC_NAT, lenp, NULL,
                                       NULL);
    }

    return nc4_get_att_ptrs(h5, grp, var, norm_name, xtypep, NC_NAT,
                            lenp, NULL, NULL);
}

/**
 * @internal Learn an attnum, given a name.
 *
 * @param ncid File and group ID.
 * @param varid Variable ID.
 * @param name Name of attribute.
 * @param attnump Pointer that gets the attribute index number.
 *
 * @return ::NC_NOERR No error.
 * @author Dennis Heimbigner, Ed Hartnett
 */
int
NCZ_inq_attid(int ncid, int varid, const char *name, int *attnump)
{
    NC_FILE_INFO_T *h5;
    NC_GRP_INFO_T *grp;
    NC_VAR_INFO_T *var = NULL;
    char norm_name[NC_MAX_NAME + 1];
    int retval;

    LOG((2, "%s: ncid 0x%x varid %d", __func__, ncid, varid));

    /* Find the file, group, and var info, and do lazy att read if
     * needed. */
    if ((retval = ncz_find_grp_var_att(ncid, varid, name, 0, 1, norm_name,
                                            &h5, &grp, &var, NULL)))
        return retval;

    /* If this is one of the reserved atts, use nc_get_att_special. */
    if (!var)
    {
        const NC_reservedatt *ra = NC_findreserved(norm_name);
        if (ra  && ra->flags & NAMEONLYFLAG)
            return ncz_get_att_special(h5, norm_name, NULL, NC_NAT, NULL, attnump,
                                       NULL);
    }

    return nc4_get_att_ptrs(h5, grp, var, norm_name, NULL, NC_NAT,
                            NULL, attnump, NULL);
}

/**
 * @internal Given an attnum, find the att's name.
 *
 * @param ncid File and group ID.
 * @param varid Variable ID.
 * @param attnum The index number of the attribute.
 * @param name Pointer that gets name of attrribute.
 *
 * @return ::NC_NOERR No error.
 * @return ::NC_EBADID Bad ncid.
 * @author Dennis Heimbigner, Ed Hartnett
 */
int
NCZ_inq_attname(int ncid, int varid, int attnum, char *name)
{
    NC_ATT_INFO_T *att;
    int retval;

    LOG((2, "%s: ncid 0x%x varid %d", __func__, ncid, varid));

    /* Find the file, group, and var info, and do lazy att read if
     * needed. */
    if ((retval = ncz_find_grp_var_att(ncid, varid, NULL, attnum, 0, NULL,
                                            NULL, NULL, NULL, &att)))
        return retval;
    assert(att);

    /* Get the name. */
    if (name)
        strcpy(name, att->hdr.name);

    return NC_NOERR;
}

/**
 * @internal Get an attribute.
 *
 * @param ncid File and group ID.
 * @param varid Variable ID.
 * @param name Name of attribute.
 * @param value Pointer that gets attribute data.
 * @param memtype The type the data should be converted to as it is
 * read.
 *
 * @return ::NC_NOERR No error.
 * @return ::NC_EBADID Bad ncid.
 * @author Dennis Heimbigner, Ed Hartnett
 */
int
NCZ_get_att(int ncid, int varid, const char *name, void *value,
                 nc_type memtype)
{
    NC_FILE_INFO_T *h5;
    NC_GRP_INFO_T *grp;
    NC_VAR_INFO_T *var = NULL;
    char norm_name[NC_MAX_NAME + 1];
    int retval;

    LOG((2, "%s: ncid 0x%x varid %d", __func__, ncid, varid));

    /* Find the file, group, and var info, and do lazy att read if
     * needed. */
    if ((retval = ncz_find_grp_var_att(ncid, varid, name, 0, 1, norm_name,
                                            &h5, &grp, &var, NULL)))
        return retval;

    /* If this is one of the reserved atts, use nc_get_att_special. */
    if (!var)
    {
        const NC_reservedatt *ra = NC_findreserved(norm_name);
        if (ra  && ra->flags & NAMEONLYFLAG)
            return ncz_get_att_special(h5, norm_name, NULL, NC_NAT, NULL, NULL,
                                       value);
    }

    return nc4_get_att_ptrs(h5, grp, var, norm_name, NULL, memtype,
                            NULL, NULL, value);
}

#if 0
static int
ncz_del_attr(NC_FILE_INFO_T* file, NC_OBJ* container, const char* name)
{
    int i,stat = NC_NOERR;

    ZTRACE();

    if(container->sort == NCGRP)
	stat = ncz_getattlist((NC_GRP_INFO_T*)container,NC_GLOBAL,NULL,&attlist);
    else
	stat = ncz_getattlist((NC_VAR_INFO_T*)container,NC_GLOBAL,NULL,&attlist);

	goto done;

    /* Iterate over the attributes to locate the matching attribute */
    for(i=0;i<nclistlength(jattrs->dict);i+=2) {
	NCjson* key = nclistget(jattrs->dict,i);
	assert(key->sort == NCJ_STRING);
	if(strcmp(key->value,name)==0) {
	    /* Remove and reclaim */
	    NCjson* value = nclistget(jattrs->dict,i+1);
	    nclistremove(jattrs->dict,i);
	    nclistremove(jattrs->dict,i+1);
	    NCJreclaim(key);
	    NCJreclaim(value);
	    break;
	}    
    }
    /* Write the json back out */
    if((stat = ncz_unload_jatts(zinfo, container, jattrs, jtypes)))
	goto done;

done:
    NCJreclaim(jattrs);
    NCJreclaim(jtypes);
    return stat;
}
#endif

/* If we do not have a _FillValue, then go ahead and create it */
int
ncz_create_fillvalue(NC_VAR_INFO_T* var)
{
    int stat = NC_NOERR;
    int i;
    NC_ATT_INFO_T* fv = NULL;

    /* Have the var's attributes been read? */
    if(!var->atts_read) goto done; /* above my pay grade */

    /* Is FillValue warranted? */
    if(!var->no_fill && var->fill_value != NULL) {
        /* Make sure _FillValue does not exist */
	for(i=0;i<ncindexsize(var->att);i++) {
	    fv = (NC_ATT_INFO_T*)ncindexith(var->att,i);
	    if(strcmp(fv->hdr.name,_FillValue)==0) break;
	    fv = NULL;
        }
	if(fv == NULL) {
	    /* Create it */
	    if((stat = ncz_makeattr((NC_OBJ*)var,var->att,_FillValue,var->type_info->hdr.id,1,var->fill_value,&fv)))
	    goto done;
	}
    }
done:
    return THROW(stat);
}

/* Create an attribute; This is an abbreviated form
   of ncz_put_att above */
int
ncz_makeattr(NC_OBJ* container, NCindex* attlist, const char* name, nc_type typeid, size_t len, void* values, NC_ATT_INFO_T** attp)
{
    int stat = NC_NOERR;
    NC_ATT_INFO_T* att = NULL;
    NCZ_ATT_INFO_T* zatt = NULL;

    if((stat=nc4_att_list_add(attlist,name,&att)))
	goto done;
    if((zatt = calloc(1,sizeof(NCZ_ATT_INFO_T))) == NULL)
	{stat = NC_ENOMEM; goto done;}
    if(container->sort == NCGRP) {
        zatt->common.file = ((NC_GRP_INFO_T*)container)->nc4_info;
    } else if(container->sort == NCVAR) {
        zatt->common.file = ((NC_VAR_INFO_T*)container)->container->nc4_info;
    } else
	abort();
    att->container = container;
    att->format_att_info = zatt;
    /* Fill in the attribute's type and value  */
    att->nc_typeid = typeid;
    att->len = len;
    att->data = values;
    att->dirty = NC_TRUE;
    if(attp) {*attp = att; att = NULL;}

done:
    if(stat) {
	if(att) nc4_att_list_del(attlist,att);
	nullfree(zatt);
    }
    return THROW(stat);
}

