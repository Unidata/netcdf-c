/*
 *	Copyright 2018, University Corporation for Atmospheric Research
 *      See netcdf/COPYRIGHT file for copying and redistribution conditions.
 */

#include <stddef.h>
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#if defined(_WIN32) && !defined(__MINGW32__)
#include "XGetopt.h"
#endif

#include <stddef.h>

#include "zincludes.h"
#include "ncpathmgr.h"
#include "nclog.h"
#include "ncuri.h"

#undef DEBUG

#define DATANAME "data"

typedef enum Mapop {
MOP_NONE=0,
MOP_OBJDUMP=1,
MOP_CLEAR=2
} Mapop;

typedef enum OBJKIND {
OK_NONE=0,
OK_META=1,
OK_CHUNK=2,
OK_GROUP=3,
OK_IGNORE=4
} OBJKIND;

static struct Mops {
    Mapop mapop;
    const char* opname;
} mapops[] = {
{MOP_NONE,"none"},
{MOP_OBJDUMP,"objdump"},
{MOP_CLEAR,"clear"},
{MOP_NONE,NULL}
};

static struct Type {
    const char* typename;
    nc_type nctype;
    size_t typesize;
    const char format[16];
} types[] = {
{"ubyte",NC_UBYTE,1,"%u"},
{"byte",NC_BYTE,1,"%d"},
{"ushort",NC_USHORT,2,"%u"},
{"short",NC_SHORT,2,"%d"},
{"uint",NC_UINT,4,"%u"},
{"int",NC_INT,4,"%d"},
{"uint64",NC_UINT64,8,"%llu"},
{"int64",NC_INT64,8,"%lld"},
{"float",NC_FLOAT,4,"%f"},
{"double",NC_DOUBLE,8,"%lf"},
{"char",NC_CHAR,1,"'%c'"},
{"string",NC_STRING,sizeof(char*),"%*s"},
{NULL,NC_NAT,0,""}
};

/* Command line options */
struct Dumpptions {
    int debug;
    int meta_only;
    Mapop mop;
    char infile[4096];
    NCZM_IMPL impl;    
    char* rootpath;
    const struct Type* nctype;
    char format[16];
    int xflags;
#	define XNOZMETADATA 1	
    int strlen;
    int zarrformat;
} dumpoptions;

/* Forward */
static int objdump(void);
static NCZM_IMPL implfor(const char* path);
static void printcontent(size64_t len, const char* content, OBJKIND kind);
static int depthfirst(NCZMAP* map, const char*, NClist* stack);
static char* rootpathfor(const char* path);
static OBJKIND keykind(const char* key);
static void sortlist(NClist* l);
static const char* filenamefor(const char* f0);

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
    fprintf(stderr,"usage: zmapio [-2|-3][-t <type>][-d][-v][-h][-x] <file>\n");
    exit(1);
}

static Mapop
decodeop(const char* name)
{
    struct Mops* p = mapops;
    for(;p->opname != NULL;p++) {
	if(strcasecmp(p->opname,name)==0) return p->mapop;
    }
    return MOP_NONE;
}

static const struct Type*
decodetype(const char* name,  int* strlenp)
{
    struct Type* p = types;
    
    if(strncmp(name,"string/",strlen("string/"))==0) {
        *strlenp = atoi(name+strlen("string/"));
        name = "string";
    }
    
    for(;p->typename != NULL;p++) {
	if(strcasecmp(p->typename,name)==0) return p;
    }
    return NULL;
}

int
main(int argc, char** argv)
{
    int stat = NC_NOERR;
    int c;
    char* p;

    nc_initialize();

    /* Init options */
    memset((void*)&dumpoptions,0,sizeof(dumpoptions));

    while ((c = getopt(argc, argv, "23dhvx:t:F:T:X:")) != EOF) {
	switch(c) {
	case '2':
	    dumpoptions.zarrformat = 2;
	    break;
	case '3':
	    dumpoptions.zarrformat = 3;
	    break;
	case 'd': 
	    dumpoptions.debug = 1;	    
	    break;
	case 'h': 
	    dumpoptions.meta_only = 1;	    
	    break;
	case 't': 
	    dumpoptions.nctype = decodetype(optarg,&dumpoptions.strlen);
	    if(dumpoptions.nctype == NULL) zmapusage();
	    break;
	case 'x': 
	    dumpoptions.mop = decodeop(optarg);
	    if(dumpoptions.mop == MOP_NONE) zmapusage();
	    break;
	case 'v': 
	    zmapusage();
	    goto done;
	case 'F': 
	    strcpy(dumpoptions.format,optarg);
	    break;
	case 'T':
	    nctracelevel(atoi(optarg));
	    break;
	case 'X':
	    for(p=optarg;*p;p++) {
		switch (*p) {
		case 'm': dumpoptions.xflags |= XNOZMETADATA; break;
	        default: fprintf(stderr,"Unknown -X argument: %c",*p); break;
		}
	    };
	    break;
	case '?':
	   fprintf(stderr,"unknown option\n");
	   goto fail;
	}
    }

    /* Default the kind */
    if(dumpoptions.nctype == NULL) {
	dumpoptions.nctype = &types[0];    
	fprintf(stderr,"Default type: %s\n",dumpoptions.nctype->typename); 
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

    {
        char* p = NC_shellUnescape(argv[0]);
        strcpy(dumpoptions.infile,filenamefor(p));
	if(p) free(p);
    }

    if((dumpoptions.impl = implfor(dumpoptions.infile))== NCZM_UNDEF)
        zmapusage();

    if((dumpoptions.rootpath = rootpathfor(dumpoptions.infile))== NULL)
        zmapusage();

    switch (dumpoptions.mop) {
    default:
	fprintf(stderr,"Default action: objdump\n");
	/* fall thru */
    case MOP_OBJDUMP:
	if((stat = objdump())) goto done;
	break;
    }    

done:
    /* Reclaim dumpoptions */
    nullfree(dumpoptions.rootpath);
    nc_finalize();
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

    NCZM_IMPL impl = NCZM_UNDEF;

    ncuriparse(path,&uri);
    if(uri == NULL) goto done;
    mode = ncurifragmentlookup(uri,"mode");
    if(mode == NULL) goto done;
    /* split on commas */
    NCCHECK(nczm_split_delim(mode,',',segments));
    for(size_t i=0;i<nclistlength(segments);i++) {
        const char* value = nclistget(segments,i);
	if(strcmp(value,"file")==0) {impl = NCZM_FILE; goto done;}
	if(strcmp(value,"zip")==0) {impl = NCZM_ZIP; goto done;}
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

    ncuriparse(path,&uri);
    if(uri == NULL) goto done;
    switch (dumpoptions.impl) {
    case NCZM_FILE:
    case NCZM_ZIP:
	rootpath = strdup("/"); /*constant*/
	break;
#ifdef NETCDF_ENABLE_S3  
    case NCZM_S3: {
	char* p = NULL;
        /* Split the path part */
        if((stat = nczm_split(uri->path,segments))) goto done;
	/* remove the bucket name */
	p = (char*)nclistremove(segments,0);
	nullfree(p); p = NULL;
        /* Put it back together */
        if((stat = nczm_join(segments,&rootpath))) goto done;
	} break;
#endif
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
    NClist* stack = nclistnew();
    char* obj = NULL;
    char* content = NULL;
    size_t depth = 0;

    if((stat=nczmap_open(dumpoptions.impl, dumpoptions.infile, NC_NOCLOBBER, 0, NULL, &map)))
        goto done;

    /* Depth first walk all the groups to get all keys */
    if((stat = depthfirst(map,"/",stack))) goto done;

    if(dumpoptions.debug) {

        fprintf(stderr,"stack:\n");
        for(size_t i=0;i<nclistlength(stack);i++)

            fprintf(stderr,"[%zu] %s\n",i,(char*)nclistget(stack,i));
    }    
    for(depth=0;depth < nclistlength(stack);depth++) {
	size64_t len = 0;
	OBJKIND kind = 0;
        size_t padlen;
	nullfree(content); content = NULL;
	obj = nclistget(stack,depth);
	kind = keykind(obj);
	/* Now print info for this obj key */
	switch (stat=nczmap_len(map,obj,&len)) {
	    case NC_NOERR: break;
	    case NC_ENOOBJECT: len = 0; stat = NC_NOERR; break;
	    case NC_EACCESS: /* fall thru */
	    default: goto done;
	}
        padlen = (len+dumpoptions.nctype->typesize);
	if((content = calloc(1,padlen+1))==NULL) {stat = NC_ENOMEM; goto done;}
        content[len] = '\0';
	if(len > 0) {
	    if((stat=nczmap_read(map,obj,0,len,content))) goto done;
	    assert(content != NULL);
	    if(kind == OK_CHUNK)
		len = ceildiv(len,dumpoptions.nctype->typesize);
	    printf("[%zu] %s : (%llu)",depth,obj,len);
	    if(kind == OK_CHUNK &&  dumpoptions.nctype->nctype != NC_STRING)
		printf(" (%s)",dumpoptions.nctype->typename);
	    printf(" |");
	    switch(kind) {
	    case OK_GROUP:
	    case OK_META:
		printcontent(len,content,kind);
		break;
	    case OK_CHUNK:
		if(dumpoptions.meta_only)
		    printf("...");
		else
		    printcontent(len,content,kind);
		break;
	    default: break;
	    }
	    printf("|\n");
	} else {
	    printf("[%zu] %s : (%llu) ||\n",depth,obj,len);
	}
    }
done:
    nullfree(content);
    nczmap_close(map,0);
    nclistfreeall(stack);
    return stat;
}

/* Depth first walk all the groups to get all keys */
static int
depthfirst(NCZMAP* map, const char* key, NClist* stack)
{
    int stat = NC_NOERR;
    NCbytes* prefix = ncbytesnew();

    if(key == NULL || key[0] == '\0')
        key = "/";
    ncbytescat(prefix,key);
    if(strlen(key) > 1 && key[strlen(key)-1]=='/') {
        ncbytessetlength(prefix,ncbyteslength(prefix)-1); /* remove trailing '/' */
	ncbytesnull(prefix);
    }
    if((stat = nczmap_listall(map,ncbytescontents(prefix),stack))) goto done;
    sortlist(stack);
    
done:
    ncbytesfree(prefix);
    return stat;
}


static void
printcontent(size64_t len, const char* content, OBJKIND kind)
{
    size64_t i, count;

    const char* format = NULL;
    size64_t strlen = (size64_t)dumpoptions.strlen;

    format = dumpoptions.nctype->format;
    if(dumpoptions.format[0] != '\0')
        format = dumpoptions.format;
    count = len;

#ifdef DEBUG
    printf("debug: len=%d strlen=%d count=%d\n",(int)len,(int)strlen,(int)count); fflush(stdout);
#endif

    for(i=0;i<count;i++) {
        /* If kind is chunk, then len is # of values, not # of bytes */
	switch(kind) {
	case OK_CHUNK:
	    if(i > 0) printf(", ");
	    switch(dumpoptions.nctype->nctype) {
	    case NC_BYTE: printf(format,((char*)content)[i]); break;
	    case NC_SHORT: printf(format,((short*)content)[i]); break;		
	    case NC_INT: printf(format,((int*)content)[i]); break;		
	    case NC_INT64: printf(format,((long long*)content)[i]); break;		
	    case NC_UBYTE: printf(format,((unsigned char*)content)[i]); break;
	    case NC_USHORT: printf(format,((unsigned short*)content)[i]); break;		
	    case NC_UINT: printf(format,((unsigned int*)content)[i]); break;		
	    case NC_UINT64: printf(format,((unsigned long long*)content)[i]); break;		
	    case NC_FLOAT: printf(format,((float*)content)[i]); break;		
	    case NC_DOUBLE: printf(format,((double*)content)[i]); break;		
	    case NC_CHAR: printf(format,((char*)content)[i]); break;
	    case NC_STRING: printf(format,(int)strlen,((char*)(&content[i*strlen]))); break;
	    default: abort();
	    }
	    break;
	case OK_META:
	    printf("%c",content[i]);
	    break;
	default:
	    printf("%.2hhx", content[i]);
        }
    }
}

static char chunkchars[] = "./c0123456789";

static OBJKIND
keykind(const char* key)
{
    OBJKIND kind = OK_NONE;
    char* suffix = NULL;
    if(nczm_divide_at(key,-1,NULL,&suffix) == NC_NOERR) {
	if(suffix) {
            if(suffix[0] != '/')
		kind = OK_NONE;
	    else if(suffix[1] == '.') {
		if(strcmp(&suffix[1],".zmetadata")==0 && (dumpoptions.xflags & XNOZMETADATA))
		    kind = OK_IGNORE;
		else
	            kind = OK_META;
		if(!dumpoptions.zarrformat) dumpoptions.zarrformat = 2;
	    } else if(strcasecmp(&suffix[1],"zarr.json")==0) {
	        kind = OK_META;
		if(!dumpoptions.zarrformat) dumpoptions.zarrformat = 3;
	    } else if(strcasecmp(&suffix[1],"zarr.json")==0) {
	        kind = OK_META;
		if(!dumpoptions.zarrformat) dumpoptions.zarrformat = 3;
            } else if(suffix[strlen(suffix)-1] == '/')
		kind = OK_GROUP;
	    else {
		char* p = suffix+1;
	        for(;*p;p++) {
	            if(strchr(chunkchars,*p) == NULL) break;
		}
		kind = OK_CHUNK;
	    }
	}
    }
    nullfree(suffix);
    return kind;
}


/* Define a static qsort comparator for strings for use with qsort */
static int
cmp_strings(const void* a1, const void* a2)
{
    const char** s1 = (const char**)a1;
    const char** s2 = (const char**)a2;
    return strcmp(*s1,*s2);
}

/* quick sort a list of strings */
static void
sortlist(NClist* l)
{
    if(nclistlength(l) <= 1) return;
    qsort(nclistcontents(l),nclistlength(l),sizeof(char*),cmp_strings);
#if 0
for(i=0;i<nclistlength(l);i++)
fprintf(stderr,"sorted: [%d] %s\n",i,(const char*)nclistget(l,i));
#endif
}

static const char* urlexts[] = {"nzf", "zip", "nz4", NULL};

static const char*
filenamefor(const char* f0)
{
    static char result[4096];
    const char** extp;
    char* p;
    NCURI* uri = NULL;

    strcpy(result,f0); /* default */
    ncuriparse(f0,&uri);
    if(uri == NULL) {
        /* Not a URL */
        p = strrchr(f0,'.'); /* look at the extension, if any */
        if(p == NULL) goto done; /* No extension */
        p++;
        for(extp=urlexts;*extp;extp++) {
            if(strcmp(p,*extp)==0) break;
        }
        if(*extp == NULL) goto done; /* not found */
        /* Assemble the url */
        strcpy(result,"file://");
        strcat(result,f0); /* core path */
        strcat(result,"#mode=nczarr,");
        strcat(result,*extp);
    }
done:
    ncurifree(uri);
    return result;
}
