/*
 *	Copyright 2018, University Corporation for Atmospheric Research
 *      See netcdf/COPYRIGHT file for copying and redistribution conditions.
 */

#include "ut_includes.h"

#undef DEBUG

#if 0
#define FILE1 "testmapnc4.ncz"
#define URL1 "file://" FILE1 "#mode=zarr"
#endif

#define META1 "/meta1"
#define META2 "/meta2"
#define DATA1 "/data1"
#define DATA1LEN 25

static const char* metadata1 = "{\n\"foo\": 42,\n\"bar\": \"apples\",\n\"baz\": [1, 2, 3, 4]}";

static const char* metadata2 = "{\n\"foo\": 42,\n\"bar\": \"apples\",\n\"baz\": [1, 2, 3, 4],\n\"extra\": 137}";

static char* url = NULL;
static NCZM_IMPL impl = NCZM_UNDEF;

/* Forward */
static int simplecreate(void);
static int simpledelete(void);
static int writemeta(void);
static int writemeta2(void);
static int readmeta(void);
static int readmeta2(void);
static int writedata(void);
static int readdata(void);
static int search(void);

struct Test tests[] = {
{"create",simplecreate},
{"delete",simpledelete},
{"writemeta", writemeta},
{"writemeta2", writemeta2},
{"readmeta", readmeta},
{"readmeta2", readmeta2},
{"writedata", writedata},
{"readdata", readdata},
{"search", search},
{NULL,NULL}
};

int
main(int argc, char** argv)
{
    int stat = NC_NOERR;
    char* tmp = NULL;

    if((stat = ut_init(argc, argv, &utoptions))) goto done;
    if(utoptions.file == NULL && utoptions.output == NULL) { stat = NC_EINVAL; goto done; }
    if(utoptions.file == NULL && utoptions.output != NULL) utoptions.file = strdup(utoptions.output);
    if(utoptions.output == NULL && utoptions.file != NULL)utoptions.output = strdup(utoptions.file);

    /* Canonicalize */
    if((stat = NCpathcanonical(utoptions.file,&tmp))) goto done;
    free(utoptions.file);
    utoptions.file = tmp;
    if((stat = NCpathcanonical(utoptions.output,&tmp))) goto done;
    free(utoptions.output);
    utoptions.output = tmp;
fprintf(stderr,"file=%s output=%s\n",utoptions.file,utoptions.output);

    impl = kind2impl(utoptions.kind);
    url = makeurl(utoptions.file,impl,&utoptions);

    if((stat = runtests((const char**)utoptions.cmds,tests))) goto done;
    
done:
    nullfree(tmp);
    ut_final();
    if(stat) usage(stat);
    return 0;
}

/* Do a simple create */
static int
simplecreate(void)
{
    int stat = NC_NOERR;
    NCZMAP* map = NULL;
    char* path = NULL;

    if((stat = nczmap_create(impl,url,0,0,NULL,&map)))
	goto done;

    if((stat=nczm_concat(NULL,NCZMETAROOT,&path)))
	goto done;

    /* Write empty metadata content */
    if((stat = nczmap_write(map, path, 0, 0, (const void*)"")))
	goto done;

done:
    /* Do not delete so we can look at it with ncdump */
    stat = nczmap_close(map,0);
    nullfree(path);
    return THROW(stat);
}

/* Do a simple delete of previously created file */
static int
simpledelete(void)
{
    int stat = NC_NOERR;
    NCZMAP* map = NULL;

    if((stat = nczmap_open(impl,url,0,0,NULL,&map)))
	goto done;
    if((stat = nczmap_close(map,1)))
	goto done;

done:
    return THROW(stat);
}

static int
writemeta(void)
{
    int stat = NC_NOERR;
    NCZMAP* map = NULL;
    char* path = NULL;

    if((stat = nczmap_open(impl,url,NC_WRITE,0,NULL,&map)))
	goto done;

    if((stat=nczm_concat(META1,ZARRAY,&path)))
	goto done;
    if((stat = nczmap_write(map, path, 0, strlen(metadata1), metadata1)))
	goto done;
    free(path); path = NULL;

done:
    /* Do not delete so we can look at it with ncdump */
    (void)nczmap_close(map,0);
    nullfree(path);
    return THROW(stat);
}

static int
writemeta2(void)
{
    int stat = NC_NOERR;
    NCZMAP* map = NULL;
    char* path = NULL;

    if((stat = nczmap_open(impl,url,NC_WRITE,0,NULL,&map)))
	goto done;

    if((stat=nczm_concat(META2,NCZARRAY,&path)))
	goto done;
    if((stat = nczmap_write(map, path, 0, strlen(metadata2), metadata2)))
	goto done;

done:
    /* Do not delete so we can look at it with ncdump */
    (void)nczmap_close(map,0);
    nullfree(path);
    return THROW(stat);
}

static int
readkey(NCZMAP* map, const char* prefix, const char* object)
{
    int stat = NC_NOERR;
    char* path = NULL;
    size64_t olen;
    char* content = NULL;

    if((stat=nczm_concat(prefix,object,&path)))
	goto done;

    /* Get length */
    if((stat = nczmap_len(map, path, &olen)))
	goto done;

    /* Allocate the space for reading the metadata (might be overkill) */
    if((content = malloc(olen+1)) == NULL)
	{stat = NC_ENOMEM; goto done;}

    if((stat = nczmap_read(map, path, 0, olen, content)))
	goto done;

    /* nul terminate */
    content[olen] = '\0';

    printf("%s: |%s|\n",path,content);

done:
    nullfree(content);
    nullfree(path);
    return THROW(stat);
}

static int
readmeta(void)
{
    int stat = NC_NOERR;
    NCZMAP* map = NULL;

    if((stat = nczmap_open(impl,url,0,0,NULL,&map)))
	goto done;

    if((stat = readkey(map,META1,ZARRAY))) goto done;

done:
    (void)nczmap_close(map,0);
    return THROW(stat);
}

static int
readmeta2(void)
{
    int stat = NC_NOERR;
    NCZMAP* map = NULL;

    if((stat = nczmap_open(impl,url,0,0,NULL,&map)))
	goto done;

    if((stat = readkey(map,META2,NCZARRAY)))
        goto done;

done:
    (void)nczmap_close(map,0);
    return THROW(stat);
}

static int
writedata(void)
{
    int stat = NC_NOERR;
    NCZMAP* map = NULL;
    char* path = NULL;
    int data1[DATA1LEN];
    int i;
    size64_t totallen;
    char* data1p = (char*)&data1[0]; /* byte level version of data1 */
    NCZM_FEATURES features;

    /* Create the data */
    for(i=0;i<DATA1LEN;i++) data1[i] = i;
    totallen = sizeof(int)*DATA1LEN;

    if((stat = nczmap_open(impl,url,NC_WRITE,0,NULL,&map)))
	goto done;

    /* ensure object */
    if((stat=nczm_concat(DATA1,"0",&path)))
	goto done;

    features = nczmap_features(impl);
    if((NCZM_ZEROSTART & features) || (NCZM_WRITEONCE & features)) {
	if((stat = nczmap_write(map, path, 0, totallen, data1p)))
	    goto done;
    } else {
        /* Write in 3 slices */
        for(i=0;i<3;i++) {
            size64_t start, count, third, last;
	    third = (totallen+2) / 3; /* round up */
            start = i * third;
	    last = start + third;
	    if(last > totallen) 
	        last = totallen;
  	    count = last - start;
	    if((stat = nczmap_write(map, path, start, count, &data1p[start])))
	        goto done;
	}
    }

done:
    /* Do not delete so we can look at it with ncdump */
    (void)nczmap_close(map,0);
    nullfree(path);
    return THROW(stat);
}

static int
readdata(void)
{
    int stat = NC_NOERR;
    NCZMAP* map = NULL;
    char* path = NULL;
    int data1[DATA1LEN];
    int i;
    size64_t chunklen, totallen;
    char* data1p = NULL; /* byte level pointer into data1 */

    if((stat = nczmap_open(impl,url,0,0,NULL,&map)))
	goto done;

    /* ensure object */
    if((stat=nczm_concat(DATA1,"0",&path)))
	goto done;

    if((stat = nczmap_exists(map,path)))
	goto done;

    /* Read chunks in size sizeof(int)*n, where is rndup(DATA1LEN/3) */
    chunklen = sizeof(int) * ((DATA1LEN+2)/3);
    data1p = (char*)&data1[0];
    totallen = sizeof(int)*DATA1LEN;

    /* Read in 3 chunks */
    memset(data1,0,sizeof(data1));
    for(i=0;i<3;i++) {
        size64_t start, count, last;
        start = i * chunklen;
	last = start + chunklen;
	if(last > totallen) 
	    last = totallen;
	count = last - start;
	if((stat = nczmap_read(map, path, start, count, &data1p[start])))
	     goto done;
    }

    /* Validate */
    for(i=0;i<DATA1LEN;i++) {
	if(data1[i] != i) {
	    fprintf(stderr,"data mismatch: is: %d should be: %d\n",data1[i],i);
	    stat = NC_EINVAL;
	    goto done;
	}
    }

done:
    /* Do not delete so we can look at it with ncdump */
    (void)nczmap_close(map,0);
    nullfree(path);
    return THROW(stat);
}

static int
searchR(NCZMAP* map, int depth, const char* prefix0, NClist* objects)
{
    int i,stat = NC_NOERR;
    NClist* matches = nclistnew();
    char prefix[4096]; /* only ok because we know testdata */
    size_t prefixlen;
    
    nclistpush(objects,strdup(prefix0));

    prefix[0] = '\0';
    strlcat(prefix,prefix0,sizeof(prefix));
    prefixlen = strlen(prefix);

    /* get next level object keys **below** the prefix: should have form: <name> */
    switch (stat = nczmap_search(map, prefix, matches)) {
    case NC_NOERR: break;
    case NC_ENOOBJECT: stat = NC_NOERR; break;/* prefix is not an object */
    default: goto done;
    }
    /* recurse */
    for(i=0;i<nclistlength(matches);i++) {
	const char* key = nclistget(matches,i);
	/* ensure trailing '/' */
        if(prefix[prefixlen-1] != '/')
	    strlcat(prefix,"/",sizeof(prefix));
	strlcat(prefix,key,sizeof(prefix));
        if((stat = searchR(map,depth+1,prefix,objects))) goto done;
	/* restore prefix */
	prefix[prefixlen] = '\0';
	if(stat != NC_NOERR)
	    goto done;
    }
done:
    nclistfreeall(matches);
    return THROW(stat);
}

static int
search(void)
{
    int i,stat = NC_NOERR;
    NCZMAP* map = NULL;
    NClist* objects = nclistnew();

    if((stat = nczmap_open(impl,url,0,0,NULL,&map)))
	goto done;

    /* Do a recursive search on root to get all object keys */
    if((stat=searchR(map,0,"/",objects)))
	goto done;
    /* Sort */
    ut_sortlist(objects);
    /* Print out the list */
    for(i=0;i<nclistlength(objects);i++) {
	const char* key = nclistget(objects,i);
	printf("[%d] %s\n",i,key);
    }

done:
    /* Do not delete so later tests can use it */
    (void)nczmap_close(map,0);
    nclistfreeall(objects);
    return THROW(stat);
}
