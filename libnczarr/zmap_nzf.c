/*
 *	Copyright 2018, University Corporation for Atmospheric Research
 *      See netcdf/COPYRIGHT file for copying and redistribution conditions.
 */

/* Not sure this has any effect */
#define _LARGEFILE_SOURCE 1
#define _LARGEFILE64_SOURCE 1

#include "zincludes.h"

#include <errno.h>

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif

#ifdef _WIN32
#include <windows.h>
#ifndef S_ISDIR
#define S_ISDIR(mode) ((mode) & _S_IFDIR)
#define S_ISREG(mode) ((mode) & _S_IFREG)
#endif
#if 0
#ifndef __cplusplus
#include <io.h>
#include <iostream>
#endif
#endif
#endif

#include "fbits.h"
#include "ncpathmgr.h"

#define VERIFY

#ifndef O_DIRECTORY
# define O_DIRECTORY  0200000
#endif

/*Mnemonic*/
#define FLAG_ISDIR 1
#define FLAG_CREATE 1
#define SKIPLAST 1
#define WHOLEPATH 0

#define NCZM_FILE_V1 1

#ifdef S_IRUSR
static int NC_DEFAULT_CREATE_PERMS =
           (S_IRUSR|S_IWUSR        |S_IRGRP|S_IWGRP);
static int NC_DEFAULT_RWOPEN_PERMS =
           (S_IRUSR|S_IWUSR        |S_IRGRP|S_IWGRP);
static int NC_DEFAULT_ROPEN_PERMS =
//           (S_IRUSR                |S_IRGRP);
           (S_IRUSR|S_IWUSR        |S_IRGRP|S_IWGRP);
static int NC_DEFAULT_DIR_PERMS =
  	   (S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IXGRP|S_IWGRP);
#else
static int NC_DEFAULT_CREATE_PERMS = 0660;
static int NC_DEFAULT_RWOPEN_PERMS = 0660;
static int NC_DEFAULT_ROPEN_PERMS = 0660;
static int NC_DEFAULT_DIR_PERMS = 0770;
#endif

/*
Do a simple mapping of our simplified map model
to a file system.

Every dataset is assumed to be rooted at some directory in the
file tree. So, its location is defined by some path to a
directory representing both the dataset and the root group of
that dataset. The root is recognized because it uniquely
contains a "superblock" file name ".nczarr" that provides
general information about a dataset. Nesting a dataset
inside a dataset is prohibited. This can be detected
by looking for an occurrence of a ".nczarr" file in any containing
directory. If such a file is found, then an illegal nested
dataset has been found.

For the object API, the mapping is as follows:
1. Every content-bearing object (e.g. .zgroup or .zarray) is mapped to a file.
   The key constraint is that the content bearing objects are files.
   This means that if a key  points to a content bearing object then
   no other key can have that content bearing key as a suffix.
2. The meta data containing files are assumed to contain
   UTF-8 character data.
3. The chunk containing files are assumed to contain raw unsigned 8-bit byte data.
*/

/* define the var name containing an objects content */
#define ZCONTENT "data"

typedef struct FD {
  int fd;
} FD;

static FD FDNUL = {-1};

/* Define the "subclass" of NCZMAP */
typedef struct ZFMAP {
    NCZMAP map;
    char* root;
} ZFMAP;

/* Forward */
static NCZMAP_API zapi;
static int zfileclose(NCZMAP* map, int delete);
static int zfcreategroup(ZFMAP*, const char* key, int nskip);
static int zflookupobj(ZFMAP*, const char* key, FD* fd);
static int zfcreateobj(ZFMAP*, const char* key,FD*);
static int zfparseurl(const char* path0, NCURI** urip);
static int zffullpath(ZFMAP* zfmap, const char* key, char**);
static void zfrelease(ZFMAP* zfmap, FD* fd);

static int platformerr(int err);
static int platformcreatefile(ZFMAP* map, const char* truepath,FD*);
static int platformcreatedir(ZFMAP* map, const char* truepath);
static int platformopenfile(ZFMAP* zfmap, const char* truepath, FD* fd);
static int platformopendir(ZFMAP* map, const char* truepath);
static int platformdircontent(ZFMAP* map, const char* path, NClist* contents);
static int platformdelete(ZFMAP* map, const char* path, int delroot);
static int platformseek(ZFMAP* map, FD* fd, int pos, size64_t* offset);
static int platformread(ZFMAP* map, FD* fd, size64_t count, void* content);
static int platformwrite(ZFMAP* map, FD* fd, size64_t count, const void* content);
static void platformrelease(ZFMAP* zfmap, FD* fd);
static int platformtestcontentbearing(ZFMAP* zfmap, const char* truepath);

#ifdef VERIFY
static int verify(const char* path, int isdir);
#endif

static int zfinitialized = 0;
static void zfinitialize(void)
{
    if(!zfinitialized) {
        ZTRACE("","");
	const char* env = NULL;
	int perms = 0;
	env = getenv("NC_DEFAULT_CREATE_PERMS");
	if(env != NULL && strlen(env) > 0) {
	    if(sscanf(env,"%d",&perms) == 1) NC_DEFAULT_CREATE_PERMS = perms;
	}
	env = getenv("NC_DEFAULT_DIR_PERMS");
	if(env != NULL && strlen(env) > 0) {
	    if(sscanf(env,"%d",&perms) == 1) NC_DEFAULT_DIR_PERMS = perms;
	}
        zfinitialized = 1;
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
zfilecreate(const char *path, int mode, size64_t flags, void* parameters, NCZMAP** mapp)
{
    int stat = NC_NOERR;
    char* truepath = NULL;
    ZFMAP* zfmap = NULL;
    NCURI* url = NULL;
	
    NC_UNUSED(parameters);
    ZTRACE("%s %d %llu %p",path,mode,flags,parameters);

    if(!zfinitialized) zfinitialize();

    /* Fixup mode flags */
    mode = (NC_NETCDF4 | NC_WRITE | mode);
    if(flags & FLAG_BYTERANGE)
        mode &=  ~(NC_CLOBBER | NC_WRITE);

    if(!(mode & NC_WRITE))
        {stat = NC_EPERM; goto done;}

    /* path must be a url with file: protocol*/
    if((stat=zfparseurl(path,&url)))
	goto done;
    if(strcasecmp(url->protocol,"file") != 0)
        {stat = NC_EURL; goto done;}

    /* Canonicalize the root path */
    if((stat = nczm_canonicalpath(url->path,&truepath))) goto done;

#ifdef CHECKNESTEDDATASETS
    if(isnesteddataset(truepath))
        {stat = NC_ENCZARR; goto done;}
#endif

    /* Build the z4 state */
    if((zfmap = calloc(1,sizeof(ZFMAP))) == NULL)
	{stat = NC_ENOMEM; goto done;}

    zfmap->map.format = NCZM_FILE;
    zfmap->map.url = ncuribuild(url,NULL,NULL,NCURIALL);
    zfmap->map.flags = flags;
    /* create => NC_WRITE */
    zfmap->map.mode = mode;
    zfmap->map.api = &zapi;
    zfmap->root = truepath;
        truepath = NULL;

    /* If NC_CLOBBER, then delete below file tree */
    if(!fIsSet(mode,NC_NOCLOBBER))
	platformdelete(zfmap,zfmap->root,0);
    
    /* make sure we can access the root directory; create if necessary */
    if((stat = platformcreatedir(zfmap, zfmap->root)))
	goto done;

    /* Dataset superblock will be written by higher layer */
     
    if(mapp) *mapp = (NCZMAP*)zfmap;    

done:
    ncurifree(url);
    nullfree(truepath);
    if(stat)
    	zfileclose((NCZMAP*)zfmap,1);
    return (stat);
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
zfileopen(const char *path, int mode, size64_t flags, void* parameters, NCZMAP** mapp)
{
    int stat = NC_NOERR;
    char* truepath = NULL;
    ZFMAP* zfmap = NULL;
    NCURI*url = NULL;
    
    NC_UNUSED(parameters);
    ZTRACE("%s %d %llu %p",path,mode,flags,parameters);

    if(!zfinitialized) zfinitialize();

    /* Fixup mode flags */
    mode = (NC_NETCDF4 | mode);
    if(flags & FLAG_BYTERANGE)
        mode &=  ~(NC_CLOBBER | NC_WRITE);

    /* path must be a url with file: protocol*/
    if((stat=zfparseurl(path,&url)))
	goto done;
    if(strcasecmp(url->protocol,"file") != 0)
        {stat = NC_EURL; goto done;}

    /* Canonicalize the root path */
    if((stat = nczm_canonicalpath(url->path,&truepath))) goto done;

    /* Build the z4 state */
    if((zfmap = calloc(1,sizeof(ZFMAP))) == NULL)
	{stat = NC_ENOMEM; goto done;}

    zfmap->map.format = NCZM_FILE;
    zfmap->map.url = ncuribuild(url,NULL,NULL,NCURIALL);
    zfmap->map.flags = flags;
    zfmap->map.mode = mode;
    zfmap->map.api = (NCZMAP_API*)&zapi;
    zfmap->root = truepath;
	truepath = NULL;
    
    /* Verify root dir exists */
    switch (stat = platformopendir(zfmap,zfmap->root)) {
    case NC_NOERR: break;
    case NC_ENOTFOUND: stat = NC_EEMPTY; /* fall thru */
    default:
	goto done;
    }
    
    /* Dataset superblock will be read by higher layer */
    
    if(mapp) *mapp = (NCZMAP*)zfmap;    

done:
    ncurifree(url);
    nullfree(truepath);
    if(stat) zfileclose((NCZMAP*)zfmap,0);
    return (stat);
}

/**************************************************/
/* Object API */

static int
zfileexists(NCZMAP* map, const char* key)
{
    int stat = NC_NOERR;
    ZFMAP* zfmap = (ZFMAP*)map;
    FD fd = FDNUL;

    ZTRACE("%s",key);
    switch(stat=zflookupobj(zfmap,key,&fd)) {
    case NC_NOERR: break;
    case NC_ENOTFOUND: stat = NC_EEMPTY;
    case NC_EEMPTY: break;
    default: break;
    }
    zfrelease(zfmap,&fd);    
    return (stat);
}

static int
zfilelen(NCZMAP* map, const char* key, size64_t* lenp)
{
    int stat = NC_NOERR;
    ZFMAP* zfmap = (ZFMAP*)map;
    size64_t len = 0;
    FD fd = FDNUL;

    ZTRACE("%s",key);

    switch (stat=zflookupobj(zfmap,key,&fd)) {
    case NC_NOERR:
        /* Get file size */
        if((stat=platformseek(zfmap, &fd, SEEK_END, &len))) goto done;
	break;
    case NC_ENOTFOUND: stat = NC_EEMPTY;
    case NC_EEMPTY: break;
    default: break;
    }
    zfrelease(zfmap,&fd);
    if(lenp) *lenp = len;

done:
    return THROW(stat);
}

static int
zfiledefineobj(NCZMAP* map, const char* key)
{
    int stat = NC_NOERR;
    FD fd = FDNUL;
    ZFMAP* zfmap = (ZFMAP*)map; /* cast to true type */

    ZTRACE("%s",key);

#ifdef VERIFY
    if(!verify(key,!FLAG_ISDIR))
        assert(!"expected file, have dir");
#endif

    /* Create the intermediate groups as directories */
    if((stat = zfcreategroup(zfmap,key,SKIPLAST)))
	goto done;
    stat = zflookupobj(zfmap,key,&fd);
    zfrelease(zfmap,&fd);
    switch (stat) {
    case NC_NOERR: /* Already exists */
	goto done;
    case NC_ENOTFOUND: stat = NC_EEMPTY; /* file does not exist */
    case NC_EEMPTY: /* empty */
        if((stat = zfcreateobj(zfmap,key,&fd)))
            goto done;
	break;
    default:
	goto done;
    }

done:
    zfrelease(zfmap,&fd);
    return (stat);
}

static int
zfileread(NCZMAP* map, const char* key, size64_t start, size64_t count, void* content)
{
    int stat = NC_NOERR;
    FD fd = FDNUL;
    ZFMAP* zfmap = (ZFMAP*)map; /* cast to true type */

    ZTRACE("%s %llu %llu",key,start,count);

#ifdef VERIFY
    if(!verify(key,!FLAG_ISDIR))
        assert(!"expected file, have dir");
#endif

    switch (stat = zflookupobj(zfmap,key,&fd)) {
    case NC_NOERR:
        if((stat = platformseek(zfmap, &fd, SEEK_SET, &start))) goto done;
        if((stat = platformread(zfmap, &fd, count, content))) goto done;
	break;
    case NC_ENOTFOUND: stat = NC_EEMPTY;
    case NC_EEMPTY: break;
    default: break;
    }
    
done:
    zfrelease(zfmap,&fd);
    return (stat);
}

static int
zfilewrite(NCZMAP* map, const char* key, size64_t start, size64_t count, const void* content)
{
    int stat = NC_NOERR;
    FD fd = FDNUL;
    ZFMAP* zfmap = (ZFMAP*)map; /* cast to true type */

    ZTRACE("%s %llu %llu",key,start,count);

#ifdef VERIFY
    if(!verify(key,!FLAG_ISDIR))
        assert(!"expected file, have dir");
#endif

    switch (stat = zflookupobj(zfmap,key,&fd)) {
    case NC_NOERR:
        if((stat = platformseek(zfmap, &fd, SEEK_SET, &start))) goto done;
        if((stat = platformwrite(zfmap, &fd, count, content))) goto done;
	break;
    case NC_ENOTFOUND: stat = NC_EEMPTY;
    case NC_EEMPTY: break;
    default: break;
    }

done:
    zfrelease(zfmap,&fd);
    return (stat);
}

static int
zfileclose(NCZMAP* map, int delete)
{
    int stat = NC_NOERR;
    ZFMAP* zfmap = (ZFMAP*)map;

    ZTRACE("%d",delete);
    if(zfmap == NULL) return NC_NOERR;
    
    /* Delete the subtree below the root and the root */
    if(delete) {
	stat = platformdelete(zfmap,zfmap->root,1);
	unlink(zfmap->root);
    }
    nczm_clear(map);
    nullfree(zfmap->root);
    zfmap->root = NULL;
    free(zfmap);
    return (stat);
}

/*
Return a list of keys immediately "below" a specified prefix key.
In theory, the returned list should be sorted in lexical order,
but it possible that it is not.
The prefix key is not included. 
*/
int
zfilesearch(NCZMAP* map, const char* prefixkey, NClist* matches)
{
    int stat = NC_NOERR;
    int i;
    ZFMAP* zfmap = (ZFMAP*)map;
    char* truepath = NULL;
    NClist* nextlevel = nclistnew();
    NCbytes* buf = ncbytesnew();
    int trailing;

    ZTRACE("%s",prefixkey);

    /* Make the root path be true */
    if(prefixkey == NULL || strlen(prefixkey)==0 || strcmp(prefixkey,"/")==0)
        truepath = strdup(zfmap->root);
    else if((stat = nczm_concat(zfmap->root,prefixkey,&truepath))) goto done;

    trailing = (prefixkey[strlen(prefixkey)-1] == '/');
    /* get names of the next level path entries */
    switch (stat = platformdircontent(zfmap, truepath, nextlevel)) {
    case NC_NOERR: /* ok */
	break;
    case NC_EEMPTY: /* not a dir */
	stat = NC_NOERR;
	goto done;
    case NC_ENOTFOUND: /* does not exist */
    default:
	goto done;
    }
    for(i=0;i<nclistlength(nextlevel);i++) {
	const char* segment = nclistget(nextlevel,i);
	ncbytescat(buf,prefixkey);
	if(!trailing) ncbytescat(buf,"/");
	ncbytescat(buf,segment);
	nclistpush(matches,ncbytesextract(buf));
    }
    
done:
    nclistfreeall(nextlevel);
    ncbytesfree(buf);
    nullfree(truepath);
    return THROW(stat);
}

/**************************************************/
/* Utilities */

/* Lookup a group by parsed path (segments)*/
/* Return NC_EEMPTY if not found, NC_EINVAL if not a directory; create if create flag is set */
static int
zfcreategroup(ZFMAP* zfmap, const char* key, int nskip)
{
    int stat = NC_NOERR;
    int i, len;
    char* fullpath = NULL;
    NCbytes* path = ncbytesnew();
    NClist* segments = nclistnew();

    ZTRACE("%s %d",key,nskip);
    if((stat=nczm_split(key,segments)))
	goto done;    
    len = nclistlength(segments);
    len -= nskip; /* leave off last nskip segments */
    ncbytescat(path,zfmap->root); /* We need path to be absolute */
    for(i=0;i<len;i++) {
	const char* seg = nclistget(segments,i);
	ncbytescat(path,"/");
	ncbytescat(path,seg);
	/* open and optionally create the directory */	
	stat = platformcreatedir(zfmap,ncbytescontents(path));
	if(stat) goto done;
    }
done:
    nullfree(fullpath);
    ncbytesfree(path);
    nclistfreeall(segments);
    return (stat);
}

/* Lookup an object
@return NC_NOERR if found and is a content-bearing object
@return NC_EEMPTY if exists but is not-content-bearing
@return NC_ENOTFOUND if not found
*/
static int
zflookupobj(ZFMAP* zfmap, const char* key, FD* fd)
{
    int stat = NC_NOERR;
    char* path = NULL;

    ZTRACE("%s",key);

    if((stat = zffullpath(zfmap,key,&path)))
	{goto done;}    

    /* See if this is content-bearing */
    if((stat = platformtestcontentbearing(zfmap,path)))
	goto done;        

    /* Open the file */
    if((stat = platformopenfile(zfmap,path,fd)))
        goto done;

done:
    errno = 0;
    nullfree(path);
    return (stat);    
}

/* When we are finished accessing object */
static void
zfrelease(ZFMAP* zfmap, FD* fd)
{
    ZTRACE("",NULL);
    platformrelease(zfmap,fd);
}

/* Create an object file corresponding to a key; create any
   necessary intermediate groups. Assumed that we actually
   want to create this as a file.
*/
static int
zfcreateobj(ZFMAP* zfmap, const char* key, FD* fd)
{
    int stat = NC_NOERR;
    char* fullpath = NULL;

    ZTRACE("%s",key);

#ifdef VERIFY
    if(!verify(key,!FLAG_ISDIR))
        assert(!"expected file, have dir");
#endif

    /* Create all the prefix groups as directories */
    if((stat = zfcreategroup(zfmap, key, SKIPLAST))) goto done;
    /* Create the final object */
    if((stat=zffullpath(zfmap,key,&fullpath))) goto done;
    if((stat = platformcreatefile(zfmap,fullpath,fd)))
	goto done;
done:
    nullfree(fullpath);
    return (stat);
}

/**************************************************/
/* External API objects */

NCZMAP_DS_API zmap_nzf = {
    NCZM_FILE_V1,
    zfilecreate,
    zfileopen,
};

static NCZMAP_API zapi = {
    NCZM_FILE_V1,
    zfileclose,
    zfileexists,
    zfilelen,
    zfiledefineobj,
    zfileread,
    zfilewrite,
    zfilesearch,
};

static int
zffullpath(ZFMAP* zfmap, const char* key, char** pathp)
{
    int stat = NC_NOERR;
    size_t klen, pxlen, flen;
    char* path = NULL;

    klen = nulllen(key);
    pxlen = strlen(zfmap->root);
    flen = klen+pxlen+1+1;
    if((path = malloc(flen)) == NULL) {stat = NC_ENOMEM; goto done;}
    path[0] = '\0';
    strlcat(path,zfmap->root,flen);
    /* look for special cases */
    if(key != NULL) {
        if(key[0] != '/') strlcat(path,"/",flen);
	if(strcmp(key,"/") != 0)
            strlcat(path,key,flen);
    }
    if(pathp) {*pathp = path; path = NULL;}
done:
    nullfree(path)
    return stat;
}

static int
zfparseurl(const char* path0, NCURI** urip)
{
    int stat = NC_NOERR;
    NCURI* uri = NULL;
    ncuriparse(path0,&uri);
    if(uri == NULL)
	{stat = NC_EURL; goto done;}
    if(urip) {*urip = uri; uri = NULL;}

done:
    ncurifree(uri);
    return stat;
}

/**************************************************/
static int
platformerr(int err)
{
     switch (err) {
     case ENOENT: err = NC_ENOTFOUND; break; /* File does not exist */
     case ENOTDIR: err = NC_EEMPTY; break; /* no content */
     case EACCES: err = NC_EAUTH; break; /* file permissions */
     case EPERM:  err = NC_EAUTH; break; /* ditto */
     default: break;
     }
     return err;
}

/* Test type of the specified file.
@return NC_NOERR if found and is a content-bearing object (file)
@return NC_EEMPTY if exists but is not-content-bearing (a directory)
@return NC_ENOTFOUND if not found
*/
static int
platformtestcontentbearing(ZFMAP* zfmap, const char* truepath)
{
    int ret = 0;
    struct stat buf;
    char* local = NULL;
    
    /* Localize */
    if((ret = nczm_localize(truepath,&local,LOCALIZE))) goto done;

    errno = 0;
    if((ret = stat(local, &buf)) < 0) {
	ret = platformerr(errno);
    } else if(S_ISDIR(buf.st_mode)) {
        ret = NC_EEMPTY;
    } else
        ret = NC_NOERR;
done:
    nullfree(local);
    errno = 0;
    return ret;
}

/* Create a file */
static int
platformcreatefile(ZFMAP* zfmap, const char* truepath, FD* fd)
{
    int stat = NC_NOERR;
    int ioflags = 0;
    int createflags = 0;
    int mode = zfmap->map.mode;
    int permissions = NC_DEFAULT_ROPEN_PERMS;

    errno = 0;
    if(!fIsSet(mode, NC_WRITE))
        ioflags |= (O_RDONLY);
    else {
        ioflags |= (O_RDWR);
	permissions = NC_DEFAULT_RWOPEN_PERMS;
    }
#ifdef O_BINARY
    fSet(ioflags, O_BINARY);
#endif

    if(fIsSet(mode, NC_NOCLOBBER))
        fSet(createflags, O_EXCL);
    else
	fSet(createflags, O_TRUNC);

    if(fIsSet(mode,NC_WRITE))
        createflags = (ioflags|O_CREAT);

    /* Try to create file (will also localize) */
    fd->fd = NCopen3(truepath, createflags, permissions);
    if(fd->fd < 0) { /* could not create */
        stat = platformerr(errno);
        goto done; /* could not open */
    }
done:
    errno = 0;
    return THROW(stat);
}

/* Open a file; fail if it does not exist */
static int
platformopenfile(ZFMAP* zfmap, const char* truepath, FD* fd)
{
    int stat = NC_NOERR;
    int ioflags = 0;
    int mode = zfmap->map.mode;
    int permissions = 0;

    errno = 0;
    if(!fIsSet(mode, NC_WRITE)) {
        ioflags |= (O_RDONLY);
	permissions = NC_DEFAULT_ROPEN_PERMS;
    } else {
        ioflags |= (O_RDWR);
	permissions = NC_DEFAULT_RWOPEN_PERMS;
    }
#ifdef O_BINARY
    fSet(ioflags, O_BINARY);
#endif

#ifdef VERIFY
    if(!verify(truepath,!FLAG_ISDIR))
        assert(!"expected file, have dir");
#endif

    /* Try to open file  (will localize) */
    fd->fd = NCopen3(truepath, ioflags, permissions);
    if(fd->fd < 0)
        {stat = platformerr(errno); goto done;} /* could not open */
done:
    errno = 0;
    return THROW(stat);
}

/* Create a dir */
static int
platformcreatedir(ZFMAP* zfmap, const char* truepath)
{
    int ret = NC_NOERR;
    int mode = zfmap->map.mode;

    errno = 0;
    /* Try to access file as if it exists */
    ret = NCaccess(truepath,ACCESS_MODE_EXISTS);
    if(ret < 0) { /* it does not exist, then it can be anything */
	if(fIsSet(mode,NC_WRITE)) {
	    /* Try to create it */
            /* Create the directory using mkdir */
   	    if(NCmkdir(truepath,NC_DEFAULT_DIR_PERMS) < 0)
	        {ret = platformerr(errno); goto done;}
	    /* try to access again */
	    ret = NCaccess(truepath,ACCESS_MODE_EXISTS);
    	    if(ret < 0)
	        {ret = platformerr(errno); goto done;}
	} else
	    {ret = platformerr(errno); goto done;}	
    }

done:
    errno = 0;
    return THROW(ret);
}

/* Open a dir; fail if it does not exist */
static int
platformopendir(ZFMAP* zfmap, const char* truepath)
{
    int ret = NC_NOERR;

    errno = 0;
    /* Try to access file as if it exists */
    ret = NCaccess(truepath,ACCESS_MODE_EXISTS);
    if(ret < 0)
	{ret = platformerr(errno); goto done;}	
done:
    errno = 0;
    return THROW(ret);
}

/**
Given a path, return the list of all files+dirs immediately below
the specified path: e.g. X s.t. path/X exists.
There are several possibilities:
1. path does not exist => return NC_ENOTFOUND
2. path is not a directory => return NC_EEMPTY and |contents| == 0
3. path is a directory => return NC_NOERR and |contents| >= 0

@return NC_NOERR if path is a directory
@return NC_EEMPTY if path is not a directory
@return NC_ENOTFOUND if path does not exist
*/

#ifdef _WIN32
static int
platformdircontent(ZFMAP* zfmap, const char* truepath, NClist* contents)
{
    int ret = NC_NOERR;
    errno = 0;
    WIN32_FIND_DATA FindFileData;
    HANDLE dir;
    char* ffpath = NULL;
    char* lpath = NULL;
    size_t len;
    char* d = NULL;

    switch (ret = platformtestcontentbearing(zfmap, truepath)) {
    case NC_EEMPTY: ret = NC_NOERR; break; /* directory */    
    case NC_NOERR: ret = NC_EEMPTY; goto done;
    default: goto done;
    }

    /* We need to process the path to make it work with FindFirstFile */
    len = strlen(truepath);
    /* Need to terminate path with '/''*' */
    ffpath = (char*)malloc(len+2+1);
    memcpy(ffpath,truepath,len);
    if(truepath[len-1] != '/') {
	ffpath[len] = '/';	
	len++;
    }
    ffpath[len] = '*'; len++;
    ffpath[len] = '\0';

    /* localize it */
    if((ret = nczm_localize(ffpath,&lpath,LOCALIZE))) goto done;
    dir = FindFirstFile(lpath, &FindFileData);
    if(dir == INVALID_HANDLE_VALUE) {
	/* Distinquish not-a-directory from no-matching-file */
        switch (GetLastError()) {
	case ERROR_FILE_NOT_FOUND: /* No matching files */ /* fall thru */
	    ret = NC_NOERR;
	    goto done;
	case ERROR_DIRECTORY: /* not a directory */
	default:
            ret = NC_EEMPTY;
	    goto done;
	}
    }
    do {
	char* p = NULL;
	const char* name = NULL;
        name = FindFileData.cFileName;
	if(strcmp(name,".")==0 || strcmp(name,"..")==0)
	    continue;
	nclistpush(contents,strdup(name));
    } while(FindNextFile(dir, &FindFileData));

done:
    if(dir) FindClose(dir);
    nullfree(lpath);
    nullfree(ffpath);
    nullfree(d);
    errno = 0;
    return THROW(ret);
}

#else /*!_WIN32*/

static int
platformdircontent(ZFMAP* zfmap, const char* truepath, NClist* contents)
{
    int ret = NC_NOERR;
    errno = 0;
    DIR* dir = NULL;

    switch (ret = platformtestcontentbearing(zfmap, truepath)) {
    case NC_EEMPTY: ret = NC_NOERR; break; /* directory */    
    case NC_NOERR: ret = NC_EEMPTY; goto done; 
    default: goto done;
    }

    dir = NCopendir(truepath);
    if(dir == NULL)
        {ret = platformerr(errno); goto done;}
    for(;;) {
	const char* name = NULL;
	struct dirent* de = NULL;
	errno = 0;
        de = readdir(dir);
        if(de == NULL)
	    {ret = platformerr(errno); goto done;}
	if(strcmp(de->d_name,".")==0 || strcmp(de->d_name,"..")==0)
	    continue;
	name = de->d_name;
	nclistpush(contents,strdup(name));
    }
done:
    if(dir) NCclosedir(dir);
    errno = 0;
    return THROW(ret);
}
#endif /*_WIN32*/

#if 0
static int
platformdeleter(ZFMAP* zfmap, NClist* segments, int depth)
{
    int ret = NC_NOERR;
    struct stat statbuf;
    struct dirent* entry = NULL;
    DIR* dir = NULL;
    char* path = NULL;
    char* tmp = NULL;

    if((ret = nczm_join(segments,&path))) goto done;

    /* When running on any platform that can accept drive letters */
    if((ret = nczm_fixpath(path,&tmp))) goto done;
    nullfree(path); path = NULL;
    if((path = NCpathcvt(tmp))==NULL) {ret = NC_ENOMEM; goto done;}

    errno = 0;
    ret = stat(path, &statbuf);
    if(ret < 0) {
        if(errno == ENOENT) {ret = NC_NOERR; goto done;}
	else {ret = platformerr(errno); goto done;}
    }
    /* process this file */
    if(S_ISDIR(statbuf.st_mode)) {
        if((dir = NCopendir(path)) == NULL)
	     {ret = platformerr(errno); goto done;}
        for(;;) {
	    char* seg = NULL;
	    errno = 0;
            entry = readdir(dir);
	    if(entry == NULL) {ret = platformerr(errno); break;}
	    /* Ignore "." and ".." */
	    if(strcmp(entry->d_name,".")==0) continue;
    	    if(strcmp(entry->d_name,"..")==0) continue;
	    /* append name to segments */
	    if((seg = strdup(entry->d_name)) == NULL)
		{ret = NC_ENOMEM; goto done;}
	    nclistpush(segments,seg);
	    /* recurse */
	    if((ret = platformdeleter(zfmap, segments, depth+1))) goto done;
	    /* remove+reclaim last segment */
	    nclistpop(segments);
	    nullfree(seg);	    	    
        }
	/* Delete this file */
	rmdir(path);
    } else {
	assert(S_ISREG(statbuf.st_mode));
	unlink(path);
    }
done:
    if(dir) NCclosedir(dir);
    /* delete this file|dir */
    remove(path);
    nullfree(path);
    nullfree(tmp);
    errno = 0;
    return THROW(ret);
}
#endif /*0*/

static int
platformdeleter(ZFMAP* zfmap, NCbytes* truepath, int delroot, int depth)
{
    int ret = NC_NOERR;
    int i;
    NClist* contents = nclistnew();
    size_t tpathlen = ncbyteslength(truepath);
    char* local = NULL;

    ret = platformdircontent(zfmap, ncbytescontents(truepath), contents);
    switch (ret) {
    case NC_NOERR: /* recurse to remove levels below */
        for(i=0;i<nclistlength(contents);i++) {
	    const char* name = nclistget(contents,i);
            /* append name to current path */
            ncbytescat(truepath, "/");
            ncbytescat(truepath, name);
            /* recurse */
            if ((ret = platformdeleter(zfmap, truepath,delroot,depth+1))) goto done;
            ncbytessetlength(truepath,tpathlen); /* reset */
	    ncbytesnull(truepath);
	}
	if(depth > 0 || delroot) {
	    /* localize and delete */
	    if((ret = nczm_localize(ncbytescontents(truepath),&local,LOCALIZE))) goto done;
            rmdir(local); /* kill this dir */
	}
	break;    
    case NC_EEMPTY: /* Not a directory */
	ret = NC_NOERR;
        /* localize and delete */
	if(local) {nullfree(local); local = NULL;}
	if((ret = nczm_localize(ncbytescontents(truepath),&local,LOCALIZE))) goto done;
        unlink(local); /* kill this file */
	break;
    case NC_ENOTFOUND:
    default:
	goto done;
    }

done:
    nclistfreeall(contents);
    nullfree(local);
    ncbytessetlength(truepath,tpathlen);
    ncbytesnull(truepath);
    return THROW(ret);
}

/* Deep file/dir deletion; depth first */
static int
platformdelete(ZFMAP* zfmap, const char* rootpath, int delroot)
{
    int stat = NC_NOERR;
    NCbytes* truepath = ncbytesnew();

    if(rootpath == NULL || strlen(rootpath) == 0) goto done;
    ncbytescat(truepath,rootpath);
    if(rootpath[strlen(rootpath)-1] == '/') /* elide trailing '/' */
	ncbytessetlength(truepath,ncbyteslength(truepath)-1);
    if((stat = platformdeleter(zfmap,truepath,delroot,0))) goto done;
done:
    ncbytesfree(truepath);
    errno = 0;
    return THROW(stat);
}

static int
platformseek(ZFMAP* zfmap, FD* fd, int pos, size64_t* sizep)
{
    int ret = NC_NOERR;
    off_t size, newsize;
    struct stat statbuf;    
    
    assert(fd && fd->fd >= 0);
    
    errno = 0;
    ret = fstat(fd->fd, &statbuf);    
    if(ret < 0)
	{ret = platformerr(errno); goto done;}
    if(sizep) size = *sizep; else size = 0;
    newsize = lseek(fd->fd,size,pos);
    if(sizep) *sizep = newsize;
done:
    errno = 0;
    return THROW(ret);
}

static int
platformread(ZFMAP* zfmap, FD* fd, size64_t count, void* content)
{
    int stat = NC_NOERR;
    size_t need = count;
    unsigned char* readpoint = content;

    assert(fd && fd->fd >= 0);

    while(need > 0) {
        ssize_t red;
        if((red = read(fd->fd,readpoint,need)) <= 0)
	    {stat = NC_EACCESS; goto done;}
        need -= red;
	readpoint += red;
    }
done:
    return THROW(stat);
}

static int
platformwrite(ZFMAP* zfmap, FD* fd, size64_t count, const void* content)
{
    int ret = NC_NOERR;
    size_t need = count;
    unsigned char* writepoint = (unsigned char*)content;

    assert(fd && fd->fd >= 0);
    
    while(need > 0) {
        ssize_t red = 0;
        if((red = write(fd->fd,(void*)writepoint,need)) <= 0)	
	    {ret = NC_EACCESS; goto done;}
        need -= red;
	writepoint += red;
    }
done:
    return THROW(ret);
}

#if 0
static int
platformcwd(char** cwdp)
{
    char buf[4096];
    char* cwd = NULL;
    cwd = NCcwd(buf,sizeof(buf));
    if(cwd == NULL) return errno;
    if(cwdp) *cwdp = strdup(buf);
    return NC_NOERR;
}
#endif

/* When we are finished accessing FD; essentially
   equivalent to closing the file descriptor.
*/
static void
platformrelease(ZFMAP* zfmap, FD* fd)
{
    if(fd->fd >=0) NCclose(fd->fd);
    fd->fd = -1;
}

#if 0
/* Close FD => return typ to FDNONE */
*/
static void
platformclose(ZFMAP* zfmap, FD* fd)
{
    if(fd->typ == FDFILE) {
        if(fd->fd >=0) close(fd->u,fd);
	fd->fd = -1;
    } else if(fd->type == FDDIR) {
	if(fd->u.dir) NCclosedir(fd->u,dir);
    }
    fd->typ = FDNONE;
}
#endif


#ifdef VERIFY
static int
verify(const char* path, int isdir)
{
    int ret = 0;
    struct stat buf;

    ret = NCaccess(path,ACCESS_MODE_EXISTS);
    if(ret < 0)
        return 1; /* If it does not exist, then it can be anything */
    ret = stat(path,&buf);
    if(ret < 0) abort();
    if(isdir && S_ISDIR(buf.st_mode)) return 1;
    if(!isdir && S_ISREG(buf.st_mode)) return 1;           
    return 0;
}
#endif

#if 0
/* Return NC_EINVAL if path does not exist; els 1/0 in isdirp and local path in truepathp */
static int
testifdir(const char* path, int* isdirp, char** truepathp)
{
    int ret = NC_NOERR;
    char* tmp = NULL;
    char* truepath = NULL;
    struct stat statbuf;

    /* Make path be windows compatible */
    if((ret = nczm_fixpath(path,&tmp))) goto done;
    if((truepath = NCpathcvt(tmp))==NULL) {ret = NC_ENOMEM; goto done;}

    errno = 0;
    ret = stat(truepath, &statbuf);
    if(ret < 0) {
        if(errno == ENOENT)
	    ret = NC_ENOTFOUND;  /* path does not exist */
	else
	    ret = platformerr(errno);
	goto done;
    }
    /* Check for being a directory */
    if(isdirp) {
        if(S_ISDIR(statbuf.st_mode)) {*isdirp = 1;} else {*isdirp = 0;}
    }
    if(truepathp) {*truepathp = truepath; truepath = NULL;}
done:
    errno = 0;
    nullfree(tmp);
    nullfree(truepath);
    return THROW(ret);    
}
#endif /* 0 */
