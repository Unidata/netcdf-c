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
#include "zfilter.h"
#include "zfill.h"

#undef ADEBUG

/* Forward */
static int charify(const NCjson* src, NCbytes* buf);
static int NCZ_json_convention_read(const NCjson* json, NCjson** jtextp);

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
ncz_getattlist(NC_GRP_INFO_T *grp, int varid, NC_VAR_INFO_T **varp, NCindex **attlistp)
{
    int stat = NC_NOERR;
    NC_FILE_INFO_T* file = grp->nc4_info;
    NCZ_FILE_INFO_T* zinfo = file->format_file_info;

    assert(grp && file && zinfo);

    if (varid == NC_GLOBAL) {
        if (varp) *varp = NULL;
        if(attlistp) *attlistp = grp->att;
    } else {
        NC_VAR_INFO_T *var;
        if (!(var = (NC_VAR_INFO_T *)ncindexith(grp->vars, (size_t)varid))) return NC_ENOTVAR;
        assert(var->hdr.id == varid);
        if (varp) *varp = var;
        if(attlistp) *attlistp = var->att;
    }
    return stat;
}

/**
 * @internal Get one of the special attributes:
 * See the reserved attribute table in libsrc4/nc4internal.c.
 * The special attributes are the ones marked with NAMEONLYFLAG.
 * For example: NCPROPS, ISNETCDF4ATT, and SUPERBLOCKATT, and CODECS.
 * Some of these atts are stored in the file, but other are constructed on the fly.
 * The later are distinguished by the VIRTUALFLAG.
 *
 * @param h5 Pointer to ZARR file info struct.
 * @param var Pointer to var info struct; NULL signals global.
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
ncz_get_att_special(NC_FILE_INFO_T* h5, NC_VAR_INFO_T* var, const char* name,
                    nc_type* filetypep, nc_type mem_type, size_t* lenp,
                    int* attnump, void* data)
{
    int stat = NC_NOERR;
    
    /* Fail if asking for att id */
    if(attnump)
        {stat = NC_EATTMETA; goto done;}

    /* Handle the per-var case(s) first */
    if(var != NULL) {
#ifdef NETCDF_ENABLE_NCZARR_FILTERS
        if(strcmp(name,NC_ATT_CODECS)==0) {	
            NClist* filters = (NClist*)var->filters;

            if(mem_type == NC_NAT) mem_type = NC_CHAR;
            if(mem_type != NC_CHAR)
                {stat = NC_ECHAR; goto done;}
            if(filetypep) *filetypep = NC_CHAR;
	    if(lenp) *lenp = 0;
	    if(filters == NULL) goto done;	  
 	    if((stat = NCZ_codec_attr(var,lenp,data))) goto done;
	}
#endif
	goto done;
    }

    /* The global reserved attributes */
    if(strcmp(name,NCPROPS)==0) {
        size_t len;
        if(h5->provenance.ncproperties == NULL)
            {stat = NC_ENOTATT; goto done;}
        if(mem_type == NC_NAT) mem_type = NC_CHAR;
        if(mem_type != NC_CHAR)
            {stat = NC_ECHAR; goto done;}
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
            iv = (unsigned long long)NCZ_isnetcdf4(h5);
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
                {stat = NC_ERANGE; goto done;}
            }
    }
done:
    return stat;

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
    int stat = NC_NOERR;

    if (!name || !newname)
        return NC_EINVAL;

    LOG((2, "nc_rename_att: ncid 0x%x varid %d name %s newname %s",
         ncid, varid, name, newname));

    /* If the new name is too long, that's an error. */
    if (strlen(newname) > NC_MAX_NAME)
        return NC_EMAXNAME;

    /* Find info for this file, group, and h5 info. */
    if ((stat = nc4_find_grp_h5(ncid, &grp, &h5)))
        return stat;
    assert(h5 && grp);

    /* If the file is read-only, return an error. */
    if (h5->no_write)
        return NC_EPERM;

    /* Check and normalize the name. */
    if ((stat = nc4_check_name(newname, norm_newname)))
        return stat;

    /* Get the list of attributes. */
    if ((stat = ncz_getattlist(grp, varid, &var, &list)))
        return stat;

    /* Is new name in use? */
    att = (NC_ATT_INFO_T*)ncindexlookup(list,norm_newname);
    if(att != NULL)
        return NC_ENAMEINUSE;

    /* Normalize name and find the attribute. */
    if ((stat = nc4_normalize_name(name, norm_name)))
        return stat;

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

    att->dirty = NC_TRUE;

    /* Rehash the attribute list so that the new name is used */
    if(!ncindexrebuild(list))
        return NC_EINTERNAL;

    /* Mark attributes on variable dirty, so they get written */
    if(var)
        var->attr_dirty = NC_TRUE;
    return stat;
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
    NCindex* attlist = NULL;
    int stat;

    /* Name must be provided. */
    if (!name)
        return NC_EINVAL;

    LOG((2, "nc_del_att: ncid 0x%x varid %d name %s", ncid, varid, name));

    /* Find info for this file, group, and h5 info. */
    if ((stat = nc4_find_grp_h5(ncid, &grp, &h5)))
        return stat;
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
        if ((stat = NCZ_redef(ncid)))
            return stat;
    }

    /* Get either the global or a variable attribute list. */
    if ((stat = ncz_getattlist(grp, varid, &var, &attlist)))
        return stat;

#ifdef LOOK
    /* Determine the location id in the ZARR file. */
    if (varid == NC_GLOBAL)
        locid = ((NCZ_GRP_INFO_T *)(grp->format_grp_info))->hdf_grpid;
    else if (var->created)
        locid = ((NCZ_VAR_INFO_T *)(var->format_var_info))->hdf_datasetid;
#endif

    /* Defer to the internal version */
    stat = NCZ_attr_delete(h5,attlist,name);

    return stat;
}

/* Internal version for deleting an attribute */
int
NCZ_attr_delete(NC_FILE_INFO_T* file, NCindex* attlist, const char* name)
{
    int stat = NC_NOERR;
    NC_ATT_INFO_T* att = NULL;
    NCZ_ATT_INFO_T* za = NULL;
    size_t i;
    int deletedid;

    /* Now find the attribute by name. */
    if (!(att = (NC_ATT_INFO_T*)ncindexlookup(attlist, name)))
        return NC_ENOTATT;

    /* Reclaim the content of the attribute */
    if(att->data) {
	if((stat = NC_reclaim_data_all(file->controller,att->nc_typeid,att->data,(size_t)att->len))) return stat;
    }
    att->data = NULL;
    att->len = 0;

    deletedid = att->hdr.id;

    /* reclaim associated NCZarr info */
    za = (NCZ_ATT_INFO_T*)att->format_att_info;
    nullfree(za);

    /* Remove this attribute in this list */
    if ((stat = nc4_att_list_del(attlist, att)))
        return stat;

    /* Renumber all attributes with higher indices. */
    for (i = 0; i < ncindexsize(attlist); i++) {
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
ncz_put_att(int ncid, int containerid, const char *name, nc_type file_type,
            size_t len, const void *data, nc_type mem_type, int force)
{
    int stat = NC_NOERR, range_error = 0;
    NC* nc;
    NC_FILE_INFO_T *file = NULL;
    NC_GRP_INFO_T *grp = NULL;
    NC_VAR_INFO_T *var = NULL;
    void* copy = NULL;
    NC_OBJ* obj = NULL;
    int isfillvalue = (strcmp(name,NC_FillValue)==0);
    char norm_name[NC_MAX_NAME + 1];
    const NC_reservedatt* ra = NULL;
    NC_ATT_INFO_T* att = NULL;
    NCindex* attlist = NULL;
    if(containerid == NC_GLOBAL) {
        if((stat= nc4_find_grp_h5(ncid, &grp, &file))) goto done;
	obj = (NC_OBJ*)grp;
    } else {
        if((stat= nc4_find_grp_h5_var(ncid, containerid, &file, &grp, &var))) goto done;
	obj = (NC_OBJ*)var;
    }
    assert(file != NULL && grp != NULL && (containerid == NC_GLOBAL || var != NULL));

    nc = file->controller;
    assert(nc && grp && file);

    if(obj->sort == NCGRP)
        attlist = ((NC_GRP_INFO_T*)obj)->att;
    else
        attlist = ((NC_VAR_INFO_T*)obj)->att;
    assert(attlist != NULL);

    LOG((1, "%s: ncid 0x%x containerid %d name %s file_type %d mem_type %d len %d",
         __func__,ncid, containerid, name, file_type, mem_type, len));

    /* We must have two valid types to continue. */
    if (file_type == NC_NAT || mem_type == NC_NAT) return NC_EBADTYPE;

    /* No character conversions are allowed. */
    if (file_type != mem_type &&
        (file_type == NC_CHAR || mem_type == NC_CHAR ||
         file_type == NC_STRING || mem_type == NC_STRING))
        return NC_ECHAR;

    /* For classic mode file, only allow atts with classic types to be
     * created. */
    if (file->cmode & NC_CLASSIC_MODEL && file_type > NC_DOUBLE)
        return NC_ESTRICTNC3;

    /* The length needs to be positive (cast needed for brain-dead systems with signed size_t). */
    if((unsigned long) len > X_INT_MAX) {stat = NC_EINVAL; goto done;}

    /* If len is not zero, then there must be some data. */
    if (len > 0 && data == NULL) {stat = NC_EINVAL; goto done;}

    /* If the file is read-only, return an error. */
    if (file->no_write) {stat = NC_EPERM; goto done;}

    /* Check and normalize the name. */
    if (!name || strlen(name) > NC_MAX_NAME) {stat = NC_EBADNAME; goto done;}
    if ((stat = nc4_check_name(name, norm_name))) goto done;

    /* Check that a reserved att name is not being used improperly */
    ra = NC_findreserved(name);
    if(ra != NULL) {
        /* case 1: grp=root, containerid==NC_GLOBAL, flags & READONLYFLAG */
        if (obj->sort == NCGRP && ((NC_GRP_INFO_T*)obj)->parent == NULL && (ra->flags & READONLYFLAG))
            {stat = NC_ENAMEINUSE; goto done;}
        /* case 2: grp=NA, objid!=NC_GLOBAL, flags & HIDDENATTRFLAG */
        if (obj->sort != NCGRP && (ra->flags & HIDDENATTRFLAG))
            {stat = NC_ENAMEINUSE; goto done;}
    }

    if(isfillvalue)
        file_type = var->type_info->hdr.id;

    /* copy and/or convert memory data to file format data */
    if(mem_type != file_type && mem_type < NC_STRING && mem_type < NC_STRING) {
	size_t mem_type_len = 0;
	size_t file_type_len = 0;
        if ((stat = nc4_get_typelen_mem(file, mem_type, &mem_type_len))) return stat;
        if ((stat = nc4_get_typelen_mem(file, file_type, &file_type_len))) return stat;
	/* Need to convert from memory data into copy buffer */
	if((copy = malloc(len*file_type_len))==NULL) {stat = NC_ENOMEM; goto done;}
        if ((stat = nc4_convert_type(data, copy, mem_type, file_type,
                                               len, &range_error, NULL,
                                               (file->cmode & NC_CLASSIC_MODEL),
					       NC_NOQUANTIZE, 0)))
	    goto done;
    } else { /* no conversion */
	/* Still need a copy of the input data */
	copy = NULL;
	if((stat = NC_copy_data_all(file->controller, mem_type, data, len, &copy))) goto done;
    }

    /* See if there is already an attribute with this name. */
    att = (NC_ATT_INFO_T*)ncindexlookup(attlist,norm_name);

    if (!att) {
        if (!(file->flags & NC_INDEF)) { /* If this is a new att, require define mode. */
            if (file->cmode & NC_CLASSIC_MODEL) {stat = NC_ENOTINDEFINE; goto done;}
	    file->flags |= NC_INDEF;/* Set define mode. */
	    file->redef = NC_TRUE; /* For nc_abort, we need to remember if we're in define mode as a redef. */
        }
    } else {
        /* For an existing att, if we're not in define mode, the len
           must not be greater than the existing len for classic model. */
        if(!(file->flags & NC_INDEF) 
	    && len * (size_t)nc4typelen(att->nc_typeid) > (size_t)att->len * (size_t)nc4typelen(att->nc_typeid)) {
            if (file->cmode & NC_CLASSIC_MODEL) {stat = NC_ENOTINDEFINE; goto done;}
	    file->flags |= NC_INDEF;/* Set define mode. */
	    file->redef = NC_TRUE; /* For nc_abort, we need to remember if we're in define mode as a redef. */
        }
    }

    if((stat = ncz_makeattr(file,obj,name,file_type,len,copy,&att))) goto done;

    if(isfillvalue) {
	if((stat = NCZ_copy_fillatt_to_var(file,att,var))) goto done;
    }
    
done:
    if(copy) (void)NC_reclaim_data_all(file->controller,file_type,copy,len);
    /* If there was an error return it, otherwise return any potential
       range error value. If none, return NC_NOERR as usual.*/
    if (range_error) return NC_ERANGE;
    if (stat) return stat;
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
    NC_FILE_INFO_T *file;
    NC_GRP_INFO_T *grp;
    int ret;

    /* Find info for this file, group, and file info. */
    if ((ret = nc4_find_grp_h5(ncid, &grp, &file)))
        return ret;
    assert(grp && file);

    return ncz_put_att(ncid, varid, name, file_type, len, data, mem_type, 0);
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
    NC_FILE_INFO_T *file;
    NC_GRP_INFO_T *grp;
    NC_VAR_INFO_T *var = NULL;
    char norm_name[NC_MAX_NAME + 1];
    int stat;

    LOG((2, "%s: ncid 0x%x varid %d", __func__, ncid, varid));

    /* Find the file, group, and var info, and do lazy att read if
     * needed. */
    if ((stat = ncz_find_grp_var_att(ncid, varid, name, 0, 1, norm_name,
                                            &file, &grp, &var, NULL)))
        return stat;

    /* If this is one of the reserved atts, use nc_get_att_special. */
    {
        const NC_reservedatt *ra = NC_findreserved(norm_name);
        if (ra  && ra->flags & NAMEONLYFLAG)
            return ncz_get_att_special(file, var, norm_name, xtypep, NC_NAT, lenp, NULL,
                                       NULL);
    }

    return nc4_get_att_ptrs(file, grp, var, norm_name, xtypep, NC_NAT,
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
    NC_FILE_INFO_T *file;
    NC_GRP_INFO_T *grp;
    NC_VAR_INFO_T *var = NULL;
    char norm_name[NC_MAX_NAME + 1];
    int stat;

    LOG((2, "%s: ncid 0x%x varid %d", __func__, ncid, varid));

    /* Find the file, group, and var info, and do lazy att read if
     * needed. */
    if ((stat = ncz_find_grp_var_att(ncid, varid, name, 0, 1, norm_name,
                                            &file, &grp, &var, NULL)))
        return stat;

    /* If this is one of the reserved atts, use nc_get_att_special. */
    {
        const NC_reservedatt *ra = NC_findreserved(norm_name);
        if (ra  && ra->flags & NAMEONLYFLAG)
            return ncz_get_att_special(file, var, norm_name, NULL, NC_NAT, NULL, attnump,
                                       NULL);
    }

    return nc4_get_att_ptrs(file, grp, var, norm_name, NULL, NC_NAT,
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
    int stat = NC_NOERR;

    ZTRACE(1,"ncid=%d varid=%d attnum=%d",ncid,varid,attnum);
    LOG((2, "%s: ncid 0x%x varid %d", __func__, ncid, varid));

    /* Find the file, group, and var info, and do lazy att read if
     * needed. */
    if ((stat = ncz_find_grp_var_att(ncid, varid, NULL, attnum, 0, NULL,
                                            NULL, NULL, NULL, &att)))
	goto done;
    assert(att);

    /* Get the name. */
    if (name)
        strcpy(name, att->hdr.name);
done:
    return ZUNTRACEX(stat,"name=%s",(stat?"":name));
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
                 nc_type mem_type)
{
    int stat = NC_NOERR;
    NC_FILE_INFO_T *file;
    NC_GRP_INFO_T *grp;
    NC_VAR_INFO_T *var = NULL;
    NC_ATT_INFO_T *att = NULL;
    char norm_name[NC_MAX_NAME + 1];
    nc_type file_type = NC_NAT;
    int range_error = 0;

    LOG((2, "%s: ncid 0x%x varid %d", __func__, ncid, varid));

    /* Check and normalize the name. */
    if (!name || strlen(name) > NC_MAX_NAME) {stat = NC_EBADNAME; goto done;}
    if ((stat = nc4_check_name(name, norm_name))) goto done;

    /* Find the file and group and (optionally) var info */
    if(varid != NC_GLOBAL) {
        if ((stat = nc4_find_grp_h5_var(ncid, varid, &file, &grp, &var))) goto done;
    } else { /* just find grp and file */
        if ((stat = nc4_find_grp_h5(ncid, &grp, &file))) goto done;
	var = NULL;
    }

    /* If this is one of the reserved global atts, use nc_get_att_special. */
    {
        const NC_reservedatt *ra = NC_findreserved(norm_name);
        if (ra  && ra->flags & NAMEONLYFLAG)
            return ncz_get_att_special(file, var, norm_name, NULL, NC_NAT, NULL, NULL,
                                       value);
    }

    /* See if the attribute exists */
    stat = nc4_find_grp_att(grp,varid,norm_name,0,&att);

    /* If asking for _FillValue and it does not exist, build it */
    if(stat == NC_ENOTATT && att != NULL && varid != NC_GLOBAL && strcmp(norm_name,"_FillValue")==0) {
	if((stat = ncz_create_fillvalue(file,var))) goto done;
        /* get fill value attribute again */
        if((stat = nc4_find_grp_att(grp,varid,norm_name,0,&att))) goto done;
    }

    /* stop if error */
    if(stat) goto done;

    assert(att != NULL);    
    file_type = att->nc_typeid;

    /* We must have two valid types to continue. */
    if (file_type == NC_NAT || mem_type == NC_NAT) return NC_EBADTYPE;

    /* No character conversions are allowed. */
    if (file_type != mem_type &&
        (file_type == NC_CHAR || mem_type == NC_CHAR ||
         file_type == NC_STRING || mem_type == NC_STRING))
        return NC_ECHAR;

    /* For classic mode file, only allow atts with classic types to be
     * created. */
    if (file->cmode & NC_CLASSIC_MODEL && file_type > NC_DOUBLE)
        return NC_ESTRICTNC3;

    /* copy and/or convert memory data to file format data */
    if(mem_type != file_type && mem_type < NC_STRING && mem_type < NC_STRING) {
	size_t mem_type_len = 0;
	size_t file_type_len = 0;
        if ((stat = nc4_get_typelen_mem(file, mem_type, &mem_type_len))) return stat;
        if ((stat = nc4_get_typelen_mem(file, file_type, &file_type_len))) return stat;
	/* Need to convert from file_type data into output buffer */
        if ((stat = nc4_convert_type(att->data, value, file_type, mem_type,
                                               att->len, &range_error, NULL,
                                               (file->cmode & NC_CLASSIC_MODEL),
					       NC_NOQUANTIZE, 0)))
	    goto done;
    } else { /* no conversion */
	/* Still need a copy of the input data */
	if((stat = NC_copy_data(file->controller, file_type, att->data, att->len, value))) goto done;
    }

done:
    /* If there was an error return it, otherwise return any potential
       range error value. If none, return NC_NOERR as usual.*/
    if (range_error) return NC_ERANGE;
    if (stat) return stat;
    return THROW(stat);
}

/* Test if fillvalue is default */
int
isdfaltfillvalue(nc_type nctype, void* fillval)
{
    switch (nctype) {
    case NC_BYTE: if(NC_FILL_BYTE == *((signed char*)fillval)) return 1; break;
    case NC_CHAR: if(NC_FILL_CHAR == *((char*)fillval)) return 1; break;
    case NC_SHORT: if(NC_FILL_SHORT == *((short*)fillval)) return 1; break;
    case NC_INT: if(NC_FILL_INT == *((int*)fillval)) return 1; break;
    case NC_FLOAT: if(NC_FILL_FLOAT == *((float*)fillval)) return 1; break;
    case NC_DOUBLE: if(NC_FILL_DOUBLE == *((double*)fillval)) return 1; break;
    case NC_UBYTE: if(NC_FILL_UBYTE == *((unsigned char*)fillval)) return 1; break;
    case NC_USHORT: if(NC_FILL_USHORT == *((unsigned short*)fillval)) return 1; break;
    case NC_UINT: if(NC_FILL_UINT == *((unsigned int*)fillval)) return 1; break;
    case NC_INT64: if(NC_FILL_INT64 == *((long long int*)fillval)) return 1; break;
    case NC_UINT64: if(NC_FILL_UINT64 == *((unsigned long long int*)fillval)) return 1; break;
    case NC_STRING: if(strcmp(NC_FILL_STRING,*((char**)fillval))) return 1; break;
    default: break;
    }
    return 0;
}

/* If we do not have a _FillValue, then go ahead and create it */
int
ncz_create_fillvalue(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var)
{
    int stat = NC_NOERR;

    /* Have the var's attributes been read? */
    if(!var->atts_read) goto done; /* above my pay grade */

    /* Is FillValue warranted? */
    if(!var->no_fill && var->fill_value != NULL) {
	if((stat = NCZ_copy_var_to_fillatt(file,var,NULL))) goto done;
    }

done:
    return THROW(stat);
}

/*
Create an attribute; This is the core of ncz_put_att above.
Caller must free values.
*/
int
ncz_makeattr(NC_FILE_INFO_T* file, NC_OBJ* container, const char* name, nc_type typeid, size_t len, void* values, NC_ATT_INFO_T** attp)
{
    int stat = NC_NOERR;
    NC_ATT_INFO_T* att = NULL;
    NCZ_ATT_INFO_T* zatt = NULL;
    int new_att = 0;
    size_t typesize = 0;
    NCindex* attlist = NULL;
    
    if(container->sort == NCGRP)
        attlist = ((NC_GRP_INFO_T*)container)->att;
    else
        attlist = ((NC_VAR_INFO_T*)container)->att;
    assert(attlist != NULL);

    if ((stat = nc4_get_typelen_mem(file, typeid, &typesize))) goto done;

    /* See if there is already an attribute with this name. */
    att = (NC_ATT_INFO_T*)ncindexlookup(attlist,name);
    new_att = (att == NULL?1:0);
    
    if(new_att) {
        if((stat=nc4_att_list_add(attlist,name,&att))) goto done;
        if((zatt = calloc(1,sizeof(NCZ_ATT_INFO_T))) == NULL) {stat = NC_ENOMEM; goto done;}
        zatt->common.file = file;
        att->container = container;
        att->format_att_info = zatt;
    } else if(att->data) {
	/* remove old att data */
	(void)NC_reclaim_data_all(file->controller,att->nc_typeid,att->data,att->len);
	att->data = NULL;
    }

    /* Fill in the attribute's type and value  */
    att->nc_typeid = typeid;
    att->len = len;
    if((stat = NC_copy_data_all(file->controller,typeid,values,len,&att->data))) goto done;
    att->dirty = NC_TRUE;
    if(attp) {*attp = att; att = NULL;}

    if(container->sort != NCGRP)
        ((NC_VAR_INFO_T*)container)->attr_dirty = NC_TRUE;

done:
    if(stat) {
	if(new_att && att) {
	    nc4_att_list_del(attlist,att);
	    nullfree(zatt);
	}
    }
    return THROW(stat);
}

/*
Extract data for an attribute
This is essentially Version 2|3 agnostic because the
data part of an attribute is the same for both versions.
*/
int
NCZ_computeattrdata(nc_type* typeidp, const NCjson* values, size_t* typelenp, size_t* countp, void** datap)
{
    int stat = NC_NOERR;
    NCbytes* buf = ncbytesnew();
    nc_type typeid = NC_NAT;
    size_t typelen;
    NCjson* jtext = NULL;
    int isjson = 0; /* 1 => json valued attribute */
    size_t count = 0; /* no. of attribute values */

    ZTRACE(3,"typehint=%d typeid=%d values=|%s|",typehint,*typeidp,NCJtotext(values));

    typeid = *typeidp; /* initial assumption */

    if(typeid == NC_JSON)
        typelen = 1; /* treat like char */
    else
        {if((stat = NC4_inq_atomic_type(typeid, NULL, &typelen))) goto done;}

    /* Handle special types */
    if(typeid == NC_JSON) {
	/* Apply the JSON attribute convention and convert to JSON string */
	typeid = NC_CHAR;
	if((stat = NCZ_json_convention_read(values,&jtext))) goto done;
        /* Convert the JSON attribute values to the actual netcdf attribute bytes */
        if((stat = NCZ_attr_convert(jtext,typeid,typelen,&count,buf))) goto done;
    } else if(typeid == NC_NAT) {
        /* If we don't know, then infer the type */
        if(typeid == NC_NAT && !isjson) {
	    if((stat = NCZ_inferattrtype(values,NC_NAT,&typeid))) goto done;
	}
        /* Convert the JSON attribute values to the actual netcdf attribute bytes */
        if((stat = NCZ_attr_convert(values,typeid,typelen,&count,buf))) goto done;
    } else { /* numeric types + NC_CHAR + NC_STRING */
        /* Convert the JSON attribute values to the actual netcdf attribute bytes */
        if((stat = NCZ_attr_convert(values,typeid,typelen,&count,buf))) goto done;
    }
    assert(typeid != NC_NAT && typeid != NC_JSON); 

    if(typelenp) *typelenp = typelen;
    if(typeidp) *typeidp = typeid; /* return possibly inferred type */
    if(countp) *countp = count;
    if(datap) *datap = ncbytesextract(buf);

done:
    ncbytesfree(buf);
    NCJreclaim(jtext); /* we created it */
    return ZUNTRACEX(THROW(stat),"typelen=%d count=%u",(typelenp?*typelenp:0),(countp?*countp:-1));
}

/* Convert a json value to actual data values of an attribute.
@param src - [in] src value
@param typeid - [in] dst type
@param countp - [out] dst length (if dict or array)
@param dst - [out] dst data
*/

int
NCZ_attr_convert(const NCjson* src, nc_type typeid, size_t typelen, size_t* countp, NCbytes* dst)
{
    int stat = NC_NOERR;
    size_t i;
    size_t count = 0;

    ZTRACE(3,"src=%s typeid=%d typelen=%u",NCJtotext(src),typeid,typelen);

    /* 3 cases:
       (1) singleton atomic value
       (2) array of atomic values
       (3) other JSON expression
    */
    switch (NCJsort(src)) {
    case NCJ_INT: case NCJ_DOUBLE: case NCJ_BOOLEAN: /* case 1 */
	count = 1;
	if((stat = NCZ_convert1(src, typeid, dst)))
	    goto done;
	break;

    case NCJ_ARRAY:
	if(typeid == NC_CHAR) {
	    if((stat = charify(src,dst))) goto done;
	    count = ncbyteslength(dst);
	} else {
	    count = NCJarraylength(src);
	    for(i=0;i<count;i++) {
		NCjson* value = NCJith(src,i);
		if((stat = NCZ_convert1(value, typeid, dst))) goto done;
	    }
	}
	break;
    case NCJ_STRING:
	if(typeid == NC_CHAR) {
	    if((stat = charify(src,dst))) goto done;
	    count = ncbyteslength(dst);
	    /* Special case for "" */
	    if(count == 0) {
		ncbytesappend(dst,'\0');
		count = 1;
	    }
	} else {
	    if((stat = NCZ_convert1(src, typeid, dst))) goto done;
	    count = 1;
	}
	break;
    default: stat = (THROW(NC_ENCZARR)); goto done;
    }
    if(countp) *countp = count;

done:
    return ZUNTRACE(THROW(stat));
}

/* Convert a JSON singleton or array of strings to a single string */
static int
charify(const NCjson* src, NCbytes* buf)
{
    int stat = NC_NOERR;
    size_t i;
    struct NCJconst jstr = NCJconst_empty;

    if(NCJsort(src) != NCJ_ARRAY) { /* singleton */
	NCJcvt(src, NCJ_STRING, &jstr);
	ncbytescat(buf,jstr.sval);
    } else for(i=0;i<NCJarraylength(src);i++) {
	NCjson* value = NCJith(src,i);
	NCJcvt(value, NCJ_STRING, &jstr);
	ncbytescat(buf,jstr.sval);
	nullfree(jstr.sval);jstr.sval = NULL;
    }

    nullfree(jstr.sval);
    return stat;
}

/**
Implement the JSON convention:
Stringify it as the value and make the attribute be of type "char".
*/
static int
NCZ_json_convention_read(const NCjson* json, NCjson** jtextp)
{
    int stat = NC_NOERR;
    NCjson* jtext = NULL;
    char* text = NULL;

    if(json == NULL) {stat = NC_EINVAL; goto done;}
    if(NCJunparse(json,0,&text)) {stat = NC_EINVAL; goto done;}
    if(NCJnewstring(NCJ_STRING,text,&jtext)) {stat = NC_EINVAL; goto done;}
    *jtextp = jtext; jtext = NULL;
done:
    NCJreclaim(jtext);
    nullfree(text);
    return stat;
}
