/*
 *	Copyright 2018, University Corporation for Atmospheric Research
 *      See netcdf/COPYRIGHT file for copying and redistribution conditions.
 */

#include "ut_includes.h"

#undef DEBUG

#define OPEN "[{("
#define CLOSE "]})"

#define LPAREN '('
#define RPAREN ')'
#define BLANK ' '

NCbytes* buf = NULL;

static Dimdef* finddim(const char* name, NClist* defs);

#if 0
static void
ranktest(int rank, char c, int count)
{
    if(rank != count) {
	fprintf(stderr,"Option '%c': rank mismatch: rank=%d count=%d\n",
		c,rank,count);
	exit(1);
    }
}
#endif

int
parseslices(const char* s0, int* nslicesp, NCZSlice* slices)
{
    int count,nchars,nslices,i;
    const char* s = NULL;
    unsigned long start,stop,stride;

    /* First, compute number of slices */
    for(s=s0,nslices=0;*s;s++) {
	if(*s == '[') nslices++;
    }
    
    if(nslices > NC_MAX_VAR_DIMS) return THROW(NC_EINVAL); /* too many */
    if(nslicesp) *nslicesp = nslices;

    /* Extract the slices */
    for(i=0,s=s0;*s;s+=nchars,i++) {
        NCZSlice* sl = &slices[i];
	/* Try 3-element slice first */
	stride = 1; /* default */
	nchars = -1;
        count = sscanf(s,"[%lu:%lu]%n",&start,&stop,&nchars);
	if(nchars == -1) {
	    nchars = -1;
            count = sscanf(s,"[%lu:%lu:%lu]%n",&start,&stop,&stride,&nchars);
	    if(count != 3) return THROW(NC_EINVAL);
	}
        sl->start = start;
        sl->stop = stop;
        sl->stride = stride;
    }
    return NC_NOERR;
}

int
parsedimdef(const char* s0, Dimdef** defp)
{
    int nchars;
    const char* s = NULL;
    Dimdef* def = NULL;    
    unsigned l;
    const char* p;
    ptrdiff_t count;
    
    if((def = calloc(1,sizeof(struct Dimdef)))==NULL)
	return THROW(NC_ENOMEM);

    /* Extract */
    s = s0;
    if((p = strchr(s,'=')) == NULL) abort();
    if((count = (p - s)) == 0) return THROW(NC_EINVAL);
    def->name = malloc(count+1);
    memcpy(def->name,s,count);
    def->name[count] = '\0';
    s = p+1;
    sscanf(s,"%u%n",&l,&nchars);
    if(nchars == -1) return NC_EINVAL;
    def->size = (size_t)l;
    s += nchars;
    if(*s != '\0') return NC_EINVAL;
    if(defp) *defp = def;
    return NC_NOERR;
}

int
parsevardef(const char* s0, NClist* dimdefs, Vardef** varp)
{
    int count;
    const char* s = NULL;
    Vardef* vd = NULL;    
    const char* p;
    ptrdiff_t len;
    char name[NC_MAX_NAME];

    if((vd = calloc(1,sizeof(Vardef)))==NULL)
        return THROW(NC_ENOMEM);

    s=s0;

    /* Scan for the end of type name */
    p = strchr(s,BLANK);
    if(p == NULL) return THROW(NC_EINVAL);
    len = (p - s);  
    if(len == 0) return THROW(NC_EINVAL);
    memcpy(name,s,len);
    name[len] = '\0';
    vd->typeid = ut_typeforname(name);
    vd->typesize = ut_typesize(vd->typeid);
    while(*p == BLANK) p++;
    s = p;

    /* Scan for the end of var name */
    p = strchr(s,LPAREN);   
    if(p == NULL) return THROW(NC_EINVAL);
    len = (p - s);  
    if(len == 0) return THROW(NC_EINVAL);
    memcpy(name,s,len);
    name[len] = '\0';
    vd->name = strdup(name);     
    /* parse a vector of dimnames and chunksizes and convert */
    s = p;
    if(*s == LPAREN) {
        char** names = NULL;
        char* p;
        s++;
        count = parsestringvector(s,RPAREN,&names);
        if(count >= NC_MAX_VAR_DIMS) return THROW(NC_EINVAL);
        vd->rank = count;
        if(vd->rank > 0) {
            int j;
            for(j=0;j<vd->rank;j++) {
                Dimdef* dimref = NULL;
                /* Split on / to get chunksize */
                p = strchr(names[j],'/');
                if(p) *p++ = '\0';              
                if((dimref = finddim(names[j],dimdefs)) == NULL)
                    return THROW(NC_EINVAL);
                vd->dimrefs[j] = dimref;
		vd->dimsizes[j] = dimref->size;
                if(p == NULL)
                    vd->chunksizes[j] = dimref->size;
                else {
                    unsigned long l;
                    sscanf(p,"%lu",&l);
                    vd->chunksizes[j] = (size_t)l;
                }
            }
            /* Skip past the trailing rparen */
            if((p = strchr(s,RPAREN)) == NULL) abort();
            p++;
        }
        freestringvec(names);
    }
    if(varp) *varp = vd;
    return NC_NOERR;
}

int
parsestringvector(const char* s0, int stopchar, char*** namesp)
{
    int nelems,i;
    const char* s;
    char** names = NULL;

    /* First, compute number of elements */
    for(s=s0,nelems=1;*s;s++) {if(*s == ',') nelems++; if(*s == stopchar) break;}
    if(nelems == 0) return THROW(NC_EINVAL);
    names = calloc(nelems+1,sizeof(char*));
    for(s=s0,i=0;i<nelems;i++) {
        ptrdiff_t len;
        const char* p = strchr(s,',');
        if(p == NULL) p = strchr(s,stopchar);
        if(p == NULL) p = s + strlen(s);
        if(names[i] == NULL) {
            char* q;
            len = (p - s);
            q = malloc(1+len);
            memcpy(q,s,len);
            q[len] = '\0';
            names[i] = q;
        }
        if(*p == '\0' || *p == stopchar) s = p; else s = p+1;
    }
    names[nelems] = NULL;
    if(namesp) *namesp = names;
    return nelems;
}

int
parseintvector(const char* s0, int typelen, void** vectorp)
{
    int count,nchars,nelems,index;
    const char* s = NULL;
    void* vector = NULL;

    /* First, compute number of elements */
    for(s=s0,nelems=1;*s;s++) {
        if(*s == ',') nelems++;
    }

    vector = calloc(nelems,typelen);

    /* Extract the elements of the vector */
    /* Skip any leading bracketchar */
    s=s0;
    if(strchr(OPEN,*s0) != NULL) s++;
    for(index=0;*s;index++) {
        long long elem;
        nchars = -1;
        count = sscanf(s,"%lld%n",&elem,&nchars);
        if(nchars == -1 || count != 1) return THROW(NC_EINVAL);
        s += nchars;
        if(*s == ',') s++;
        switch (typelen) {
        case 1: ((char*)vector)[index] = (char)elem; break;
        case 2: ((short*)vector)[index] = (short)elem; break;
        case 4: ((int*)vector)[index] = (int)elem; break;
        case 8: ((long long*)vector)[index] = (long long)elem; break;
        default: abort();
        }
    }
    assert(nelems == index);
    if(vectorp) *vectorp = vector;
    return nelems;
}

void
freedimdefs(NClist* defs)
{
    int i;
    for(i=0;i<nclistlength(defs);i++) {
	Dimdef* dd = nclistget(defs,i);
	nullfree(dd->name);
	nullfree(dd);
    }
}

void
freevardefs(NClist* defs)
{
    int i;
    for(i=0;i<nclistlength(defs);i++) {
	Vardef* vd = nclistget(defs,i);
	nullfree(vd->name);
	nullfree(vd);
    }
}

void
freeranges(NCZChunkRange* ranges)
{
}

void
freeslices(NCZSlice* slices)
{
}

void
freestringvec(char** vec)
{
    if(vec != NULL) {
        char** p;
	for(p=vec;*p;p++) free(*p);
    }
    nullfree(vec);
}

void
freeprojvector(int rank, NCZProjection** vec)
{
    if(vec != NULL) {
        int r;
        for(r=0;r<rank;r++) free(vec[r]);
    }
    nullfree(vec);
}

/**************************************************/

#if 0
char*
printvec(int len, size64_t* vec)
{
    char* result = NULL;
    int i;
    char value[128];
    NCbytes* buf = ncbytesnew();

    ncbytescat(buf,"(");
    for(i=0;i<len;i++) {
        if(i > 0) ncbytescat(buf,",");
        snprintf(value,sizeof(value),"%lu",(unsigned long)vec[i]);      
        ncbytescat(buf,value);
    }
    ncbytescat(buf,")");
    result = ncbytesextract(buf);
    ncbytesfree(buf);
    return result;
}
#endif /*0*/

/**************************************************/
int
ut_typesize(nc_type t)
{
    switch (t) {
    case NC_BYTE: case NC_UBYTE: return 1;
    case NC_SHORT: case NC_USHORT: return 2;
    case NC_INT: case NC_UINT: return 4;
    case NC_INT64: case NC_UINT64: return 8;
    case NC_FLOAT: return 4;
    case NC_DOUBLE: return 8;
    default: usage(THROW(NC_EINVAL));
    }
    return 0;
}

nc_type
ut_typeforname(const char* tname)
{
    if(strcasecmp("byte",tname)==0) return NC_BYTE;
    if(strcasecmp("ubyte",tname)==0) return NC_UBYTE;
    if(strcasecmp("short",tname)==0) return NC_SHORT;
    if(strcasecmp("ushort",tname)==0) return NC_USHORT;
    if(strcasecmp("int",tname)==0) return NC_INT;
    if(strcasecmp("uint",tname)==0) return NC_UINT;
    if(strcasecmp("int64",tname)==0) return NC_INT64;
    if(strcasecmp("uint64",tname)==0) return NC_UINT64;
    if(strcasecmp("float",tname)==0) return NC_FLOAT;
    if(strcasecmp("double",tname)==0) return NC_DOUBLE;
    usage(THROW(NC_EINVAL));
    return NC_NAT;
}

static Dimdef*
finddim(const char* name, NClist* defs)
{
    int i;
    for(i=0;i<nclistlength(defs);i++) {
        Dimdef* dd = nclistget(defs,i);
        if(strcmp(dd->name,name) == 0)
            return dd;
    }    
    return NULL;
}

NCZM_IMPL
kind2impl(const char* kind)
{
    if(strcasecmp("s3",kind)==0) return NCZM_S3;
    else if(strcasecmp("nz4",kind)==0) return NCZM_NC4;
    else if(strcasecmp("nzf",kind)==0) return NCZM_FILE;
    else return NCZM_UNDEF;
}

const char*
impl2kind(NCZM_IMPL impl)
{
    switch (impl) {
    case NCZM_S3: return "s3";
    case NCZM_NC4: return "nz4";
    case NCZM_FILE: return "nzf";
    case NCZM_UNDEF: break;
    }
    return NULL;
}

/* Goal: Given a set of per-dimension indices,
     compute the corresponding linear position.
*/
size64_t
computelinearoffset(int R, const size64_t* indices, const size64_t* max, size64_t* productp)
{
      size64_t offset, product;
      int i;

      offset = 0; product = 1;
      for(i=0;i<R;i++) {
          offset *= max[i];
          offset += indices[i];
	  product *= max[i];
      } 
      if(productp) *productp = product;
      return offset;
}

void
slices2vector(int rank, NCZSlice* slices, size64_t** startp, size64_t** stopp, size64_t** stridep, size64_t** maxp)
{
    static size64_t start[NC_MAX_VAR_DIMS];
    static size64_t stop[NC_MAX_VAR_DIMS];
    static size64_t stride[NC_MAX_VAR_DIMS];
    static size64_t max[NC_MAX_VAR_DIMS];
    int i;
    for(i=0;i<rank;i++) {
	start[i] = slices[i].start;
	stop[i] = slices[i].stop;
	stride[i] = slices[i].stride;
	max[i] = slices[i].len;
    }
    if(startp) *startp = start;
    if(stopp) *stopp = stop;
    if(stridep) *stridep = stride;
    if(maxp) *maxp = max;
}

void
printoptions(struct UTOptions* opts)
{
    char** p;
    int i;
    printf("Options:");
#if 0
    printf(" debug=%d",opts->debug);
    printf(" file=|%s|",opts->file);
    printf(" output=|%s|",opts->output);
#endif
    if(opts->kind)
        printf(" kind=%s",opts->kind);
    if(opts->cmds) {
        printf(" cmds=");
        for(i=0,p=opts->cmds;*p;p++,i++)
            printf("%s%s",(i==0?"(":","),*p);
        printf(")");
    }

    for(i=0;i<nclistlength(opts->dimdefs);i++) {
	struct Dimdef* dd = (struct Dimdef*)nclistget(opts->dimdefs,i);
        printf(" -d%s=%llu",dd->name,dd->size);
    }

    for(i=0;i<nclistlength(opts->vardefs);i++) {
	int j;
	struct Vardef* vd = (struct Vardef*)nclistget(opts->vardefs,i);
        printf(" -v '%d %s[",vd->typeid,vd->name);
	for(j=0;j<vd->rank;j++) {
	    Dimdef* vdd = vd->dimrefs[j];
	    if(j > 0) printf(",");
            printf("%s/%llu",vdd->name,vd->chunksizes[j]);
	}
	printf("]'");
    }

    printf(" -s ");
    for(i=0;i<opts->nslices;i++) {
	NCZSlice* sl = &opts->slices[i];
	printf("%s",nczprint_slicex(*sl,1));
    }
    printf("\n");
}

int
hasdriveletter(const char* f)
{
    if(f == NULL || *f == '\0' || strlen(f) < 3) return 0;
    if(f[1] != ':') return 0;
    if(f[2] != '/' && f[2] != '\\') return 0;
    if((f[0] < 'z' && f[0] >= 'a') || (f[0] < 'Z' && f[0] >= 'A'))
        return 1;
    return 0;
}

/* bubble sort a list of strings */
void
ut_sortlist(NClist* l)
{
    int i, switched;

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
#ifdef DEBUG
for(i=0;i<nclistlength(l);i++)
fprintf(stderr,"sorted: [%d] %s\n",i,(const char*)nclistget(l,i));
#endif
}
