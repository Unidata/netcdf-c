/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#include "zincludes.h"
#include <math.h>
#ifdef _MSC_VER
#include <crtdbg.h>
#endif

#include "isnan.h"

/*
Code taken directly from libdap4/d4cvt.c
*/

/* Intermediate results */
struct ZCVT {
    signed long long int64v;
    unsigned long long uint64v;
    double float64v;
};

/* Forward */
static int typeid2jtype(nc_type typeid);

/* Convert an NCJ_STRING to a memory equivalent value of specified dsttype */
int
NCZ_convert1(NCjson* jsrc, nc_type dsttype, unsigned char* memory)
{
    int stat = NC_NOERR;
    nc_type srctype;
    struct ZCVT zcvt;
    int outofrange = 0;

    /* Convert the incoming jsrc string to a restricted set of values */
    switch (NCJsort(jsrc)) {
    case NCJ_INT: /* convert to (u)int64 */
	if(NCJstring(jsrc)[0] == '-') {
	    if(sscanf(NCJstring(jsrc),"%lld",&zcvt.int64v) != 1)
		{stat = NC_EINVAL; goto done;}
	    srctype = NC_INT64;
	} else {
	    if(sscanf(NCJstring(jsrc),"%llu",&zcvt.uint64v) != 1)
		{stat = NC_EINVAL; goto done;}
	    srctype = NC_UINT64;
	}
	break;
    case NCJ_STRING:
    case NCJ_DOUBLE:
	/* Capture nan and infinity values */
	if(strcasecmp(NCJstring(jsrc),"nan")==0)
	    zcvt.float64v = NAN;
	else if(strcasecmp(NCJstring(jsrc),"-nan")==0)
	    zcvt.float64v = - NAN;
	else if(strcasecmp(NCJstring(jsrc),"infinity")==0)
	    zcvt.float64v = INFINITY;
	else if(strcasecmp(NCJstring(jsrc),"-infinity")==0)
	    zcvt.float64v = (- INFINITY);
	else {
	    if(sscanf(NCJstring(jsrc),"%lg",&zcvt.float64v) != 1)
	        {stat = NC_EINVAL; goto done;}
	}
	srctype = NC_DOUBLE;
	break;
    case NCJ_BOOLEAN:
	srctype = NC_UINT64;
	if(strcasecmp(NCJstring(jsrc),"false")==0)
	    zcvt.uint64v = 0;
	else
	    zcvt.uint64v = 1;
	break;
    default: stat = NC_EINTERNAL; goto done;
    }

    /* Now, do the down conversion */
    switch (dsttype) {
    case NC_BYTE: {
        signed char* p = (signed char*)memory;
	switch (srctype) {
	case NC_DOUBLE:
	    zcvt.int64v = (long long)zcvt.float64v; /* Convert to int64 */
	    /* fall thru */
	case NC_INT64:
	    if(zcvt.int64v < NC_MIN_BYTE || zcvt.int64v > NC_MAX_BYTE) outofrange = 1;
	    *p = (signed char)zcvt.int64v;
	    break;
	case NC_UINT64:
	    if(zcvt.uint64v > NC_MAX_BYTE) outofrange = 1;
	    *p = (signed char)zcvt.uint64v;
	    break;
	}
	} break;
    case NC_UBYTE: {
	unsigned char* p = (unsigned char*)memory;
	switch (srctype) {
	case NC_DOUBLE:
	    zcvt.int64v = (long long)zcvt.float64v; /* Convert to int64 */
	    /* fall thru */
	case NC_INT64:
	    if(zcvt.int64v < 0 || zcvt.int64v > NC_MAX_BYTE) outofrange = 1;
	    *p = (unsigned char)zcvt.int64v;
	    break;
	case NC_UINT64:
	    if(zcvt.uint64v > NC_MAX_UBYTE) outofrange = 1;
	    *p = (unsigned char)zcvt.uint64v;
	    break;
	}
	} break;
    case NC_SHORT: {
	signed short* p = (signed short*)memory;
	switch (srctype) {
	case NC_DOUBLE:
	    zcvt.int64v = (long long)zcvt.float64v; /* Convert to int64 */
	    /* fall thru */
	case NC_INT64:
	    if(zcvt.int64v < NC_MIN_SHORT || zcvt.int64v > NC_MAX_SHORT) outofrange = 1;
	    *p = (signed short)zcvt.int64v;
	    break;
	case NC_UINT64:
	    if(zcvt.uint64v > NC_MAX_SHORT) outofrange = 1;
	    *p = (signed short)zcvt.uint64v;
	    break;
	}
	} break;
    case NC_USHORT: {
	unsigned short* p = (unsigned short*)memory;
	switch (srctype) {
	case NC_DOUBLE:
	    zcvt.int64v = (long long)zcvt.float64v; /* Convert to int64 */
	    /* fall thru */
	case NC_INT64:
	    if(zcvt.int64v < 0 || zcvt.int64v > NC_MAX_USHORT) outofrange = 1;
	    *p = (unsigned short)zcvt.int64v;
	    break;
	case NC_UINT64:
	    if(zcvt.uint64v > NC_MAX_USHORT) outofrange = 1;
	    *p = (unsigned short)zcvt.uint64v;
	    break;
	}
	} break;
    case NC_INT: {
	signed int* p = (signed int*)memory;
	switch (srctype) {
	case NC_DOUBLE:
	    zcvt.int64v = (long long)zcvt.float64v; /* Convert to int64 */
	    /* fall thru */
	case NC_INT64:
	    if(zcvt.int64v < NC_MIN_INT || zcvt.int64v > NC_MAX_INT) outofrange = 1;
	    *p = (signed int)zcvt.int64v;
	    break;
	case NC_UINT64:
	    if(zcvt.uint64v > NC_MAX_INT) outofrange = 1;
	    *p = (signed int)zcvt.uint64v;
	    break;
	}
	} break;
    case NC_UINT: {
	unsigned int* p = (unsigned int*)memory;
	switch (srctype) {
	case NC_DOUBLE:
	    zcvt.int64v = (long long)zcvt.float64v; /* Convert to int64 */
	    /* fall thru */
	case NC_INT64:
	    if(zcvt.int64v < 0 || zcvt.int64v > NC_MAX_UINT) outofrange = 1;
	    *p = (unsigned int)zcvt.int64v;
	    break;
	case NC_UINT64:
	    if(zcvt.uint64v > NC_MAX_UINT) outofrange = 1;
	    *p = (unsigned int)zcvt.uint64v;
	    break;
	}
	} break;
    case NC_INT64: {
	signed long long* p = (signed long long*)memory;
	switch (srctype) {
	case NC_DOUBLE:
	    zcvt.int64v = (long long)zcvt.float64v; /* Convert to int64 */
	    /* fall thru */
	case NC_INT64:
	    *p = (signed long long)zcvt.int64v;
	    break;
	case NC_UINT64:
	    if(zcvt.uint64v > NC_MAX_INT64) outofrange = 1;
	    *p = (signed long long)zcvt.uint64v;
	    break;
	}
	} break;
    case NC_UINT64: {
	unsigned long long* p = (unsigned long long*)memory;
	switch (srctype) {
	case NC_DOUBLE:
	    zcvt.int64v = (signed long long)zcvt.float64v;
	    /* fall thru */
	case NC_INT64:
	    if(zcvt.int64v < 0) outofrange = 1;
	    *p = (unsigned long long)zcvt.int64v;
	    break;
	case NC_UINT64:
	    *p = (unsigned long long)zcvt.uint64v;
	    break;
	}
	} break;
    case NC_FLOAT: {
	float* p = (float*)memory;
	switch (srctype) {
	case NC_DOUBLE:
	    *p = (float)zcvt.float64v;
	    break;
	case NC_INT64:
	    *p = (float)zcvt.int64v;
	    break;
	case NC_UINT64:
	    *p = (float)zcvt.uint64v;
	    break;
	}
	} break;
    case NC_DOUBLE: {
	double* p = (double*)memory;
	switch (srctype) {
	case NC_DOUBLE:
	    *p = (double)zcvt.float64v;
	    break;
	case NC_INT64:
	    *p = (double)zcvt.int64v;
	    break;
	case NC_UINT64:
	    *p = (double)zcvt.uint64v;
	    break;
	}
	} break;
    default: stat = NC_EINTERNAL; goto done;
    }

done:
    if(stat == NC_NOERR && outofrange) stat = NC_ERANGE;
    return stat;
}

/* Convert a memory value to a JSON string value */
int
NCZ_stringconvert1(nc_type srctype, size_t len, char* src, NCjson* jvalue)
{
    int stat = NC_NOERR;
    struct ZCVT zcvt;
    nc_type dsttype = NC_NAT;
    char s[1024];

    assert(srctype >= NC_NAT && srctype != NC_CHAR && srctype < NC_STRING);    
    /* Convert to a restricted set of values */
    switch (srctype) {
    case NC_BYTE: {
	zcvt.int64v = (signed long long)(*((signed char*)src));
	dsttype = NC_INT64;
	} break;
    case NC_UBYTE: {
	zcvt.uint64v = (unsigned long long)(*((unsigned char*)src));
	dsttype = NC_UINT64;
	} break;
    case NC_SHORT: {
	zcvt.int64v = (signed long long)(*((signed short*)src));
	dsttype = NC_INT64;
	} break;
    case NC_USHORT: {
	zcvt.uint64v = (unsigned long long)(*((unsigned short*)src));
	dsttype = NC_UINT64;
	} break;
    case NC_INT: {
	zcvt.int64v = (signed long long)(*((signed int*)src));
	dsttype = NC_INT64;
	} break;
    case NC_UINT: {
	zcvt.uint64v = (unsigned long long)(*((unsigned int*)src));
	dsttype = NC_UINT64;
	} break;
    case NC_INT64: {
	zcvt.int64v = (signed long long)(*((signed long long*)src));
	dsttype = NC_INT64;
	} break;
    case NC_UINT64: {
	zcvt.uint64v = (unsigned long long)(*((unsigned long long*)src));
	dsttype = NC_UINT64;
	} break;
    case NC_FLOAT: {
	zcvt.float64v = (double)(*((float*)src));
	dsttype = NC_DOUBLE;
	} break;
    case NC_DOUBLE: {
	dsttype = NC_DOUBLE;
	zcvt.float64v= (double)(*((double*)src));
	} break;
    default: stat = NC_EINTERNAL; goto done;
    }

    /* Convert from restricted set of values to standardized string form*/
    switch (dsttype) {
    case NC_INT64: {
	snprintf(s,sizeof(s),"%lld",zcvt.int64v);
	} break;
    case NC_UINT64: {
	snprintf(s,sizeof(s),"%llu",zcvt.uint64v);
	} break;
    case NC_DOUBLE: {
#ifdef _WIN32
	switch (_fpclass(zcvt.float64v)) {
	case _FPCLASS_SNAN: case _FPCLASS_QNAN:
	     strcpy(s,"Nan"); break;
	case _FPCLASS_NINF:
	     strcpy(s,"-Infinity"); break;
	case _FPCLASS_PINF:
	     strcpy(s,"Infinity"); break;
	default:
	      snprintf(s,sizeof(s),"%lg",zcvt.float64v); /* handles NAN? */
	      break;
	}
#else
	if(isnan(zcvt.float64v))
	     strcpy(s,"NaN");
	else if(isinf(zcvt.float64v) && zcvt.float64v < 0)
	     strcpy(s,"-Infinity");
	else if(isinf(zcvt.float64v) && zcvt.float64v > 0)
	     strcpy(s,"Infinity");
	else {
             snprintf(s,sizeof(s),"%lg",zcvt.float64v); /* handles NAN? */
	}
#endif
	} break;
    default: stat = NC_EINTERNAL; goto done;
    }
    NCJsetstring(jvalue,strdup(s));
done:
    return stat;
}

int
NCZ_stringconvert(nc_type typeid, size_t len, void* data0, NCjson** jdatap)
{
    int stat = NC_NOERR;
    int i;
    char* src = data0; /* so we can do arithmetic on it */
    size_t typelen;
    char* str = NULL;
    NCjson* jvalue = NULL;
    NCjson* jdata = NULL;
    int jtype = NCJ_UNDEF;

    jtype = typeid2jtype(typeid);

    if((stat = NC4_inq_atomic_type(typeid, NULL, &typelen)))
	goto done;

    /* Handle char type specially */
    if(typeid == NC_CHAR) {
	/* Create a string valued json object */
	if((stat = NCJnewstringn(NCJ_STRING,len,src,&jdata)))
	    goto done;
    } else if(len == 1) { /* create singleton */
	if((stat = NCJnew(jtype,&jdata))) goto done;
        if((stat = NCZ_stringconvert1(typeid, len, src, jdata))) goto done;
    } else { /* len > 1 create array of values */
	if((stat = NCJnew(NCJ_ARRAY,&jdata))) goto done;
	for(i=0;i<len;i++) {
	    if((stat = NCJnew(jtype,&jvalue))) goto done;
	    if((stat = NCZ_stringconvert1(typeid, len, src, jvalue))) goto done;
	    NCJappend(jdata,jvalue);
	    jvalue = NULL;
	    src += typelen;
	}
    }
    if(jdatap) {*jdatap = jdata; jdata = NULL;}

done:
    nullfree(str);
    NCJreclaim(jvalue);
    NCJreclaim(jdata);
    return stat;
}

static int
typeid2jtype(nc_type typeid)
{
    switch (typeid) {
    case NC_BYTE: case NC_SHORT: case NC_INT: case NC_INT64:
    case NC_UBYTE: case NC_USHORT: case NC_UINT: case NC_UINT64:
	return NCJ_INT;
    case NC_FLOAT:
    case NC_DOUBLE:
	return NCJ_DOUBLE;
    case NC_CHAR:
	return NCJ_STRING;
    default: break;
    }
    return NCJ_UNDEF;
}
