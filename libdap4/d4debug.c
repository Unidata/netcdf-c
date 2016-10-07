/*********************************************************************
 *   Copyright 2016, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/
#include "config.h"
#include <stdarg.h>
#include <stdio.h>

#include "nclog.h"
#include "ncd4.h"
#include "d4debug.h"

int ncdap4debug = 0;

#ifdef D4CATCH
/* Place breakpoint here to catch errors close to where they occur*/
int
d4breakpoint(int err) {return err;}

int
d4throw(int err)
{
    if(err == 0) return err;
    return d4breakpoint(err);
}
#endif

int
d4panic(const char* fmt, ...)
{
    va_list args;
    if(fmt != NULL) {
      va_start(args, fmt);
      vfprintf(stderr, fmt, args);
      fprintf(stderr, "\n" );
      va_end( args );
    } else {
      fprintf(stderr, "panic" );
    }
    fprintf(stderr, "\n" );
    fflush(stderr);
    return 0;
}

const char*
NCD4_sortname(NCD4sort sort)
{
    switch (sort) {
    case NCD4_NULL: return "NCD4_NULL";
    case NCD4_ATTR: return "NCD4_ATTR";
    case NCD4_ATTRSET: return "NCD4_ATTRSET";
    case NCD4_XML: return "NCD4_XML";
    case NCD4_DIM: return "NCD4_DIM";
    case NCD4_GROUP: return "NCD4_GROUP";
    case NCD4_TYPE: return "NCD4_TYPE";
    case NCD4_VAR: return "NCD4_VAR";
    case NCD4_ECONST: return "NCD4_ECONST";
    default: break;
    }
    return "unknown";
}

const char*
NCD4_subsortname(nc_type subsort)
{
    switch (subsort) {
    case NC_NAT: return "NC_NAT";
    case NC_BYTE: return "NC_BYTE";
    case NC_CHAR: return "NC_CHAR";
    case NC_SHORT: return "NC_SHORT";
    case NC_INT: return "NC_INT";
    case NC_FLOAT: return "NC_FLOAT";
    case NC_DOUBLE: return "NC_DOUBLE";
    case NC_UBYTE: return "NC_UBYTE";
    case NC_USHORT: return "NC_USHORT";
    case NC_UINT: return "NC_UINT";
    case NC_INT64: return "NC_INT64";
    case NC_UINT64: return "NC_UINT64";
    case NC_STRING: return "NC_STRING";
    case NC_VLEN: return "NC_VLEN";
    case NC_OPAQUE: return "NC_OPAQUE";
    case NC_ENUM: return "NC_ENUM";
    case NC_COMPOUND: return "NC_COMPOUND";
    default: break;
    }
    return "unknown";
}
