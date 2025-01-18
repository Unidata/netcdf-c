/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#include "zincludes.h"
#include "zformat.h"
#ifdef NETCDF_ENABLE_NCZARR_FILTERS
#include "zfilter.h"
#endif

/**************************************************/

struct ZOBJ NCZ_emptyzobj(void)
{
    static struct ZOBJ empty = {NULL,NULL,0};
    return empty;
}

/**************************************************/

extern int NCZF2_initialize(void);
extern int NCZF2_finalize(void);
extern int NCZF3_initialize(void);
extern int NCZF3_finalize(void);

/**************************************************/

int
NCZF_initialize(void)
{
    int stat = NC_NOERR;
    if((stat=NCZF2_initialize())) goto done;
    if((stat=NCZF3_initialize())) goto done;
done:
    return THROW(stat);
}

int
NCZF_finalize(void)
{
    int stat = NC_NOERR;
    if((stat=NCZF2_finalize())) goto done;
    if((stat=NCZF3_finalize())) goto done;
done:
    return THROW(stat);
}


/**************************************************/
/*File-Level Operations*/
int
NCZF_create(NC_FILE_INFO_T* file, NCURI* uri, NCZMAP* map)
{
    int stat = NC_NOERR;
    NCZ_FILE_INFO_T* zfile = NULL;

    zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    assert(zfile != NULL);
    stat = zfile->dispatcher->create(file,uri,map);
    return THROW(stat);
}

int
NCZF_open(NC_FILE_INFO_T* file, NCURI* uri, NCZMAP* map)
{
    int stat = NC_NOERR;
    NCZ_FILE_INFO_T* zfile = NULL;

    zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    assert(zfile != NULL);
    stat = zfile->dispatcher->open(file,uri,map);
    return THROW(stat);
}

int
NCZF_close(NC_FILE_INFO_T* file)
{
    int stat = NC_NOERR;
    NCZ_FILE_INFO_T* zfile = NULL;

    zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    assert(zfile != NULL);
    stat = zfile->dispatcher->close(file);
    return THROW(stat);
}

/*Read JSON Metadata*/
int
NCZF_download_grp(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, struct ZOBJ* zobj)
{
    int stat = NC_NOERR;
    NCZ_FILE_INFO_T* zfile = NULL;

    zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    assert(zfile != NULL);
    stat = zfile->dispatcher->download_grp(file,grp,zobj);
    return THROW(stat);
}

int
NCZF_download_var(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, struct ZOBJ* zobj)
{
    int stat = NC_NOERR;
    NCZ_FILE_INFO_T* zfile = NULL;

    zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    assert(zfile != NULL);
    stat = zfile->dispatcher->download_var(file,var,zobj);
    return THROW(stat);
}

int
NCZF_decode_group(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, struct ZOBJ* zgroup, NCjson** jzgrpp, NCjson** jzsuperp)
{
    int stat = NC_NOERR;
    NCZ_FILE_INFO_T* zfile = NULL;

    zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    assert(zfile != NULL);
    stat = zfile->dispatcher->decode_group(file,grp,zgroup,jzgrpp,jzsuperp);
    return THROW(stat);
}

int
NCZF_decode_superblock(NC_FILE_INFO_T* file, const NCjson* jsuper, int* zformat, int* nczformat)
{
    int stat = NC_NOERR;
    NCZ_FILE_INFO_T* zfile = NULL;
    zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    assert(zfile != NULL);
    stat = zfile->dispatcher->decode_superblock(file,jsuper,zformat,nczformat);
    return THROW(stat);
}

int
NCZF_decode_nczarr_group(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, const NCjson* jnczgrp, NClist* vars, NClist* subgrps, NClist* dimrefs)
{
    int stat = NC_NOERR;
    NCZ_FILE_INFO_T* zfile = NULL;

    zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    assert(zfile != NULL);
    stat = zfile->dispatcher->decode_nczarr_group(file,grp,jnczgrp,vars,subgrps,dimrefs);
    return THROW(stat);
}

int
NCZF_decode_var(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, struct ZOBJ* zobj, NClist* jfilters, size64_t** shapesp, size64_t** chunksp, NClist* dimrefs)
{
    int stat = NC_NOERR;
    NCZ_FILE_INFO_T* zfile = NULL;

    zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    assert(zfile != NULL);
    stat = zfile->dispatcher->decode_var(file,var,zobj,jfilters,shapesp,chunksp,dimrefs);
    return THROW(stat);
}

int
NCZF_decode_attributes(NC_FILE_INFO_T* file, NC_OBJ* container, const NCjson* jatts)
{
    int stat = NC_NOERR;
    NCZ_FILE_INFO_T* zfile = NULL;

    zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    assert(zfile != NULL);
    stat = zfile->dispatcher->decode_attributes(file,container,jatts);
    return THROW(stat);
}

int
NCZF_upload_grp(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, struct ZOBJ* zobj)
{
    int stat = NC_NOERR;
    NCZ_FILE_INFO_T* zfile = NULL;

    zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    assert(zfile != NULL);
    stat = zfile->dispatcher->upload_grp(file,grp,zobj);
    return THROW(stat);
}

int
NCZF_upload_var(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, struct ZOBJ* zobj)
{
    int stat = NC_NOERR;
    NCZ_FILE_INFO_T* zfile = NULL;

    zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    assert(zfile != NULL);
    stat = zfile->dispatcher->upload_var(file,var,zobj);
    return THROW(stat);
}

/*Write JSON Metadata*/
int
NCZF_encode_superblock(NC_FILE_INFO_T* file, NCjson** jsuperp)
{
    int stat = NC_NOERR;
    NCZ_FILE_INFO_T* zfile = NULL;

    zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    assert(zfile != NULL);
    stat = zfile->dispatcher->encode_superblock(file,jsuperp);
    return THROW(stat);
}

int
NCZF_encode_nczarr_group(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, NCjson** jzgroupp)
{
    int stat = NC_NOERR;
    NCZ_FILE_INFO_T* zfile = NULL;

    zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    assert(zfile != NULL);
    stat = zfile->dispatcher->encode_nczarr_group(file,grp,jzgroupp);
    return THROW(stat);
}

int
NCZF_encode_group(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, NCjson** jgroupp)
{
    int stat = NC_NOERR;
    NCZ_FILE_INFO_T* zfile = NULL;

    zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    assert(zfile != NULL);
    stat = zfile->dispatcher->encode_group(file,grp,jgroupp);
    return THROW(stat);
}

int
NCZF_encode_nczarr_array(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, NCjson** jzvarp)
{
    int stat = NC_NOERR;
    NCZ_FILE_INFO_T* zfile = NULL;

    zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    assert(zfile != NULL);
    stat = zfile->dispatcher->encode_nczarr_array(file,var,jzvarp);
    return THROW(stat);
}

int
NCZF_encode_var(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, NClist* filtersj, NCjson** jvarp)
{
    int stat = NC_NOERR;
    NCZ_FILE_INFO_T* zfile = NULL;

    zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    assert(zfile != NULL);
    stat = zfile->dispatcher->encode_var(file,var,filtersj,jvarp);
    return THROW(stat);
}

int
NCZF_encode_attributes(NC_FILE_INFO_T* file, NC_OBJ* container, NCjson** jnczconp, NCjson** jsuperp, NCjson** jattsp)
{
    int stat = NC_NOERR;
    NCZ_FILE_INFO_T* zfile = NULL;

    zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    assert(zfile != NULL);
    stat = zfile->dispatcher->encode_attributes(file,container,jnczconp,jsuperp,jattsp);
    return THROW(stat);
}


/*Filter Processing*/
int
NCZF_encode_filter(NC_FILE_INFO_T* file, NCZ_Filter* filter, NCjson** jfilterp)
{
    int stat = NC_NOERR;
    NCZ_FILE_INFO_T* zfile = NULL;

    zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    assert(zfile != NULL);
    stat = zfile->dispatcher->encode_filter(file,filter,jfilterp);
    return THROW(stat);
}

int
NCZF_decode_filter(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, NCjson* jfilter, NCZ_Filter* filter)
{
    int stat = NC_NOERR;
    NCZ_FILE_INFO_T* zfile = NULL;

    zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    assert(zfile != NULL);
    stat = zfile->dispatcher->decode_filter(file,var,jfilter,filter);
    return THROW(stat);
}

/*Search*/
int
NCZF_searchobjects(NC_FILE_INFO_T* file, NC_GRP_INFO_T* grp, NClist* varnames, NClist* subgrpnames)
{
    int stat = NC_NOERR;
    NCZ_FILE_INFO_T* zfile = NULL;

    zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    assert(zfile != NULL);
    stat = zfile->dispatcher->searchobjects(file,grp,varnames,subgrpnames);
    return THROW(stat);
}

/*Chunkkeys*/

/*
From Zarr Specification:
"The compressed sequence of bytes for each chunk is stored under
a key formed from the index of the chunk within the grid of
chunks representing the array.  To form a string key for a
chunk, the indices are converted to strings and concatenated
with the dimension_separator character ('.' or '/') separating
each index. For example, given an array with shape (10000,
10000) and chunk shape (1000, 1000) there will be 100 chunks
laid out in a 10 by 10 grid. The chunk with indices (0, 0)
provides data for rows 0-1000 and columns 0-1000 and is stored
under the key "0.0"; the chunk with indices (2, 4) provides data
for rows 2000-3000 and columns 4000-5000 and is stored under the
key "2.4"; etc."
*/

int
NCZF_encode_chunkkey(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, size_t rank, const size64_t* chunkindices, char dimsep, char** keyp)
{
    int stat = NC_NOERR;
    NCZ_FILE_INFO_T* zfile = NULL;

    zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    assert(zfile != NULL);
    stat = zfile->dispatcher->encode_chunkkey(file,var,rank,chunkindices,dimsep,keyp);
    return THROW(stat);
}

int
NCZF_decode_chunkkey(NC_FILE_INFO_T* file, NC_VAR_INFO_T* var, const char* chunkname, size_t* rankp, size64_t** chunkindicesp)
{
    int stat = NC_NOERR;
    NCZ_FILE_INFO_T* zfile = NULL;

    zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    assert(zfile != NULL);
    stat = zfile->dispatcher->decode_chunkkey(file,var,chunkname,rankp,chunkindicesp);
    return THROW(stat);
}

/* _ARRAY_DIMENSIONS (xarray) encode/decode */
int
NCZF_encode_xarray(NC_FILE_INFO_T* file, size_t rank, NC_DIM_INFO_T** dims, char** xarraydims, size_t* zarr_rank)
{
    int stat = NC_NOERR;
    NCZ_FILE_INFO_T* zfile = NULL;

    zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    assert(zfile != NULL);
    stat = zfile->dispatcher->encode_xarray(file,rank,dims,xarraydims,zarr_rank);
    return THROW(stat);
}

/* Format specific default dimension separator */
char
NCZF_default_dimension_separator(NC_FILE_INFO_T* file)
{
    NCZ_FILE_INFO_T* zfile = NULL;

    zfile = (NCZ_FILE_INFO_T*)file->format_file_info;
    assert(zfile != NULL);
    return zfile->dispatcher->default_dimension_separator(file);
}

/**************************************************/
/* Misc. */

void
NCZ_clear_zobj(struct ZOBJ* zobj)
{
    if(zobj != NULL) {
        NCZ_reclaim_json(zobj->jobj);
	if(!zobj->constjatts) NCZ_reclaim_json(zobj->jatts);
	memset(zobj,0,sizeof(struct ZOBJ));
    }
}

void
NCZ_reclaim_zobj(struct ZOBJ* zobj)
{
    NCZ_clear_zobj(zobj);
    nullfree(zobj);
}

void
NCZ_reclaim_json(NCjson* json)
{
    NCJreclaim(json);
}

void
NCZ_reclaim_json_list(NClist* listj)
{
    size_t i;
    for(i=0;i<nclistlength(listj);i++)
        NCZ_reclaim_json((NCjson*)nclistget(listj,i));
    nclistfree(listj);
}
