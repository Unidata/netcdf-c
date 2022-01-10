/*
 *	Copyright 2018, University Corporation for Atmospheric Research
 *      See netcdf/COPYRIGHT file for copying and redistribution conditions.
 */

#include "zincludes.h"
#include "zmap.h"
#include "ncs3sdk.h"

#undef S3DEBUG

/*
Map our simplified map model to an S3 bucket + objects.

For the API, the mapping is as follows:
1. A bucket contains multiple datasets.
2. A key to the root of a dataset is assumed to be with respect to
   the bucket name from the original open/create url.
2. Containment is simulated using the S3 key conventions.
3. Every object (e.g. group or array) is mapped to an S3 object
4. Meta data objects (e.g. .zgroup, .zarray, etc) are kept as an S3 object.
5. Actual variable data (for e.g. chunks) is stored as
   using an S3 object per chunk.

Given a key, one of three things is true.
1. The key points to a content-bearing object
2. The key has no associated object
This basically means that there is no notion of not-found:
all keys are assumed to exist, but may have no content.

Notes:
1. Our canonical URLs use path style rather than virtual-host, although
   virtual-host URLs will be accepted anc converted to path-style.
*/

#undef DEBUG
#define DEBUGERRORS

#define NCZM_S3SDK_V1 1

#define ZS3_PROPERTIES (0)

/* Define the "subclass" of NCZMAP */
typedef struct ZS3MAP {
    NCZMAP map;
    NCS3INFO s3;
    void* s3client;
    char* errmsg;
} ZS3MAP;

/* Forward */
static NCZMAP_API nczs3sdkapi; // c++ will not allow static forward variables
static int zs3len(NCZMAP* map, const char* key, size64_t* lenp);

static void freevector(size_t nkeys, char** list);

static void zs3initialize(void);
static int s3clear(ZS3MAP* z3map, const char* key);

static int maketruekey(const char* rootpath, const char* key, char** truekeyp);

static void
errclear(ZS3MAP* z3map)
{
    if(z3map) {
        if(z3map->errmsg)
            free(z3map->errmsg);
	z3map->errmsg = NULL;
    }
}

#ifdef DEBUGERRORS
static void
reporterr(ZS3MAP* z3map)
{
    if(z3map) {
        if(z3map->errmsg) {
            nclog(NCLOGERR,z3map->errmsg);
	}
	errclear(z3map);
    }
}
#else
#define reporterr(map)
#endif

/* Define the Dataset level API */

static int zs3initialized = 0;

static void
zs3initialize(void)
{
    if(!zs3initialized) {
        ZTRACE(7,NULL);
        zs3initialized = 1;
	(void)ZUNTRACE(NC_NOERR);
    }
}

void
NCZ_s3finalize(void)
{
    zs3initialized = 0;
}

static int
zs3create(const char *path, int mode, size64_t flags, void* parameters, NCZMAP** mapp)
{
    int stat = NC_NOERR;
    ZS3MAP* z3map = NULL;
    NCURI* url = NULL;
    char* prefix = NULL;
    char* truekey = NULL;
	
    NC_UNUSED(flags);
    NC_UNUSED(parameters);

    ZTRACE(6,"path=%s mode=%d flag=%llu",path,mode,flags);

    if(!zs3initialized) zs3initialize();

    /* Build the z3 state */
    if((z3map = (ZS3MAP*)calloc(1,sizeof(ZS3MAP))) == NULL)
	{stat = NC_ENOMEM; goto done;}

    z3map->map.format = NCZM_S3;
    z3map->map.url = strdup(path);
    z3map->map.mode = mode;
    z3map->map.flags = flags;
    z3map->map.api = (NCZMAP_API*)&nczs3sdkapi;

    /* Parse the URL */
    ncuriparse(path,&url);
    if(url == NULL)
        {stat = NC_EURL; goto done;}

    /* Convert to canonical path-style */
    if((stat = NC_s3urlprocess(url,&z3map->s3))) goto done;
    /* Verify the root path */
    if(z3map->s3.rootkey == NULL)
        {stat = NC_EURL; goto done;}

    z3map->s3client = NC_s3sdkcreateclient(&z3map->s3);

    {
	int exists = 0;
        /* Does bucket already exist */
	if((stat = NC_s3sdkbucketexists(z3map->s3client,z3map->s3.bucket,&exists, &z3map->errmsg))) goto done;
	if(!exists) {
	    /* create it */
	    if((stat = NC_s3sdkbucketcreate(z3map->s3client,z3map->s3.region,z3map->s3.bucket,&z3map->errmsg)))
	        goto done;
	}
	/* The root object should not exist */
        switch (stat = NC_s3sdkinfo(z3map->s3client,z3map->s3.bucket,z3map->s3.rootkey,NULL,&z3map->errmsg)) {
	case NC_EEMPTY: /* no such object */
	    stat = NC_NOERR;  /* which is what we want */
	    errclear(z3map);
	    break;
	case NC_NOERR: stat = NC_EOBJECT; goto done; /* already exists */
	default: reporterr(z3map); goto done;
	}
	if(!stat) {
            /* Delete objects inside root object tree */
            s3clear(z3map,z3map->s3.rootkey);
	}
    }
    
    if(mapp) *mapp = (NCZMAP*)z3map;    

done:
    reporterr(z3map);
    ncurifree(url);
    nullfree(prefix);
    nullfree(truekey);
    if(stat) nczmap_close((NCZMAP*)z3map,1);
    return ZUNTRACE(stat);
}

/* The problem with open is that there
no obvious way to test for existence.
So, we assume that the dataset must have
some content. We look for that */
static int
zs3open(const char *path, int mode, size64_t flags, void* parameters, NCZMAP** mapp)
{
    int stat = NC_NOERR;
    ZS3MAP* z3map = NULL;
    NCURI* url = NULL;
    NClist* content = NULL;
    size_t nkeys = 0;

    NC_UNUSED(flags);
    NC_UNUSED(parameters);

    ZTRACE(6,"path=%s mode=%d flags=%llu",path,mode,flags);

    if(!zs3initialized) zs3initialize();

    /* Build the z3 state */
    if((z3map = (ZS3MAP*)calloc(1,sizeof(ZS3MAP))) == NULL)
	{stat = NC_ENOMEM; goto done;}

    z3map->map.format = NCZM_S3;
    z3map->map.url = strdup(path);
    z3map->map.mode = mode;
    z3map->map.flags = flags;
    z3map->map.api = (NCZMAP_API*)&nczs3sdkapi;

    /* Parse the URL */
    if((stat = ncuriparse(path,&url))) goto done;
    if(url == NULL)
        {stat = NC_EURL; goto done;}

    /* Convert to canonical path-style */
    if((stat = NC_s3urlprocess(url,&z3map->s3))) goto done;
    /* Verify root path */
    if(z3map->s3.rootkey == NULL)
        {stat = NC_EURL; goto done;}

    z3map->s3client = NC_s3sdkcreateclient(&z3map->s3);

    /* Search the root for content */
    content = nclistnew();
    if((stat = NC_s3sdkgetkeys(z3map->s3client,z3map->s3.bucket,z3map->s3.rootkey,&nkeys,NULL,&z3map->errmsg)))
	goto done;
    if(nkeys == 0) {
	/* dataset does not actually exist; we choose to return ENOOBJECT instead of EEMPTY */
	stat = NC_ENOOBJECT;
	goto done;
    }
    if(mapp) *mapp = (NCZMAP*)z3map;    

done:
    reporterr(z3map);
    nclistfreeall(content);
    ncurifree(url);
    if(stat) nczmap_close((NCZMAP*)z3map,0);
    return ZUNTRACE(stat);
}

/**************************************************/
/* Object API */

/*
@return NC_NOERR if key points to a content-bearing object.
@return NC_EEMPTY if object at key has no content.
@return NC_EXXX return true error
*/
static int
zs3exists(NCZMAP* map, const char* key)
{
    int stat = NC_NOERR;
    ZTRACE(6,"map=%s key=%s",map->url,key);
    stat = zs3len(map,key,NULL);
    return ZUNTRACE(stat);
}

/*
@return NC_NOERR if key points to a content-bearing object.
@return NC_EEMPTY if object at key has no content.
@return NC_EXXX return true error
*/
static int
zs3len(NCZMAP* map, const char* key, size64_t* lenp)
{
    int stat = NC_NOERR;
    ZS3MAP* z3map = (ZS3MAP*)map;
    char* truekey = NULL;

    ZTRACE(6,"map=%s key=%s",map->url,key);

    if((stat = maketruekey(z3map->s3.rootkey,key,&truekey))) goto done;

    switch (stat = NC_s3sdkinfo(z3map->s3client,z3map->s3.bucket,truekey,lenp,&z3map->errmsg)) {
    case NC_NOERR: break;
    case NC_EEMPTY:
	if(lenp) *lenp = 0;
	goto done;
    default:
        goto done;
    }
done:
    nullfree(truekey);
    reporterr(z3map);
    return ZUNTRACE(stat);
}

/*
@return NC_NOERR if object at key was read
@return NC_EEMPTY if object at key has no content.
@return NC_EXXX return true error
*/
static int
zs3read(NCZMAP* map, const char* key, size64_t start, size64_t count, void* content)
{
    int stat = NC_NOERR;
    ZS3MAP* z3map = (ZS3MAP*)map; /* cast to true type */
    size64_t size = 0;
    char* truekey = NULL;
    
    ZTRACE(6,"map=%s key=%s start=%llu count=%llu",map->url,key,start,count);

    if((stat = maketruekey(z3map->s3.rootkey,key,&truekey))) goto done;
    
    switch (stat=NC_s3sdkinfo(z3map->s3client, z3map->s3.bucket, truekey, &size, &z3map->errmsg)) {
    case NC_NOERR: break;
    case NC_EEMPTY: goto done;
    default: goto done; 	
    }
    /* Sanity checks */
    if(start >= size || start+count > size)
        {stat = NC_EEDGE; goto done;}
    if(count > 0)  {
        if((stat = NC_s3sdkread(z3map->s3client, z3map->s3.bucket, truekey, start, count, content, &z3map->errmsg)))
            goto done;
    }
done:
    nullfree(truekey);
    reporterr(z3map);
    return ZUNTRACE(stat);
}

/*
@return NC_NOERR if key content was written
@return NC_EEMPTY if object at key has no content.
@return NC_EXXX return true error
*/
static int
zs3write(NCZMAP* map, const char* key, size64_t start, size64_t count, const void* content)
{
    int stat = NC_NOERR;
    ZS3MAP* z3map = (ZS3MAP*)map; /* cast to true type */
    char* chunk = NULL; /* use char* so we can do arithmetic with it */
    size64_t objsize = 0;
    size64_t memsize = 0;
    size64_t endwrite = start+count; /* first pos just above overwritten data */
    char* truekey = NULL;
    int isempty = 0;
	
    ZTRACE(6,"map=%s key=%s start=%llu count=%llu",map->url,key,start,count);

    if((stat = maketruekey(z3map->s3.rootkey,key,&truekey))) goto done;

    /* Apparently S3 has no write byterange operation, so we need to read the whole object,
       copy data, and then rewrite */       
    switch (stat=NC_s3sdkinfo(z3map->s3client, z3map->s3.bucket, truekey, &objsize, &z3map->errmsg)) {
    case NC_NOERR: /* Figure out the memory size of the object */
	memsize = (endwrite > objsize ? endwrite : objsize);
        break;
    case NC_EEMPTY:
	memsize = endwrite;
	isempty = 1;
        break;
    default: reporterr(z3map); goto done;
    }

    if(isempty)
        chunk = (char*)calloc(1,memsize); /* initialize it */
    else
        chunk = (char*)malloc(memsize);
    if(chunk == NULL)
	{stat = NC_ENOMEM; goto done;}
    if(start > 0 && objsize > 0) { /* must read to preserve data before start */
        if((stat = NC_s3sdkread(z3map->s3client, z3map->s3.bucket, truekey, 0, objsize, (void*)chunk, &z3map->errmsg)))
            goto done;
    }
#if 0
    if(newsize > objsize) {
        /* Zeroize the part of the object added */
	memset(((char*)chunk)+objsize,0,(newsize-objsize));
	objsize = newsize;
    }
#endif
    /* overwrite the relevant part of the memory with the contents */
    if(count > 0)
        memcpy(((char*)chunk)+start,content,count); /* there may be data above start+count */
    /* (re-)write */
    if((stat = NC_s3sdkwriteobject(z3map->s3client, z3map->s3.bucket, truekey, memsize, (void*)chunk, &z3map->errmsg)))
        goto done;

done:
    nullfree(truekey);
    reporterr(z3map);
    nullfree(chunk);
    return ZUNTRACE(stat);
}

static int
zs3close(NCZMAP* map, int deleteit)
{
    int stat = NC_NOERR;
    ZS3MAP* z3map = (ZS3MAP*)map;

    ZTRACE(6,"map=%s deleteit=%d",map->url, deleteit);

    if(deleteit) 
        s3clear(z3map,z3map->s3.rootkey);
    if(z3map->s3client && z3map->s3.bucket && z3map->s3.rootkey) {
        NC_s3sdkclose(z3map->s3client, &z3map->s3, deleteit, &z3map->errmsg);
    }
    reporterr(z3map);
    z3map->s3client = NULL;
    NC_s3clear(&z3map->s3);
    nullfree(z3map->errmsg);
    nczm_clear(map);
    nullfree(map);
    return ZUNTRACE(stat);
}

/*
Return a list of full keys immediately "below" a specified prefix,
but not including the prefix.
In theory, the returned list should be sorted in lexical order,
but it possible that it is not.
@return NC_NOERR if success, even if no keys returned.
@return NC_EXXX return true error
*/
static int
zs3search(NCZMAP* map, const char* prefix, NClist* matches)
{
    int i,stat = NC_NOERR;
    ZS3MAP* z3map = (ZS3MAP*)map;
    char** list = NULL;
    size_t nkeys;
    NClist* tmp = NULL;
    char* trueprefix = NULL;
    char* newkey = NULL;
    const char* p;

    ZTRACE(6,"map=%s prefix0=%s",map->url,prefix);
    
    if((stat = maketruekey(z3map->s3.rootkey,prefix,&trueprefix))) goto done;
    
    if(*trueprefix != '/') return NC_EINTERNAL;
    if((stat = NC_s3sdkgetkeys(z3map->s3client,z3map->s3.bucket,trueprefix,&nkeys,&list,&z3map->errmsg)))
        goto done;
    if(nkeys > 0) {
	size_t tplen = strlen(trueprefix);
	tmp = nclistnew();
	/* Remove the trueprefix from the front of all the returned keys */
        for(i=0;i<nkeys;i++) {
	    const char* l = list[i];
	    if(memcmp(trueprefix,l,tplen)==0) {
		p  = l+tplen; /* Point to start of suffix */
		/* If the key is same as trueprefix, ignore it */
		if(*p == '\0') continue;
		if(nczm_segment1(p,&newkey)) goto done;
	        nclistpush(tmp,newkey); newkey = NULL;
	    }
        }
	/* Now remove duplicates */
	for(i=0;i<nclistlength(tmp);i++) {
	    int j;
	    int duplicate = 0;
	    const char* is = nclistget(tmp,i);
	    for(j=0;j<nclistlength(matches);j++) {
	        const char* js = nclistget(matches,j);
	        if(strcmp(js,is)==0) {duplicate = 1; break;} /* duplicate */
	    }	    
	    if(!duplicate)
	        nclistpush(matches,strdup(is));
	}
	nclistfreeall(tmp); tmp = NULL;
    }
	
#ifdef DEBUG
    for(i=0;i<nclistlength(matches);i++) {
	const char* is = nclistget(matches,i);
	fprintf(stderr,"search: %s\n",is);
    }
#endif

done:
    nullfree(newkey);
    nullfree(trueprefix);
    reporterr(z3map);
    nclistfreeall(tmp);
    freevector(nkeys,list);
    return ZUNTRACEX(stat,"|matches|=%d",(int)nclistlength(matches));
}

/**************************************************/
/* S3 Utilities */

/*
Remove all objects with keys which have
rootkey as prefix; rootkey is a truekey
*/
static int
s3clear(ZS3MAP* z3map, const char* rootkey)
{
    int stat = NC_NOERR;
    char** list = NULL;
    char** p;
    size_t nkeys = 0;
    if(z3map->s3client && z3map->s3.bucket && rootkey) {
        if((stat = NC_s3sdksearch(z3map->s3client, z3map->s3.bucket, rootkey, &nkeys, &list, &z3map->errmsg)))
            goto done;
        if(list != NULL) {
            for(p=list;*p;p++) {
	        /* If the key is the rootkey, skip it */
	        if(strcmp(rootkey,*p)==0) continue;
#ifdef S3DEBUG
fprintf(stderr,"s3clear: %s\n",*p);
#endif
                if((stat = NC_s3sdkdeletekey(z3map->s3client, z3map->s3.bucket, *p, &z3map->errmsg)))	
	            goto done;
	    }
        }
    }

done:
    reporterr(z3map);
    NCZ_freestringvec(nkeys,list);
    return THROW(stat);
}

/* Prefix key with path to root to make true key */
static int
maketruekey(const char* rootpath, const char* key, char** truekeyp)
{
    int  stat = NC_NOERR;
    char* truekey = NULL;
    size_t len, rootlen, keylen;

    if(truekeyp == NULL) goto done;
    rootlen = strlen(rootpath);
    keylen = strlen(key);
    len = (rootlen+keylen+1+1+1);
    
    truekey = (char*)malloc(len+1);
    if(truekey == NULL) {stat = NC_ENOMEM; goto done;}
    truekey[0] = '\0';
    if(rootpath[0] != '/')    
        strlcat(truekey,"/",len+1);
    strlcat(truekey,rootpath,len+1);
    if(rootpath[rootlen-1] != '/')
        strlcat(truekey,"/",len+1);
    if(key[0] == '/') key++;
    strlcat(truekey,key,len+1);
    if(key[keylen-1] == '/') /* remove any trailing '/' */
	truekey[strlen(truekey)-1] = '\0';
    *truekeyp = truekey; truekey = NULL;       

done:
    nullfree(truekey);
    return stat;
}

static void
freevector(size_t nkeys, char** list)
{
    size_t i;
    if(list) {
        for(i=0;i<nkeys;i++) nullfree(list[i]);
	nullfree(list);
    }
}

/**************************************************/
/* External API objects */

NCZMAP_DS_API zmap_s3sdk;
NCZMAP_DS_API zmap_s3sdk = {
    NCZM_S3SDK_V1,
    ZS3_PROPERTIES,
    zs3create,
    zs3open,
};

static NCZMAP_API
nczs3sdkapi = {
    NCZM_S3SDK_V1,
    zs3close,
    zs3exists,
    zs3len,
    zs3read,
    zs3write,
    zs3search,
};
