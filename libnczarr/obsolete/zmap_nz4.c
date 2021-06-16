/*
 *	Copyright 2018, University Corporation for Atmospheric Research
 *      See netcdf/COPYRIGHT file for copying and redistribution conditions.
 */

#include "zincludes.h"

#include "fbits.h"
#include "ncpathmgr.h"

/*
Do a simple mapping of our simplified map model
to a netcdf-4 file.

For the object API, the mapping is as follows:
1. Every object (e.g. group or array) is mapped to a netcdf-4 group.
2. Object content is kept as a ubyte typed variable with one
   unlimited dimension
*/

#undef DEBUG

#define NCZM_NC4_V1 1

/* What to replace ZDOT with */
#define ZDOTNC4 '_'

/* define the attr/var name containing an objects content */
#define ZCONTENT "data"

/* Define the dimension for the ZCONTENT variable */
/* Avoid creating a coordinate variable */
#define ZCONTENTDIM "data_dim"

/* Define the "subclass" of NCZMAP */
typedef struct Z4MAP {
    NCZMAP map;
    char* root;
    int ncid;
} Z4MAP;


/* Forward */
static NCZMAP_API zapi;
static int znc4close(NCZMAP* map, int delete);
static int zlookupgroup(Z4MAP*, NClist* segments, int nskip, int* grpidp);
static int zlookupobj(Z4MAP*, NClist* segments, int* objidp);
static int zcreategroup(Z4MAP* z4map, NClist* segments, int nskip, int* grpidp);
static int zcreateobj(Z4MAP*, NClist* segments, int* objidp);
static int zcreatedim(Z4MAP*, int, int* dimidp);
static int parseurl(const char* path0, NCURI** urip);
static void nc4ify(const char* zname, char* nc4name);
static void zify(const char* nc4name, char* zname);
static int testcontentbearing(int grpid);
static int errno2ncerr(int err);

/* Define the Dataset level API */

static int
znc4create(const char *path, int mode, size64_t flags, void* parameters, NCZMAP** mapp)
{
    int stat = NC_NOERR;
    char* truepath = NULL;
    char* local = NULL; /* localized truepath */
    Z4MAP* z4map = NULL;
    int ncid;
    NCURI* url = NULL;
    
    /* Fix up mode */
    mode = (NC_NETCDF4 | NC_WRITE | mode);
    if(flags & FLAG_BYTERANGE)
        mode &=  ~(NC_CLOBBER | NC_WRITE);

    if(!(mode & NC_WRITE))
        {stat = NC_EPERM; goto done;}

    if((stat=parseurl(path,&url)))
	goto done;

    /* Canonicalize the root path */
    if((stat = nczm_canonicalpath(url->path,&truepath))) goto done;
    /* Also get local path */
    if((stat = nczm_localize(truepath,&local,LOCALIZE))) goto done;

    /* Build the z4 state */
    if((z4map = calloc(1,sizeof(Z4MAP))) == NULL)
	{stat = NC_ENOMEM; goto done;}

    z4map->map.format = NCZM_NC4;
    z4map->map.url = ncuribuild(url,NULL,NULL,NCURIALL);
    z4map->map.mode = mode;
    z4map->map.flags = flags;
    z4map->map.api = &zapi;
    z4map->root= truepath;
        truepath = NULL;

    if((stat=nc_create(local,mode,&ncid)))
        {stat = NC_EEMPTY; goto done;} /* could not open */
    z4map->ncid = ncid;
    
    if(mapp) *mapp = (NCZMAP*)z4map;    

done:
    ncurifree(url);
    nullfree(truepath);
    nullfree(local);
    if(stat) znc4close((NCZMAP*)z4map,1);
    return (stat);
}

static int
znc4open(const char *path, int mode, size64_t flags, void* parameters, NCZMAP** mapp)
{
    int stat = NC_NOERR;
    char* truepath = NULL;
    char* local = NULL;
    Z4MAP* z4map = NULL;
    int ncid;
    NCURI* url = NULL;
	
    /* Fixup mode */
    mode = NC_NETCDF4 | mode;
    if(flags & FLAG_BYTERANGE)
        mode &= ~(NC_CLOBBER | NC_WRITE);

    if((stat=parseurl(path,&url)))
	goto done;

    /* Canonicalize the root path */
    if((stat = nczm_canonicalpath(url->path,&truepath))) goto done;
    /* Also get local path */
    if((stat = nczm_localize(truepath,&local,LOCALIZE))) goto done;

    /* Build the z4 state */
    if((z4map = calloc(1,sizeof(Z4MAP))) == NULL)
	{stat = NC_ENOMEM; goto done;}

    z4map->map.format = NCZM_NC4;
    z4map->map.url = ncuribuild(url,NULL,NULL,NCURIALL);
    z4map->map.mode = mode;
    z4map->map.flags = flags;
    z4map->map.api = (NCZMAP_API*)&zapi;
    z4map->root = truepath;
        truepath = NULL;

    if((stat=nc_open(local,mode,&ncid)))
       goto done; /* could not open */
    z4map->ncid = ncid;
    
    if(mapp) *mapp = (NCZMAP*)z4map;    

done:
    nullfree(truepath);
    nullfree(local);
    ncurifree(url);
    if(stat) znc4close((NCZMAP*)z4map,0);
    return errno2ncerr(stat);
}

/**************************************************/
/* Object API */

static int
znc4close(NCZMAP* map, int delete)
{
    int stat = NC_NOERR;
    Z4MAP* z4map = (Z4MAP*)map;
    char* path = NULL;

    if(map == NULL) return NC_NOERR;

    path = z4map->root;
        
    if((stat = nc_close(z4map->ncid)))
	goto done;
    if(delete) {
        if((stat = nc_delete(path)))
	    goto done;
    }

done:
    nullfree(z4map->root);
    nczm_clear(map);
    free(z4map);
    return (stat);
}

static int
znc4exists(NCZMAP* map, const char* key)
{
    int stat = NC_NOERR;
    Z4MAP* z4map = (Z4MAP*)map;
    NClist* segments = nclistnew();
    int grpid;
    
    if((stat=nczm_split(key,segments)))
	goto done;    
    switch(stat=zlookupobj(z4map,segments,&grpid)) {
    case NC_NOERR: break;
    case NC_ENOTFOUND: stat = NC_EEMPTY; /* Does not exist */
    case NC_EEMPTY: /* Not an object */
    default: break; /* other error */
    }

done:
    nclistfreeall(segments);
    return (stat);
}

static int
znc4len(NCZMAP* map, const char* key, size64_t* lenp)
{
    int stat = NC_NOERR;
    Z4MAP* z4map = (Z4MAP*)map;
    NClist* segments = nclistnew();
    int grpid, vid;
    size_t dimlen;
    int dimids[1];

    if((stat=nczm_split(key,segments)))
	goto done;    

    switch(stat=zlookupobj(z4map,segments,&grpid)) {
    case NC_NOERR:
        /* Look for the data variable */
        if((stat = nc_inq_varid(grpid,ZCONTENT,&vid))) goto done;
        /* Get size for this variable */
        if((stat = nc_inq_vardimid(grpid,vid,dimids))) goto done;
        /* Get size of the one and only dim */
        if((stat = nc_inq_dimlen(z4map->ncid,dimids[0],&dimlen))) goto done;
        if(lenp) *lenp = (size64_t)dimlen;
	break;
    case NC_ENOTFOUND: stat = NC_EEMPTY; /* does not exist */
    case NC_EEMPTY: /* Not an object */
	if(lenp) *lenp = 0;
	break;
    default: break;
    }

done:
    nclistfreeall(segments);
    return (stat);
}

static int
znc4defineobj(NCZMAP* map, const char* key)
{
    int stat = NC_NOERR;
    int grpid;
    Z4MAP* z4map = (Z4MAP*)map; /* cast to true type */
    NClist* segments = nclistnew();

    if((stat=nczm_split(key,segments)))
	goto done;    
    switch (stat = zlookupobj(z4map,segments,&grpid)) {
    case NC_NOERR: break; /* already exists */
    case NC_ENOTFOUND: stat = NC_EEMPTY;
    case NC_EEMPTY:
        if((stat = zcreateobj(z4map,segments,&grpid))) goto done;
	break;
    default: break; /* other error */
    }

done:
    nclistfreeall(segments);
    return (stat);
}

static int
znc4read(NCZMAP* map, const char* key, size64_t start, size64_t count, void* content)
{
    int stat = NC_NOERR;
    int grpid,vid;
    Z4MAP* z4map = (Z4MAP*)map; /* cast to true type */
    size_t vstart[1];
    size_t vcount[1];
    NClist* segments = nclistnew();

    if((stat=nczm_split(key,segments)))
	goto done;    
    switch (stat = zlookupobj(z4map,segments,&grpid)) {
    case NC_NOERR: /* exists */
        /* Look for the data variable */
        if((stat = nc_inq_varid(grpid,ZCONTENT,&vid))) goto done;
        vstart[0] = (size_t)start;
        vcount[0] = (size_t)count;
        if((stat = nc_get_vara(grpid,vid,vstart,vcount,content))) goto done;
	break;
    case NC_ENOTFOUND: stat = NC_EEMPTY;
    case NC_EEMPTY: break; /* no data */
    default: break; /* other error */
    }

done:
    nclistfreeall(segments);
    return (stat);
}

static int
znc4write(NCZMAP* map, const char* key, size64_t start, size64_t count, const void* content)
{
    int stat = NC_NOERR;
    int grpid,vid;
    Z4MAP* z4map = (Z4MAP*)map; /* cast to true type */
    size_t vstart[1];
    size_t vcount[1];
    NClist* segments = nclistnew();

    if((stat=nczm_split(key,segments)))
	goto done;    
    switch (stat = zlookupobj(z4map,segments,&grpid)) {
    case NC_NOERR: /* exists */
        /* Look for the data variable */
        if((stat = nc_inq_varid(grpid,ZCONTENT,&vid))) goto done;
        vstart[0] = (size_t)start;
        vcount[0] = (size_t)count;
        if((stat = nc_put_vara(grpid,vid,vstart,vcount,content))) goto done;
	break;
    case NC_ENOTFOUND: stat = NC_EEMPTY;
    case NC_EEMPTY: break; /* no data */
    default: break; /* other error */
    }
done:
    nclistfreeall(segments);
    return (stat);
}

/*
Return a list of keys immediately "below" a specified prefix.
In theory, the returned list should be sorted in lexical order,
but it is not.
*/
int
znc4search(NCZMAP* map, const char* prefix, NClist* matches)
{
    int stat = NC_NOERR;
    Z4MAP* z4map = (Z4MAP*)map;
    NClist* segments = nclistnew();
    int grpid, ngrps;
    int* subgrps = NULL;
    int* vars = NULL;
    int i;
    NCbytes* key = ncbytesnew();
	
    if((stat=nczm_split(prefix,segments)))
	goto done;    
    if(nclistlength(segments) > 0) {
        /* Fix the last name */
        size_t pos = nclistlength(segments)-1;
        char* name = nclistget(segments,pos);
        char zname[NC_MAX_NAME];
        zify(name,zname);
        nclistset(segments,pos,strdup(zname));
        nullfree(name);
    }
#ifdef DEBUG
  {
  int i;
  fprintf(stderr,"segments: %d: ",nclistlength(segments));
  for(i=0;i<nclistlength(segments);i++)
	fprintf(stderr," |%s|",(char*)nclistget(segments,i));
  }
  fprintf(stderr,"\n");
#endif

    /* Get grpid of the group for the prefix */
    if((stat = zlookupgroup(z4map,segments,0,&grpid)))
	goto done;

    /* get subgroup ids */
    if((stat = nc_inq_grps(grpid,&ngrps,NULL)))
	goto done;
    if((subgrps = calloc(1,sizeof(int)*ngrps)) == NULL)
	{stat = NC_ENOMEM; goto done;}
    if((stat = nc_inq_grps(grpid,&ngrps,subgrps)))
	goto done;
    /* Add the subgroup keys to the list of matches (zified) */
    for(i=0;i<ngrps;i++) {
	char gname[NC_MAX_NAME];
	char zname[NC_MAX_NAME];
	/* See if this group is content-bearing */
        if((stat = nc_inq_grpname(subgrps[i],gname))) goto done;
        zify(gname,zname);
	nclistpush(matches,strdup(zname));
    }

done:
    ncbytesfree(key);
    nullfree(vars);
    nullfree(subgrps);
    nclistfreeall(segments);
    return stat;
}

/**************************************************/
/* Utilities */

static int
testcontentbearing(int grpid)
{
    int stat = NC_NOERR;
    int varid;
    
    /* See if there is a content variable */
    switch (stat = nc_inq_varid(grpid,ZCONTENT,&varid)) {
    default: goto done; /* true error */
    case NC_NOERR: /* This is a data bearing object */ 
	return NC_NOERR;
    case NC_ENOTVAR:		
	return NC_EEMPTY;
    }

done:
    return stat;
}

/* Lookup a group by parsed path (segments)*/
/* Return NC_ENOTFUND if not found */
static int
zlookupgroup(Z4MAP* z4map, NClist* segments, int nskip, int* grpidp)
{
    int stat = NC_NOERR;
    int i, len, grpid;

    len = nclistlength(segments);
    len += nskip; /* leave off last nskip segments */
    grpid = z4map->ncid;
    for(i=0;i<len;i++) {
	int grpid2;
	const char* seg = nclistget(segments,i);
	char nc4name[NC_MAX_NAME];
	nc4ify(seg,nc4name);	
	if((stat=nc_inq_grp_ncid(grpid,nc4name,&grpid2)))
	    {stat = NC_ENOTFOUND; goto done;}
	grpid = grpid2;
    }
    /* ok, so grpid should be it */
    if(grpidp) *grpidp = grpid;

done:
    return (stat);
}

/* Lookup an object.
@return NC_NOERR if found and is a content-bearing object
@return NC_EEMPTY if exists but is not-content-bearing
@return NC_ENOTFOUND if not found
*/

static int
zlookupobj(Z4MAP* z4map, NClist* segments, int* grpidp)
{
    int stat = NC_NOERR;
    int grpid;

    /* Lookup thru the final object group */
    if((stat = zlookupgroup(z4map,segments,0,&grpid))) {
	goto done;
    }
    /* See if this is content-bearing */
    if((stat = testcontentbearing(grpid)))
	goto done;        

    if(grpidp) *grpidp = grpid;

done:
    return (stat);    
}

/* Create a group; assume all intermediate groups exist
   (do nothing if it already exists) */
static int
zcreategroup(Z4MAP* z4map, NClist* segments, int nskip, int* grpidp)
{
    int stat = NC_NOERR;
    int i, len, grpid, grpid2;
    const char* gname = NULL;
    char nc4name[NC_MAX_NAME];

    len = nclistlength(segments);
    len -= nskip; /* leave off last nskip segments (assume nskip > 0) */
    gname = nclistget(segments,len-1);
    grpid = z4map->ncid;
    /* Do all but last group */
    for(i=0;i<(len-1);i++) {
	const char* seg = nclistget(segments,i);
	nc4ify(seg,nc4name);	
	/* Does this group exist? */
	if((stat=nc_inq_grp_ncid(grpid,nc4name,&grpid2)) == NC_ENOGRP) {
	    {stat = NC_ENOTFOUND; goto done;} /* missing intermediate */
	}
	grpid = grpid2;
    }
    /* Check status of last group */
    nc4ify(gname,nc4name);
    if((stat = nc_inq_grp_ncid(grpid,nc4name,&grpid2))) {
	if(stat != NC_ENOGRP) goto done;
        if((stat = nc_def_grp(grpid,nc4name,&grpid2)))
	    goto done;
	grpid = grpid2;
    }

    if(grpidp) *grpidp = grpid;

done:
    return (stat);
}

static int
zcreatedim(Z4MAP* z4map, int grpid, int* dimidp)
{
    int stat = NC_NOERR;
    int dimid;

    NC_UNUSED(z4map);

    if((stat=nc_inq_dimid(grpid,ZCONTENTDIM,&dimid))) {
	/* create it */
        if((stat=nc_def_dim(grpid,ZCONTENTDIM,NC_UNLIMITED,&dimid)))
	    goto done;
    }
    if(dimidp) *dimidp = dimid;

done:
    return (stat);
}

/* Create an object group corresponding to a key; create any
   necessary intermediates.
 */
static int
zcreateobj(Z4MAP* z4map, NClist* segments, int* grpidp)
{
    int skip,stat = NC_NOERR;
    int grpid, varid;
    int dimid[1];

    /* Create the whole path */
    skip = nclistlength(segments);
    for(skip--;skip >= 0; skip--) {
        if((stat = zcreategroup(z4map,segments,skip,&grpid)))
	goto done;
    }
    /* Last grpid should be one we want */
    if(grpidp) *grpidp = grpid;
    /* Create the content-bearer */
    /* Create the corresponding dimension */
    if((stat = zcreatedim(z4map,grpid,&dimid[0])))
	    goto done;
    /* Create the variable */
    if((stat=nc_def_var(grpid, ZCONTENT, NC_UBYTE, 1, dimid, &varid)))
	goto done;
done:
    return (stat);    
}

static int
parseurl(const char* path0, NCURI** urip)
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

/* Convert _z... name to .z... name */
static void
zify(const char* nc4name, char* zname)
{
    zname[0] = '\0';
    strlcat(zname,nc4name,NC_MAX_NAME);
    if(zname[0] == ZDOTNC4) zname[0] = NCZM_DOT;
}

/* Convert .z... name to _z... name */
static void
nc4ify(const char* zname, char* nc4name)
{
    nc4name[0] = '\0';
    strlcat(nc4name,zname,NC_MAX_NAME);
    if(nc4name[0] == NCZM_DOT) nc4name[0] = ZDOTNC4;
}

/* Convert errno to closest NC_EXXX error */
static int
errno2ncerr(int err)
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

/**************************************************/
/* External API objects */

NCZMAP_DS_API zmap_nz4 = {
    NCZM_NC4_V1,
    0,
    znc4create,
    znc4open,
};

static NCZMAP_API zapi = {
    NCZM_NC4_V1,
    znc4close,
    znc4exists,
    znc4len,
    znc4defineobj,
    znc4read,
    znc4write,
    znc4search,
};
