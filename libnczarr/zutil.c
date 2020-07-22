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

#undef DEBUG

/**************************************************/
/* Static zarr type name table */

static const char* znames_little[NUM_ATOMIC_TYPES] = {
NULL,  /*NC_NAT*/ "<i1", /*NC_BYTE*/ "<U1", /*NC_CHAR*/ "<i2", /*NC_SHORT*/
"<i4", /*NC_INT*/ "<f4", /*NC_FLOAT*/ "<f8", /*NC_DOUBLE*/ "<u1", /*NC_UBYTE*/
"<u2", /*NC_USHORT*/ "<u4", /*NC_UINT*/ "<i8", /*NC_INT64*/ "<u8", /*NC_UINT64*/
NULL,  /*NC_STRING*/
};

static const char* znames_big[NUM_ATOMIC_TYPES] = {
NULL,  /*NC_NAT*/
">i1", /*NC_BYTE*/   ">U1", /*NC_CHAR*/   ">i2", /*NC_SHORT*/ ">i4", /*NC_INT*/
">f4", /*NC_FLOAT*/  ">f8", /*NC_DOUBLE*/ ">u1", /*NC_UBYTE*/
">u2", /*NC_USHORT*/ ">u4", /*NC_UINT*/   ">i8", /*NC_INT64*/ ">u8", /*NC_UINT64*/
NULL,  /*NC_STRING*/
};

#if 0
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
#endif

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
@internal Get key for a group
@param grp - [in] group
@param pathp - [out] full path
@return NC_NOERR
@author Dennis Heimbigner
*/
int
NCZ_grpkey(const NC_GRP_INFO_T* grp, char** pathp)
{
    int stat = NC_NOERR;
    NClist* segments = nclistnew();
    NCbytes* path = NULL;
    NC_GRP_INFO_T* parent = NULL;
    int i;

    nclistinsert(segments,0,(void*)grp);
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
@internal Get key for a var
@param var - [in] var
@param pathp - [out] full path
@return NC_NOERR
@author Dennis Heimbigner
*/
int
NCZ_varkey(const NC_VAR_INFO_T* var, char** pathp)
{
    int stat = NC_NOERR;
    char* grppath = NULL;
    char* varpath = NULL;

    /* Start by creating the full path for the parent group */
    if((stat = NCZ_grpkey(var->container,&grppath)))
	goto done;
    /* Create the suffix path using the var name */
    if((stat = nczm_concat(grppath,var->hdr.name,&varpath)))
	goto done;
    /* return path */
    if(pathp) {*pathp = varpath; varpath = NULL;}

done:
    nullfree(grppath);
    nullfree(varpath);
    return stat;
}

/**
@internal Get key for a dimension
@param dim - [in] dim
@param pathp - [out] full path
@return NC_NOERR
@author Dennis Heimbigner
*/
int
NCZ_dimkey(const NC_DIM_INFO_T* dim, char** pathp)
{
    int stat = NC_NOERR;
    char* grppath = NULL;
    char* dimpath = NULL;

    /* Start by creating the full path for the parent group */
    if((stat = NCZ_grpkey(dim->container,&grppath)))
	goto done;
    /* Create the suffix path using the dim name */
    if((stat = nczm_concat(grppath,dim->hdr.name,&dimpath)))
	goto done;
    /* return path */
    if(pathp) {*pathp = dimpath; dimpath = NULL;}

done:
    nullfree(grppath);
    nullfree(dimpath);
    return stat;
}

/**
@internal Split a key into pieces along '/' character; elide any leading '/'
@param  key - [in]
@param segments - [out] split path
@return NC_NOERR
@author Dennis Heimbigner
*/
int
ncz_splitkey(const char* key, NClist* segments)
{
    return nczm_split(key,segments);
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
    size64_t len;
    char* content = NULL;
    NCjson* json = NULL;

    if((stat = nczmap_len(zmap, key, &len)))
	goto done;
    if((content = malloc(len+1)) == NULL)
	{stat = NC_ENOMEM; goto done;}
    if((stat = nczmap_read(zmap, key, 0, len, (void*)content)))
	goto done;
    content[len] = '\0';

    if((stat = NCJparse(content,0,&json)))
	goto done;

    if(jsonp) {*jsonp = json; json = NULL;}

done:
    NCJreclaim(json);
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
NCZ_createobject(NCZMAP* zmap, const char* key, size64_t size)
{
    int stat = NC_NOERR;

    /* create the target */
    stat = nczmap_defineobj(zmap, key);
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
#ifdef DEBUG
fprintf(stderr,"uploadjson: %s\n",key); fflush(stderr);
#endif
    /* Unparse the modified json tree */
    if((stat = NCJunparse(json,0,&content)))
	goto done;

    /* create the target */
    if((stat = nczmap_defineobj(zmap, key)))
	goto done;

    /* Write the metadata */
    if((stat = nczmap_write(zmap, key, 0, strlen(content), content)))
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
	if(stat == NC_EEMPTY) {/* create it */
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
    if(jsonp) {*jsonp = json; json = NULL;}
done:
    NCJreclaim(json);
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
	if(stat == NC_EEMPTY) {/* create it */
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
    if(jsonp) {*jsonp = json; json = NULL;}
done:
    NCJreclaim(json);
    return stat;
}
#endif /*0*/

/**
@internal Get contents of a meta object; fail it it does not exist
@param zmap - [in] map
@param key - [in] key of the object
@param jsonp - [out] return parsed json
@return NC_NOERR
@return NC_EEMPTY [object did not exist]
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
    if(jsonp) {*jsonp = json; json = NULL;}
done:
    NCJreclaim(json);
    return stat;
}

/**
@internal Get contents of a meta object; fail it it does not exist
@param zmap - [in] map
@param key - [in] key of the object
@param jsonp - [out] return parsed json
@return NC_NOERR
@return NC_EEMPTY [object did not exist]
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
    if(jsonp) {*jsonp = json; json = NULL;}
done:
    NCJreclaim(json);
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

#if 0
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
#endif

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
    NCbytes* path = ncbytesnew();

    /* Get the list of object keys just below prefix */
    if((stat = nczmap_search(map,prefix,matches))) goto done;
    for(i=0;i<nclistlength(matches);i++) {
	const char* p;
	const char* key = nclistget(matches,i);
	size_t keylen = strlen(key);	
	/* Ignore keys that start with .z or .nc or a potential chunk name */
	if(keylen >= 3 && key[0] == '.' && key[1] == 'n' && key[2] == 'c')
	    continue;
	if(keylen >= 2 && key[0] == '.' && key[1] == 'z')
	    continue;
	for(p=key;*p;p++) {
	    if(*p != '.' && strchr("0123456789",*p) == NULL) break;
	}
	if(*p == '\0') continue; /* looks like a chunk name */
	/* Create <prefix>/<key>/<tag> and see if it exists */
	ncbytesclear(path);
	ncbytescat(path,prefix);
	ncbytescat(path,"/");
	ncbytescat(path,key);
	ncbytescat(path,tag);
	/* See if this object exists */
        if((stat = nczmap_exists(map,ncbytescontents(path))) == NC_NOERR)
	    nclistpush(objlist,key);
    }

done:
    nclistfreeall(matches);
    ncbytesfree(path);
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
	case 'U': nctype = NC_CHAR; break;
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

/**************************************************/
/* Endianness support */
/* signature: void swapinline16(void* ip) */
#define swapinline16(ip) \
{ \
    union {char b[2]; unsigned short i;} u; \
    char* src = (char*)(ip); \
    u.b[0] = src[1]; \
    u.b[1] = src[0]; \
    *((unsigned short*)ip) = u.i; \
}

/* signature: void swapinline32(void* ip) */
#define swapinline32(ip) \
{ \
    union {char b[4]; unsigned int i;} u; \
    char* src = (char*)(ip); \
    u.b[0] = src[3]; \
    u.b[1] = src[2]; \
    u.b[2] = src[1]; \
    u.b[3] = src[0]; \
    *((unsigned int*)ip) = u.i; \
}

/* signature: void swapinline64(void* ip) */
#define swapinline64(ip) \
{ \
    union {char b[8]; unsigned long long i;} u; \
    char* src = (char*)(ip); \
    u.b[0] = src[7]; \
    u.b[1] = src[6]; \
    u.b[2] = src[5]; \
    u.b[3] = src[4]; \
    u.b[4] = src[3]; \
    u.b[5] = src[2]; \
    u.b[6] = src[1]; \
    u.b[7] = src[0]; \
    *((unsigned long long*)ip) = u.i; \
}

int
NCZ_swapatomicdata(size_t datalen, void* data, int typesize)
{
    int stat = NC_NOERR;
    int i;

    assert(datalen % typesize == 0);

    if(typesize == 1) goto done;

    /*(typesize > 1)*/
    for(i=0;i<datalen;) {
	char* p = ((char*)data) + i;
        switch (typesize) {
        case 2: swapinline16(p); break;
        case 4: swapinline32(p); break;
        case 8: swapinline64(p); break;
        default: break;
	}
	i += typesize;
    }
done:
    return THROW(stat);
}

char**
NCZ_clonestringvec(size_t len, const char** vec)
{
    char** clone = NULL;
    size_t i;
    if(vec == NULL) return NULL;
    if(len == 0) { /* Figure out size as envv vector */
        const char** p;
        for(p=vec;*p;p++) len++;
    }
    clone = malloc(sizeof(char*) * (1+len));
    if(clone == NULL) return NULL;
    for(i=0;i<len;i++) {
	char* s = strdup(vec[i]);
	if(s == NULL) return NULL;
	clone[i] = s;
    }
    clone[len] = NULL;
    return clone;
}

void
NCZ_freestringvec(size_t len, char** vec)
{
    size_t i;
    if(vec == NULL) return;
    if(len == 0) { /* Figure out size as envv vector */
        char** p;
        for(p=vec;*p;p++) len++;
    }
    for(i=0;i<len;i++) {
	nullfree(vec[i]);
    }
    nullfree(vec);
}

/* create a fill chunk */
int
NCZ_create_fill_chunk(size64_t chunksize, size_t typesize, void* fill, void** fillchunkp)
{
    int i;
    void* fillchunk = NULL;
    if((fillchunk = malloc(chunksize))==NULL)
        return NC_ENOMEM;
    switch (typesize) {
    case 1: {
        unsigned char c = *((unsigned char*)fill);
        memset(fillchunk,c,chunksize);
        } break;
    case 2: {
        unsigned short fv = *((unsigned short*)fill);
        unsigned short* p2 = (unsigned short*)fillchunk;
        for(i=0;i<chunksize;i+=typesize) *p2++ = fv;
        } break;
    case 4: {
        unsigned int fv = *((unsigned int*)fill);
        unsigned int* p4 = (unsigned int*)fillchunk;
        for(i=0;i<chunksize;i+=typesize) *p4++ = fv;
        } break;
    case 8: {
        unsigned long long fv = *((unsigned long long*)fill);
        unsigned long long* p8 = (unsigned long long*)fillchunk;
        for(i=0;i<chunksize;i+=typesize) *p8++ = fv;
        } break;
    default: {
        unsigned char* p;
        for(p=fillchunk,i=0;i<chunksize;i+=typesize,p+=typesize)
            memcpy(p,fill,typesize);
        } break;
    }
    if(fillchunkp) {*fillchunkp = fillchunk; fillchunk = NULL;}
    nullfree(fillchunk);
    return NC_NOERR;
}
    
