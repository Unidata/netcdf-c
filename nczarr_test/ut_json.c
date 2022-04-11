/*
 *	Copyright 2018, University Corporation for Atmospheric Research
 *      See netcdf/COPYRIGHT file for copying and redistribution conditions.
 */

#include "ut_includes.h"

#undef DEBUG

typedef enum Cmds {
    cmd_none = 0,
    cmd_build = 1,
    cmd_parse = 2,
} Cmds;

/* Forward */
static int testbuild(void);
static int testparse(void);

static void dump(NCjson* json);
static void dumpR(NCjson* json, int depth);
static char* sortname(int sort);

static int jclone(NCjson* json, NCjson** clonep);
static int cloneArray(NCjson* array, NCjson** clonep);
static int cloneDict(NCjson* dict, NCjson** clonep);

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

    if((stat = ut_init(argc, argv, &utoptions))) goto done;
    if((stat = runtests((const char**)utoptions.cmds,tests))) goto done;

done:
    ut_final();
    if(stat) usage(stat);
    return 0;
}

/* Build a reasonably complex json structure */
static int
build(NCJ* ncj)
{
    int stat = NC_NOERR;
    NCjson* clone = NULL;

    memset(ncj,0,sizeof(NCJ));

    /* Build instances of primitives */
    if((stat = NCJnew(NCJ_STRING,&ncj->ncj_string))) goto done;
    NCJsetstring(ncj->ncj_string, strdup("string"));
    if((stat = NCJnew(NCJ_INT,&ncj->ncj_int))) goto done;
    NCJsetstring(ncj->ncj_int, strdup("117"));
    if((stat = NCJnew(NCJ_DOUBLE,&ncj->ncj_double))) goto done;
    NCJsetstring(ncj->ncj_double, strdup("3.1415926"));
    if((stat = NCJnew(NCJ_BOOLEAN,&ncj->ncj_boolean))) goto done;
    NCJsetstring(ncj->ncj_boolean, strdup("true"));
    if((stat = NCJnew(NCJ_NULL,&ncj->ncj_null))) goto done;

    /* Create an empty array */
    if((stat = NCJnew(NCJ_ARRAY,&ncj->ncj_array1))) goto done;

    /* Create a filled array */
    if((stat = NCJnew(NCJ_ARRAY,&ncj->ncj_array2))) goto done;
    if((stat = jclone(ncj->ncj_string,&clone))) goto done;
    if((stat = NCJappend(ncj->ncj_array2,clone))) goto done;
    if((stat = jclone(ncj->ncj_int,&clone))) goto done;
    if((stat = NCJappend(ncj->ncj_array2,clone))) goto done;
    if((stat = jclone(ncj->ncj_double,&clone))) goto done;
    if((stat = NCJappend(ncj->ncj_array2,clone))) goto done;
    if((stat = jclone(ncj->ncj_boolean,&clone))) goto done;
    if((stat = NCJappend(ncj->ncj_array2,clone))) goto done;
    if((stat = jclone(ncj->ncj_null,&clone))) goto done;
    if((stat = NCJappend(ncj->ncj_array2,clone))) goto done;
    if((stat = jclone(ncj->ncj_array1,&clone))) goto done;
    if((stat = NCJappend(ncj->ncj_array2,clone))) goto done;

    /* Create an empty dict */
    if((stat = NCJnew(NCJ_DICT,&ncj->ncj_dict1))) goto done;

    /* Create a filled dict */
    if((stat = NCJnew(NCJ_DICT,&ncj->ncj_dict2))) goto done;
    if((stat = jclone(ncj->ncj_string,&clone))) goto done;
    if((stat = NCJinsert(ncj->ncj_dict2,"string",clone))) goto done;
    if((stat = jclone(ncj->ncj_int,&clone))) goto done;
    if((stat = NCJinsert(ncj->ncj_dict2,"int",clone))) goto done;
    if((stat = jclone(ncj->ncj_double,&clone))) goto done;
    if((stat = NCJinsert(ncj->ncj_dict2,"double",clone))) goto done;
    if((stat = jclone(ncj->ncj_boolean,&clone))) goto done;
    if((stat = NCJinsert(ncj->ncj_dict2,"boolean",clone))) goto done;
    if((stat = jclone(ncj->ncj_null,&clone))) goto done;
    if((stat = NCJinsert(ncj->ncj_dict2,"null",clone))) goto done;
    if((stat = jclone(ncj->ncj_array1,&clone))) goto done;
    if((stat = NCJinsert(ncj->ncj_dict2,"array1",clone))) goto done;
    if((stat = jclone(ncj->ncj_array2,&clone))) goto done;
    if((stat = NCJinsert(ncj->ncj_dict2,"array2",clone))) goto done;
    if((stat = jclone(ncj->ncj_dict1,&clone))) goto done;
    if((stat = NCJinsert(ncj->ncj_dict2,"dict1",clone))) goto done;

done:
    return THROW(stat);
}

static int
jclone(NCjson* json, NCjson** clonep)
{
    int stat = NC_NOERR;
    NCjson* clone = NULL;

    if(json == NULL) goto done;

    switch(json->sort) {
    case NCJ_INT:
    case NCJ_DOUBLE:
    case NCJ_BOOLEAN:
    case NCJ_STRING:
	if((stat=NCJnew(json->sort,&clone))) goto done;
	NCJsetstring(clone,strdup(NCJstring(json)));
        if(NCJstring(clone) == NULL)
	    {stat = NC_ENOMEM; goto done;}
	break;
    case NCJ_NULL:
	if((stat=NCJnew(json->sort,&clone))) goto done;
	break;
    case NCJ_DICT:
	if((stat=cloneDict(json,&clone))) goto done;
	break;
    case NCJ_ARRAY:
	if((stat=cloneArray(json,&clone))) goto done;
	break;
    default: break; /* nothing to clone */
    }
done:
    if(stat == NC_NOERR && clonep) {*clonep = clone; clone = NULL;}
    NCJreclaim(clone);    
    return stat;
}

static int
cloneArray(NCjson* array, NCjson** clonep)
{
    int i, stat=NC_NOERR;
    NCjson* clone = NULL;
    if((stat=NCJnew(NCJ_ARRAY,&clone))) goto done;
    for(i=0;i<NCJlength(array);i++) {
	NCjson* elem = NCJith(array,i);
	NCjson* elemclone = NULL;
	if((stat=jclone(elem,&elemclone))) goto done;
	if((stat=NCJappend(clone,elemclone))) goto done;
    }
done:
    if(stat == NC_NOERR && clonep) {*clonep = clone; clone = NULL;}
    NCJreclaim(clone);    
    return stat;
}

static int
cloneDict(NCjson* dict, NCjson** clonep)
{
    int stat = NC_NOERR;
    NCjson* clone = NULL;
    if((stat=cloneArray(dict,&clone))) goto done;
    /* Convert from array to dict */
    NCJsetsort(clone,NCJ_DICT);
done:
    if(stat == NC_NOERR && clonep) {*clonep = clone; clone = NULL;}
    NCJreclaim(clone);    
    return stat;
}

static void
clear(NCJ* ncj)
{
    NCJreclaim(ncj->ncj_array1);
    NCJreclaim(ncj->ncj_array2);
    NCJreclaim(ncj->ncj_dict1);
    NCJreclaim(ncj->ncj_dict2);
    NCJreclaim(ncj->ncj_string);
    NCJreclaim(ncj->ncj_int);
    NCJreclaim(ncj->ncj_double);
    NCJreclaim(ncj->ncj_boolean);
    NCJreclaim(ncj->ncj_null);
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
    clear(&ncj);
    return THROW(stat);
}

/* Test the parser */
static int
testparse(void)
{
    NCJ ncj;
    int stat = NC_NOERR;
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
    nullfree(text);
    nullfree(result);
    NCJreclaim(json);
    clear(&ncj);
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

    printf("/%s/ ",sortname(NCJsort(json)));
    switch(NCJsort(json)) {
    case NCJ_STRING: printf("\"%s\"",NCJstring(json)); break;
    case NCJ_INT:
	ok = sscanf(NCJstring(json),"%lld%n",&int64v,&count);
	if(ok != 1 || count != strlen(NCJstring(json))) goto fail;
	printf("%lld",int64v);
	break;
    case NCJ_DOUBLE: 
	ok = sscanf(NCJstring(json),"%lg%n",&float64v,&count);
	if(ok != 1 || count != strlen(NCJstring(json))) goto fail;
	printf("%lg",float64v);
	break;
    case NCJ_BOOLEAN: 
	if(strcasecmp(NCJstring(json),"true") != 0
	   && strcasecmp(NCJstring(json),"false") != 0) goto fail;
	printf("%s",NCJstring(json));
	break;
    case NCJ_NULL: 
	printf("null");
	break;
    case NCJ_DICT: 
	if(NCJlength(json) == 0) {
	    printf("{}");
	} else {
	    printf("\n");
	    for(i=0;i<NCJlength(json);i+=2) {
		NCjson* j = NULL;
		j = (NCjson*)NCJith(json,i);
		assert(NCJsort(j) == NCJ_STRING);
	        printf("{%d} ",depth+1);
	        printf("\"%s\" => ",NCJstring(j));
		if(i+1 >= NCJlength(json)) {/* malformed */
		    printf("<malformed>");
		} else
	            dumpR((NCjson*)NCJith(json,i+1),depth+1);
	    }
	}
	break;
    case NCJ_ARRAY: 
	if(NCJlength(json) == 0) {
	    printf("[]");
	} else {
	    printf("\n");
	    for(i=0;i<NCJlength(json);i++) {
	        printf("[%d] ",depth+1);
	        dumpR((NCjson*)NCJith(json,i),depth+1);
	    }
	}
	break;
    default: printf("unknown"); break;
    }
    printf("\n");
    return;
fail:
    printf("clone?\n");
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
