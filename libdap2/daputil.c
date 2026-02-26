/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#include "config.h"
#include <stddef.h>

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#include "oc.h"
extern int oc_dumpnode(OClink, OCddsnode);

#include "dapincludes.h"
#include "ncoffsets.h"

#define LBRACKET '['
#define RBRACKET ']'


static char* repairname(const char* name, const char* badchars);
static size_t nccpadding(unsigned long offset, size_t alignment);

/**************************************************/

/**
Convert a legal DAP name to an equivalent legal CDF name.
Currently the only transformation is to percent-encode '/' as %2F.
A leading '/' is stripped before encoding.
@param name the DAP name to convert (modified in place for leading slash)
@return a newly allocated string containing the CDF-legal name
*/
char*
cdflegalname(char* name)
{
    if(name != NULL && name[0] == '/')
	name = name+1; /* remove leading / so name will be legal */
    return repairname(name,"/");
}

/**
Convert a DAP variable type to the external netCDF variable type.
Follows the libnc-dap convention of direct type mapping rather than
value-preserving upgrades. For example, NC_UBYTE maps to NC_BYTE
rather than NC_SHORT.
@param drno the common DAP state (unused, reserved for future use)
@param nctype the input netCDF/DAP type
@return the corresponding external netCDF type, or NC_NAT if unrecognized
*/
nc_type
nctypeconvert(NCDAPCOMMON* drno, nc_type nctype)
{
    nc_type upgrade = NC_NAT;
	/* libnc-dap mimic invariant is to maintain type size */
	switch (nctype) {
	case NC_CHAR:    upgrade = NC_CHAR; break;
	case NC_BYTE:    upgrade = NC_BYTE; break;
	case NC_UBYTE:   upgrade = NC_BYTE; break;
	case NC_SHORT:   upgrade = NC_SHORT; break;
	case NC_USHORT:  upgrade = NC_SHORT; break;
	case NC_INT:     upgrade = NC_INT; break;
	case NC_UINT:    upgrade = NC_INT; break;
	case NC_FLOAT:   upgrade = NC_FLOAT; break;
	case NC_DOUBLE:  upgrade = NC_DOUBLE; break;
	case NC_URL:
	case NC_STRING:  upgrade = NC_CHAR; break;
	default: break;
	}
    return upgrade;
}

/**
Convert an OC type to the corresponding netCDF type.
@param etype the OC type to convert
@return the corresponding nc_type, or NC_NAT if unrecognized
*/
nc_type
octypetonc(OCtype etype)
{
    switch (etype) {
    case OC_Char:	return NC_CHAR;
    case OC_Byte:	return NC_UBYTE;
    case OC_UByte:	return NC_UBYTE;
    case OC_Int16:	return NC_SHORT;
    case OC_UInt16:	return NC_USHORT;
    case OC_Int32:	return NC_INT;
    case OC_UInt32:	return NC_UINT;
    case OC_Int64:	return NC_INT64;
    case OC_UInt64:	return NC_UINT64;
    case OC_Float32:	return NC_FLOAT;
    case OC_Float64:	return NC_DOUBLE;
    case OC_String:	return NC_STRING;
    case OC_URL:	return NC_STRING;
    case OC_Dataset:	return NC_Dataset;
    case OC_Sequence:	return NC_Sequence;
    case OC_Structure:	return NC_Structure;
    case OC_Grid:	return NC_Grid;
    case OC_Dimension:	return NC_Dimension;
    case OC_Atomic:	return NC_Atomic;
    default: break;
    }
    return NC_NAT;
}

/**
Convert a netCDF type to the corresponding OC/DAP type.
@param nctype the netCDF type to convert
@return the corresponding OCtype, or OC_NAT if unrecognized
*/
OCtype
nctypetodap(nc_type nctype)
{
    switch (nctype) {
    case NC_CHAR:	return OC_Char;
    case NC_BYTE:	return OC_Byte;
    case NC_UBYTE:	return OC_UByte;
    case NC_SHORT:	return OC_Int16;
    case NC_USHORT:	return OC_UInt16;
    case NC_INT:	return OC_Int32;
    case NC_UINT:	return OC_UInt32;
    case NC_INT64:	return OC_Int64;
    case NC_UINT64:	return OC_UInt64;
    case NC_FLOAT:	return OC_Float32;
    case NC_DOUBLE:	return OC_Float64;
    case NC_STRING:	return OC_String;
    default : break;
    }
    return OC_NAT;
}

/**
Return the size in bytes of a single element of the given netCDF type.
@param nctype the netCDF type
@return the size in bytes; calls PANIC for unrecognized types
*/
size_t
nctypesizeof(nc_type nctype)
{
    switch (nctype) {
    case NC_CHAR:	return sizeof(char);
    case NC_BYTE:	return sizeof(signed char);
    case NC_UBYTE:	return sizeof(unsigned char);
    case NC_SHORT:	return sizeof(short);
    case NC_USHORT:	return sizeof(unsigned short);
    case NC_INT:	return sizeof(int);
    case NC_UINT:	return sizeof(unsigned int);
    case NC_INT64:	return sizeof(long long);
    case NC_UINT64:	return sizeof(unsigned long long);
    case NC_FLOAT:	return sizeof(float);
    case NC_DOUBLE:	return sizeof(double);
    case NC_STRING:	return sizeof(char*);
    default: PANIC("nctypesizeof");
    }
    return 0;
}

/**
Return a string representation of a netCDF type constant.
@param nctype the netCDF type
@return a static string such as "NC_INT", or NULL if unrecognized
*/
char*
nctypetostring(nc_type nctype)
{
    switch (nctype) {
    case NC_NAT:	return "NC_NAT";
    case NC_BYTE:	return "NC_BYTE";
    case NC_CHAR:	return "NC_CHAR";
    case NC_SHORT:	return "NC_SHORT";
    case NC_INT:	return "NC_INT";
    case NC_FLOAT:	return "NC_FLOAT";
    case NC_DOUBLE:	return "NC_DOUBLE";
    case NC_UBYTE:	return "NC_UBYTE";
    case NC_USHORT:	return "NC_USHORT";
    case NC_UINT:	return "NC_UINT";
    case NC_INT64:	return "NC_INT64";
    case NC_UINT64:	return "NC_UINT64";
    case NC_STRING:	return "NC_STRING";
    case NC_VLEN:	return "NC_VLEN";
    case NC_OPAQUE:	return "NC_OPAQUE";
    case NC_ENUM:	return "NC_ENUM";
    case NC_COMPOUND:	return "NC_COMPOUND";
    case NC_URL:	return "NC_URL";
    case NC_SET:	return "NC_SET";
    case NC_Dataset:	return "NC_Dataset";
    case NC_Sequence:	return "NC_Sequence";
    case NC_Structure:	return "NC_Structure";
    case NC_Grid:	return "NC_Grid";
    case NC_Dimension:	return "NC_Dimension";
    case NC_Atomic:	return "NC_Atomic";
    default: break;
    }
    return NULL;
}

/**
Pad an NCbytes buffer to the specified alignment boundary.
Appends zero bytes as needed so that the buffer length is a
multiple of alignment.
@param buf the buffer to pad
@param alignment the required alignment in bytes
@return 1 on success, 0 if buf is NULL
*/
int
dapalignbuffer(NCbytes* buf, int alignment)
{
    unsigned long len;
    if(buf == NULL) return 0;
    len = ncbyteslength(buf);
    size_t pad = nccpadding(len, (size_t)alignment);

#ifdef TEST
    for(;pad > 0;pad--)
        ncbytesappend(buf,0x3a); /* 0x3a was chosen at random */
#else
    ncbytessetlength(buf,len+pad);
#endif
    return 1;
}

/**
Compute the product of all declared dimension sizes in a dimension list.
@param dimensions NClist of CDFnode dimension nodes
@return the product of all declsize values, or 1 if the list is NULL or empty
*/
size_t
dapdimproduct(NClist* dimensions)
{
    size_t size = 1;
    unsigned int i;
    if(dimensions == NULL) return size;
    for(i=0;i<nclistlength(dimensions);i++) {
	CDFnode* dim = (CDFnode*)nclistget(dimensions,i);
	size *= dim->dim.declsize;
    }
    return size;
}


/**
Look up a URL fragment parameter value by key.
@param nccomm the common DAP state containing the parsed URL
@param key the parameter key to look up
@return the parameter value string, or NULL if not found or inputs are NULL
*/
const char*
dapparamvalue(NCDAPCOMMON* nccomm, const char* key)
{
    const char* value;

    if(nccomm == NULL || key == NULL) return 0;
    value=ncurifragmentlookup(nccomm->oc.url,key);
    return value;
}

static const char* checkseps = "+,:;";

/**
Check whether a URL fragment parameter contains a given substring.
If subkey is NULL, simply tests whether the parameter key is defined.
@param nccomm the common DAP state containing the parsed URL
@param key the parameter key to look up
@param subkey the substring to search for within the parameter value, or NULL
@return 1 if the key exists and (if subkey is non-NULL) the subkey is found, 0 otherwise
*/
int
dapparamcheck(NCDAPCOMMON* nccomm, const char* key, const char* subkey)
{
    const char* value;
    char* p;

    if(nccomm == NULL || key == NULL) return 0;
    if((value=ncurifragmentlookup(nccomm->oc.url,key)) == NULL)
	return 0;
    if(subkey == NULL) return 1;
    p = strstr(value,subkey);
    if(p == NULL) return 0;
    p += strlen(subkey);
    if(*p != '\0' && strchr(checkseps,*p) == NULL) return 0;
    return 1;
}


/**
Append all elements of l2 onto l1 (concatenation, not union).
Duplicate elements are not removed.
@param l1 the destination list
@param l2 the source list whose elements are appended to l1
@return 1 always
*/
int
nclistconcat(NClist* l1, NClist* l2)
{
    unsigned int i;
    for(i=0;i<nclistlength(l2);i++) nclistpush(l1,nclistget(l2,i));
    return 1;
}

/**
Remove all elements of l2 from l1 (set difference, in place).
@param l1 the list to remove elements from
@param l2 the list of elements to remove
@return 1 if any elements were removed, 0 otherwise
*/
int
nclistminus(NClist* l1, NClist* l2)
{
    size_t i, len;
    int found;
    len = nclistlength(l2);
    found = 0;
    for(i=0;i<len;i++) {
	if(nclistdeleteall(l1,nclistget(l2,i))) found = 1;
    }
    return found;
}

/**
Remove all occurrences of a specific element pointer from a list.
@param l the list to modify
@param elem the element pointer to remove
@return 1 if any elements were removed, 0 otherwise
*/
int
nclistdeleteall(NClist* l, void* elem)
{
    int found = 0;
    for(size_t i = nclistlength(l); i-->0;) {
	void* test = nclistget(l,i);
	if(test==elem) {
	    nclistremove(l,i);
            found=1;
        }
    }
    return found;
}

/**
Collect the ancestor container path from the root down to a given node.
The resulting path list begins at the root and ends with node.
@param node the CDFnode whose path is to be collected
@param path the NClist to which path nodes are prepended/appended
@param withdataset if non-zero, include the NC_Dataset root node in the path
*/
void
collectnodepath(CDFnode* node, NClist* path, int withdataset)
{
    if(node == NULL) return;
    nclistpush(path,(void*)node);
    while(node->container != NULL) {
	node = node->container;
	if(!withdataset && node->nctype == NC_Dataset) break;
	nclistinsert(path,0,(void*)node);
    }
}

/**
Collect the ancestor path of an OC DDS node from root down to node.
@param conn the OC connection handle
@param node the OC DDS node whose path is collected
@param path the NClist to which ancestor nodes are appended
*/
void
collectocpath(OClink conn, OCddsnode node, NClist* path)
{
    OCddsnode container;
    OCtype octype;
    if(node == NULL) return;
    oc_dds_class(conn,node,&octype);
    if(octype != OC_Dataset) {
        oc_dds_container(conn,node,&container);
        if(container != NULL)
            collectocpath(conn,container,path);
    }
    nclistpush(path,(void*)node);
}

/**
Build a path string for an OC DDS node using a given separator.
Dataset-level nodes return just the dataset name.
@param conn the OC connection handle
@param node the OC DDS node to build a path string for
@param sep the separator string to use between path components
@return a newly allocated path string; caller must free
*/
char*
makeocpathstring(OClink conn, OCddsnode node, const char* sep)
{
    size_t i,len,first;
    char* result;
    char* name;
    OCtype octype;
    NClist* ocpath = NULL;
    NCbytes* pathname = NULL;

    /* If we are asking for the dataset path only,
       then include it, otherwise elide it
    */
    oc_dds_type(conn,node,&octype);
    if(octype == OC_Dataset) {
        oc_dds_name(conn,node,&name);
	return nulldup(name);
    }

    ocpath = nclistnew();
    collectocpath(conn,node,ocpath);
    len = nclistlength(ocpath);
    assert(len > 0); /* dataset at least */

    pathname = ncbytesnew();
    for(first=1,i=1;i<len;i++) { /* start at 1 to skip dataset name */
	OCddsnode node = (OCddsnode)nclistget(ocpath,i);
	char* name;
        oc_dds_type(conn,node,&octype);
        oc_dds_name(conn,node,&name);
	if(!first) ncbytescat(pathname,sep);
	ncbytescat(pathname,name);
	nullfree(name);
	first = 0;
    }
    result = ncbytesextract(pathname);
    ncbytesfree(pathname);
    nclistfree(ocpath);
    return result;
}

/**
Build a path string from a list of CDFnodes using a given separator.
@param path NClist of CDFnode pointers from root to target
@param separator the separator string between path components
@param flags bitfield controlling path construction (e.g., PATHELIDE, PATHNC)
@return a newly allocated path string; caller must free
*/
char*
makepathstring(NClist* path, const char* separator, int flags)
{
    size_t i,len,first;
    NCbytes* pathname = NULL;
    char* result;
    CDFnode* node;

    len = nclistlength(path);
    ASSERT(len > 0); /* dataset at least */

    if(len == 1) {/* dataset only */
        node = (CDFnode*)nclistget(path,0);
	return nulldup(node->ncbasename);
    }

    pathname = ncbytesnew();
    for(first=1,i=0;i<len;i++) {
	CDFnode* node = (CDFnode*)nclistget(path,i);
	char* name;
	if(!node->elided || (flags & PATHELIDE)==0) {
    	    if(node->nctype != NC_Dataset) {
                name = node->ncbasename;
		assert(name != NULL);
	        if(!first) ncbytescat(pathname,separator);
                ncbytescat(pathname,name);
	        first = 0;
	    }
	}
    }
    result = ncbytesextract(pathname);
    ncbytesfree(pathname);
    return result;
}

/**
Build a full CDF path string for a CDFnode using the ncbasename field.
@param var the CDFnode for which to build the path string
@param separator the separator string between path components
@return a newly allocated path string; caller must free
*/
char*
makecdfpathstring(CDFnode* var, const char* separator)
{
    char* spath;
    NClist* path = nclistnew();
    collectnodepath(var,path,WITHDATASET); /* <= note */
    spath = makepathstring(path,separator,PATHNC);
    nclistfree(path);
    return spath;
}

/**
Collect a list of cloned ncbasename strings for the ancestor path of a node.
@param node the CDFnode whose ancestor names are collected
@param path the NClist to which name strings are appended
@param withdataset if non-zero, include the NC_Dataset root name
*/
void
clonenodenamepath(CDFnode* node, NClist* path, int withdataset)
{
    if(node == NULL) return;
    /* stop at the dataset container as well*/
    if(node->nctype != NC_Dataset)
        clonenodenamepath(node->container,path,withdataset);
    if(node->nctype != NC_Dataset || withdataset)
        nclistpush(path,(void*)nulldup(node->ncbasename));
}

/**
Join a list of name strings into a single path string with a separator.
@param names NClist of char* name strings
@param separator the separator string to insert between names
@return a newly allocated joined string; caller must free
*/
char*
simplepathstring(NClist* names,  char* separator)
{
    size_t i;
    size_t len;
    char* result;
    if(nclistlength(names) == 0) return nulldup("");
    for(len=0,i=0;i<nclistlength(names);i++) {
	char* name = (char*)nclistget(names,i);
	len += strlen(name);
	len += strlen(separator);
    }
    len++; /* room for strlcat to null terminate */
    result = (char*)malloc(len+1);
    result[0] = '\0';
    for(i=0;i<nclistlength(names);i++) {
	char* segment = (char*)nclistget(names,i);
	if(i > 0) strlcat(result,separator,len);
	strlcat(result,segment,len);
    }
    return result;
}

/**
Test whether a CDFnode is transitively contained within a sequence.
@param node the CDFnode to test
@return TRUE if node has a sequence ancestor, FALSE otherwise
*/
int
dapinsequence(CDFnode* node)
{
    if(node == NULL || node->container == NULL) return TRUE;
    for(node=node->container;node->nctype != NC_Dataset;node=node->container) {
       if(node->nctype == NC_Sequence) return TRUE;
    }
    return FALSE;
}

/**
Test whether a CDFnode is transitively contained within a dimensioned structure.
@param node the CDFnode to test
@return TRUE if node has a dimensioned NC_Structure ancestor, FALSE otherwise
*/
int
dapinstructarray(CDFnode* node)
{
    if(node == NULL) return TRUE;
    for(node=node->container;node->nctype != NC_Dataset;node=node->container) {
       if(node->nctype == NC_Structure
	  && nclistlength(node->array.dimset0) > 0)
	    return TRUE;
    }
    return FALSE;
}

/**
Test whether a CDFnode is a map field (non-array subnode) of a DAP Grid.
@param node the CDFnode to test
@return TRUE if node is a map field of its parent grid, FALSE otherwise
*/
int
dapgridmap(CDFnode* node)
{
    if(node != NULL && node->container != NULL
       && node->container->nctype == NC_Grid) {
	CDFnode* array = (CDFnode*)nclistget(node->container->subnodes,0);
	return (node != array);
    }
    return FALSE;
}

/**
Test whether a CDFnode is the array field (first subnode) of a DAP Grid.
@param node the CDFnode to test
@return TRUE if node is the array field of its parent grid, FALSE otherwise
*/
int
dapgridarray(CDFnode* node)
{
    if(node != NULL && node->container != NULL
       && node->container->nctype == NC_Grid) {
	CDFnode* array = (CDFnode*)nclistget(node->container->subnodes,0);
	return (node == array);
    }
    return FALSE;
}

/**
Test whether a CDFnode is any field (array or map) of a DAP Grid.
@param node the CDFnode to test
@return TRUE if node is either the array or a map field of a parent grid
*/
int
dapgridelement(CDFnode* node)
{
    return dapgridarray(node)
           || dapgridmap(node);
}

/**
Test whether a CDFnode is a top-level DAP Grid node.
@param grid the CDFnode to test
@return TRUE if grid is an NC_Grid node directly under the dataset root
*/
int
daptopgrid(CDFnode* grid)
{
    if(grid == NULL || grid->nctype != NC_Grid) return FALSE;
    return daptoplevel(grid);
}

/**
Test whether a CDFnode is a top-level DAP Sequence node.
@param seq the CDFnode to test
@return TRUE if seq is an NC_Sequence node directly under the dataset root
*/
int
daptopseq(CDFnode* seq)
{
    if(seq == NULL || seq->nctype != NC_Sequence) return FALSE;
    return daptoplevel(seq);
}

/**
Test whether a CDFnode is a direct child of the dataset root.
@param node the CDFnode to test
@return TRUE if node's container is the NC_Dataset root, FALSE otherwise
*/
int
daptoplevel(CDFnode* node)
{
    if(node->container == NULL
       || node->container->nctype != NC_Dataset) return FALSE;
    return TRUE;
}

/**
Decode a model string into a set of flags using a translation table.
@param translation the translation mode to match in the table
@param smodel the model string to look up
@param models the null-terminated array of NCTMODEL entries to search
@param dfalt the default flags value to return if no match is found
@return the flags from the matching table entry, or dfalt if not found
*/
unsigned int
modeldecode(int translation, const char* smodel,
            const struct NCTMODEL* models,
            unsigned int dfalt)
{
    for(;models->translation;models++) {
	if(translation != models->translation) continue;
	if(smodel == models->model
	   || (models->model != NULL && strcasecmp(smodel,models->model)==0)) {
	    /* We have a match */
            return models->flags;
	}
    }
    return dfalt;
}

/**
Parse a size limit string with optional K/M/G suffix into bytes.
Recognizes suffixes: K/k (kilobytes), M/m (megabytes), G/g (gigabytes).
@param limit the string to parse, e.g. "10M" or "512"
@return the numeric value in bytes, or 0 if limit is NULL, empty, or unparseable
*/
unsigned long
getlimitnumber(const char* limit)
{
    size_t slen;
    unsigned long multiplier = 1;
    unsigned long lu;

    if(limit == NULL) return 0;
    slen = strlen(limit);
    if(slen == 0) return 0;
    switch (limit[slen-1]) {
    case 'G': case 'g': multiplier = GIGBYTE; break;
    case 'M': case 'm': multiplier = MEGBYTE; break;
    case 'K': case 'k': multiplier = KILOBYTE; break;
    default: break;
    }
    if(sscanf(limit,"%lu",&lu) != 1)
	return 0;
    return (lu*multiplier);
}

/**
Expand C-style backslash escape sequences in a string in place.
Handles \a, \b, \f, \n, \r, \t, \v, \\, \?, \xHH (hex), and \OOO (octal).
The string is only ever shortened or unchanged, never lengthened.
@param termstring the string to process in place
*/
void
dapexpandescapes(char *termstring)
{
    char *s, *t, *endp;

    /* expand "\" escapes, e.g. "\t" to tab character;
       will only shorten string length, never increase it
    */
    s = termstring;
    t = termstring;
    while(*t) {
	if (*t == '\\') {
	    t++;
	    switch (*t) {
	      case 'a':
		*s++ = '\007'; t++; /* will use '\a' when STDC */
		break;
	      case 'b':
		*s++ = '\b'; t++;
		break;
	      case 'f':
		*s++ = '\f'; t++;
		break;
	      case 'n':
		*s++ = '\n'; t++;
		break;
	      case 'r':
		*s++ = '\r'; t++;
		break;
	      case 't':
		*s++ = '\t'; t++;
		break;
	      case 'v':
		*s++ = '\v'; t++;
		break;
	      case '\\':
		*s++ = '\\'; t++;
		break;
	      case '?':
		*s++ = '\177'; t++;
		break;
	      case 'x':
		t++; /* now t points to one or more hex digits */
		*s++ = (char) strtol(t, &endp, 16);
		t = endp;
		break;
	      case '0':
	      case '1':
	      case '2':
	      case '3':
	      case '4':
	      case '5':
	      case '6':
	      case '7': {
		/* t should now point to 3 octal digits */
		int c;
		c = t[0];
		if(c == 0 || c < '0' || c > '7') goto normal;
		c = t[1];
		if(c == 0 || c < '0' || c > '7') goto normal;
		c = t[2];
		if(c == 0 || c < '0' || c > '7') goto normal;
		c = ((t[0]-'0')<<6)+((t[1]-'0')<<3)+(t[2]-'0');
		*s++ = (char)c;
		t += 3;
		} break;
	      default:
		if(*t == 0)
		    *s++ = '\\';
		else
		    *s++ = *t++;
		break;
	    }
	} else {
normal:	    *s++ = *t++;
	}
    }
    *s = '\0';
}


#ifdef HAVE_GETTIMEOFDAY
static double
deltatime(struct timeval time0, struct timeval time1)
{
    double t0, t1;
    t0 = ((double)time0.tv_sec);
    t0 += ((double)time0.tv_usec) / 1000000.0;
    t1 = ((double)time1.tv_sec);
    t1 += ((double)time1.tv_usec) / 1000000.0;
    return (t1 - t0);
}
#endif

/**
Fetch a DAP object from the server, with optional logging and timing.
Wraps oc_fetch with fetch logging (if NCF_SHOWFETCH is set) and maps
HTTP error codes to appropriate NC error codes.
@param nccomm the common DAP state (used for control flags and URL)
@param conn the OC connection handle
@param ce the constraint expression string, or NULL for no constraint
@param dxd the type of object to fetch (OCDDS, OCDAS, or OCDATADDS)
@param rootp output pointer to the fetched OC DDS root node
@return NC_NOERR on success, or an NC error code on failure
*/
NCerror
dap_fetch(NCDAPCOMMON* nccomm, OClink conn, const char* ce,
             OCdxd dxd, OCddsnode* rootp)
{
    NCerror ncstat = NC_NOERR;
    OCerror ocstat = OC_NOERR;
    char* ext = NULL;
    int httpcode = 0;
    OCflags ocflags = 0;
#ifdef HAVE_GETTIMEOFDAY
    struct timeval time0;
    struct timeval time1;
#endif

    if(dxd == OCDDS) ext = ".dds";
    else if(dxd == OCDAS) ext = ".das";
    else ext = ".dods";

    if(ce != NULL && strlen(ce) == 0)
	ce = NULL;

    if(FLAGSET(nccomm->controls,NCF_UNCONSTRAINABLE))
	ce = NULL;
    if(FLAGSET(nccomm->controls,NCF_ONDISK))
	ocflags |= OCONDISK;
    if(FLAGSET(nccomm->controls,NCF_ENCODE_PATH))
	ocflags |= OCENCODEPATH;
    if(FLAGSET(nccomm->controls,NCF_ENCODE_QUERY))
	ocflags |= OCENCODEQUERY;

    if(SHOWFETCH) {
	/* Build uri string minus the constraint and #tag */
	char* baseurl = ncuribuild(nccomm->oc.url,NULL,ext,NCURIBASE);
	if(ce == NULL)
            LOG1(NCLOGNOTE,"fetch: %s",baseurl);
	else
            LOG2(NCLOGNOTE,"fetch: %s?%s",baseurl,ce);
	nullfree(baseurl);
#ifdef HAVE_GETTIMEOFDAY
	gettimeofday(&time0,NULL);
#endif
    }
    ocstat = oc_fetch(conn,ce,dxd,ocflags,rootp);
    if(FLAGSET(nccomm->controls,NCF_SHOWFETCH)) {
#ifdef HAVE_GETTIMEOFDAY
        double secs;
	gettimeofday(&time1,NULL);
	secs = deltatime(time0,time1);
	nclog(NCLOGNOTE,"fetch complete: %0.3f secs",secs);
#else
	nclog(NCLOGNOTE,"fetch complete.");
#endif
    }
#ifdef DEBUG2
fprintf(stderr,"fetch: dds:\n");
oc_dumpnode(conn,*rootp);
#endif

    /* Look at the HTTP return code */
    httpcode = oc_httpcode(conn);
    if(httpcode < 400) {
        ncstat = ocerrtoncerr(ocstat);
    } else if(httpcode >= 500) {
        ncstat = NC_EDAPSVC;
    } else if(httpcode == 401) {
	ncstat = NC_EACCESS;
    } else if(httpcode == 403) {
	ncstat = NC_EAUTH;
    } else if(httpcode == 404) {
	ncstat = NC_ENOTFOUND;
    } else {
	ncstat = NC_EACCESS;
    }
    return ncstat;
}

/* Check a name to see if it contains illegal dap characters
*/

static const char* baddapchars = "./";

/**
Test whether a name contains characters that are illegal in DAP names.
Currently checks for '.' and '/'.
@param name the name string to check
@return 1 if the name contains illegal characters, 0 otherwise
*/
int
dap_badname(char* name)
{
    const char* p;
    if(name == NULL) return 0;
    for(p=baddapchars;*p;p++) {
        if(strchr(name,*p) != NULL)
	    return 1;
    }
    return 0;
}

#if 0
/* Repair a dap name */
char*
dap_repairname(char* name)
{
    /* assume that dap_badname was called on this name and returned 1 */
    return repairname(name,baddapchars);
}
#endif

/* Check a name to see if it contains illegal dap characters
   and repair them
*/

static const char* hexdigits = "0123456789abcdef";

static char*
repairname(const char* name, const char* badchars)
{
    char* newname;
    const char *p;
    char *q;
    char c;
    size_t nnlen = 0;

    if(name == NULL) return NULL;
    nnlen = (3*strlen(name)); /* max needed */
    nnlen++; /* room for strlcat to add nul */
    newname = (char*)malloc(1+nnlen); /* max needed */
    newname[0] = '\0'; /* so we can use strlcat */
    for(p=name,q=newname;(c=*p);p++) {
        if(strchr(badchars,c) != NULL) {
	    int digit;
            char newchar[4];
	    newchar[0] = '%';
            digit = (c & 0xf0) >> 4;
	    newchar[1] = hexdigits[digit];
            digit = (c & 0x0f);
	    newchar[2] = hexdigits[digit];
	    newchar[3] = '\0';
            strlcat(newname,newchar,nnlen);
            q += 3; /*strlen(newchar)*/
        } else
            *q++ = c;
	*q = '\0'; /* so we can always do strlcat */
    }
    *q = '\0'; /* ensure trailing null */
    return newname;
}

/**
Extract the selection clause (everything after the first '&') from a DAP URL query.
@param uri the parsed NCURI whose query field is examined
@return a newly allocated string containing the selection, or NULL if none exists
*/
char*
dap_getselection(NCURI* uri)
{
    char* p;
    char* q = uri->query;
    if(q == NULL) return NULL;
    p = strchr(q,'&');
    if(p == NULL) return NULL;
    return strdup(p+1);
}

/* Compute padding */
static size_t
nccpadding(unsigned long offset, size_t alignment)
{
    size_t rem = (alignment==0?0:(offset % alignment));
    size_t pad = (rem==0?0:(alignment - rem));
    return pad;
}

/**
Split a delimited string into tokens and append them to a list.
Empty tokens (consecutive delimiters) are skipped.
@param s0 the input string to split
@param delim the delimiter character
@param list the NClist to which non-empty token strings are appended
@return NC_NOERR always
*/
int
dapparamparselist(const char* s0, int delim, NClist* list)
{
    int stat = NC_NOERR;
    char* s = strdup(s0);
    char* p;
    int i,count = 1;
    if(s0 == NULL || strlen(s) == 0) goto done;
    for(p=s;*p;p++) {if(*p == delim) {*p = '\0'; count++;}}
    for(i=0,p=s;i<count;i++,p+=(strlen(p)+1)) {
	if(strlen(p)>0)
	    nclistpush(list,strdup(p));
    }
done:
    nullfree(s);
    return stat;
}
