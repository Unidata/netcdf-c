/*
 *	Copyright 2018, University Corporation for Atmospheric Research
 *      See netcdf/COPYRIGHT file for copying and redistribution conditions.
 */

#include "ztest.h"

#define SETUP
#define DEBUG

/* Forward */
static int testbuild(void);
static int testparse(void);

static void dump(NCjson* json);
static void dumpR(NCjson* json, int depth);
static char* sortname(int sort);

struct Test tests[] = {
{"build", testbuild},
{"parse", testparse},
{NULL, NULL}
};

typedef struct NCJ {
    NCjson* ncj_string;
    NCjson* ncj_int;
    NCjson* ncj_double;
    NCjson* ncj_boolean;
    NCjson* ncj_null;
    NCjson* ncj_array1;
    NCjson* ncj_array2;
    NCjson* ncj_dict1;
    NCjson* ncj_dict2;
} NCJ;

int
main(int argc, char** argv)
{
    int stat = NC_NOERR;
    struct Test* test = NULL;

    if((stat = setup(argc,argv))) goto done;
    if((stat = findtest(options.cmd,tests,&test))) goto done;

    /* Execute */
    test->test();

done:
    if(stat)
	nc_strerror(stat);
    return (stat ? 1 : 0);    
}

/* Build a reasonably complex json structure */
static int
build(NCJ* ncj)
{
    int stat = NC_NOERR;

    memset(ncj,0,sizeof(NCJ));

    /* Build instances of primitives */
    if((stat = NCJnew(NCJ_STRING,&ncj->ncj_string))) goto done;
    ncj->ncj_string->value = strdup("string");
    if((stat = NCJnew(NCJ_INT,&ncj->ncj_int))) goto done;
    ncj->ncj_int->value = strdup("117");
    if((stat = NCJnew(NCJ_DOUBLE,&ncj->ncj_double))) goto done;
    ncj->ncj_double->value = strdup("3.1415926");
    if((stat = NCJnew(NCJ_BOOLEAN,&ncj->ncj_boolean))) goto done;
    ncj->ncj_boolean->value = strdup("true");
    if((stat = NCJnew(NCJ_NULL,&ncj->ncj_null))) goto done;

    /* Create an empty array */
    if((stat = NCJnew(NCJ_ARRAY,&ncj->ncj_array1))) goto done;

    /* Create a filled array */
    if((stat = NCJnew(NCJ_ARRAY,&ncj->ncj_array2))) goto done;
    if((stat = NCJappend(ncj->ncj_array2,ncj->ncj_string))) goto done;
    if((stat = NCJappend(ncj->ncj_array2,ncj->ncj_int))) goto done;
    if((stat = NCJappend(ncj->ncj_array2,ncj->ncj_double))) goto done;
    if((stat = NCJappend(ncj->ncj_array2,ncj->ncj_boolean))) goto done;
    if((stat = NCJappend(ncj->ncj_array2,ncj->ncj_null))) goto done;
    if((stat = NCJappend(ncj->ncj_array2,ncj->ncj_array1))) goto done;

    /* Create an empty dict */
    if((stat = NCJnew(NCJ_DICT,&ncj->ncj_dict1))) goto done;

    /* Create a filled dict */
    if((stat = NCJnew(NCJ_DICT,&ncj->ncj_dict2))) goto done;
    if((stat = NCJinsert(ncj->ncj_dict2,"string",ncj->ncj_string))) goto done;
    if((stat = NCJinsert(ncj->ncj_dict2,"int",ncj->ncj_int))) goto done;
    if((stat = NCJinsert(ncj->ncj_dict2,"double",ncj->ncj_double))) goto done;
    if((stat = NCJinsert(ncj->ncj_dict2,"boolean",ncj->ncj_boolean))) goto done;
    if((stat = NCJinsert(ncj->ncj_dict2,"null",ncj->ncj_null))) goto done;
    if((stat = NCJinsert(ncj->ncj_dict2,"array1",ncj->ncj_array1))) goto done;
    if((stat = NCJinsert(ncj->ncj_dict2,"array2",ncj->ncj_array2))) goto done;
    if((stat = NCJinsert(ncj->ncj_dict2,"dict1",ncj->ncj_dict1))) goto done;

done:
    return THROW(stat);
}

/* Create test netcdf4 file via netcdf.h API*/
static int
testbuild(void)
{
    int stat = NC_NOERR;
    NCJ ncj;

    /* Build */
    if((stat = build(&ncj))) goto done;

    /* Primitives */
    dump(ncj.ncj_string);
    dump(ncj.ncj_int);
    dump(ncj.ncj_double);
    dump(ncj.ncj_boolean);
    dump(ncj.ncj_null);

    /* Empty array */
    dump(ncj.ncj_array1);

    /* Filled array */
    dump(ncj.ncj_array2);

    /* Empty dict */
    dump(ncj.ncj_dict1);

    /* Filled dict */
    dump(ncj.ncj_dict2);

done:
    return THROW(stat);
}

/* Test the parser */
static int
testparse(void)
{

    int stat = NC_NOERR;
    NCJ ncj;
    char* text = NULL;
    char* result = NULL;
    NCjson* json = NULL;

    /* Build */
    if((stat = build(&ncj))) goto done;

    if((stat = NCJunparse(ncj.ncj_dict2,0,&text))) goto done;

    if((stat = NCJparse(text,0,&json))) goto done;

    if((stat = NCJunparse(json,0,&result))) goto done;

    printf("text  : |%s|\nresult: |%s|\n",text,result);

done:
    return stat;
}

static void
dump(NCjson* json)
{
    dumpR(json,0);
    fflush(stdout);
    fflush(stderr);
}

static void
dumpR(NCjson* json, int depth)
{
    int ok, count, i;
    long long int64v;
    double float64v;

    printf("/%s/ ",sortname(json->sort));
    switch(json->sort) {
    case NCJ_STRING: printf("\"%s\"",json->value); break;
    case NCJ_INT:
	ok = sscanf(json->value,"%lld%n",&int64v,&count);
	if(ok != 1 || count != strlen(json->value)) goto fail;
	printf("%lld",int64v);
	break;
    case NCJ_DOUBLE: 
	ok = sscanf(json->value,"%lg%n",&float64v,&count);
	if(ok != 1 || count != strlen(json->value)) goto fail;
	printf("%lg",float64v);
	break;
    case NCJ_BOOLEAN: 
	if(strcasecmp(json->value,"true") != 0
	   && strcasecmp(json->value,"false") != 0) goto fail;
	printf("%s",json->value);
	break;
    case NCJ_NULL: 
	printf("null");
	break;
    case NCJ_DICT: 
	if(nclistlength(json->dict) == 0) {
	    printf("{}");
	} else {
	    printf("\n");
	    for(i=0;i<nclistlength(json->dict);i+=2) {
		NCjson* j = NULL;
		j = (NCjson*)nclistget(json->dict,i);
		assert(j->sort == NCJ_STRING);
	        printf("{%d} ",depth+1);
	        printf("\"%s\" => ",j->value);
		if(i+1 >= nclistlength(json->dict)) {/* malformed */
		    printf("<malformed>");
		} else
	            dumpR((NCjson*)nclistget(json->dict,i+1),depth+1);
	    }
	}
	break;
    case NCJ_ARRAY: 
	if(nclistlength(json->array) == 0) {
	    printf("[]");
	} else {
	    printf("\n");
	    for(i=0;i<nclistlength(json->array);i++) {
	        printf("[%d] ",depth+1);
	        dumpR((NCjson*)nclistget(json->array,i),depth+1);
	    }
	}
	break;
    default: printf("unknown"); break;
    }
    printf("\n");
    return;
fail:
    printf("????\n");
}

static char*
sortname(int sort)
{
    switch (sort) {
    case NCJ_STRING: return "String";
    case NCJ_INT: return "Int";
    case NCJ_DOUBLE: return "Double";
    case NCJ_BOOLEAN: return "Boolean";
    case NCJ_NULL: return "Null";
    case NCJ_DICT: return "Dict";
    case NCJ_ARRAY: return "Array";
    default: break;
    }
    return "Unknown";
}
