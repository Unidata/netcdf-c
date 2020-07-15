/*
 *	Copyright 2018, University Corporation for Atmospheric Research
 *      See netcdf/COPYRIGHT file for copying and redistribution conditions.
 */

#include "ut_includes.h"

#undef DEBUG

#define META1 "/meta1"
#define META2 "/meta2"
#define DATA1 "/data1"
#define DATA1LEN 25

#define PASS 1
#define FAIL 0
#define XFAIL -1


static const char* metadata1 = "{\n\"foo\": 42,\n\"bar\": \"apples\",\n\"baz\": [1, 2, 3, 4]}";
static const char* metaarray1 = "{\n\"shape\": [1,2,3],\n\"dtype\": \"<1\"}";

static char* url = NULL;
static NCZM_IMPL impl = NCZM_UNDEF;
static char* keyprefix = NULL; /* Hold, e.g. S3 bucket name */

/* Forward */
static void title(const char*);
static int report(int pf, const char* op, NCZMAP*);
static int reportx(int pf, const char* tag, const char* op, NCZMAP*);
static char* makekey(const char* key);

static int simplecreate(void);
static int simpledelete(void);
static int simplemeta(void);
static int simpledata(void);
static int search(void);

struct Test tests[] = {
{"create",simplecreate},
{"delete",simpledelete},
{"simplemeta", simplemeta},
{"simpledata", simpledata},
{"search", search},
{NULL,NULL}
};

int
main(int argc, char** argv)
{
    int stat = NC_NOERR;

    if((stat = ut_init(argc, argv, &utoptions))) goto done;
    if(utoptions.file == NULL && utoptions.output != NULL) utoptions.file = strdup(utoptions.output);
    if(utoptions.output == NULL && utoptions.file != NULL)utoptions.output = strdup(utoptions.file);
    impl = kind2impl(utoptions.kind);
//    if(impl == NCZM_S3) setkeyprefix(utoptions.file);
    url = makeurl(utoptions.file,impl);

    if((stat = runtests((const char**)utoptions.cmds,tests))) goto done;
    
done:
    nullfree(url); url = NULL;
    nullfree(keyprefix);
    if(stat) usage(THROW(stat));
    return 0;
}

/* Do a simple create */
static int
simplecreate(void)
{
    int stat = NC_NOERR;
    NCZMAP* map = NULL;
    char* truekey = NULL;

    title(__func__);

    switch(stat = nczmap_create(impl,url,0,0,NULL,&map)) {
    case NC_NOERR: break; /* already exists */
    case NC_EEMPTY: break; /*created*/
    default: goto done;
    }
    
    printf("Pass: create: create: %s\n",url);

    truekey = makekey(NCZMETAROOT);
    if((stat = nczmap_defineobj(map, truekey)))
	goto done;
    printf("Pass: create: defineobj: %s\n",truekey);
    
    /* Do not delete */
    if((stat = nczmap_close(map,0)))
	goto done;
    printf("Pass: create: close\n");
    
    /* Reopen and see if exists */
    if((stat = nczmap_open(impl,url,0,0,NULL,&map)))
	goto done;
    printf("Pass: create: open: %s\n",url);
    
    if((stat = nczmap_exists(map,truekey)))
	goto done;
    printf("Pass: create: exists: %s\n",truekey);
    
    /* close again */
    if((stat = nczmap_close(map,0)))
	goto done;
    printf("Pass: create: close\n");

done:
    nullfree(truekey);
    return THROW(stat);
}

/* Do a simple delete of previously created file */
static int
simpledelete(void)
{
    int stat = NC_NOERR;
    NCZMAP* map = NULL;

    title(__func__);

    switch ((stat = nczmap_open(impl,url,0,0,NULL,&map))) {
    case NC_NOERR:
        report(PASS,"open",map);
	break;
    case NC_EEMPTY:
        {report(XFAIL,"open",map); stat = NC_NOERR; goto done;}
    default:
        {report(FAIL,"open",map); goto done;}
    }     
    /* Delete dataset while closing */
    if((stat = nczmap_close(map,1))) goto done;
    report(PASS,"close: delete",map);

    switch ((stat = nczmap_open(impl,url,0,0,NULL,&map))) {
    default:
    case NC_NOERR:
        report(FAIL,"open",map);
	break;
    case NC_EEMPTY:
        report(XFAIL,"open",map);
	stat = NC_NOERR;
	break;
    }     

done:
    return THROW(stat);
}

static int
simplemeta(void)
{
    int stat = NC_NOERR;
    NCZMAP* map = NULL;
    char* key = NULL;
    char* truekey = NULL;
    size64_t size = 0;
    char* content = NULL;

    title(__func__);

    if((stat = nczmap_open(impl,url,NC_WRITE,0,NULL,&map)))
	goto done;
    report(PASS,"open",map);
	
    truekey = makekey(NCZMETAROOT);
    if((stat = nczmap_defineobj(map, truekey)))
	goto done;
    report(PASS,".nczarr: def",map);
    free(truekey); truekey = NULL;

    if((stat=nczm_concat(META1,ZARRAY,&key)))
	goto done;
    truekey = makekey(key);
    nullfree(key); key = NULL;
    if((stat = nczmap_defineobj(map, truekey)))
	goto done;
    report(PASS,".zarray: def",map);
    free(truekey); key = NULL;

    truekey = makekey(NCZMETAROOT);
    if((stat = nczmap_write(map, truekey, 0, strlen(metadata1), metadata1)))
	goto done;
    report(PASS,".nczarr: writemetadata",map);
    free(truekey); truekey = NULL;
    
    if((stat=nczm_concat(META1,ZARRAY,&key)))
	goto done;
    truekey = makekey(key);
    free(key); key = NULL;    

    if((stat = nczmap_write(map, truekey, 0, strlen(metaarray1), metaarray1)))
	goto done;
    report(PASS,".zarray: writemetaarray1",map);
    free(truekey); truekey = NULL;
    
    if((stat = nczmap_close(map,0)))
	goto done;
    report(PASS,"close",map);

    if((stat = nczmap_open(impl,url,0,0,NULL,&map)))
	goto done;
    report(PASS,"re-open",map);

    /* Read previously written */
    truekey = makekey(NCZMETAROOT);
    if((stat = nczmap_exists(map, truekey)))
	goto done;
    report(PASS,".nczarr: exists",map);
    if((stat = nczmap_len(map, truekey, &size)))
	goto done;
    report(PASS,".nczarr: len",map);
    if(size != strlen(metadata1))
        report(FAIL,".nczarr: len verify",map);
    if((content = calloc(1,strlen(metadata1)+1))==NULL)
        {stat = NC_ENOMEM; goto done;}
    if((stat = nczmap_read(map, truekey, 0, strlen(metadata1), content)))
	goto done;
    report(PASS,".nczarr: readmetadata",map);
    free(truekey); truekey = NULL;
    if(memcmp(content,metadata1,size)!=0)
        report(FAIL,".nczarr: content verify",map);
    else report(PASS,".nczarr: content verify",map);
    nullfree(content); content = NULL;

    if((stat=nczm_concat(META1,ZARRAY,&key)))
	goto done;
    truekey = makekey(key);
    nullfree(key); key = NULL;
    if((stat = nczmap_exists(map, truekey)))
	goto done;
    report(PASS,".zarray: exists",map);
    if((stat = nczmap_len(map, truekey, &size)))
	goto done;
    report(PASS,".zarray: len",map);
    if(size != strlen(metaarray1))
        report(FAIL,".zarray: len verify",map);
    content = calloc(1,strlen(metaarray1)+1);
    if((stat = nczmap_read(map, truekey, 0, strlen(metaarray1), content)))
	goto done;
    report(PASS,".zarray: readmeta",map);
    free(truekey); truekey = NULL;
    if(memcmp(content,metaarray1,size)!=0)
        report(FAIL,".zarray: content verify",map);
    else
        report(PASS,".zarray:content verify",map);
    nullfree(content); content = NULL;
    
    if((stat = nczmap_close(map,0)))
	goto done;
    report(PASS,"close",map);

done:
    nullfree(content);
    nullfree(truekey);
    nullfree(key);
    return THROW(stat);
}

static int
simpledata(void)
{
    int stat = NC_NOERR;
    NCZMAP* map = NULL;
    char* truekey = NULL;
    int data1[DATA1LEN];
    int readdata[DATA1LEN];
    int i;
    size64_t totallen, size;
    char* data1p = (char*)&data1[0]; /* byte level version of data1 */

    title(__func__);

    /* Create the data */
    for(i=0;i<DATA1LEN;i++) data1[i] = i;
    totallen = sizeof(int)*DATA1LEN;

    if((stat = nczmap_open(impl,url,NC_WRITE,0,NULL,&map)))
	goto done;
    report(PASS,"open",map);
	
    truekey = makekey(DATA1);
    if((stat = nczmap_defineobj(map, truekey)))
	goto done;
    report(PASS,DATA1": def",map);

    /* Write in 3 slices */
    for(i=0;i<3;i++) {
        size64_t start, count, third, last;
	third = (totallen+2) / 3; /* round up */
        start = i * third;
	last = start + third;
	if(last > totallen) 
	    last = totallen;
	count = last - start;
	if((stat = nczmap_write(map, truekey, start, count, &data1p[start])))
	     goto done;
    }
    report(PASS,DATA1": write",map);
    
    if((stat = nczmap_close(map,0)))
	goto done;
    report(PASS,"close",map);

    if((stat = nczmap_open(impl,url,0,0,NULL,&map)))
	goto done;
    report(PASS,"re-open",map);

    /* Read previously written */
    if((stat = nczmap_exists(map, truekey)))
	goto done;
    report(PASS,DATA1":exists",map);
    if((stat = nczmap_len(map, truekey, &size)))
	goto done;
    report(PASS,DATA1": len",map);
    if(size != totallen)
        report(FAIL,DATA1": len verify",map);
    if((stat = nczmap_read(map, truekey, 0, totallen, readdata)))
	goto done;
    report(PASS,DATA1": read",map);
    if(memcmp(data1,readdata,size)!=0)
        report(FAIL,DATA1": content verify",map);
    else report(PASS,DATA1": content verify",map);
    free(truekey); truekey = NULL;

done:
    /* Do not delete so we can look at it with ncdump */
    if((stat = nczmap_close(map,0)))
	goto done;
    nullfree(truekey);
    return THROW(stat);
}

static int
searchR(NCZMAP* map, int depth, const char* prefix, NClist* objects)
{
    int i,stat = NC_NOERR;
    NClist* matches = nclistnew();
    
    /* add this prefix to object list */
    nclistpush(objects,strdup(prefix));
    
    /* get next level object keys **below** the prefix */
    if((stat = nczmap_search(map, prefix, matches)))
	goto done;
    reportx(PASS,prefix,"search",map);
    for(i=0;i<nclistlength(matches);i++) {
	const char* key = nclistget(matches,i);
        if((stat = searchR(map,depth+1,key,objects))) goto done;
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
    report(PASS,"open",map);

    /* Do a recursive search on root to get all object keys */
    if((stat=searchR(map,0,"/",objects)))
	goto done;
    /* sort list */
    ut_sortlist(objects);
    /* Print out the list */
    for(i=0;i<nclistlength(objects);i++) {
	const char* key = nclistget(objects,i);
	printf("[%d] %s\n",i,key);
    }

done:
    /* Do not delete so later tests can use it */
    (void)nczmap_close(map,0);
    report(PASS,"close",map);
    nclistfreeall(objects);
    return THROW(stat);
}

#if 0
/* S3 requires knowledge of the bucket+dataset root in order to create the true key */
static void
setkeyprefix(const char* file)
{
    NCURI* uri = NULL;
    NClist* segments = NULL;

    assert(impl == NCZM_S3);
    ncuriparse(file,&uri);
    /* verify that this could be an S3 url */
    if(uri == NULL) return; /* oh well */
    if(strcmp(uri->protocol,"file")==0) return;
    
    segments = nclistnew();
    nczm_split_delim(uri->path,'/',segments);
    /* Extract the first two segments */
    if(nclistlength(segments) < 1) return; /* not enough to qualify */
    /* Remove the bucket */
    { char* s = nclistremove(segments,0);
    nullfree(s); /* do not nest because arg is eval'd twice */
    }
    nczm_join(segments,&keyprefix);
    nclistfreeall(segments);
    ncurifree(uri);
}
#endif

static char*
makekey(const char* key)
{
    char* truekey = NULL;
    nczm_concat(keyprefix,key,&truekey);
    return truekey;
}

static void
title(const char* func)
{
    printf("testing: %s:\n",func);
    fflush(stdout);
}

static int
reportx(int pf, const char* tag, const char* op, NCZMAP* map)
{
    char s[4096];
    snprintf(s,sizeof(s),"%s: %s",tag,op);
    return report(pf,s,map);
}

static int
report(int pf, const char* op, NCZMAP* map)
{
    const char* result;
    switch (pf) {
    case PASS: result = "Pass"; break;
    case XFAIL: result = "XFail"; break;
    case FAIL: default: result = "Fail"; break;
    }
    fprintf(stderr,"%s: %s\n",result,op);
    fflush(stderr);
    if(pf == FAIL) {
        if(map) (void)nczmap_close(map,0);
        exit(1);
    }
    return NC_NOERR;
}
