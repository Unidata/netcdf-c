/*********************************************************************
 *   Copyright 2016, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#include "d4includes.h"
#include <stdarg.h>
#include <assert.h>
#include "ezxml.h"
#include "d4includes.h"
#include "d4odom.h"

/**
 * Walk the incoming data and insert into the substrate file

We classify the variables based on the following flags:
* HASSEQ   -- transitively contains sequence(s)
* HASSTR   -- transitively contains strings
* HASOPFIX -- transitively contains fixed size opaques
* HASOPVAR -- transitively contains variable size opaques

If a variable has none of the flags set, then we can
use it directly with nc_put_var.

If it has only strings and/or fixed opaques, we can
easily modify it to work with nc_put_var.

if it has variable size opaque, the fix is a little harder.

If the variable has sequences, then we have to do major rebuilds.

*/

/***************************************************/
/* Forwards */

static int compile(NCD4meta*);
static int compileFixedStruct(NCD4meta*, NCD4node* structvar);
static int getToplevelVars(NCD4meta*, NCD4node* group, NClist* toplevel);
static int writeFixedVar(NCD4meta*, NCD4node* var);

/***************************************************/
/* Macro define procedures */

#if 0
#define D4MALLOC(n) d4bytesalloc(d4bytes,(n))
#endif

#define ISTOPLEVEL(var) ((var)->container == NULL || (var)->container->sort == NCD4_GROUP)

#define PUTVAR(var,memp) NCCHECK(nc_put_var((var)->container->meta.id,(var)->meta.id,(memp)))

#define PUTVAR1(var,index,memp) NCCHECK(nc_put_var1((var)->container->meta.id,(var)->meta.id,(index),(memp)))

/***************************************************/
/* API */

int
NCD4_databuild(NCD4meta* compiler)
{
    int ret = NC_NOERR;
    ret = compile(compiler);
    return THROW(ret);
}

/**************************************************/

/* Recursively walk the tree to extract data */
/* Assume data has already been dechunked */
static int
compile(NCD4meta* compiler)
{
    int i,ret = NC_NOERR;
    NClist* toplevel;
    NCD4node* root = compiler->root;
    void* offset;

    /* Recursively walk the tree in prefix order 
       to get the top-level variables; also mark as unvisited */
    toplevel = nclistnew();
    getToplevelVars(compiler,root,toplevel);

    /* Mark all variable length objects */
    NCD4_markflags(compiler,toplevel);

    /* Do checsums and, if necessary, byte swap the serialized data */

    /* There are two state wrt checksumming.
       1. the incoming data may have checksums,
          but we are not computing a local checksum.
          Flag for this is compiler->checksumming
       2. the incoming data does not have checksums at all.
          Flag for this is compiler->nochecksum
    */
    compiler->checksumming = (compiler->checksummode != NCD4_CSUM_NONE);
    /* However, if the data sent by the server says its does not have checksums,
       then do not bother */
    if(compiler->serial.nochecksum)
	compiler->checksumming = 0;
    compiler->swap = (compiler->serial.hostlittleendian != compiler->serial.remotelittleendian);
    NCD4_serial(compiler,toplevel);

    /* Compute the offset and size of the vars and fields in the raw dap data.
       WARNING: this info is transient.
    */
    offset = compiler->serial.dap;
    for(i=0;i<nclistlength(toplevel);i++) {
	NCD4node* var = (NCD4node*)nclistget(toplevel,i);
        if((ret=NCD4_delimit(compiler,var,&offset))) goto done;
    }

    /* verify checksum */
    if(compiler->checksummode != NCD4_CSUM_NONE) {
        for(i=0;i<nclistlength(toplevel);i++) {
	    NCD4node* var = (NCD4node*)nclistget(toplevel,i);
	    if(var->data.localchecksum != var->data.remotechecksum) {
		fprintf(stderr,"Checksum mismatch: %s\n",var->name);
		abort();
	    }
        }
    }

    /* Fix up toplevel string-typed vars. Moves the strings out-of-band
       and replaces with a pointer to the new null-terminated strings.
    */
    for(i=0;i<nclistlength(toplevel);i++) {
	NCD4node* var = (NCD4node*)nclistget(toplevel,i);
	if(var->subsort == NC_STRING) {
	    if((ret=NCD4_fixstr(compiler,var))) goto done;
	}
	var->data.flags &= ~HASSTR; /* no longer needed */
    }    

    /* Fix up toplevel fix-sized opaque-typed vars. */
    for(i=0;i<nclistlength(toplevel);i++) {
	NCD4node* var = (NCD4node*)nclistget(toplevel,i);
	if(var->subsort == NC_OPAQUE && (var->data.flags & HASOPFIX)) {
	    if((ret=NCD4_fixopfixed(compiler,var))) goto done;
	}
	var->data.flags &= ~HASOPFIX; /* no longer needed */
    }    

    /* Fix up toplevel variable-length opaque-typed vars. */
    for(i=0;i<nclistlength(toplevel);i++) {
	NCD4node* var = (NCD4node*)nclistget(toplevel,i);
	if(var->subsort == NC_OPAQUE && (var->data.flags & HASOPVAR)) {
	    if((ret=NCD4_fixopvar(compiler,var))) goto done;
	}
	var->data.flags &= ~HASOPVAR; /* no longer needed */
    }    

    /* Process structures that can be made fixed size */
    for(i=0;i<nclistlength(toplevel);i++) {
	NCD4node* var = (NCD4node*)nclistget(toplevel,i);
	if(var->data.flags & HASSEQ) continue; /* deal with this later */
	if((ret=compileFixedStruct(compiler, var))) goto done;
    }

    /* Fix up Sequences */
    for(i=0;i<nclistlength(toplevel);i++) {
	NCD4node* var = (NCD4node*)nclistget(toplevel,i);
	if((var->subsort == NC_STRUCT || var->subsort == NC_SEQ)
	    && (var->data.flags & HASSEQ)) {
	    if((ret=NCD4_fixsequences(compiler,var))) goto done;
	}
	var->data.flags &= ~HASSEQ; /* no longer needed */
    }    

    /* At this point all variables should be writeable directly */
    for(i=0;i<nclistlength(toplevel);i++) {
	NCD4node* var = (NCD4node*)nclistget(toplevel,i);
        if((ret=writeFixedVar(compiler,var))) goto done;
    }    

done:
    return THROW(ret);
}

/**************************************************/

static int
writeFixedVar(NCD4meta* compiler, NCD4node* var)
{
    int ret = NC_NOERR;
    ASSERT((ISTOPLEVEL(var)));
    PUTVAR(var,var->data.topvardata.memory); /* write it out */
done:
    return THROW(ret);
}

/**************************************************/

/**
Note that this does not fix sequences; that is done elsewhere.
*/
static int
compileFixedStruct(NCD4meta* compiler, NCD4node* structvar)
{
    int i, ret=NC_NOERR;
    /* Fix up string-typed fields. Moves the strings out-of-band
       and replaces with a pointer to the new null-terminated strings.
    */
    for(i=nclistlength(structvar->vars)-1;i>=0;i--) {
	NCD4node* field = (NCD4node*)nclistget(structvar->vars,i);
	if(field->subsort == NC_STRING) {
	    if((ret=NCD4_fixstr(compiler,field))) goto done;
	}
	field->data.flags &= ~HASSTR; /* no longer needed */
    }    
    /* Fix up fix-sized opaque-typed fields. */
    for(i=nclistlength(structvar->vars)-1;i>=0;i--) {
	NCD4node* field = (NCD4node*)nclistget(structvar->vars,i);
	if(field->subsort == NC_OPAQUE && (field->data.flags & HASOPFIX)) {
	    if((ret=NCD4_fixopfixed(compiler,field))) goto done;
	}
	field->data.flags &= ~HASOPFIX; /* no longer needed */
    }    

    /* Fix up variable-length opaque-typed fields. */
    for(i=nclistlength(structvar->vars)-1;i>=0;i--) {
	NCD4node* field = (NCD4node*)nclistget(structvar->vars,i);
	if(field->subsort == NC_OPAQUE && (field->data.flags & HASOPVAR)) {
	    if((ret=NCD4_fixopvar(compiler,field))) goto done;
	}
	field->data.flags &= ~HASOPVAR; /* no longer needed */
    }    
done:
    return THROW(ret);
}


/**************************************************/
/* Utilities */

static int
getToplevelVars(NCD4meta* compiler, NCD4node* group, NClist* toplevel)
{
    int ret = NC_NOERR;
    int i;
    /* Collect vars in this group */
    for(i=0;i<nclistlength(group->vars);i++) {
        NCD4node* node = (NCD4node*)nclistget(group->vars,i);
        nclistpush(toplevel,node);
        node->visited = 0; /* We will set later to indicate written vars */
#ifdef D4DEBUGDATA
fprintf(stderr,"toplevel: var=%s\n",node->name);
#endif
    }
    /* Now, recurse into subgroups; will produce prefix order */
    for(i=0;i<nclistlength(group->groups);i++) {
        NCD4node* g = (NCD4node*)nclistget(group->groups,i);
	if((ret=getToplevelVars(compiler,g,toplevel))) goto done;
    }
done:
    return THROW(ret);
}

