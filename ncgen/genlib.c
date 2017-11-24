/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/ncgen/genlib.c,v 1.57 2010/04/04 19:39:47 dmh Exp $
 *********************************************************************/

#include "includes.h"

/* invoke netcdf calls (or generate C or Fortran code) to create netcdf
 * from in-memory structure.
The output file name is chosen by using the following in priority order:
1. -o flag name
2. command line input file with .cdl changed to .nc
3. dataset name as specified in netcdf <name> {...} 
*/
void
define_netcdf(void)
{
    char filename[2048+1]; /* +1 for strlcat */

    /* Rule for specifying the dataset name:
	1. use -o name
	2. use the datasetname from the .cdl file
	3. use input cdl file name (with .cdl removed)
	It would be better if there was some way
	to specify the datasetname independently of the
	file name, but oh well.
    */
    if(netcdf_name) { /* -o flag name */
      strncpy(filename,netcdf_name,sizeof(filename));
    } else { /* construct a usable output file name */
	if (cdlname != NULL && strcmp(cdlname,"-") != 0) {/* cmd line name */
	    char* p;

	    strncpy(filename,cdlname,sizeof(filename));
	    /* remove any suffix and prefix*/
	    p = strrchr(filename,'.');
	    if(p != NULL) {*p= '\0';}
	    p = strrchr(filename,'/');
	    if(p != NULL) {memmove(filename,(p+1),sizeof(filename));}
	    
       } else {/* construct name from dataset name */
	    strncpy(filename,datasetname,sizeof(filename)); /* Reserve space for extension, terminating '\0' */
        }
        /* Append the proper extension */
	strlcat(filename,binary_ext,sizeof(filename));
    }

    /* Execute exactly one of these */
#ifdef ENABLE_C
    if (l_flag == L_C) gen_ncc(filename); else /* create C code to create netcdf */
#endif
#ifdef ENABLE_F77
    if (l_flag == L_F77) gen_ncf77(filename); else /* create Fortran code */
#endif
#ifdef ENABLE_JAVA
    if(l_flag == L_JAVA) {
	gen_ncjava(filename);
    } else
#endif
/* Binary is the default */
#ifdef ENABLE_BINARY
    gen_netcdf(filename); /* create netcdf */
#else
    derror("No language specified");
#endif
    close_netcdf();
    cleanup();
}

void
close_netcdf(void)
{
#ifdef ENABLE_C
    if (l_flag == L_C) cl_c(); else /* create C code to close netcdf */
#endif
#ifdef ENABLE_F77
    if (l_flag == L_F77) cl_f77(); else
#endif
#ifdef ENABLE_JAVA
    if (l_flag == L_JAVA) cl_java(); else
#endif
#ifdef ENABLE_BINARY
    if (l_flag == L_BINARY) cl_netcdf();
#endif
}

/**
Return a string representing
the fully qualified name of the symbol.
Symbol must be top level
Caller must free.
*/
void
topfqn(Symbol* sym)
{
    char* fqn;
    char* fqnname;
    char* parentfqn;
    Symbol* parent;
    
    if(sym->fqn != NULL)
	return; /* already defined */

#ifdef USE_NETCDF4
    if(!usingclassic) {
	size_t len;
        parent = sym->container;
        /* Recursively compute parent fqn */
        if(parent == NULL) { /* implies this is the rootgroup */
            assert(sym->grp.is_root);
            sym->fqn = strdup("");
            return;
        } else if(parent->fqn == NULL) {
            topfqn(parent);
        }
        parentfqn = parent->fqn;
    
        fqnname = fqnescape(sym->name);
        len = (strlen(fqnname) + strlen(parentfqn) + 1);
        len++; /* strlcat nul*/
        fqn = (char*)malloc(len+1);
        strncpy(fqn,parentfqn,len);
        strlcat(fqn,"/",len);
        strlcat(fqn,fqnname,len);
        sym->fqn = fqn;
    } else
#endif /*USE_NETCDF4*/
    {
	sym->fqn = strdup(sym->name);
    }
}

/**
Return a string representing
the fully qualified name of a nested symbol
(i.e. field or econst).
Caller must free.
*/
void
nestedfqn(Symbol* sym)
{
    char* fqn;
    char* fqnname;
    Symbol* parent;
    size_t len;
    
    if(sym->fqn != NULL)
	return; /* already defined */

    /* Parent must be a type */
    parent = sym->container;
    assert (parent->objectclass == NC_TYPE);

    assert(parent->fqn != NULL);

    fqnname = fqnescape(sym->name);
    len = (strlen(fqnname) + strlen(parent->fqn) + 1);
    len++; /* strlcat nul*/
    fqn = (char*)malloc(len+1);
    strncpy(fqn,parent->fqn,len);
    strlcat(fqn,".",len);
    strlcat(fqn,fqnname,len);
    sym->fqn = fqn;
}

/**
Return a string representing
the fully qualified name of an attribute.
Caller must free.
*/
void
attfqn(Symbol* sym)
{
    char* fqn;
    char* fqnname;
    char* parentfqn;
    Symbol* parent;
    size_t len;
    
    if(sym->fqn != NULL)
	return; /* already defined */

    assert (sym->objectclass == NC_ATT);

    parent = sym->container;
    if(parent == NULL)
	parentfqn = "";
    else
	parentfqn = parent->fqn;

    fqnname = fqnescape(sym->name);
    len = (strlen(fqnname) + strlen(parentfqn) + 1);
    len++; /* strlcat nul*/
    fqn = (char*)malloc(len+1);
    strncpy(fqn,parentfqn,len);
    strlcat(fqn,"_",len);
    strlcat(fqn,fqnname,len);
    sym->fqn = fqn;
}

#if 0
/* Result is pool alloc'd*/
char*
cprefixed(List* prefix, char* suffix, char* separator)
{
    int slen;
    int plen;
    int i;
    char* result;

    ASSERT(suffix != NULL);
    plen = prefixlen(prefix);
    if(prefix == NULL || plen == 0) return codify(suffix);
    /* plen > 0*/
    slen = 0;
    for(i=0;i<plen;i++) {
	Symbol* sym = (Symbol*)listget(prefix,i);
	slen += (strlen(sym->name)+strlen(separator));
    }
    slen += strlen(suffix);
    slen++; /* for strlcat */
    result = poolalloc(slen+1);
    result[0] = '\0';
    /* Leave off the root*/
    i = (rootgroup == (Symbol*)listget(prefix,0))?1:0;
    for(;i<plen;i++) {
	Symbol* sym = (Symbol*)listget(prefix,i);
        strlcat(result,sym->name,slen); /* append "<prefix[i]/>"*/
	strlcat(result,separator,slen);
    }    
    strlcat(result,suffix,slen); /* append "<suffix>"*/
    return result;
}
#endif /*0*/
