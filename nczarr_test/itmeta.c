/*
 *	Copyright 2018, University Corporation for Atmospheric Research
 *      See netcdf/COPYRIGHT file for copying and redistribution conditions.
 */

#include "ut_includes.h"

#define SETUP
#define DEBUG

#define FILE "testmeta.ncz"

#define VAR1 "var1"
#define VAR2 "var2"

static char* url = NULL;

/* Arguments from command line */
struct Options {
    int debug;
    char* cmd;
} options;

typedef union AtomicsR {
    char bytev[1];
    short shortv[1];
    int intv[1];
    float floatv[1];
    double doublev[1];
    unsigned char ubytev[1];
    unsigned short ushortv[1];
    unsigned int uintv[1];
    long long int64v[1];
    unsigned long long uint64v[1];
    char charv[1];
} AtomicsR;

typedef struct AtomicsW {
    char bytev[1];
    short shortv[1];
    int intv[1];
    float floatv[1];
    double doublev[1];
    unsigned char ubytev[1];
    unsigned short ushortv[1];
    unsigned int uintv[1];
    long long int64v[1];
    unsigned long long uint64v[1];
    char charv[1];
} AtomicsW;

/* Forward */
static int testcreate(void);
static int testdim1(void);
static int testvar1(void);
static int testvar2(void);
static int testattr1(void);
static int testattr2(void);
static int testread1(void);
static int testread2(void);
static int testread3(void);

struct Test {
    const char* cmd;
    int (*test)(void);
} tests[] = {
{"create", testcreate},
{"dim1",   testdim1},
{"var1",   testvar1},
{"var2",   testvar2},
{"attr1",   testattr1},
{"attr2",   testattr2},
{"read1",   testread1},
{"read2",   testread2},
{"read3",   testread3},
{NULL, NULL}
};

#ifdef SETUP
#define NCCHECK(expr) nccheck((expr),__LINE__)
void nccheck(int stat, int line)
{
    if(stat) {
	fprintf(stderr,"%d: %s\n",line,nc_strerror(stat));
	fflush(stderr);
	exit(1);
    }
}
#endif

static void
makeurl(const char* file)
{
    char wd[4096];
    NCbytes* buf = ncbytesnew();
    ncbytescat(buf,"file://");
    (void)getcwd(wd, sizeof(wd));
    ncbytescat(buf,wd);
    ncbytescat(buf,"/");
    ncbytescat(buf,file);
    ncbytescat(buf,"#mode=zarr"); /* => use default file: format */
    url = ncbytesextract(buf);
    ncbytesfree(buf);
}

int
main(int argc, char** argv)
{
    int stat = NC_NOERR;
    int c;
    struct Test* t = NULL;
    struct Test* test = NULL;
    char* file = NULL;

    memset((void*)&options,0,sizeof(options));
    while ((c = getopt(argc, argv, "dc:o:")) != EOF) {
	switch(c) {
	case 'd': 
	    options.debug = 1;	    
	    break;
	case 'c':
	    if(options.cmd != NULL) {
		fprintf(stderr,"error: multiple tests specified\n");
		stat = NC_EINVAL;
		goto done;
	    }
	    options.cmd = strdup(optarg);
	    break;
	case 'o':
	    if(file != NULL) {
		fprintf(stderr,"error: multiple output files specified\n");
		stat = NC_EINVAL;
		goto done;
	    }
	    file = strdup(optarg);
	    break;
	case '?':
	   fprintf(stderr,"unknown option\n");
	   stat = NC_EINVAL;
	   goto done;
	}
    }
    if(options.cmd == NULL) {
	fprintf(stderr,"no command specified\n");
	stat = NC_EINVAL;
	goto done;
    }

    if(file == NULL)
	file = strdup(FILE);
    makeurl(file);
#ifdef DEBUG
    fprintf(stderr,"test: %s url=|%s|\n",options.cmd,url);
    fflush(stderr);
#endif

    /* Find the corresponding test for command */
    for(t=tests;t->cmd;t++) {
	if(strcmp(options.cmd,t->cmd)==0) {test = t; break;}
    }
    if(test == NULL) {
	fprintf(stderr,"unknown command: %s\n",optarg);
	stat = NC_EINVAL;
	goto done;
    }

    /* Execute */
    test->test();

done:
    if(stat)
	nc_strerror(stat);
    return (stat ? 1 : 0);    
}

/* Create test netcdf4 file via netcdf.h API*/
static int
testcreate(void)
{
    int stat = NC_NOERR;
    int ncid;

    unlink(FILE);

    if((stat = nc_create(url, 0, &ncid)))
	goto done;

    if((stat = nc_close(ncid)))
	goto done;

done:
    return THROW(stat);
}

/* Create file and add a dimension */
static int
testdim1(void)
{
    int stat = NC_NOERR;
    int ncid, dimid;

    unlink(FILE);

    if((stat = nc_create(url, 0, &ncid)))
	goto done;

    if((stat = nc_def_dim(ncid, "dim1", (size_t)1, &dimid)))
	goto done;

    if((stat = nc_close(ncid)))
	goto done;

done:
    return THROW(stat);
}

/* Create file and add a variable */
static int
testvar1(void)
{
    int stat = NC_NOERR;
    int ncid, varid;

    unlink(FILE);

    if((stat = nc_create(url, 0, &ncid)))
	goto done;

    if((stat = nc_def_var(ncid, VAR1, NC_INT, 0, NULL, &varid)))
	goto done;

    if((stat = nc_close(ncid)))
	goto done;

done:
    return THROW(stat);
}

/* Create file and add a rank 2 variable */
static int
testvar2(void)
{
    int stat = NC_NOERR;
    int ncid, varid1, varid2;
    int dimids[2];
    int ndims;
    int fillval;

    unlink(FILE);

    if((stat = nc_create(url, 0, &ncid)))
	goto done;

    ndims = 0;

    if((stat = nc_def_dim(ncid, "dim2", 2, &dimids[0])))
	goto done;
    ndims++;

    if((stat = nc_def_dim(ncid, "dim3", 3, &dimids[1])))
	goto done;
    ndims++;

    if((stat = nc_def_var(ncid, VAR1, NC_INT, ndims, dimids, &varid1)))
	goto done;

    /* make VAR1 no-fill */
    if((stat = nc_def_var_fill(ncid,varid1,NC_NOFILL,NULL)))
	goto done;

    /* make VAR1 contiguous */
    if((stat = nc_def_var_chunking(ncid,varid1,NC_CONTIGUOUS,NULL)))
	goto done;

    /* Define var2 as chunked, but use default chunking */
    if((stat = nc_def_var(ncid, VAR2, NC_INT, ndims, dimids, &varid2)))
	goto done;

    if((stat = nc_def_var_chunking(ncid,varid2,NC_CHUNKED,NULL)))
	goto done;

    /* make VAR1 use non-default fill */
    fillval = 137;
    if((stat = nc_def_var_fill(ncid,varid2,NC_FILL,&fillval)))
	goto done;

    if((stat = nc_close(ncid)))
	goto done;

done:
    return THROW(stat);
}

/* Create file and add global attributes of all supported types */
static int
testattr1(void)
{
    int stat = NC_NOERR;
    int ncid;
    AtomicsW atoms = {
	{-17},
	{-17},
	{-17},
	{17.0f},
	{17.0},
	{17u},
	{17u},
	{17u},
	{17ll},
	{17ull},
	{'7'}};

    unlink(FILE);

    if((stat = nc_create(url, 0, &ncid)))
	goto done;

    if((stat = nc_put_att(ncid,NC_GLOBAL,"byte_a",NC_BYTE,1,atoms.bytev))) goto done;
    if((stat = nc_put_att(ncid,NC_GLOBAL,"short_a",NC_SHORT,1,atoms.shortv))) goto done;
    if((stat = nc_put_att(ncid,NC_GLOBAL,"int_a",NC_INT,1,atoms.intv))) goto done;
    if((stat = nc_put_att(ncid,NC_GLOBAL,"float_a",NC_FLOAT,1,atoms.floatv))) goto done;
    if((stat = nc_put_att(ncid,NC_GLOBAL,"double_a",NC_DOUBLE,1,atoms.doublev))) goto done;
    if((stat = nc_put_att(ncid,NC_GLOBAL,"ubyte_a",NC_UBYTE,1,atoms.ubytev))) goto done;
    if((stat = nc_put_att(ncid,NC_GLOBAL,"ushort_a",NC_USHORT,1,atoms.ushortv))) goto done;
    if((stat = nc_put_att(ncid,NC_GLOBAL,"uint_a",NC_UINT,1,atoms.uintv))) goto done;
    if((stat = nc_put_att(ncid,NC_GLOBAL,"int64_a",NC_INT64,1,atoms.int64v))) goto done;
    if((stat = nc_put_att(ncid,NC_GLOBAL,"uint64_a",NC_UINT64,1,atoms.uint64v))) goto done;
    if((stat = nc_put_att(ncid,NC_GLOBAL,"char_a",NC_CHAR,1,atoms.charv))) goto done;

    if((stat = nc_close(ncid)))
	goto done;

done:
    return THROW(stat);
}

/* Create file and a var and add some attributes to the var */
static int
testattr2(void)
{
    int stat = NC_NOERR;
    int ncid, varid1;

    int intv[1] = {-17};
    float floatv[1] = {17.0f};
    long long int64v[1] = {17ll};
    char charv[1] = {'7'};

    unlink(FILE);

    if((stat = nc_create(url, 0, &ncid)))
	goto done;

    if((stat = nc_def_var(ncid, VAR1, NC_INT, 0, NULL, &varid1)))
	goto done;

    if((stat = nc_put_att(ncid,varid1,"int_a",NC_INT,1,intv))) goto done;
    if((stat = nc_put_att(ncid,varid1,"float_a",NC_FLOAT,1,floatv))) goto done;
    if((stat = nc_put_att(ncid,varid1,"int64_a",NC_INT64,1,int64v))) goto done;
    if((stat = nc_put_att(ncid,varid1,"char_a",NC_CHAR,1,charv))) goto done;

    if((stat = nc_close(ncid)))
	goto done;

done:
    return THROW(stat);
}

static int
fv2string(nc_type xtype, size_t xtypesize, void* fill_value, char** fillstringp)
{
    char sval[1024];
    switch (xtype) {
    case NC_BYTE:
	snprintf(sval,sizeof(sval),"%d",*((char*)fill_value));
	break;
    case NC_CHAR:
	snprintf(sval,sizeof(sval),"'%c'",*((char*)fill_value));
	break;
    case NC_SHORT:
	snprintf(sval,sizeof(sval),"%d",*((short*)fill_value));
	break;
    case NC_INT:
	snprintf(sval,sizeof(sval),"%d",*((int*)fill_value));
	break;
    case NC_FLOAT:
	snprintf(sval,sizeof(sval),"%g",*((float*)fill_value));
	break;
    case NC_DOUBLE:
	snprintf(sval,sizeof(sval),"%lg",*((double*)fill_value));
	break;
    case NC_UBYTE:
	snprintf(sval,sizeof(sval),"%u",*((unsigned char*)fill_value));
	break;
    case NC_USHORT:
	snprintf(sval,sizeof(sval),"%u",*((unsigned short*)fill_value));
	break;
    case NC_UINT:
	snprintf(sval,sizeof(sval),"%u",*((unsigned int*)fill_value));
	break;
    case NC_INT64:
	snprintf(sval,sizeof(sval),"%lld",*((long long*)fill_value));
	break;
    case NC_UINT64:
	snprintf(sval,sizeof(sval),"%llu",*((unsigned long long*)fill_value));
	break;
    default:
	strlcat(sval,"<unknown>",sizeof(sval));
	break;
    }
    if(fillstringp) *fillstringp = strdup(sval);
    return NC_NOERR;
}

static int
reportvar(int ncid, int varid)
{
    int i,stat = NC_NOERR;
    char varname[NC_MAX_NAME];
    nc_type xtype;
    int ndims, natts;
    char typename[NC_MAX_NAME];
    size_t typesize;
    int dimids[NC_MAX_VAR_DIMS];
    char* dimname[NC_MAX_VAR_DIMS];
    size_t dimsize[NC_MAX_VAR_DIMS];
    int no_fill;
    void* fill_value = NULL;
    int shuffle, deflate, deflate_level;
    int fletcher32;
    int endian;
    unsigned int* filterids
    size_t nfilters;
    int storage;
    size_t chunksizes[NC_MAX_VAR_DIMS];
    const char* tmp = NULL;
    char* fillstring = NULL;

    if((stat = nc_inq_var(ncid,varid,varname,&xtype,&ndims,dimids,&natts)))
	goto done;
    /* get basetype name and size*/
    if((stat = nc_inq_type(ncid, xtype, typename, &typesize))) goto done;
    /* Get dimension sizes */
    for(i=0;i<ndims;i++) {
	char name[NC_MAX_NAME];
	if((stat = nc_inq_dim(ncid, dimids[i], name, &dimsize[i])))
	    goto done;
	dimname[i] = strdup(name);
    }
    if((stat = nc_inq_var_endian(ncid, varid, &endian)))
	goto done;
    if((stat = nc_inq_var_deflate(ncid, varid, &shuffle, &deflate, &deflate_level)))
	goto done;
    if((stat = nc_inq_var_fletcher32(ncid, varid, &fletcher32)))
	goto done;
    if((stat = nc_inq_var_filters(ncid, varid, &nfilters, NULL)))
	    goto done;
    filterids = NULL;
    if(nfilters > 0)
	filterids = malloc(sizeof(unsigned int)*nfilters);
    if((stat = nc_inq_var_filters(ncid, varid, &nfilters, filterids)))
	goto done;
    /* Allocate space for fill_value */
    if((fill_value = calloc(1,typesize)) == NULL)
	{stat = NC_ENOMEM; goto done;}
    if((stat = nc_inq_var_fill(ncid, varid, &no_fill, fill_value)))
	goto done;
    if((stat = nc_inq_var_chunking(ncid, varid, &storage, chunksizes)))
	goto done;

    /* Now dump everything */
    printf("%s %s",typename, varname);
    printf("(");
    for(i=0;i<ndims;i++) {
	if(i > 0) printf(",");
	printf("%s=%lu",dimname[i],(unsigned long)dimsize[i]);
    }
    printf(");\n");

    switch (endian) {
    case NC_ENDIAN_NATIVE: tmp = "native"; break;
    case NC_ENDIAN_LITTLE: tmp = "little"; break;
    case NC_ENDIAN_BIG: tmp = "big"; break;
    default: tmp = "unknown"; break;
    }
    printf("\t_Endianness=%s\n",tmp);

    printf("\t_Fletcher32=%s\n",(fletcher32?"true":"false"));

    printf("\t_Shuffle=%s\n",(shuffle?"true":"false"));
    if(deflate)	printf("\t_DeflateLevel=%d\n",deflate_level);

    if(filterids != NULL) {
	printf("\t_Filters=(");
	for(i=0;i<nfilters;i++) {
	    if(i>0) printf(",");
	    printf("%u",filterids[i]);
	}
	printf(")\n");	
    }

    if((stat=fv2string(xtype,typesize,fill_value,&fillstring))) goto done;
    printf("\t_FillValue=%s\n",fillstring);

    if(storage == NC_CONTIGUOUS) {
        printf("\t_Storage=contiguous\n");
    } else { /* chunked */
        printf("\t_Storage=chunked\n");
        printf("\t_ChunkSizes=");
	for(i=0;i<ndims;i++) {
	    if(i>0) printf(",");
	    printf("%lu",(unsigned long)chunksizes[i]);
	}
	printf("\n");	
    }

done:
    return stat;
}

/* Report variables from testvar2 */
static int
testread1(void)
{
    int stat = NC_NOERR;
    int ncid, varid1, varid2;
    int ndims,nvars,natts,unlimdimid;

    unlink(FILE);

#ifdef SETUP
    NCCHECK(testvar2());
#endif

    if((stat = nc_open(url, 0, &ncid)))
	goto done;

    /* Report file level info */
    if((stat = nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid)))
	goto done;
    printf("file info: ndims=%d nvars=%d natts=%d unlimdimid=%d\n",
	    ndims,nvars,natts,unlimdimid);

    if((stat = nc_inq_varid(ncid, VAR1, &varid1))) goto done;

    /* Report info about the var */
    if((stat = reportvar(ncid,varid1))) goto done;

    if((stat = nc_inq_varid(ncid, VAR2, &varid2))) goto done;

    /* Report info about the var */
    if((stat = reportvar(ncid,varid2))) goto done;

    if((stat = nc_close(ncid)))
	goto done;

done:
    return THROW(stat);
}

static int
reportatt(int ncid, int varid, int natts, int index)
{
    int i,stat = NC_NOERR;
    void* data = NULL;
    char aname[NC_MAX_NAME];
    nc_type atype;
    size_t alen;
    char typename[NC_MAX_NAME];
    size_t typesize;
    char* p = NULL; /* char* so we can do arithmetic */

    /* Get some meta info about the attribute */
    if((stat = nc_inq_attname(ncid,varid,index,aname)))
	goto done;
    if((stat = nc_inq_att(ncid, varid, aname, &atype, &alen))) goto done;

    /* Get type info about atype */
    if((stat = nc_inq_type(ncid, atype, typename, &typesize))) goto done;

    /* Allocate space to read the attribute */
    if((data = malloc(typesize * alen)) == NULL)
	{stat = NC_ENOMEM; goto done;}

    /* Read the attribute */
    if((stat = nc_get_att(ncid,varid,aname,data))) goto done;

    /* Now report */
    printf("\t[%d] %s:%s(%lu) = ",index,typename,aname,(unsigned long)alen);
    for(p=(char*)data,i=0;i<alen;i++,p+=typesize) {
	AtomicsR* atom = (AtomicsR*)p;
	printf("%s",(i==0?"{":" "));
	switch (atype) {
	case NC_BYTE: printf("%db",atom->bytev[0]); break;
	case NC_SHORT: printf("%ds",atom->shortv[0]); break;
	case NC_INT: printf("%d",atom->intv[0]); break;
	case NC_FLOAT: printf("%gf",atom->floatv[0]); break;
	case NC_DOUBLE: printf("%lg",atom->doublev[0]); break;
	case NC_UBYTE: printf("%uub",atom->ubytev[0]); break;
	case NC_USHORT: printf("%uus",atom->ushortv[0]); break;
	case NC_UINT: printf("%uu",atom->intv[0]); break;
	case NC_INT64: printf("%lldll",atom->int64v[0]); break;
	case NC_UINT64: printf("%lluull",atom->uint64v[0]); break;
	case NC_CHAR: printf("'%c'",atom->charv[0]); break;
	}
    }
    printf("}\n");

done:
    return stat;
}

/* Print out selected reserved attributes by name */
static int
reportreserved(int ncid)
{
    int stat = NC_NOERR;
    char* ncpropdata = NULL;
    size_t len;
    int superblockversion;
    int isnetcdf4;
    nc_type xtype;

    /* Get and print _NCProperties */
    if((stat = nc_inq_attlen(ncid, NC_GLOBAL, "_NCProperties", &len)))
	goto done;
    if((ncpropdata = calloc(1,len+1)) == NULL) {stat = NC_ENOMEM; goto done;}
    if((stat = nc_get_att(ncid,NC_GLOBAL, "_NCProperties", ncpropdata)))
	goto done;
    ncpropdata[len] = '\0'; /* nul term */
    printf("\t%s:%s(%lu) = |%s|\n", "char","_NCProperties", (unsigned long)len, ncpropdata);

    /* Get and print _SuperblockVersion */
    if((stat = nc_inq_att(ncid, NC_GLOBAL, "_SuperblockVersion", &xtype,&len)))
	goto done;
    if(len != 1 || xtype != NC_INT) {stat = NC_ENCZARR; goto done;}
    if((stat = nc_get_att_int(ncid,NC_GLOBAL, "_SuperblockVersion", &superblockversion)))
	goto done;
    printf("\t%s:%s(%lu) = %d\n", "int","_SuperblockVersion", (unsigned long)len, superblockversion);

    /* Get and print _Isnetcdf4 */
    if((stat = nc_inq_att(ncid, NC_GLOBAL, "_IsNetcdf4", &xtype,&len)))
	goto done;
    if(len != 1 || xtype != NC_INT) {stat = NC_ENCZARR; goto done;}
    if((stat = nc_get_att_int(ncid,NC_GLOBAL, "_IsNetcdf4", &isnetcdf4)))
	goto done;
    printf("\t%s:%s(%lu) = %d\n", "int","_IsNetcdf4", (unsigned long)len, isnetcdf4);

done:
    nullfree(ncpropdata);
    return stat;
}

/* Read the global attributes in testattr1 */
static int
testread2(void)
{
    int i,stat = NC_NOERR;
    int ncid;
    int natts;

    unlink(FILE);

#ifdef SETUP
    NCCHECK(testattr1());
#endif

    if((stat = nc_open(url, 0, &ncid)))
	goto done;

    printf("Reserved Attributes:\n");
    if((stat = reportreserved(ncid))) goto done;
    fflush(stdout);

    printf("\nGlobal Attributes:\n");
    /* Get and print the global attributes */
    if((stat = nc_inq(ncid, NULL, NULL, &natts, NULL))) goto done;

    for(i=0;i<natts;i++) {
        if((stat = reportatt(ncid,NC_GLOBAL,natts,i))) goto done;
    }
    fflush(stdout);

    if((stat = nc_close(ncid)))
	goto done;

done:
    return THROW(stat);
}

/* Read the variable attributes in testattr2 */
static int
testread3(void)
{
    int i,stat = NC_NOERR;
    int ncid, varid1;
    int natts;

    unlink(FILE);

#ifdef SETUP
    NCCHECK(testattr2());
#endif

    if((stat = nc_open(url, 0, &ncid)))
	goto done;

    /* Get and print the variable attributes */
    printf("\nVariable Attributes:\n");
    if((stat = nc_inq_varid(ncid, VAR1, &varid1))) goto done;
    if((stat = nc_inq_var(ncid,varid1,NULL,NULL,NULL,NULL,&natts)))
	goto done;
    for(i=0;i<natts;i++) {
        if((stat = reportatt(ncid,varid1,natts,i))) goto done;
    }
    fflush(stdout);

    if((stat = nc_close(ncid)))
	goto done;

done:
    return THROW(stat);
}

