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

/* Build table of dual attributes;
   build unsorted, but sort at first reference
   so we can do binary search.
*/
static struct DUALATT {const char* name; DualAtt dasort; NC_SORT container;} dualatts[] = {
{NC_FillValue, DA_FILLVALUE,NCVAR},
{NC_NCZARR_MAXSTRLEN_ATTR, DA_MAXSTRLEN,NCVAR},
{NC_NCZARR_DFALT_MAXSTRLEN_ATTR, DA_DFALTSTRLEN,NCFILE}, /* NCGRP refers to root group in file */
{NC_NCZARR_SEPARATOR_ATTR, DA_SEP,NCVAR},
{NC_NCZARR_DFALT_SEPARATOR_ATTR, DA_DFALTSEP,NCFILE},
{NC_QUANTIZE_BITGROOM_ATT_NAME, DA_QUANTIZE,NCVAR},
{NC_QUANTIZE_GRANULARBR_ATT_NAME, DA_QUANTIZE,NCVAR},
{NC_QUANTIZE_BITROUND_ATT_NAME, DA_QUANTIZE,NCVAR}
};
#define NDUALATTS (sizeof(dualatts)/sizeof(struct DUALATT))
static int dualsorted = 0;

/* Forward */
static int charify(const NCjson* src, NCbytes* buf);
static int json_convention_read(const NCjson* json, NCjson** jtextp);
static int islegalatt(NC_FILE_INFO_T* file, NC_ATT_INFO_T* att, size_t alen);

static int
dasort(const void* a, const void* b)
{
    const struct DUALATT *daa, *dab;
    daa = a; dab = b;
    return strcasecmp(daa->name,dab->name);
}

static int
dacmp(const void* key, const void* elem)
{
    const struct DUALATT *delem = elem;
    return strcasecmp(key,delem->name);
}

/* Some attributes are reflected in var|file structure */
DualAtt
NCZ_is_dual_att(const char* aname)
{
    void* match = NULL;
    if(!dualsorted) {
	qsort((void*)dualatts, NDUALATTS, sizeof(struct DUALATT),dasort);
	dualsorted = 1;
    }
    /* Binary search the set of set of atomictypes */
    assert(dualsorted);
    match = bsearch((void*)aname,(void*)dualatts,NDUALATTS,sizeof(struct DUALATT),dacmp);
    if(match == NULL) return DA_NOT;
    return ((struct DUALATT*)match)->dasort;
}

/* For dual attributes, specify if they are associated with a var vs file */
NC_SORT
NCZ_dual_att_container(DualAtt da)
{
    size_t i;
    /* Since we cannot simultaneously binary search on two fields, do simple linear search */
    for(i=0;i<NDUALATTS;i++) {
	if(dualatts[i].dasort == da) {return dualatts[i].container;}
    }
    return NCNAT;
}

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
        return THROW(NC_ENAMEINUSE);

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
 * @return Type size in bytes, or 0 if type not found.
 * @author Dennis Heimbigner, Ed Hartnett
 */
static size_t
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
    return 0;
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
            size_t len, const void *data, nc_type mem_type)
{
    int stat = NC_NOERR, range_error = 0;
    NC* nc;
    NC_FILE_INFO_T *file = NULL;
    NC_GRP_INFO_T *grp = NULL;
    NC_VAR_INFO_T *var = NULL;
    char norm_name[NC_MAX_NAME + 1];
    const NC_reservedatt* ra = NULL;
    NC_ATT_INFO_T* att = NULL;
    NCindex* attlist = NULL;
    NC_OBJ* obj = NULL;
    void* src = NULL;
    int isnew = 0;
    int isconverted = 0;
    DualAtt dualatt;
    
    if(containerid == NC_GLOBAL) {
        if((stat= nc4_find_grp_h5(ncid, &grp, &file))) goto done;
        attlist = grp->att;
        obj = (NC_OBJ*)grp;
    } else {
        if((stat= nc4_find_grp_h5_var(ncid, containerid, &file, &grp, &var))) goto done;
        attlist = var->att; 
        obj = (NC_OBJ*)var;
    }
    assert(file != NULL && grp != NULL && (containerid == NC_GLOBAL || var != NULL));
    assert(attlist != NULL);
    
    nc = file->controller;
    assert(nc && grp && file);

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

    /* Check that a reserved att name is not being used improperly, meaning:
    1. attr is in root group and is readonly
    2. attr is in var and is readonly
    */
    ra = NC_findreserved(name);
    if(ra != NULL) {
        /* case 1: grp=root, containerid==NC_GLOBAL, flags & READONLYFLAG */
        if (grp != NULL && grp->parent == NULL && (ra->flags & READONLYFLAG))
            {stat = NC_ENAMEINUSE; goto done;}
        /* case 2: grp=NA, objid!=NC_GLOBAL, flags & HIDDENATTRFLAG */
        if (grp != NULL && (ra->flags & HIDDENATTRFLAG))
            {stat = NC_ENAMEINUSE; goto done;}
    }

    /* copy and/or convert memory data to file format data */
    if(mem_type != file_type && mem_type < NC_STRING && mem_type < NC_STRING) {
        size_t mem_type_len = 0;
        size_t file_type_len = 0;
        if ((stat = nc4_get_typelen_mem(file, mem_type, &mem_type_len))) return stat;
        if ((stat = nc4_get_typelen_mem(file, file_type, &file_type_len))) return stat;
        /* Need to convert from memory data into copy buffer */
        if((src = malloc(len*file_type_len))==NULL) {stat = NC_ENOMEM; goto done;}
        if ((stat = nc4_convert_type(data, src, mem_type, file_type,
                                               len, &range_error, NULL,
                                               (file->cmode & NC_CLASSIC_MODEL),
                                               NC_NOQUANTIZE, 0)))
            goto done;
        isconverted = 1;
    } else {/*no conversion */
        src = (void*)data;
        isconverted = 0;
    }
    
    /* See if there is already an attribute with this name or create  */
    if((stat = NCZ_getattr(file,obj,norm_name,file_type,&att,&isnew))) goto done;
    /* insert/overwrite data */
    if(len > 0 && data != NULL) { /* overwrite old value */
        if((stat = NCZ_set_att_data(file,att,len,src))) goto done;
    }

    /* Some attributes are reflected in var|grp structure so must be sync'd */
    switch (NCZ_dual_att_container(dualatt=NCZ_is_dual_att(name))) {
    case NCVAR:
        assert(var != NULL);
        if((stat = NCZ_sync_dual_att(file,(NC_OBJ*)var,name,dualatt,FIXOBJ))) goto done;
        break;
    case NCFILE:
        if((stat = NCZ_sync_dual_att(file,(NC_OBJ*)file->root_grp,name,dualatt,FIXOBJ))) goto done;
        break;
    default: break; /* ignore */
    }
    
done:
    if(isconverted) (void)NC_reclaim_data_all(file->controller,file_type,src,len);
    /* If there was an error return it, otherwise return any potential
       range error value. If none, return NC_NOERR as usual.*/
    if (range_error) return THROW(NC_ERANGE);
    if (stat) return THROW(stat);
    return THROW(NC_NOERR);
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

    return ncz_put_att(ncid, varid, name, file_type, len, data, mem_type);
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
NCZ_get_att(int ncid, int varid, const char *name, void *value, nc_type mem_type)
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

#if 0
????    /* If asking for _FillValue and it does not exist, build it using either var->fill_value or the default fill */
    if(stat == NC_ENOTATT && att == NULL && var != NULL  && strcmp(norm_name,NC_FillValue)==0) {
        int isnew = 0;
        if((stat = NCZ_getattr(file,(NC_OBJ*)var,NC_FillValue,var->type_info->hdr.id,&att,&isnew))) goto done;
        assert(isnew && att->data == NULL);
        if((stat = NC_copy_data_all(file->controller,att->nc_typeid,NCZ_getdfaltfillvalue(att->nc_typeid),1,&att->data))) goto done;
assert(var->fill_value != att->data);
    }
#endif

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
assert(var == NULL || (var->fill_value != att->data));
    } else { /* no conversion */
        /* Still need a copy of the input data */
        if((stat = NC_copy_data(file->controller, file_type, att->data, att->len, value))) goto done;
assert(var == NULL || (var->fill_value != att->data));
    }

done:
    /* If there was an error return it, otherwise return any potential
       range error value. If none, return NC_NOERR as usual.*/
    if (range_error) return NC_ERANGE;
    if (stat) return stat;
    return THROW(stat);
}


/*
Create an attribute; This is the core of ncz_put_att above.
Caller must free values.
*/
int
ncz_makeattr(NC_FILE_INFO_T* file, NC_OBJ* container, struct NCZ_AttrInfo* ainfo, NC_ATT_INFO_T** attp)
{
    int stat = NC_NOERR;
    NC_ATT_INFO_T* att = NULL;
    NCZ_ATT_INFO_T* zatt = NULL;
    int new_att = 0;
    NCindex* attlist = NULL;
    
    if(container->sort == NCGRP)
        attlist = ((NC_GRP_INFO_T*)container)->att;
    else
        attlist = ((NC_VAR_INFO_T*)container)->att;
    assert(attlist != NULL);

    if ((stat = nc4_get_typelen_mem(file, ainfo->nctype, &ainfo->typelen))) goto done;

    /* See if there is already an attribute with this name. */
    att = (NC_ATT_INFO_T*)ncindexlookup(attlist,ainfo->name);
    new_att = (att == NULL?1:0);

    if(new_att) {
        if((stat=nc4_att_list_add(attlist,ainfo->name,&att))) goto done;
        if((zatt = calloc(1,sizeof(NCZ_ATT_INFO_T))) == NULL) {stat = NC_ENOMEM; goto done;}
        zatt->common.file = file;
        att->container = container;
        att->format_att_info = zatt;
    }

    att->nc_typeid = ainfo->nctype;
    if(ainfo->datalen > 0 && ainfo->data) {
        /* Fill in the attribute's type and value; ainfo->data is copied  */
        if((stat = NCZ_set_att_data(file,att,ainfo->datalen,ainfo->data))) goto done;
    }
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

/* Write/overwrite NC_XXX_INFO_T fields for fields that have a dual attribute.
   Reclaim old data and replace with copy of data argument
   Do not synchronize with the corresponding attribute.
*/
int
NCZ_set_dual_obj_data(NC_FILE_INFO_T* file, NC_OBJ* object, const char* name, DualAtt which, size_t len, const void* data)
{
    int stat = NC_NOERR;

    NC_UNUSED(name); /* for now */
    if(object->sort == NCGRP) {
	assert(file->root_grp == (NC_GRP_INFO_T*)object);
        switch(which) {
        case DA_DFALTSTRLEN:
            assert(len == 1);
            zsetdfaltstrlen((size_t)((int*)data)[0],file);
            break;
        default: stat = NC_EINVAL; break;
        }
    } else {
        NC_VAR_INFO_T* var = NULL;
        nc_type tid;
        assert(object->sort == NCVAR);
        var = (NC_VAR_INFO_T*)object;
        tid = var->type_info->hdr.id;
        switch(which) {
        case DA_FILLVALUE:
            assert(len == 1);       
            if((stat = NC_reclaim_data_all(file->controller,tid,var->fill_value,len))) goto done;
            var->fill_value = NULL;
            if((stat = NC_copy_data_all(file->controller,tid,data,len,&var->fill_value))) goto done;
            break;
        case DA_MAXSTRLEN:
            assert(len == 1);
            zsetmaxstrlen((size_t)((int*)data)[0],var);
            break;
        case DA_QUANTIZE:
            assert(len == 1);
            var->nsd = ((int*)data)[0];
            break;
        case DA_SEP:
            assert(len == 1);
            zsetmaxstrlen((size_t)((int*)data)[0],var);
            break;
        default: assert(0);
        }
    }
done:
    return THROW(stat);
}

/* Write/overwrite attribute data for any attribute.
   Reclaim old data and replace with copy of data argument.
   Do not synchronize with any corresponding NC_XXX_INFO_T field.
*/
int
NCZ_set_att_data(NC_FILE_INFO_T* file, NC_ATT_INFO_T* att, size_t len, const void* data)
{
    int stat = NC_NOERR;
    void* copy = NULL;

    /* Consistency checks */
    assert((len == 0 && data == NULL) || (len > 0 && data != NULL));
    assert((att->len == 0 && att->data == NULL) || (att->len > 0 && att->data != NULL));

    /* Reclaim any old att->data */
    if(att->data != NULL) {
        /* remove old att data */
        (void)NC_reclaim_data_all(file->controller,att->nc_typeid,att->data,att->len);
        att->data = NULL;
        att->len = 0;
    }

    /* set att->data with a copy of data */
    if(len > 0 && data != NULL) {
        if((stat = NC_copy_data_all(file->controller, att->nc_typeid, data, len, &copy))) goto done;
        /* set the att data */
        att->len = len;
        att->data = copy; copy = NULL;
    }
    att->dirty = NC_TRUE;

done:
    /* cleanup */
    if(copy != NULL) (void)NC_reclaim_data_all(file->controller,att->nc_typeid,copy,len);
    return THROW(stat);
}

/*
Extract data for an attribute
This is essentially Version 2|3 agnostic because the
data part of an attribute is (currently) the same for both versions.
*/
int
NCZ_computeattrdata(NC_FILE_INFO_T* file, const NCjson* jdata, struct NCZ_AttrInfo* ainfo)
{
    int stat = NC_NOERR;
    NCbytes* buf = ncbytesnew();
    NCjson* jtext = NULL;
    int isjson = 0; /* 1 => attribute value is neither scalar nor array of scalars */
    int reclaimvalues = 0;

    NC_UNUSED(file);

    ZTRACE(3,"typeid=%d values=|%s|",ainfo->nctype,NCJtotext(jdata,0));

    /* See if this is a simple vector (or scalar) of atomic types vs more complex json */
    isjson = (ainfo->nctype == NC_JSON || NCZ_iscomplexjson(ainfo->name,jdata));

    /* Get assumed type */
    if(ainfo->nctype == NC_NAT && !isjson) {
        if((stat = NCZ_inferattrtype(ainfo->name,NC_NAT,jdata,&ainfo->nctype))) goto done;
    }

    if(isjson) {
        /* Apply the JSON attribute convention and convert to JSON string */
        ainfo->nctype = NC_CHAR;
        if((stat = json_convention_read(jdata,&jtext))) goto done;
        jdata = jtext; jtext = NULL;
        reclaimvalues = 1;
    } 

    if((stat = NC4_inq_atomic_type(ainfo->nctype, NULL, &ainfo->typelen))) goto done;

    /* Convert the JSON attribute values to the actual netcdf attribute bytes */
    assert(ainfo->data == NULL);
    if((stat = NCZ_attr_convert(jdata,ainfo->nctype,&ainfo->datalen,buf))) goto done;
    ainfo->data = ncbytesextract(buf);

done:
    ncbytesfree(buf);
    if(reclaimvalues) {NCJreclaim((NCjson*)jdata); jdata = NULL; /* we created it */}
    return ZUNTRACEX(THROW(stat),"typeid=%d typelen=%d datalen=%zu",ainfo->nctype,ainfo->typelen,ainfo->datalen);
}

/* Convert a json value to actual data values of an attribute.
@param src - [in] src value
@param typeid - [in] dst type
@param countp - [out] dst length (if dict or array)
@param dst - [out] dst data
*/

int
NCZ_attr_convert(const NCjson* src, nc_type typeid, size_t* countp, NCbytes* dst)
{
    int stat = NC_NOERR;
    size_t i;
    size_t count = 0;

    ZTRACE(3,"src=%s typeid=%d",NCJtotext(src,0),typeid);

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
    struct NCJconst jstr;

    memset(&jstr,0,sizeof(jstr));

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
json_convention_read(const NCjson* json, NCjson** jtextp)
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

/* Syncronize NC_XXX_INFO_T* fields and corresponding Attributes */
int
NCZ_sync_dual_att(NC_FILE_INFO_T* file, NC_OBJ* container, const char* aname, DualAtt which, int direction)
{
    int stat = NC_NOERR;
    NC_VAR_INFO_T* var = NULL;
    NC_ATT_INFO_T* att = NULL;
    int isnew = 0;
    NCZ_FILE_INFO_T* zinfo = (NCZ_FILE_INFO_T*)file->format_file_info;
    NCZ_VAR_INFO_T* zvar = NULL;

    if(container->sort == NCVAR) {
        var = (NC_VAR_INFO_T*)container;
        zvar = (NCZ_VAR_INFO_T*)var->format_var_info;
    }

    if(direction == FIXATT) { /* transfer from NC_XXX_INFO_T* to attribute */
        switch (which) {
        case DA_FILLVALUE:
            if(var->no_fill) {
               stat=NCZ_disable_fill(file,var);
               goto done;
            }
            att = (NC_ATT_INFO_T*)ncindexlookup(var->att,NC_FillValue);
            /*  If _FillValue is new and the proposed value is the dfalt value, then suppress the attribute
                and force user to build default fill values; otherwise set the attribute value */
            if(att == NULL && (var->fill_value == NULL || !NCZ_isdfaltfillvalue(var->type_info->hdr.id,var->fill_value))) {
                /* Not suppressing _FillValue attribute, so go ahead and create it and set value */
                if((stat = NCZ_getattr(file,container,aname,var->type_info->hdr.id,&att,&isnew))) goto done;
                assert(isnew);
                if((stat = NCZ_set_att_data(file,att,1,var->fill_value))) goto done;
            }
            break;
        case DA_MAXSTRLEN:
	    assert(zvar != NULL);
	    assert(zinfo != NULL);
	    if(zvar->maxstrlen > 0) {
		/* If the value is the current max strlen default, then suppress it */
		if(zvar->maxstrlen != zinfo->default_maxstrlen) {
		    if((stat = NCZ_getattr(file,container,aname,NC_INT,&att,&isnew))) goto done;
		    if((stat = NCZ_set_att_data(file,att,1,&zvar->maxstrlen))) goto done;
		}
            } break;
        case DA_DFALTSTRLEN:
	    assert(zinfo != NULL);
	    if(zinfo->default_maxstrlen > 0) {
		/* If the value is the global max strlen default, then suppress it */
		if(zinfo->default_maxstrlen != NCZ_MAXSTR_DFALT) {
		    if((stat = NCZ_getattr(file,container,aname,NC_INT,&att,&isnew))) goto done;
		    if((stat = NCZ_set_att_data(file,att,1,&zinfo->default_maxstrlen))) goto done;
		}
            } break;
        case DA_QUANTIZE:
	    if(var->quantize_mode > 0) {
		if((stat = NCZ_getattr(file,container,aname,NC_INT,&att,&isnew))) goto done;
		if((stat = NCZ_set_att_data(file,att,1,&var->nsd))) goto done;
            } break;
	case DA_SEP: {
		NCglobalstate* gs = NC_getglobalstate();
		assert(gs != NULL);
		assert(gs->zarr.dimension_separator != 0);
		/* If separator is not new and the proposed value is the dfalt value, then suppress the attribute value */
		if(att == NULL && zvar->dimension_separator != '\0'
			&& zvar->dimension_separator != gs->zarr.dimension_separator) {
		    if((stat = NCZ_getattr(file,container,aname,NC_CHAR,&att,&isnew))) goto done;
		    if((stat = NCZ_set_att_data(file,att,1,&zvar->dimension_separator))) goto done;
		}
	    } break;
        case DA_DFALTSEP: {
		NCglobalstate* gs = NC_getglobalstate();
		assert(gs != NULL);
		assert(gs->zarr.dimension_separator != 0);
		/* If separator is not new and the proposed value is the dfalt value, then suppress the attribute value */
	        if(att == NULL && gs->zarr.dimension_separator != '\0'
			&& gs->zarr.dimension_separator != NCZF_default_dimension_separator(file)) {
		    if((stat = NCZ_getattr(file,container,aname,NC_CHAR,&att,&isnew))) goto done;
		    if((stat = NCZ_set_att_data(file,att,1,&gs->zarr.dimension_separator))) goto done;
		}
            } break;
        default:
            stat = NC_ENOTATT;      
            goto done;
        }
    } else if(direction == FIXOBJ) {/* Transfer value from att to NC_XXX_INFO_T* */
        switch(which) {
        case DA_FILLVALUE:
            if((stat = NCZ_getattr(file,container,aname,var->type_info->hdr.id,&att,&isnew))) goto done;
            assert(!isnew);
            if(att->len != 1) goto done; /* some other fill value attribute */
            if((stat = NC_reclaim_data_all(file->controller,var->type_info->hdr.id,var->fill_value,1))) goto done;
            var->fill_value = NULL;
            if((stat = NC_copy_data_all(file->controller,att->nc_typeid,att->data,att->len,&var->fill_value))) goto done;
            break;
        case DA_MAXSTRLEN:
            if((stat = NCZ_getattr(file,container,aname,NC_INT,&att,&isnew))) goto done;
            assert(!isnew);
            if(att->len != 1 || att->nc_typeid != NC_INT) goto done; /* some other _nczarr_maxstrlen */
            zsetmaxstrlen((size_t)((int*)att->data)[0],var);
            break;
        case DA_DFALTSTRLEN:
            if((stat = NCZ_getattr(file,container,aname,NC_INT,&att,&isnew))) goto done;
            assert(!isnew);
            if(att->len != 1 || att->nc_typeid != NC_INT) goto done; /* some other _nczarr_default_maxstrlen */
            zsetdfaltstrlen((size_t)((int*)att->data)[0],file);
            break;
        case DA_QUANTIZE:
            if((stat = NCZ_getattr(file,container,aname,NC_INT,&att,&isnew))) goto done;
            assert(!isnew);
            if(att->len != 1 || att->nc_typeid != NC_INT) goto done; /* some other _QuantXXX */
            var->nsd = ((int*)att->data)[0];
            break;
        case DA_SEP:
            if((stat = NCZ_getattr(file,container,aname,NC_CHAR,&att,&isnew))) goto done;
            assert(!isnew);
            if(att->len != 1 || att->nc_typeid != NC_CHAR) goto done; /* some other _nczarr_dimension_separator */
            zsetdimsep(((char*)att->data)[0],var);
            break;
        case DA_DFALTSEP:
            if((stat = NCZ_getattr(file,container,aname,NC_CHAR,&att,&isnew))) goto done;
            assert(!isnew);
            if(att->len != 1 || att->nc_typeid != NC_CHAR) goto done; /* some other _nczarr_default_dimension_separator */
            zsetdfaltdimsep(((char*)att->data)[0],file);
            break;
        default: assert(0);
        }
    }
    
done:
    return THROW(stat);
}

/* For those attributes that reflect fields in NC_VAR/GRP_INFO_T objects,
 * ensure that those attributes are up-to-date (creating if necessary).
 */
int
NCZ_ensure_dual_attributes(NC_FILE_INFO_T* file, NC_OBJ* container)
{
    int stat = NC_NOERR;
    NC_VAR_INFO_T* var = NULL;

    if(container->sort == NCVAR)
        var = (NC_VAR_INFO_T*)container;    

    /* Some attributes are reflected in var|grp structure so must be sync'd to the attribute */
    if(var != NULL) {
        const char* qname = NULL;

        /* _FillValue */
        /* If var->no_fill == NC_NOFILL, then suppress the _FillValue attribute and
           optionally set the _NoFill attribute */
        if(var->no_fill == NC_NOFILL) {
            if((stat = NCZ_disable_fill(file,var))) goto done;
        } else { /* var->no_fill == NC_FILL*/
            if(var->fill_value != NULL) { /* only update if a fill value is defined */
                if((stat = NCZ_sync_dual_att(file,(NC_OBJ*)var,NC_FillValue,DA_FILLVALUE,FIXATT))) goto done;
            }
        }

        /* _nczarr_maxstrlen */
	{
	    NCZ_VAR_INFO_T* vinfo = (NCZ_VAR_INFO_T*)var->format_var_info;
	    if(vinfo->maxstrlen > 0) {
		if((stat = NCZ_sync_dual_att(file,(NC_OBJ*)var,NC_NCZARR_MAXSTRLEN_ATTR,DA_MAXSTRLEN,FIXATT))) goto done;
	    }
        }
        
        /* __Quantizexxx */
        if(var->quantize_mode > 0) {
            qname = NC_findquantizeattname(var->quantize_mode);
            if(qname != NULL) {/* quantize_mode was set */
                if((stat = NCZ_sync_dual_att(file,(NC_OBJ*)var,qname,DA_QUANTIZE,FIXATT))) goto done;
            }
        }

        /* _nczarr_dimension_separator */
	{
	    NCZ_VAR_INFO_T* vinfo = (NCZ_VAR_INFO_T*)var->format_var_info;
	    if(vinfo->dimension_separator != '\0') {
		if((stat = NCZ_sync_dual_att(file,(NC_OBJ*)var,NC_NCZARR_SEPARATOR_ATTR,DA_SEP,FIXATT))) goto done;
	    }
        }

    } else {
	NCZ_FILE_INFO_T* zinfo = (NCZ_FILE_INFO_T*)file->format_file_info;
	NCglobalstate* gs = NC_getglobalstate();

        assert(file != NULL);
	if(zinfo->default_maxstrlen > 0) {
            if((stat = NCZ_sync_dual_att(file,(NC_OBJ*)file->root_grp,NC_NCZARR_DFALT_MAXSTRLEN_ATTR,DA_DFALTSTRLEN,FIXATT))) goto done;
        }

	assert(gs != NULL);
	assert(gs->zarr.dimension_separator != 0);
        if((stat = NCZ_sync_dual_att(file,(NC_OBJ*)file->root_grp,NC_NCZARR_DFALT_SEPARATOR_ATTR,DA_DFALTSEP,FIXATT))) goto done;

    }

done:
    return THROW(stat);
}

/* Get/create attribute */
int
NCZ_getattr(NC_FILE_INFO_T* file, NC_OBJ* container, const char* aname, nc_type nctype, NC_ATT_INFO_T** attp, int* isnewp)
{
    int stat = NC_NOERR;
    struct NCZ_AttrInfo ainfo = NCZ_emptyAttrInfo();
    NC_GRP_INFO_T* grp = NULL;
    NC_VAR_INFO_T* var = NULL;
    NC_ATT_INFO_T* att = NULL;
    NCindex* attlist = NULL;
    
    if(container->sort == NCGRP) {
        grp = (NC_GRP_INFO_T*)container;
        attlist = grp->att;
    } else { /*container->sort == NCVAR*/
        var = (NC_VAR_INFO_T*)container;
        attlist = var->att;
    }

    att = (NC_ATT_INFO_T*)ncindexlookup(attlist,aname);
    if(att == NULL) { /* create it */
        NCZ_clearAttrInfo(file,&ainfo);
        ainfo.name = aname;
        ainfo.nctype = nctype;
        if((stat = ncz_makeattr(file,container,&ainfo,&att))) goto done;
        if(isnewp) *isnewp = 1;
    } else {if(isnewp) *isnewp = 0;}

    if((stat = islegalatt(file,att,att->len))) goto done; /* verify legality */

    if(attp) *attp = att;
done:
    NCZ_clearAttrInfo(file,&ainfo);
    return THROW(stat);
}

static int
islegalatt(NC_FILE_INFO_T* file, NC_ATT_INFO_T* att, size_t alen)
{
    int stat = NC_NOERR;
    if (!att) {
        if (!(file->flags & NC_INDEF)) { /* if this is a new att, require define mode. */
            if (file->cmode & NC_CLASSIC_MODEL) {stat = NC_ENOTINDEFINE; goto done;}
            file->flags |= NC_INDEF;/* set define mode. */
            file->redef = NC_TRUE; /* for nc_abort, we need to remember if we're in define mode as a redef. */
        }
    } else {
        /* for an existing att, if we're not in define mode, the len
           must not be greater than the existing len for classic model. */
        if(!(file->flags & NC_INDEF) 
            && alen * nc4typelen(att->nc_typeid) > (size_t)att->len * nc4typelen(att->nc_typeid)) {
            if (file->cmode & NC_CLASSIC_MODEL) {stat = NC_ENOTINDEFINE; goto done;}
            file->flags |= NC_INDEF;/* set define mode. */
            file->redef = NC_TRUE; /* for nc_abort, we need to remember if we're in define mode as a redef. */
        }
    }
done:
    return THROW(stat);
}

int
NCZ_reclaim_att_data(NC_FILE_INFO_T* file, NC_ATT_INFO_T* att)
{
    int stat = NC_NOERR;
    int tid = att->nc_typeid;

    if(att->data != NULL) {
        stat = NC_reclaim_data_all(file->controller,tid,att->data,att->len);
        att->data = NULL;
        att->len = 0;
    }
    return stat;
}
