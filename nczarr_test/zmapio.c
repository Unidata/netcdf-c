/*
 *	Copyright 2018, University Corporation for Atmospheric Research
 *      See netcdf/COPYRIGHT file for copying and redistribution conditions.
 */

#include "config.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif
#ifdef _WIN32
#include "XGetopt.h"
#endif

#include "zincludes.h"
#include "ncpathmgr.h"

#define DEBUG

#define DATANAME "data"

typedef enum Mapop {
MOP_NONE=0,
MOP_OBJDUMP=1,
MOP_CLEAR=2
} Mapop;

static struct Mops {
    Mapop mapop;
    const char* opname;
} mapops[] = {
{MOP_NONE,"none"},
{MOP_OBJDUMP,"objdump"},
{MOP_CLEAR,"clear"},
{MOP_NONE,NULL}
};

/* Command line options */
struct Dumpptions {
    int debug;
    Mapop mop;
    char* infile;
    NCZM_IMPL impl;    
    char* rootpath;
} dumpoptions;

/* Forward */
static int objdump(void);
static NCZM_IMPL implfor(const char* path);
static void printcontent(size64_t len, const char* content,int ismeta);
static int depthR(NCZMAP* map, char* key, NClist* stack);
static char* rootpathfor(const char* path);
static int ismetakey(const char* key);
static void sortlist(NClist* l);

#define NCCHECK(expr) nccheck((expr),__LINE__)
static void nccheck(int stat, int line)
{
    if(stat) {
	fprintf(stderr,"%d: %s\n",line,nc_strerror(stat));
	fflush(stderr);
	exit(1);
    }
}

static void
zmapusage(void)
{
    fprintf(stderr,"usage: zmapio [-d][-v][-x] <file>\n");
    exit(1);
}

static Mapop
decodeop(const char* name)
{
    struct Mops* p = mapops;
    while(p->opname != NULL) {
	if(strcasecmp(p->opname,name)==0) return p->mapop;
    }
    return MOP_NONE;
}

int
main(int argc, char** argv)
{
    int stat = NC_NOERR;
    int c;

    memset((void*)&dumpoptions,0,sizeof(dumpoptions));

    /* Set defaults */
    dumpoptions.mop = MOP_OBJDUMP;

    while ((c = getopt(argc, argv, "dvx:")) != EOF) {
	switch(c) {
	case 'd': 
	    dumpoptions.debug = 1;	    
	    break;
	case 'v': 
	    zmapusage();
	    goto done;
	case 'x': 
	    dumpoptions.mop = decodeop(optarg);
	    if(dumpoptions.mop == MOP_NONE) zmapusage();
	    break;
	case '?':
	   fprintf(stderr,"unknown option\n");
	   goto fail;
	}
    }

    /* get file argument */
    argc -= optind;
    argv += optind;

    if (argc > 1) {
	fprintf(stderr, "zmapio: only one input file argument permitted\n");
	goto fail;
    }
    if (argc == 0) {
	fprintf(stderr, "zmapio: no input file specified\n");
	goto fail;
    }
    dumpoptions.infile = NCdeescape(argv[0]);

    if((dumpoptions.impl = implfor(dumpoptions.infile))== NCZM_UNDEF)
        zmapusage();

    if((dumpoptions.rootpath = rootpathfor(dumpoptions.infile))== NULL)
        zmapusage();

    switch (dumpoptions.mop) {
    case MOP_OBJDUMP:
	if((stat = objdump())) goto done;
	break;
    default:
	fprintf(stderr,"Unimplemented action\n");
	goto fail;
    }    

done:
    /* Reclaim dumpoptions */
    nullfree(dumpoptions.infile);
    nullfree(dumpoptions.rootpath);
    if(stat)
	fprintf(stderr,"fail: %s\n",nc_strerror(stat));
    return (stat ? 1 : 0);    
fail:
    stat = NC_EINVAL;
    goto done;
}

static NCZM_IMPL
implfor(const char* path)
{
    NCURI* uri = NULL;
    const char* mode = NULL;
    NClist* segments = nclistnew();
    int i;
    NCZM_IMPL impl = NCZM_UNDEF;

    ncuriparse(path,&uri);
    if(uri == NULL) goto done;
    mode = ncurifragmentlookup(uri,"mode");
    if(mode == NULL) goto done;
    /* split on commas */
    NCCHECK(nczm_split_delim(mode,',',segments));
    for(i=0;i<nclistlength(segments);i++) {
        const char* value = nclistget(segments,i);
	if(strcmp(value,"nz4")==0) {impl = NCZM_NC4; goto done;}
	if(strcmp(value,"nzf")==0) {impl = NCZM_FILE; goto done;}
	if(strcmp(value,"s3")==0) {impl = NCZM_S3; goto done;}
    }
done:
    ncurifree(uri);
    nclistfreeall(segments);    
    return impl;
}

static char*
rootpathfor(const char* path)
{
    int stat = NC_NOERR;
    NCURI* uri = NULL;
    char* rootpath = NULL;
    NClist* segments = nclistnew();
    char* p = NULL;

    ncuriparse(path,&uri);
    if(uri == NULL) goto done;
    switch (dumpoptions.impl) {
    case NCZM_FILE:
    case NCZM_NC4:
	rootpath = strdup("/"); /*constant*/
	break;
    case NCZM_S3:
        /* Split the path part */
        if((stat = nczm_split(uri->path,segments))) goto done;
	/* remove the bucket name */
	p = (char*)nclistremove(segments,0);
	nullfree(p); p = NULL;
        /* Put it back together */
        if((stat = nczm_join(segments,&rootpath))) goto done;
	break;
    default:
        stat = NC_EINVAL;
	goto done;
    }

done:
    nclistfreeall(segments); segments = NULL;
    ncurifree(uri); uri = NULL;
    if(stat)
        {nullfree(rootpath); rootpath = NULL;}
    return rootpath;
}

static int
objdump(void)
{
    int stat = NC_NOERR;
    NCZMAP* map = NULL;
    NClist* matches = nclistnew();
    NClist* stack = nclistnew();
    char* obj = NULL;
    char* content = NULL;
    int depth;

    if((stat=nczmap_open(dumpoptions.impl, dumpoptions.infile, NC_NOCLOBBER, 0, NULL, &map)))
        goto done;

    
    /* Depth first walk all the groups to get all keys */
    obj = strdup("/");
    if((stat = depthR(map,obj,stack))) goto done;
    obj = NULL; /* its now in the stack */

    if(dumpoptions.debug) {
	int i;
        fprintf(stderr,"stack:\n");
        for(i=0;i<nclistlength(stack);i++)
            fprintf(stderr,"[%d] %s\n",i,(char*)nclistget(stack,i));
    }    
    for(depth=0;nclistlength(stack) > 0;depth++) {
        size64_t len = 0;
	int ismeta = 0;
	int hascontent = 0;
	obj = nclistremove(stack,0); /* zero pos is always top of stack */
	/* Now print info for this obj key */
        switch (stat=nczmap_len(map,obj,&len)) {
	    case NC_NOERR: hascontent = 1; break;
	    case NC_EEMPTY: /* fall thru */ /* this is not a content bearing key */
	    case NC_EACCESS: hascontent = 0; len = 0; stat = NC_NOERR; break;
	    default: goto done;
	}
	if(!hascontent) goto next; /* ignore it */
	if(len > 0) {
	    content = malloc(len+1);
  	    if((stat=nczmap_read(map,obj,0,len,content))) goto done;
	    content[len] = '\0';
        } else {
	    content = NULL;
	}
	if(hascontent) {
	    if(len > 0) {
	        assert(content != NULL);
                printf("[%d] %s : (%llu) |",depth,obj,len);
	        if(ismetakey(obj))
		    ismeta = 1;
	        printcontent(len,content,ismeta);
	        printf("|\n");
	    } else {
	        printf("[%d] %s : (%llu) ||\n",depth,obj,len);
	    }
	} else {
	    printf("[%d] %s\n",depth,obj);
	}
	nullfree(content); content = NULL;
next:
	nullfree(obj); obj = NULL;
    }
done:
    nullfree(obj);
    nullfree(content);
    nczmap_close(map,0);
    nclistfreeall(matches);
    nclistfreeall(stack);
    return stat;
}

/* Depth first walk all the groups to get all keys */
static int
depthR(NCZMAP* map, char* key, NClist* stack)
{
    int stat = NC_NOERR;
    NClist* nextlevel = nclistnew();

    nclistpush(stack,key);
    if((stat=nczmap_search(map,key,nextlevel))) goto done;
    /* Sort nextlevel */
    sortlist(nextlevel);
    /* Push new names onto the stack and recurse */
    while(nclistlength(nextlevel) > 0) {
        char* subkey = nclistremove(nextlevel,0);
	if((stat = depthR(map,subkey,stack))) goto done;
    }
done:
   nclistfreeall(nextlevel);
   return stat;
}

static char hex[16] = "0123456789abcdef";

static void
printcontent(size64_t len, const char* content, int ismeta)
{
    size64_t i;
    for(i=0;i<len;i++) {
	if(ismeta) {
	    printf("%c",content[i]);
	} else {
            unsigned int c0,c1;
	    c1 = (unsigned char)(content[i]);
            c0 = c1 & 0xf;
	    c1 = (c1 >> 4);
            c0 = hex[c0];
            c1 = hex[c1];
	    printf("%c%c",(char)c1,(char)c0);
        }
    }
}

static int
ismetakey(const char* key)
{
    char* suffix = NULL;
    int ismeta = 0;
    if(nczm_divide_at(key,-1,NULL,&suffix) == NC_NOERR) {
        if(suffix && suffix[0] == '/' && suffix[1] == '.')
	    ismeta = 1;
    }
    nullfree(suffix);
    return ismeta;
}

/* bubble sort a list of strings */
static void
sortlist(NClist* l)
{
    size_t i, switched;

    if(nclistlength(l) <= 1) return;
    do {
	switched = 0;
        for(i=0;i<nclistlength(l)-1;i++) {
	    char* ith = nclistget(l,i);
	    char* ith1 = nclistget(l,i+1);
	    if(strcmp(ith,ith1) > 0) {
	        nclistset(l,i,ith1);
    	        nclistset(l,i+1,ith);
	        switched = 1;
	    }
	}
    } while(switched);
#if 0
for(i=0;i<nclistlength(l);i++)
fprintf(stderr,"sorted: [%d] %s\n",i,(const char*)nclistget(l,i));
#endif
}
