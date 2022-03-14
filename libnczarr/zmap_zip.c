/*
 *	Copyright 2018, University Corporation for Atmospheric Research
 *      See netcdf/COPYRIGHT file for copying and redistribution conditions.
 */

#undef DEBUG

/* Not sure this has any effect */
#define _LARGEFILE_SOURCE 1
#define _LARGEFILE64_SOURCE 1

#include "zincludes.h"

#include <errno.h>
#include <zip.h>

#include "fbits.h"
#include "ncpathmgr.h"

#undef CACHESEARCH

#define VERIFY

/*Mnemonic*/
#define FLAG_ISDIR 1
#define FLAG_CREATE 1
#define SKIPLAST 1
#define WHOLEPATH 0

#define NCZM_ZIP_V1 1

#define ZIP_PROPERTIES (NCZM_WRITEONCE|NCZM_ZEROSTART)

/*
Do a simple mapping of our simplified map model
to a zip-file

Every dataset is assumed to be rooted at some directory in the
zip file tree. So, its location is defined by some path to a
zip file representing the dataset.

For the object API, the mapping is as follows:
1. Every content-bearing object (e.g. .zgroup or .zarray) is mapped to a zip entry.
   This means that if a key  points to a content bearing object then
   no other key can have that content bearing key as a suffix.
2. The meta data containing files are assumed to contain
   UTF-8 character data.
3. The chunk containing files are assumed to contain raw unsigned 8-bit byte data.
4. The objects may or may not be compressed; this implementation writes uncompressed objects.
*/

/* define the var name containing an objects content */
#define ZCONTENT "data"

/* Define the "subclass" of NCZMAP */
typedef struct ZZMAP {
    NCZMAP map;
    char* root;
    char* dataset; /* prefix for all keys in zip file */
    zip_t* archive;
    char** searchcache;
} ZZMAP;

typedef zip_int64_t ZINDEX;;    

/* Forward */
static NCZMAP_API zapi;
static int zipclose(NCZMAP* map, int delete);
static int zzcreategroup(ZZMAP*, const char* key, int nskip);
static int zzlookupobj(ZZMAP*, const char* key, ZINDEX* fd);
static int zzlen(ZZMAP* zzmap, ZINDEX zindex, size64_t* lenp);
static int zipmaperr(ZZMAP* zzmap);
static int ziperr(zip_error_t* zerror);
static int ziperrno(int zerror);
static void freesearchcache(char** cache);

static int zzinitialized = 0;

static void
zipinitialize(void)
{
    if(!zzinitialized) {
        ZTRACE(7,NULL);
        zzinitialized = 1;
	(void)ZUNTRACE(NC_NOERR);
    }
}

/* Define the Dataset level API */

/*
@param datasetpath abs path in the file tree of the root of the dataset'
       might be a relative path.
@param mode the netcdf-c mode flags
@param flags extra flags
@param flags extra parameters
@param mapp return the map object in this
*/

static int
zipcreate(const char *path, int mode, size64_t flags, void* parameters, NCZMAP** mapp)
{
    int stat = NC_NOERR;
    ZZMAP* zzmap = NULL;
    NCURI* url = NULL;
    zip_flags_t zipflags = 0;
    int zerrno = ZIP_ER_OK;
    ZINDEX zindex = -1;
    char* abspath = NULL;
    
    NC_UNUSED(parameters);
    ZTRACE(6,"path=%s mode=%d flag=%llu",path,mode,flags);

    if(!zzinitialized) zipinitialize();

    /* Fixup mode flags */
    mode = (NC_NETCDF4 | NC_WRITE | mode);

    /* path must be a url with file: protocol*/
    ncuriparse(path,&url);
    if(url == NULL)
	{stat = NC_EURL; goto done;}
    if(strcasecmp(url->protocol,"file") != 0)
        {stat = NC_EURL; goto done;}

    /* Build the zz state */
    if((zzmap = calloc(1,sizeof(ZZMAP))) == NULL)
	{stat = NC_ENOMEM; goto done;}

    zzmap->map.format = NCZM_ZIP;
    zzmap->map.url = ncuribuild(url,NULL,NULL,NCURIALL);
    zzmap->map.flags = flags;
    /* create => NC_WRITE */
    zzmap->map.mode = mode;
    zzmap->map.api = &zapi;

    /* Since root is in canonical form, we need to convert to local form */
    if((zzmap->root = NCpathcvt(url->path))==NULL)
        {stat = NC_ENOMEM; goto done;}

    /* Make the root path be absolute */
    if((abspath = NCpathabsolute(zzmap->root)) == NULL)
	{stat = NC_EURL; goto done;}
    nullfree(zzmap->root);
    zzmap->root = abspath;    
    abspath = NULL;

    /* Extract the dataset name */
    if((stat = nczm_basename(url->path,&zzmap->dataset))) goto done;

    /* Set zip openflags */
    zipflags |= ZIP_CREATE;
    if(fIsSet(mode,NC_NOCLOBBER))
	zipflags |= ZIP_EXCL;
    else
        zipflags |= ZIP_TRUNCATE;

#ifdef VERIFY
    zipflags |= ZIP_CHECKCONS;
#endif
    if((zzmap->archive = zip_open(zzmap->root,zipflags,&zerrno))==NULL)
        {stat = ziperrno(zerrno); goto done;}

    /* Tell it about the dataset as a dir */
    if((zindex = zip_dir_add(zzmap->archive, zzmap->dataset, ZIP_FL_ENC_UTF_8))<0)
	{stat = zipmaperr(zzmap); goto done;}
    
    /* Dataset superblock will be written by higher layer */
     
    if(mapp) {*mapp = (NCZMAP*)zzmap; zzmap = NULL;}

done:
    nullfree(abspath);
    ncurifree(url);
    if(zzmap) zipclose((NCZMAP*)zzmap,1);
    return ZUNTRACE(stat);
}

/*
@param datasetpath abs path in the file tree of the root of the dataset'
       might be a relative path.
@param mode the netcdf-c mode flags
@param flags extra flags
@param flags extra parameters
@param mapp return the map object in this
*/

static int
zipopen(const char *path, int mode, size64_t flags, void* parameters, NCZMAP** mapp)
{
    int stat = NC_NOERR;
    ZZMAP* zzmap = NULL;
    NCURI*url = NULL;
    zip_flags_t zipflags = 0;
    int zerrno = ZIP_ER_OK;
    char* abspath = NULL;
    
    NC_UNUSED(parameters);
    ZTRACE(6,"path=%s mode=%d flags=%llu",path,mode,flags);

    if(!zzinitialized) zipinitialize();

    /* Fixup mode flags */
    mode = (NC_NETCDF4 | mode);

    /* path must be a url with file: protocol*/
    ncuriparse(path,&url);
    if(url == NULL)
	{stat = NC_EURL; goto done;}
    if(strcasecmp(url->protocol,"file") != 0)
        {stat = NC_EURL; goto done;}

    /* Build the zz state */
    if((zzmap = calloc(1,sizeof(ZZMAP))) == NULL)
	{stat = NC_ENOMEM; goto done;}

    zzmap->map.format = NCZM_ZIP;
    zzmap->map.url = ncuribuild(url,NULL,NULL,NCURIALL);
    zzmap->map.flags = flags;
    zzmap->map.mode = mode;
    zzmap->map.api = (NCZMAP_API*)&zapi;
    /* Since root is in canonical form, we need to convert to local form */
    if((zzmap->root = NCpathcvt(url->path))==NULL)
        {stat = NC_ENOMEM; goto done;}
    /* Make the root path be absolute */
    if((abspath = NCpathabsolute(zzmap->root)) == NULL)
	{stat = NC_EURL; goto done;}
    nullfree(zzmap->root);
    zzmap->root = abspath;    
    abspath = NULL;

    /* Set zip open flags */
    zipflags |= ZIP_CHECKCONS;
    if(!fIsSet(mode,NC_WRITE))
	zipflags |= ZIP_RDONLY;

#ifdef VERIFY
    zipflags |= ZIP_CHECKCONS;
#endif
    /* Open the file */
    if((zzmap->archive = zip_open(zzmap->root,zipflags,&zerrno))==NULL)
        {stat = ziperrno(zerrno); goto done;}

    /* Use entry 0 to obtain the dataset name */
    {
        const char* name;
        zip_int64_t num_entries;

        num_entries = zip_get_num_entries(zzmap->archive, (zip_flags_t)0);
        if(num_entries == 0) {stat = NC_EEMPTY; goto done;}    
        /* get 0'th entry name */
	if((name = zip_get_name(zzmap->archive, 0, (zip_flags_t)0))==NULL)
	    {stat = zipmaperr(zzmap); goto done;}
        if(name[0] == '\0' || name[0] == '/')
	    {stat = NC_EBADID; goto done;}
	/* Extract the first segment as the dataset name */
	if((nczm_segment1(name,&zzmap->dataset))) goto done;
    }

    /* Dataset superblock will be read by higher layer */
    
    if(mapp) {*mapp = (NCZMAP*)zzmap; zzmap = NULL;}

done:
    nullfree(abspath);
    ncurifree(url);
    if(zzmap) zipclose((NCZMAP*)zzmap,0);

    return ZUNTRACE(stat);
}

/**************************************************/
/* Object API */

static int
zipexists(NCZMAP* map, const char* key)
{
    int stat = NC_NOERR;
    ZZMAP* zzmap = (ZZMAP*)map;
    ZINDEX zindex = -1;

    ZTRACE(6,"map=%s key=%s",map->url,key);
    switch(stat=zzlookupobj(zzmap,key,&zindex)) {
    case NC_NOERR: break;
    case NC_ENOOBJECT: stat = NC_EEMPTY; break;
    case NC_EEMPTY: break;
    default: break;
    }
    return ZUNTRACE(stat);
}

static int
ziplen(NCZMAP* map, const char* key, size64_t* lenp)
{
    int stat = NC_NOERR;
    ZZMAP* zzmap = (ZZMAP*)map;
    size64_t len = 0;
    ZINDEX zindex = -1;

    ZTRACE(6,"map=%s key=%s",map->url,key);

    switch(stat = zzlookupobj(zzmap,key,&zindex)) {
    case NC_NOERR:
	if((stat = zzlen(zzmap,zindex,&len))) goto done;
        break;
    case NC_ENOOBJECT: stat = NC_EEMPTY; len = 0; break;
    case NC_EEMPTY: len = 0; break; /* |dir|==0 */
    default: goto done;
    }

    if(lenp) *lenp = len;

done:
    return ZUNTRACEX(stat,"len=%llu",(lenp?*lenp:777777777777));
}

static int
zipread(NCZMAP* map, const char* key, size64_t start, size64_t count, void* content)
{
    int stat = NC_NOERR;
    ZZMAP* zzmap = (ZZMAP*)map; /* cast to true type */
    zip_file_t* zfile = NULL;
    ZINDEX zindex = -1;
    zip_flags_t zipflags = 0;
    int zerrno;
    size64_t endpoint;
    char* buffer = NULL;
    char* truekey = NULL;
    zip_int64_t red = 0;

    ZTRACE(6,"map=%s key=%s start=%llu count=%llu",map->url,key,start,count);

    switch(stat = zzlookupobj(zzmap,key,&zindex)) {
    case NC_NOERR: break;
    case NC_ENOOBJECT: stat = NC_EEMPTY; /* fall thru */
    case NC_EEMPTY: /* its a dir; fall thru*/
    default: goto done;
    }
    
    /* Note, assume key[0] == '/' */
    if((stat = nczm_appendn(&truekey,2,zzmap->dataset,key)))
        goto done;

    zfile = zip_fopen(zzmap->archive, truekey, zipflags);
    if(zfile == NULL)
	{stat = (zipmaperr(zzmap)); goto done;}

    /* Ideally, we would like to seek to the start point,
       but that will fail if the file is compressed, so
       we need to read whole thing and extract what we need
    */

    /* read data starting at zero */
    if(start == 0) { /*optimize to read directly into content */
        if((red = zip_fread(zfile, content, (zip_uint64_t)count)) < 0)
	    {stat = (zipmaperr(zzmap)); goto done;}
	if(red < count) {stat = NC_EINTERNAL; goto done;}
    } else {
        endpoint = start + count;
        if((buffer = malloc(endpoint))==NULL) /* consider caching this */
            {stat = NC_ENOMEM; goto done;}
        if((red = zip_fread(zfile, buffer, (zip_uint64_t)endpoint)) < 0)
	    {stat = (zipmaperr(zzmap)); goto done;}
	if(red < endpoint) {stat = NC_EINTERNAL; goto done;}
        /* Extract what we need */
        memcpy(content,buffer+start,count);
    }

done:
    nullfree(truekey);
    nullfree(buffer);
    if(zfile != NULL && (zerrno=zip_fclose(zfile)) != 0)
        {stat = ziperrno(zerrno);}
    return ZUNTRACE(stat);
}

static int
zipwrite(NCZMAP* map, const char* key, size64_t start, size64_t count, const void* content)
{
    int stat = NC_NOERR;
    ZZMAP* zzmap = (ZZMAP*)map; /* cast to true type */
    char* truekey = NULL;
    zip_flags_t zflags = 0;
    zip_source_t* zs = NULL;
    ZINDEX zindex = -1;
    zip_int32_t compression = 0;
    zip_error_t zerror;
    void* localbuffer = NULL;

    ZTRACE(6,"map=%s key=%s start=%llu count=%llu",map->url,key,start,count);

    zip_error_init(&zerror);

    if(start != 0 && (ZIP_PROPERTIES & NCZM_ZEROSTART))
        {stat = NC_EEDGE; goto done;}

    /* Create directories */
    if((stat = zzcreategroup(zzmap,key,SKIPLAST))) goto done;

    switch(stat = zzlookupobj(zzmap,key,&zindex)) {
    case NC_NOERR:
        stat = NC_EOBJECT; //goto done; /* Zip files are write once */
	zflags |= ZIP_FL_OVERWRITE;
	break;
    case NC_ENOOBJECT: stat = NC_NOERR; break;
    case NC_EEMPTY: /* its a dir; fall thru */
    default: goto done;
    }
    
    zflags |= ZIP_FL_ENC_UTF_8;
    compression = ZIP_CM_STORE;

    /* prepend the dataset to get truekey */
    /* Note, assume key[0] == '/' */
    if((stat = nczm_appendn(&truekey,2,zzmap->dataset,key)))
        goto done;

    if(count > 0) {
        /* Apparently, the buffer to be written needs to be around at zip_close
           so we need to make a local copy that will be freed by libzip after it is
           no longer needed */
        /* Duplicate the buffer */
        if((localbuffer = malloc((size_t)count))==NULL)
            {stat = NC_ENOMEM; goto done;}
        memcpy(localbuffer,content,count);
    }
    if((zs = zip_source_buffer(zzmap->archive, localbuffer, (zip_uint64_t)count, 1)) == NULL)
        {stat = zipmaperr(zzmap); goto done;}

    if((zindex = zip_file_add(zzmap->archive, truekey, zs, zflags))<0)
        {stat = zipmaperr(zzmap); goto done;}
 
    zs = NULL; localbuffer = NULL;

    if(zip_set_file_compression(zzmap->archive, zindex, compression, 0) < 0)
	{stat = zipmaperr(zzmap); goto done;}

    freesearchcache(zzmap->searchcache); zzmap->searchcache = NULL;

done:
    if(zs) zip_source_free(zs);
    nullfree(localbuffer);
    zip_error_fini(&zerror);
    nullfree(truekey);
    return ZUNTRACE(stat);
}

static int
zipclose(NCZMAP* map, int delete)
{
    int stat = NC_NOERR;
    int zerrno = 0;
    ZZMAP* zzmap = (ZZMAP*)map;

    if(zzmap == NULL) return NC_NOERR;

    ZTRACE(6,"map=%s delete=%d",map->url,delete);
    
    /* Close the zip */
    if(delete)
        zip_discard(zzmap->archive);
    else {
        if((zerrno=zip_close(zzmap->archive)))
	    stat = ziperrno(zerrno);
    }
    if(delete)
        NCremove(zzmap->root);

    zzmap->archive = NULL;
    nczm_clear(map);
    nullfree(zzmap->root);
    nullfree(zzmap->dataset);
    zzmap->root = NULL;
    freesearchcache(zzmap->searchcache);
    free(zzmap);
    return ZUNTRACE(stat);
}

/*
Return a list of full keys immediately under a specified prefix key.
In theory, the returned list should be sorted in lexical order,
but it possible that it is not.
Note that for zip, it is not possible to get just the keys of length n+1,
so, this search must get all keys and process them one by one.
@return NC_NOERR if success, even if no keys returned.
@return NC_EXXX return true error
*/
int
zipsearch(NCZMAP* map, const char* prefix0, NClist* matches)
{
    int stat = NC_NOERR;
    ZZMAP* zzmap = (ZZMAP*)map;
    char* trueprefix = NULL;
    size_t truelen;
    zip_int64_t num_entries, i;
    char** cache = NULL;
    size_t prefixlen;
    NClist* tmp = NULL;

    ZTRACE(6,"map=%s prefix0=%s",map->url,prefix0);

    /* prefix constraints:
       1. prefix is "/"
       2. or prefix has leading '/' and no trailing '/'
    */

    /* Fix up the prefix; including adding the dataset name to the front */
    if(prefix0 == NULL || strlen(prefix0)==0)
        prefix0 = "/";
    /* make sure that prefix0 has leading '/' */
    if(prefix0[0] != '/')
        {stat = NC_EINVAL; goto done;}
    prefixlen = strlen(prefix0);
    truelen = prefixlen+strlen(zzmap->dataset)+1; /* possible trailing '/'*/
    if((trueprefix = (char*)malloc(truelen+1+1))==NULL) /* nul term */
	{stat = NC_ENOMEM; goto done;}
    /* Build the true prefix */
    trueprefix[0] = '\0';
    strlcat(trueprefix,zzmap->dataset,truelen+1);
    strlcat(trueprefix,prefix0,truelen+1); /* recall prefix starts with '/' */
    /* If the prefix did not end in '/', then add it */
    if(prefixlen > 1 && prefix0[prefixlen-1] != '/')
	strlcat(trueprefix,"/",truelen+1);
    truelen = strlen(trueprefix);

    /* Get number of entries */
    num_entries = zip_get_num_entries(zzmap->archive, (zip_flags_t)0);
#ifdef CACHESEARCH
    if(num_entries > 0 && zzmap->searchcache == NULL) {
        /* Release the current cache */
        freesearchcache(zzmap->searchcache);
        zzmap->searchcache = NULL;
	/* Re-build the searchcache */
        if((cache = calloc(sizeof(char*),num_entries+1))==NULL)
	    {stat = NC_ENOMEM; goto done;}
        for(i=0;i < num_entries; i++) {
	    const char *name = NULL;
	    /* get ith entry */
	    name = zip_get_name(zzmap->archive, i, (zip_flags_t)0);
	    /* Add to cache */
	    if((cache[i] = strdup(name))==NULL)
	        {stat = NC_ENOMEM; goto done;}
	}
	cache[num_entries] = NULL;
	zzmap->searchcache = cache; cache = NULL;
     }
#endif
#ifdef CACHESEARCH
    if(zzmap->searchcache != NULL)
#endif
    {
	const char *key = NULL;
	size_t keylen = 0;
	char* match = NULL;
	const char* p;

        tmp = nclistnew();
	/* Walk cache looking for names with prefix plus exactly one other segment */
        for(i=0;i < num_entries; i++) {
	    /* get ith entry */
#ifdef CACHESEARCH
	    key = zzmap->searchcache[i];
#else
	    key = zip_get_name(zzmap->archive, i, (zip_flags_t)0);
#endif
	    keylen = strlen(key);
	    /* Does this name begin with trueprefix? */
	    if(keylen > 0
	       && (keylen <= truelen || strncmp(key,trueprefix,truelen) != 0))
	        continue; /* no match */
	    /* skip trueprefix and extract first segment */
	    p = (key+truelen);
	    if(*p == '\0') continue; /* key is all there is, so ignore it */
	    /* get seg 1 */
	    if((nczm_segment1(p,&match))) goto done;
	    nclistpush(tmp,match); match = NULL;
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
    }

done:
    nclistfreeall(tmp);
    if(cache != NULL) freesearchcache(cache);
    nullfree(trueprefix);
    return ZUNTRACEX(stat,"|matches|=%d",(int)nclistlength(matches));
}

/**************************************************/
/* Utilities */

/* Guarantee existence of a group */
static int
zzcreategroup(ZZMAP* zzmap, const char* key, int nskip)
{
    int stat = NC_NOERR;
    int i, len;
    char* fullpath = NULL;
    NCbytes* path = ncbytesnew();
    NClist* segments = nclistnew();
    ZINDEX zindex;
    zip_flags_t zipflags = ZIP_FL_ENC_UTF_8;
    
    ZTRACE(7,"map=%s key=%s nskip=%d",zzmap->map.url,key,nskip);
    if((stat=nczm_split(key,segments)))
	goto done;    
    len = nclistlength(segments);
    len -= nskip; /* leave off last nskip segments */
    /* Start with the dataset */
    ncbytescat(path,zzmap->dataset);
    for(i=0;i<len;i++) {
	const char* seg = nclistget(segments,i);
	ncbytescat(path,"/");
	ncbytescat(path,seg);
	/* open and/or create the directory */	
	if((zindex = zip_dir_add(zzmap->archive, ncbytescontents(path), zipflags))<0) {
	    switch(stat = zipmaperr(zzmap)) {
	    case NC_EOBJECT: stat = NC_NOERR; break; /* ok */
	    default:
		goto done;
	    }
	}
    }
done:
    nullfree(fullpath);
    ncbytesfree(path);
    nclistfreeall(segments);
    return ZUNTRACE(stat);
}

/* Lookup a key
@return NC_NOERR if found and is a content-bearing object
@return NC_ENOOBJECT if not found
@return NC_EEMPTY if a dir
*/
static int
zzlookupobj(ZZMAP* zzmap, const char* key, ZINDEX* zindex)
{
    int stat = NC_NOERR;
    char* zipfile = NULL;
    char* zipdir = NULL;

    ZTRACE(7,"map=%s key=%s",zzmap->map.url,key);

    if(key == NULL)
        {stat = NC_EINVAL; goto done;}

    /* Note, assume key[0] == '/' */
    if((stat = nczm_appendn(&zipfile,2,zzmap->dataset,key)))
        goto done;

    /* See if exists as a file */
    if((*zindex = zip_name_locate(zzmap->archive, zipfile, 0))<0) {
	/* do a second check to see if zippath as a dir */
        if((stat = nczm_appendn(&zipdir,2,zipfile,"/")))
            goto done;
        if((*zindex = zip_name_locate(zzmap->archive, zipdir, 0))<0)
            {stat = zipmaperr(zzmap); goto done;}
	stat = NC_EEMPTY; /* signal a directory */
    }

done:
    nullfree(zipfile);
    nullfree(zipdir);
    return ZUNTRACE(stat);
}

/* Get length given the index */
static int
zzlen(ZZMAP* zzmap, ZINDEX zindex, size64_t* lenp)
{
    int stat = NC_NOERR;
    size64_t len = 0;
    zip_stat_t statbuf;
    zip_flags_t zipflags = 0;

    assert(zindex >= 0);
    
    ZTRACE(6,"zzmap=%s index=%llu",zzmap,zindex);

    zip_stat_init(&statbuf);
    if(zip_stat_index(zzmap->archive,zindex,zipflags,&statbuf) < 0)
        {stat = (zipmaperr(zzmap)); goto done;}
    assert(statbuf.valid & ZIP_STAT_SIZE);
    len = statbuf.size; /* Always return uncompressed size */

    if(lenp) *lenp = len;

done:
    return ZUNTRACEX(stat,"len=%llu",(lenp?*lenp:777777777777));
}

static void
freesearchcache(char** cache)
{
    char** p;
    if(cache == NULL) return;
    for(p=cache;*p;p++) {
	free(*p);
    }
    free(cache);
}

/**************************************************/
/* External API objects */

NCZMAP_DS_API zmap_zip = {
    NCZM_ZIP_V1,
    ZIP_PROPERTIES,
    zipcreate,
    zipopen,
};

static NCZMAP_API zapi = {
    NCZM_ZIP_V1,
    zipclose,
    zipexists,
    ziplen,
    zipread,
    zipwrite,
    zipsearch,
};

static int
zipmaperr(ZZMAP* zzmap)
{
    zip_error_t* zerr = (zip_error_t*)zip_get_error(zzmap->archive);
    return ziperr(zerr);
}

static int
ziperr(zip_error_t* zerror)
{
    int zerrno = zip_error_code_zip(zerror);
    return ziperrno(zerrno);
}

static int
ziperrno(int zerror)
{
    int stat = NC_NOERR;
    switch (zerror) {
    case ZIP_ER_OK: stat = NC_NOERR; break;
    case ZIP_ER_EXISTS: stat = NC_EOBJECT; break;
    case ZIP_ER_MEMORY: stat = NC_ENOMEM; break;
    case ZIP_ER_SEEK:
    case ZIP_ER_READ:
    case ZIP_ER_WRITE:
    case ZIP_ER_TMPOPEN:
    case ZIP_ER_CRC: stat = NC_EIO; break;
    case ZIP_ER_ZIPCLOSED: stat = NC_EBADID; break;
    case ZIP_ER_NOENT: stat = NC_ENOOBJECT; break;
    case ZIP_ER_OPEN: stat = NC_EACCESS; break;
    case ZIP_ER_INVAL: stat = NC_EINVAL; break;
    case ZIP_ER_INTERNAL: stat = NC_EINTERNAL; break;
    case ZIP_ER_REMOVE: stat = NC_ECANTREMOVE; break;
    case ZIP_ER_DELETED: stat = NC_ENOOBJECT; break;
    case ZIP_ER_RDONLY: stat = NC_EPERM; break;
    case ZIP_ER_CHANGED: stat = NC_EOBJECT; break;
    default: stat = NC_ENCZARR; break;
    }
    return stat;
}
