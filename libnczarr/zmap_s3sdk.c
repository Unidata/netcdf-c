/*
 *	Copyright 2018, University Corporation for Atmospheric Research
 *      See netcdf/COPYRIGHT file for copying and redistribution conditions.
 */

#include "zincludes.h"
#include "zmap.h"
#include "zs3sdk.h"

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

#define AWSHOST ".amazonaws.com"

enum URLFORMAT {UF_NONE, UF_VIRTUAL, UF_PATH, UF_OTHER};

/* Define the "subclass" of NCZMAP */
typedef struct ZS3MAP {
    NCZMAP map;
    enum URLFORMAT urlformat;
    char* host; /* non-null if other*/
    char* region; /* region */
    void* s3config;
    void* s3client;
    char* errmsg;
    char* bucket; /* bucket name */
    char* rootkey;
} ZS3MAP;

/* Forward */
static NCZMAP_API nczs3sdkapi; // c++ will not allow static forward variables
static int zs3exists(NCZMAP* map, const char* key);
static int zs3len(NCZMAP* map, const char* key, size64_t* lenp);
static int zs3defineobj(NCZMAP* map, const char* key);
static int zs3read(NCZMAP* map, const char* key, size64_t start, size64_t count, void* content);
static int zs3write(NCZMAP* map, const char* key, size64_t start, size64_t count, const void* content);
static int zs3search(NCZMAP* map, const char* prefix, NClist* matches);

static int zs3close(NCZMAP* map, int deleteit);
static int z3createobj(ZS3MAP*, const char* key);

static int processurl(ZS3MAP* z3map, NCURI* url);
static int endswith(const char* s, const char* suffix);
static void freevector(size_t nkeys, char** list);

static void zs3initialize(void);
static int s3clear(ZS3MAP* z3map, const char* key);
static int isLegalBucketName(const char* bucket);

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
    if(!zs3initialized)
        NCZ_s3sdkinitialize();
    zs3initialized = 1;
}

#if 0
static void
zs3finalize(void)
{
    if(zs3initialized)
        NCZ_s3sdkfinalize();
    zs3initialized = 0;
}
#endif

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

    ZTRACE("","");

    if(!zs3initialized) zs3initialize();

    /* Build the z4 state */
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
    if((stat = processurl(z3map,url))) goto done;

    if((stat=NCZ_s3sdkcreateconfig(z3map->host, z3map->region, &z3map->s3config))) goto done;
    if((stat = NCZ_s3sdkcreateclient(z3map->s3config,&z3map->s3client))) goto done;

    {
	int exists = 0;
        /* Does bucket already exist */
	if((stat = NCZ_s3sdkbucketexists(z3map->s3client,z3map->bucket,&exists, &z3map->errmsg))) goto done;
	if(!exists) {
	    /* create it */
	    if((stat = NCZ_s3sdkbucketcreate(z3map->s3client,z3map->region,z3map->bucket,&z3map->errmsg)))
	        goto done;
	}
	/* The root object should not exist */
        switch (stat = NCZ_s3sdkinfo(z3map->s3client,z3map->bucket,z3map->rootkey,NULL,&z3map->errmsg)) {
	case NC_EEMPTY: /* no such object */
	    stat = NC_NOERR;  /* which is what we want */
	    errclear(z3map);
	    break;
	case NC_NOERR: stat = NC_EEXIST; goto done; /* already exists */
	default: reporterr(z3map); goto done;
	}
	if(!stat) {
            /* Delete objects inside root object tree */
            s3clear(z3map,z3map->rootkey);
	}
    }
    
    if(mapp) *mapp = (NCZMAP*)z3map;    

done:
    reporterr(z3map);
    ncurifree(url);
    nullfree(prefix);
    nullfree(truekey);
    if(stat) nczmap_close((NCZMAP*)z3map,1);
    return (stat);
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

    ZTRACE("","");

    if(!zs3initialized) zs3initialize();

    /* Build the z4 state */
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
    if((stat = processurl(z3map,url))) goto done;

    if((stat=NCZ_s3sdkcreateconfig(z3map->host,z3map->region,&z3map->s3config))) goto done;
    if((stat=NCZ_s3sdkcreateclient(z3map->s3config,&z3map->s3client))) goto done;

    /* Search the root for content */
    content = nclistnew();
    if((stat = NCZ_s3sdkgetkeys(z3map->s3client,z3map->bucket,z3map->rootkey,&nkeys,NULL,&z3map->errmsg)))
	goto done;
    if(nkeys == 0) {
	/* dataset does not actually exist */
	stat = NC_EEMPTY;
	goto done;
    }

    if(mapp) *mapp = (NCZMAP*)z3map;    

done:
    reporterr(z3map);
    nclistfreeall(content);
    ncurifree(url);
    if(stat) nczmap_close((NCZMAP*)z3map,0);
    return (stat);
}

static int
isLegalBucketName(const char* bucket)
{
    return 1; /*TBD*/
}

/**************************************************/
/* Object API */

static int
zs3close(NCZMAP* map, int deleteit)
{
    int stat = NC_NOERR;
    ZS3MAP* z3map = (ZS3MAP*)map;

    ZTRACE("","");
    if(deleteit)
        s3clear(z3map,z3map->rootkey);
    NCZ_s3sdkclose(z3map->s3client, z3map->s3config, z3map->bucket, z3map->rootkey, deleteit, &z3map->errmsg);
    reporterr(z3map);
    z3map->s3client = NULL;
    z3map->s3config = NULL;
    nullfree(z3map->bucket);
    nullfree(z3map->region);
    nullfree(z3map->host);
    nullfree(z3map->errmsg);
    nullfree(z3map->rootkey)
    nczm_clear(map);
    nullfree(map);
    return (stat);
}

/*
@return NC_NOERR if key points to a content-bearing object.
@return NC_EEMPTY if object at key has no content.
@return NC_EXXX return true error
*/
static int
zs3exists(NCZMAP* map, const char* key)
{
    return zs3len(map,key,NULL);
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

    ZTRACE("","");

    if((stat = maketruekey(z3map->rootkey,key,&truekey))) goto done;

    switch (stat = NCZ_s3sdkinfo(z3map->s3client,z3map->bucket,truekey,lenp,&z3map->errmsg)) {
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
    return (stat);
}

/*
@return NC_NOERR if key points to a content-bearing object.
@return NC_EEMPTY if object at key has no content, if so, then create
                   a content-bearing object at that key.
@return NC_EXXX return true error
*/
static int
zs3defineobj(NCZMAP* map, const char* key)
{
    int stat = NC_NOERR;
    ZS3MAP* z3map = (ZS3MAP*)map; /* cast to true type */
    
    ZTRACE("%s",key);

    switch(stat = zs3exists(map,key)) {
    case NC_NOERR: goto done; /* Already exists */
    case NC_EEMPTY:
        if((stat = z3createobj(z3map,key)))
	    goto done;
	break;
    default: /* Some other kind of failure */
	goto done;
    }

done:
    reporterr(z3map);
    return (stat);
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
 
    ZTRACE("%s",key);

    if((stat = maketruekey(z3map->rootkey,key,&truekey))) goto done;
    
    switch (stat=NCZ_s3sdkinfo(z3map->s3client, z3map->bucket, truekey, &size, &z3map->errmsg)) {
    case NC_NOERR: break;
    case NC_EEMPTY: goto done;
    default: goto done; 	
    }
    /* Sanity checks */
    if(start >= size || start+count > size)
        {stat = NC_EEDGE; goto done;}
    if(count > 0)  {
        if((stat = NCZ_s3sdkread(z3map->s3client, z3map->bucket, truekey, start, count, content, &z3map->errmsg)))
            goto done;
    }
done:
    nullfree(truekey);
    reporterr(z3map);
    return (stat);
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
    void* chunk = NULL;
    size64_t objsize = 0;
    size64_t newsize = start+count;
    char* truekey = NULL;
	
    ZTRACE("%s",key);

    if(count == 0) {stat = NC_EEDGE; goto done;}

    if((stat = maketruekey(z3map->rootkey,key,&truekey))) goto done;

    /* Apparently S3 has no write byterange operation, so we need to read the whole object,
       copy data, and then rewrite */       
    switch (stat=NCZ_s3sdkinfo(z3map->s3client, z3map->bucket, truekey, &objsize, &z3map->errmsg)) {
    case NC_NOERR:
	newsize = (newsize > objsize ? newsize : objsize);
        break;
    case NC_EEMPTY:
	newsize = newsize;
        break;
    default: reporterr(z3map); goto done;
    }
    chunk = malloc(newsize);
    if(chunk == NULL)
	{stat = NC_ENOMEM; goto done;}
    if(objsize > 0) {
        if((stat = NCZ_s3sdkread(z3map->s3client, z3map->bucket, truekey, 0, objsize, chunk, &z3map->errmsg)))
            goto done;
    }
    if(newsize > objsize) {
        /* Zeroize the part of the object added */
	memset(((char*)chunk)+objsize,0,(newsize-objsize));
	objsize = newsize;
    }
    /* overwrite with the contents */
    memcpy(((char*)chunk)+start,content,count); /* remember there may be data above start+count */
    if((stat = NCZ_s3sdkwriteobject(z3map->s3client, z3map->bucket, truekey, objsize, chunk, &z3map->errmsg)))
        goto done;

done:
    nullfree(truekey);
    reporterr(z3map);
    nullfree(chunk);
    return (stat);
}

/*
Return a list of keys immediately "below" a specified prefix,
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

    ZTRACE("%s",prefix);
    
    if((stat = maketruekey(z3map->rootkey,prefix,&trueprefix))) goto done;
    
    if(*trueprefix != '/') return NC_EINTERNAL;
    if((stat = NCZ_s3sdkgetkeys(z3map->s3client,z3map->bucket,trueprefix,&nkeys,&list,&z3map->errmsg)))
        goto done;
    if(nkeys > 0) {
	size_t tplen = strlen(trueprefix);
	tmp = nclistnew();
	/* Remove the trueprefix from the front of all the returned keys */
	/* The returned keys may be of any depth, so capture and prune the keys */
        for(i=0;i<nkeys;i++) {
	    char* newkey = NULL;
	    if(memcmp(trueprefix,list[i],tplen)==0) {
		newkey = list[i];
		newkey = newkey+tplen; /* Point to start of suffix */
		/* If the key is same as trueprefix, ignore it */
		if(*newkey == '\0') continue;
		newkey--; /* Point to trailing '/' */
		assert(newkey[0] == '/');
	        newkey = strdup(newkey);
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
    nullfree(trueprefix);
    reporterr(z3map);
    nclistfreeall(tmp);
    freevector(nkeys,list);
    return THROW(stat);
}

/**************************************************/
/* Utilities */

static int
processurl(ZS3MAP* z3map, NCURI* url)
{
    int stat = NC_NOERR;
    NClist* segments = NULL;
    NCbytes* buf = ncbytesnew();

    if(url == NULL)
        {stat = NC_EURL; goto done;}
    /* do some verification */
    if(strcmp(url->protocol,"https") != 0)
        {stat = NC_EURL; goto done;}

    /* Path better look absolute */
    if(!nczm_isabsolutepath(url->path))
    	{stat = NC_EURL; goto done;}

    /* Distinguish path-style from virtual-host style from other:
       Virtual: https://bucket-name.s3.Region.amazonaws.com/<root>
       Path: https://s3.Region.amazonaws.com/bucket-name/<root>
       Other: https://<host>/bucketname/<root>
    */
    if(url->host == NULL || strlen(url->host) == 0)
        {stat = NC_EURL; goto done;}
    if(endswith(url->host,AWSHOST)) { /* Virtual or path */
        segments = nclistnew();
        /* split the hostname by "." */
        if((stat = nczm_split_delim(url->host,'.',segments))) goto done;
	switch (nclistlength(segments)) {
	default: stat = NC_EURL; goto done;
	case 4:
            if(strcasecmp(nclistget(segments,0),"s3")!=0)
	        {stat = NC_EURL; goto done;}
	    z3map->urlformat = UF_PATH; 
	    z3map->region = strdup(nclistget(segments,1));
	    break;
	case 5:
            if(strcasecmp(nclistget(segments,1),"s3")!=0)
	        {stat = NC_EURL; goto done;}
	    z3map->urlformat = UF_VIRTUAL;
	    z3map->region = strdup(nclistget(segments,2));
    	    z3map->bucket = strdup(nclistget(segments,0));
	    break;
	}
	/* Rebuild host to look like path-style */
	ncbytescat(buf,"s3.");
	ncbytescat(buf,z3map->region);
	ncbytescat(buf,AWSHOST);
        z3map->host = ncbytesextract(buf);
    } else {
        z3map->urlformat = UF_OTHER;
        if((z3map->host = strdup(url->host))==NULL)
	    {stat = NC_ENOMEM; goto done;}
    }
    /* Do fixups to make everything look like it was path style */
    switch (z3map->urlformat) {
    case UF_PATH:
    case UF_OTHER:
	/* We have to process the path to get the bucket, and remove it in the path */
	if(url->path != NULL && strlen(url->path) > 0) {
            /* split the path by "/"  */
   	    nclistfreeall(segments);
	    segments = nclistnew();
            if((stat = nczm_split_delim(url->path,'/',segments))) goto done;
	    if(nclistlength(segments) == 0)
	    	{stat = NC_EURL; goto done;}
	    z3map->bucket = ((char*)nclistremove(segments,0));
	    if((stat = nczm_join(segments,&z3map->rootkey))) goto done;
	    nclistfreeall(segments); segments = NULL;
	}
	break;
    case UF_VIRTUAL:
	z3map->rootkey = strdup(url->path);
	break;
    default: stat = NC_EURL; goto done;
    }
    /* Verify bucket name */
    if(z3map->bucket != NULL && !isLegalBucketName(z3map->bucket))
	{stat = NC_EURL; goto done;}
    /* Verify that this refers to an object that is below the bucket.
       That is, there must be at least one segment past the bucket in the path */
    nclistfreeall(segments);
    segments = nclistnew();
    if((stat = nczm_split_delim(z3map->rootkey,'/',segments))) goto done;
    if(nclistlength(segments) == 0)
	{stat = NC_EINTERNAL; goto done;} /* oops */        
    
done:
    ncbytesfree(buf);
    nclistfreeall(segments);
    return stat;
}

/* Create an object corresponding to a key 
@return NC_NOERR if key points to a content-bearing object already
@return NC_EEMPTY if object at key has no content, if so, then create
                   a content-bearing object at that key.
@return NC_EXXX return true error
*/
static int
z3createobj(ZS3MAP* z3map, const char* key)
{
    int stat = NC_NOERR;
    char* truekey = NULL;
    
    if((stat = maketruekey(z3map->rootkey,key,&truekey))) goto done;
    stat = NCZ_s3sdkcreatekey(z3map->s3client, z3map->bucket, truekey, &z3map->errmsg);

done:
    nullfree(truekey);
    reporterr(z3map);
    return THROW(stat);
}

/**************************************************/
/* S3 Utilities */

static int
endswith(const char* s, const char* suffix)
{
    ssize_t ls, lsf, delta;
    if(s == NULL || suffix == NULL) return 0;
    ls = strlen(s);
    lsf = strlen(suffix);
    delta = (ls - lsf);
    if(delta < 0) return 0;
    if(memcmp(s+delta,suffix,lsf)!=0) return 0;
    return 1;
}

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

    if((stat = NCZ_s3sdkgetkeys(z3map->s3client, z3map->bucket, rootkey, &nkeys, &list, &z3map->errmsg)))
        goto done;
    if(list != NULL) {
        for(p=list;*p;p++) {
	    /* If the key is the rootkey, skip it */
	    if(strcmp(rootkey,*p)==0) continue;
#ifdef S3DEBUG
fprintf(stderr,"s3clear: %s\n",*p);
#endif
            if((stat = NCZ_s3sdkdeletekey(z3map->s3client, z3map->bucket, *p, &z3map->errmsg)))	
	        goto done;
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
    zs3create,
    zs3open,
};

static NCZMAP_API
nczs3sdkapi = {
    NCZM_S3SDK_V1,
    zs3close,
    zs3exists,
    zs3len,
    zs3defineobj,
    zs3read,
    zs3write,
    zs3search,
};
