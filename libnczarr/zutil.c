/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

/**
 * @file
 * @internal Misc. utility code
 *
 * @author Dennis Heimbigner
 */

#include "zincludes.h"

/**************************************************/
/* Static zarr type name table */

static const char* znames_little[NUM_ATOMIC_TYPES] = {
NULL,  /*NC_NAT*/ "<i1", /*NC_BYTE*/ "<u1", /*NC_CHAR*/ "<i2", /*NC_SHORT*/
"<i4", /*NC_INT*/ "<f4", /*NC_FLOAT*/ "<f8", /*NC_DOUBLE*/ "<u1", /*NC_UBYTE*/
"<u2", /*NC_USHORT*/ "<u4", /*NC_UINT*/ "<i8", /*NC_INT64*/ "<u8", /*NC_UINT64*/
NULL,  /*NC_STRING*/
};

static const char* znames_big[NUM_ATOMIC_TYPES] = {
NULL,  /*NC_NAT*/
">i1", /*NC_BYTE*/   ">u1", /*NC_CHAR*/   ">i2", /*NC_SHORT*/ ">i4", /*NC_INT*/
">f4", /*NC_FLOAT*/  ">f8", /*NC_DOUBLE*/ ">u1", /*NC_UBYTE*/
">u2", /*NC_USHORT*/ ">u4", /*NC_UINT*/   ">i8", /*NC_INT64*/ ">u8", /*NC_UINT64*/
NULL,  /*NC_STRING*/
};

static const char* zfillvalue[NUM_ATOMIC_TYPES] = {
NULL, /*NC_NAT*/
"-127", /*NC_BYTE*/
"0", /*NC_CHAR*/
"-32767", /*NC_SHORT*/
"-2147483647", /*NC_INT*/
"9.9692099683868690e+36f", /* near 15 * 2^119 */ /*NC_FLOAT*/
"9.9692099683868690e+36", /*NC_DOUBLE*/
"255", /*NC_UBYTE*/
"65535", /*NC_USHORT*/
"4294967295", /*NC_UINT*/
"-9223372036854775806", /*NC_INT64*/
"18446744073709551614", /*NC_UINT64*/
};

/* map nc_type -> NCJ_SORT */
static int zjsonsort[NUM_ATOMIC_TYPES] = {
NCJ_UNDEF, /*NC_NAT*/
NCJ_INT, /*NC_BYTE*/
NCJ_INT, /*NC_CHAR*/
NCJ_INT, /*NC_SHORT*/
NCJ_INT, /*NC_INT*/
NCJ_DOUBLE, /*NC_FLOAT*/
NCJ_DOUBLE, /*NC_DOUBLE*/
NCJ_INT, /*NC_UBYTE*/
NCJ_INT, /*NC_USHORT*/
NCJ_INT, /*NC_UINT*/
NCJ_INT, /*NC_INT64*/
NCJ_INT, /*NC_UINT64*/
};


/**************************************************/

/**
@internal Get full path for a group
@param grp - [in] group
@param pathp - [out] full path
@return NC_NOERR
@author Dennis Heimbigner
*/
int
NCZ_grppath(NC_GRP_INFO_T* grp, char** pathp)
{
    int stat = NC_NOERR;
    NClist* segments = nclistnew();
    NCbytes* path = NULL;
    NC_GRP_INFO_T* parent = NULL;
    int i;

    nclistinsert(segments,0,grp);
    parent = grp->parent;
    while(parent != NULL) {
        nclistinsert(segments,0,parent);
        parent = parent->parent;
    }
    path = ncbytesnew();
    for(i=0;i<nclistlength(segments);i++) {
	grp = nclistget(segments,i);
	if(i > 1) ncbytescat(path,"/"); /* Assume root is named "/" */
	ncbytescat(path,grp->hdr.name);
    }        
    if(pathp) *pathp = ncbytesextract(path);

    nclistfree(segments);
    ncbytesfree(path);
    return stat;

}

/**
@internal Get full path for a var
@param var - [in] var
@param pathp - [out] full path
@return NC_NOERR
@author Dennis Heimbigner
*/
int
NCZ_varpath(NC_VAR_INFO_T* var, char** pathp)
{
    int stat = NC_NOERR;
    char* grppath = NULL;
    char* varpath = NULL;

    /* Start by creating the full path for the parent group */
    if((stat = NCZ_grppath(var->container,&grppath)))
	goto done;
    /* Create the suffix path using the var name */
    if((stat = nczm_suffix(grppath,var->hdr.name,&varpath)))
	goto done;
    /* return path */
    if(pathp) {*pathp = varpath; varpath = NULL;}

done:
    nullfree(grppath);
    nullfree(varpath);
    return stat;
}

/**
@internal Get full path for a dimension
@param dim - [in] dim
@param pathp - [out] full path
@return NC_NOERR
@author Dennis Heimbigner
*/
int
NCZ_dimpath(NC_DIM_INFO_T* dim, char** pathp)
{
    int stat = NC_NOERR;
    char* grppath = NULL;
    char* dimpath = NULL;

    /* Start by creating the full path for the parent group */
    if((stat = NCZ_grppath(dim->container,&grppath)))
	goto done;
    /* Create the suffix path using the dim name */
    if((stat = nczm_suffix(grppath,dim->hdr.name,&dimpath)))
	goto done;
    /* return path */
    if(pathp) {*pathp = dimpath; dimpath = NULL;}

done:
    nullfree(grppath);
    nullfree(dimpath);
    return stat;
}

/**
@internal Split a name path on '/' characters
@param  path - [in]
@param segments - [out] split path
@return NC_NOERR
@author Dennis Heimbigner
*/
int
ncz_splitpath(const char* path, NClist* segments)
{
    int stat = NC_NOERR;
    const char* p = NULL;
    const char* q = NULL;
    ptrdiff_t len = 0;
    char* seg = NULL;

    if(path == NULL || strlen(path)==0 || segments == NULL)
	{stat = NC_EINVAL; goto done;}

    p = path;
    if(p[0] == '/') p++;
    for(;*p;) {
	q = strchr(p,'/');
	if(q==NULL)
	    q = p + strlen(p); /* point to trailing nul */
        len = (q - p);
	if(len == 0)
	    {stat = NC_EURL; goto done;}
	if((seg = malloc(len+1)) == NULL)
	    {stat = NC_ENOMEM; goto done;}
	memcpy(seg,p,len);
	seg[len] = '\0';
	nclistpush(segments,seg);
	seg = NULL; /* avoid mem errors */
	if(*q) p = q+1; else p = q;
    }

done:
    if(seg != NULL) free(seg);
    return THROW(stat);
}

/**************************************************/
/* Json sync code */

/**
@internal Down load a .z... structure into memory
@param zmap - [in] controlling zarr map
@param key - [in] .z... object to load
@param jsonp - [out] root of the loaded json
@return NC_NOERR
@author Dennis Heimbigner
*/
int
NCZ_downloadjson(NCZMAP* zmap, const char* key, NCjson** jsonp)
{
    int stat = NC_NOERR;
    ssize64_t len;
    char* content = NULL;
    NCjson* json = NULL;

    if((stat = nczmap_len(zmap, key, &len)))
	goto done;
    if((content = malloc(len+1)) == NULL)
	{stat = NC_ENOMEM; goto done;}
    if((stat = nczmap_readmeta(zmap, key, len, content)))
	goto done;
    content[len] = '\0';

    if((stat = NCJparse(content,0,&json)))
	goto done;

    if(jsonp) *jsonp = json;
    json = NULL;

done:
    if(stat) NCJreclaim(json);
    nullfree(content);
    return stat;
}

/**
@internal  Create a specified object exists; do nothing if already exists.
@param zmap - [in] controlling zarr map
@param key - [in] .z... object to create

@return NC_NOERR
@author Dennis Heimbigner
*/
int
NCZ_createobject(NCZMAP* zmap, const char* key, ssize64_t size)
{
    int stat = NC_NOERR;

    /* create the target */
    stat = nczmap_def(zmap, key, size);
    return stat;
}

/**
@internal  Upload a modified json tree to a .z... structure.
@param zmap - [in] controlling zarr map
@param key - [in] .z... object to load
@param json - [in] root of the json tree
@return NC_NOERR
@author Dennis Heimbigner
*/
int
NCZ_uploadjson(NCZMAP* zmap, const char* key, NCjson* json)
{
    int stat = NC_NOERR;
    char* content = NULL;
    
    /* Unparse the modified json tree */
    if((stat = NCJunparse(json,0,&content)))
	goto done;

    /* create the target */
    if((stat = nczmap_def(zmap, key, NCZ_ISMETA)))
	goto done;

    /* Write the metadata */
    if((stat = nczmap_writemeta(zmap, key, strlen(content), content)))
	goto done;

done:
    nullfree(content);
    return stat;
}

#if 0
/**
@internal create object, return empty dict; ok if already exists.
@param zmap - [in] map
@param key - [in] key of the object
@param jsonp - [out] return parsed json
@return NC_NOERR
@return NC_EINVAL if object exists
@author Dennis Heimbigner
*/
int
NCZ_createdict(NCZMAP* zmap, const char* key, NCjson** jsonp)
{
    int stat = NC_NOERR;
    NCjson* json = NULL;

    /* See if it already exists */
    stat = NCZ_downloadjson(zmap,key,&json);
    if(stat != NC_NOERR) {
	if(stat == NC_EACCESS) {/* create it */
	    if((stat = nczmap_def(zmap,key,NCZ_ISMETA)))
		goto done;	    
        } else
	    goto done;
    } else {
	/* Already exists, fail */
	stat = NC_EINVAL;
	goto done;
    }
    /* Create the empty dictionary */
    if((stat = NCJnew(NCJ_DICT,&json)))
	goto done;
    if(jsonp) *jsonp = json;
    json = NULL;
done:
    if(stat) NCJreclaim(json);
    return stat;
}

/**
@internal create object, return empty array; ok if already exists.
@param zmap - [in] map
@param key - [in] key of the object
@param jsonp - [out] return parsed json
@return NC_NOERR
@return NC_EINVAL if object exits
@author Dennis Heimbigner
*/
int
NCZ_createarray(NCZMAP* zmap, const char* key, NCjson** jsonp)
{
    int stat = NC_NOERR;
    NCjson* json = NULL;

    stat = NCZ_downloadjson(zmap,key,&json);
    if(stat != NC_NOERR) {
	if(stat == NC_EACCESS) {/* create it */
	    if((stat = nczmap_def(zmap,key,NCZ_ISMETA)))
		goto done;	    
	    /* Create the initial array */
	    if((stat = NCJnew(NCJ_ARRAY,&json)))
		goto done;
        } else {
	    stat = NC_EINVAL;
	    goto done;
	}
    }
    if(json->sort != NCJ_ARRAY) {stat = NC_ENCZARR; goto done;}
    if(jsonp) *jsonp = json;
    json = NULL;
done:
    if(stat) NCJreclaim(json);
    return stat;
}
#endif /*0*/

/**
@internal Get contents of a meta object; fail it it does not exist
@param zmap - [in] map
@param key - [in] key of the object
@param jsonp - [out] return parsed json
@return NC_NOERR
@return NC_EACCESS [object did not exist]
@author Dennis Heimbigner
*/
int
NCZ_readdict(NCZMAP* zmap, const char* key, NCjson** jsonp)
{
    int stat = NC_NOERR;
    NCjson* json = NULL;

    if((stat = NCZ_downloadjson(zmap,key,&json)))
	goto done;
    if(json->sort != NCJ_DICT) {stat = NC_ENCZARR; goto done;}
    if(jsonp) *jsonp = json;
    json = NULL;
done:
    if(stat) NCJreclaim(json);
    return stat;
}

/**
@internal Get contents of a meta object; fail it it does not exist
@param zmap - [in] map
@param key - [in] key of the object
@param jsonp - [out] return parsed json
@return NC_NOERR
@return NC_EACCESS [object did not exist]
@author Dennis Heimbigner
*/
int
NCZ_readarray(NCZMAP* zmap, const char* key, NCjson** jsonp)
{
    int stat = NC_NOERR;
    NCjson* json = NULL;

    if((stat = NCZ_downloadjson(zmap,key,&json)))
	goto done;
    if(json->sort != NCJ_ARRAY) {stat = NC_ENCZARR; goto done;}
    if(jsonp) *jsonp = json;
    json = NULL;
done:
    if(stat) NCJreclaim(json);
    return stat;
}

/**
@internal Given an nc_type+other, produce the corresponding
zarr type name.
@param nctype - [in] nc_type
@param little - [in] 1=>little, 0 => big
@param namep - [out] pointer to hold pointer to the name
@return NC_NOERR
@author Dennis Heimbigner
*/

int
ncz_zarr_type_name(nc_type nctype, int little, const char** znamep)
{
    if(nctype <= 0 || nctype >= NC_STRING) return NC_EINVAL;
    if(little) {
	if(znamep) *znamep = znames_little[nctype];
    } else {
	if(znamep) *znamep = znames_big[nctype];
    }
    return NC_NOERR;	        
}

/**
@internal Given an nc_type, produce the corresponding
default fill value as a string.
@param nctype - [in] nc_type
@param defaltp - [out] pointer to hold pointer to the value
@return NC_NOERR
@author Dennis Heimbigner
*/

int
ncz_default_fill_value(nc_type nctype, const char** dfaltp)
{
    if(nctype <= 0 || nctype >= NC_STRING) return NC_EINVAL;
    if(dfaltp) *dfaltp = zfillvalue[nctype];
    return NC_NOERR;	        
}

/**
@internal Given an nc_type, produce the corresponding
fill value sort
@param nctype - [in] nc_type
@param sortp - [out] pointer to hold pointer to the sort
@return NC_NOERR
@author Dennis Heimbigner
*/

int
ncz_fill_value_sort(nc_type nctype, int* sortp)
{
    if(nctype <= 0 || nctype >= NC_STRING) return NC_EINVAL;
    if(sortp) *sortp = zjsonsort[nctype];
    return NC_NOERR;	        
}

/* Return 1 if this machine is little endian */
int
NCZ_isLittleEndian(void)
{
    union {
        unsigned char bytes[SIZEOF_INT];
	int i;
    } u;
    u.i = 1;
    return (u.bytes[0] == 1 ? 1 : 0);
}


/*
Given a path to a group, return the list of objects
that contain another object with the name of the tag.
For example, we can get the immediate list of subgroups
by using the tag ".zgroup".
Basically we return the set of X where <prefix>/X/<tag>
is an object in the map.
Note: need to test with "/", "", and with and without trailing "/".
*/
int
NCZ_subobjects(NCZMAP* map, const char* prefix, const char* tag, NClist* objlist)
{
    int i,stat=NC_NOERR;
    NClist* matches = nclistnew();
    size_t prelen = strlen(prefix);
    size_t taglen = strlen(tag);

    /* Get the path list */
    if((stat = nczmap_search(map,prefix,matches))) goto done;
    for(i=0;i<nclistlength(matches);i++) {
	const char* path = nclistget(matches,i);
	const char* p0;
	const char* p;
	/*point past the prefix (including any trailing '/') */
	p0 = path+prelen;
	p = strchr(p0,NCZM_SEP[0]); /* point where tag should start */
	if(p == NULL) continue; /* There is no possible tag */
	if(strlen(p) != taglen+1) continue; /* tag is too long */
	if(memcmp(p+1,tag,taglen)==0) {
	    nclistpush(objlist,strdup(path)); /* save match */
	}			
    }

done:
    nclistfreeall(matches);
    return stat;
}

int
ncz_dtype2typeinfo(const char* dtype, nc_type* nctypep, int* endianp)
{
    int stat = NC_NOERR;
    int typelen = 0;
    int count;
    char tchar;
    nc_type nctype = NC_NAT;
    int endianness = 0;

    if(endianness) *endianp = NC_ENDIAN_NATIVE;
    if(nctypep) *nctypep = NC_NAT;

    if(dtype == NULL) goto zerr;
    if(strlen(dtype) < 3) goto zerr;
    switch (dtype[0]) {
    case '<': endianness = NC_ENDIAN_LITTLE; break;
    case '>': endianness = NC_ENDIAN_BIG; break;
    default: goto zerr;
    }
    /* Decode the type length */
    count = sscanf(dtype+2,"%d",&typelen);
    if(count != 1) goto zerr;
    tchar = dtype[1];
    switch(typelen) {
    case 1:
	switch (tchar) {
	case 'i': nctype = NC_BYTE; break;
	case 'u': nctype = NC_UBYTE; break;
	default: goto zerr;
	}
	break;
    case 2:
	switch (tchar) {
	case 'i': nctype = NC_SHORT; break;
	case 'u': nctype = NC_USHORT; break;
	default: goto zerr;
	}
	break;
    case 4:
	switch (tchar) {
	case 'i': nctype = NC_INT; break;
	case 'u': nctype = NC_UINT; break;
	case 'f': nctype = NC_FLOAT; break;
	default: goto zerr;
	}
	break;
    case 8:
	switch (tchar) {
	case 'i': nctype = NC_INT64; break;
	case 'u': nctype = NC_UINT64; break;
	case 'f': nctype = NC_DOUBLE; break;
	default: goto zerr;
	}
	break;
    default: goto zerr;
    }

    if(nctypep) *nctypep = nctype;
    if(endianp) *endianp = endianness;

done:
    return stat;
zerr:
    stat = NC_ENCZARR;
    goto done;
}


/**
@internal Similar to NCZ_grppath, but using group ids.
@param gid - [in] group id
@param pathp - [out] full path
@return NC_NOERR
@author Dennis Heimbigner
*/
int
NCZ_grpname_full(int gid, char** pathp)
{
    int stat = NC_NOERR;
    size_t len;
    char* path = NULL;

    if((stat = nc_inq_grpname_full(gid,&len,NULL))) return stat;
    if((path=malloc(len+1)) == NULL) return NC_ENOMEM;    
    if((stat = nc_inq_grpname_full(gid,&len,path))) return stat;
    path[len] = '\0'; /* ensure null terminated */
    if(pathp) {*pathp = path; path = NULL;}
    return stat;
}

/**
@internal Parse a commified string list
@param s [in] string to parse
@param list - [in/out] storage for the parsed list
@return NC_NOERR
@author Dennis Heimbigner
*/
int
NCZ_comma_parse(const char* s, NClist* list)
{
    int stat = NC_NOERR;
    const char* p = NULL;
    const char* endp = NULL;

    if(s == NULL || *s == '\0') goto done;

    /* Split s at the commas or EOL */
    p = s;
    for(;;) {
	char* s;
	ptrdiff_t slen;
	endp = strchr(p,',');
	if(endp == NULL) endp = p + strlen(p);
	slen = (endp - p);
	if((s = malloc(slen+1)) == NULL) {stat = NC_ENOMEM; goto done;}
	memcpy(s,p,slen);
	s[slen] = '\0';
	if(nclistmatch(list,s,0)) {
	    nullfree(s); /* duplicate */
	} else {
	    nclistpush(list,s);
	}
	if(*endp == '\0') break;
	p = endp+1;
    }

done:
    return stat;
}
