/*********************************************************************
 *   Copyright 2016, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#include <stdarg.h>
#include <assert.h>

#include "d4includes.h"
#include "ezxml.h"

/*
The primary purpose of this code is to provide node and data walkers
to do a variety of things.

* (topo)
  a. topologically sort the set of allnodes in postfix order.

* (mark)
  a. compute the HASSEQ, HASSTR, HASOPFIX, HASOPVAR flags.

* (delimit)
  a. Compute the offset and sizes of variable and fields

* (fixstr)
  a. for a atomic string typed variable:
       out-of-band (oob) the strings.

* (fixopfixed)
  a. for a variable length atomic opaque typed variable
       out-of-band (oob) the opaque instances

* (fixsequences)
  a. Find all sequence instances and convert them to a single vlen;
     Ideally, we would like to just have it point to the record data
     as it is (if they are fixed size), but this significantly complicates
     the code, so we out-of-band the records. Not very efficient but ok for now.

WARNING: because we may move a top-level variable's space inside
sizefixxxx, you have to be extra careful to relativize pointers
if there is a chance that a recursive call will reallocate.

*/

/**************************************************/

#define COUNTSIZE 8

/**************************************************/
/* Forward */

static d4size_t computeInstanceSize(NCD4meta*, NCD4node* var, int flags, void** offsetp);
static int delimitAtomicVar(NCD4meta*, NCD4node* var, void** offsetp);
static int delimitOpaqueVar(NCD4meta*,  NCD4node* var, void** offsetp);
static int delimitSeq(NCD4meta*, NCD4node* var, void** offsetp);
static int delimitSeqArray(NCD4meta*, NCD4node* var,  void** offsetp);
static int delimitStruct(NCD4meta*, NCD4node* var, void** offsetp);
static int delimitStructArray(NCD4meta*, NCD4node* var,  void** offsetp);
static int fixseqSeq(NCD4meta* compiler, NCD4node* top, NCD4node* var, void** offsetp);
static int fixseqWalk(NCD4meta*, NCD4node* top, NCD4node* cmpd, void** offsetp);
static int flagWalk(NCD4meta* compiler, NCD4node* node, int* flagsp);
static int resize(NCD4meta*, NCD4node* top, ptrdiff_t needed, void* empty, void* suffixdata);
static NCD4node* toplevel(NCD4node* field);
static void walk(NCD4node* node, NClist* sorted);

#ifdef D4DEBUGDATA
static void
ADDNODE(NClist* list, NCD4node* node)
{
    fprintf(stderr,"addnode: %s: %s %s (%llu)\n",
	node->name,
	NCD4_sortname(node->sort),
	NCD4_subsortname(node->subsort),
	node);
    fflush(stderr);
    nclistpush(list,node);
}
#else
#define ADDNODE(list,node) nclistpush((list),(node))
#endif


	
/**************************************************/
/* Topo sort in postfix order */

int
NCD4_toposort(NCD4meta* compiler)
{
    int i, ret=NC_NOERR;
    size_t len = nclistlength(compiler->allnodes);
    NCD4node** list = (NCD4node**)nclistcontents(compiler->allnodes);
    NCD4node** p;
    NClist* sorted = nclistnew();
    nclistsetalloc(sorted,len);
    for(i=0,p=list;i<len;p++,i++) {
	NCD4node* node = *p;
	switch (node->sort) { /* Collect things known to have no dependencies */
	case NCD4_DIM:
	    node->visited = 1;
	    ADDNODE(sorted,node);
	    break;
	case NCD4_TYPE:
	    if(node->subsort <= NC_MAX_ATOMIC_TYPE || node->subsort == NC_OPAQUE) {
		node->visited = 1;
		ADDNODE(sorted,node);
	    } else
		node->visited = 0;
	    break;
	default:
	    node->visited = 0;
	}
    }
    walk(compiler->root,sorted);
    /* Last step is to add in any remaining unvisited nodes, but report them */
    for(i=0,p=list;i<len;p++,i++) {
	NCD4node* node = *p;
	if(node->visited) continue;
#ifdef D4DEBUGDATA
fprintf(stderr,"unvisited node: %s\n",node->name); fflush(stderr);
#endif
	node->visited = 1;
	ADDNODE(sorted,node);
    }
    nclistfree(compiler->allnodes);
    compiler->allnodes = sorted;
    return ret;
}

/*
Do depth first search
*/
static void
walk(NCD4node* node, NClist* sorted)
{
    int i;
    if(node->visited) return;
    node->visited = 1;

    switch (node->sort) {
    case NCD4_GROUP: /* depends on its elements and attributes and subgroups */
	for(i=0;i<nclistlength(node->group.elements);i++) {
	    NCD4node* elem = (NCD4node*)nclistget(node->group.elements,i);
	    walk(elem,sorted);
	}		
	break;	
    case NCD4_TYPE: /* Need to discriminate on the subsort */
	switch (node->subsort) {
	case NC_SEQ:
	case NC_STRUCT: /* Depends on its fields */
            for(i=0;i<nclistlength(node->vars);i++) {
	        NCD4node* f = (NCD4node*)nclistget(node->vars,i);
		walk(f,sorted);
	    }
	    break;	
	case NC_ENUM: /* Depends on its basetype, but since that is atomic, we can ignore */
	    /* fall thru */
	default: /* Atomic or opaque, so already marked */
	   break;
	}
	break;

    case NCD4_VAR: /* Depends on: dimensions and basetype and maps */
        for(i=0;i<nclistlength(node->dims);i++) {
	    NCD4node* d = (NCD4node*)nclistget(node->dims,i);
  	    walk(d,sorted);
	}
	walk(node->basetype,sorted);
        for(i=0;i<nclistlength(node->maps);i++) {
	    NCD4node* m = (NCD4node*)nclistget(node->maps,i);
  	    walk(m,sorted);
	}
	break;
	
    case NCD4_ATTR: /* Depends on its base type */
	walk(node->basetype,sorted);
	break;

    case NCD4_ATTRSET: /* Depends on its contained attributes, but handled after switch */
	/* fall thru */
    default: /* depends on nothing else */
	break;
    }

    /* Do Attributes last */
    for(i=0;i<nclistlength(node->attributes);i++) {
	NCD4node* a = (NCD4node*)nclistget(node->attributes,i);
	walk(a,sorted);
    }		
    ADDNODE(sorted,node);    
}


/**************************************************/
/**
Mark variables that directly
or transitively contain 
variable length data.
*/

int
NCD4_markflags(NCD4meta* compiler, NClist* toplevel)
{
    int i,ret = NC_NOERR;
    for(i=0;i<nclistlength(toplevel);i++) {
        int flags = 0;
	NCD4node* var = nclistget(toplevel,i);
        ret = flagWalk(compiler,var,&flags);
    }
    return ret;
}

/*
Do depth first search looking for flagable fields
We will propagate them as we unwind the recursion.
*/
static int
flagWalk(NCD4meta* compiler, NCD4node* node, int* flagsp)
{
    int i, ret = NC_NOERR;
    int flags = 0;
    int fieldflags;

    /* Mark this node */
    switch (node->basetype->subsort) {
    case NC_SEQ:
	fieldflags = 0;
        for(i=0;i<nclistlength(node->basetype->vars);i++) {
	    NCD4node* field = (NCD4node*)nclistget(node->basetype->vars,i);
 	    if((ret=flagWalk(compiler,field,&fieldflags))) goto done;
	}
	if((fieldflags & HASSEQ) == 0) fieldflags |= LEAFSEQ;
        flags |= fieldflags;
	flags |= HASSEQ;
        break;
    case NC_STRUCT: /* recurse*/
	fieldflags = 0;
        for(i=0;i<nclistlength(node->basetype->vars);i++) {
	    NCD4node* field = (NCD4node*)nclistget(node->basetype->vars,i);
	    if((ret=flagWalk(compiler,field,&fieldflags))) goto done;
	}
        flags |= fieldflags;
	break;
    case NC_STRING:
        flags |= HASSTR;
	break;
    case NC_OPAQUE:
	if(node->basetype->opaque.size == 0)
	    flags |= HASOPVAR;
	else
	    flags |= HASOPFIX;
	break;
    default: /* ignore */
	break;
    }
    node->data.flags = flags;
#ifdef D4DEBUGDATA
fprintf(stderr,"flagWalk: %s: %d\n",node->name,flags);
#endif
    *flagsp |= flags;
done:
    return ret;
}

/**************************************************/

/* Mark the offset and length of each var/field
   inside the raw dapdata. This is transient info
   because some data will be compressed out
   by other code (here or in d4data).
*/

int
NCD4_delimit(NCD4meta* compiler, NCD4node* var, void** offsetp)
{
    int ret = NC_NOERR;
    void* offset;

    offset = *offsetp;
    var->data.vardata.memory = offset;
    if(var->sort == NCD4_VAR) {
	switch (var->subsort) {
        case NC_STRUCT:
            if((ret=delimitStructArray(compiler,var,&offset))) goto done;
            break;
        case NC_SEQ:
            if((ret=delimitSeqArray(compiler,var,&offset))) goto done;
            break;
	default:
            if((ret=delimitAtomicVar(compiler,var,&offset))) goto done;
            break;
	}
    }
    /* skip checksum */
    if(ISTOPLEVEL(var) &&
       (compiler->checksummode == NCD4_DAP || compiler->checksummode == NCD4_ALL)) {
        memcpy(var->data.checksum,offset,CHECKSUMSIZE);
        offset += CHECKSUMSIZE;
    }
#ifdef D4DEBUGDATA
{
unsigned long pos = (*offsetp - compiler->serial.dap);
fprintf(stderr,"delimit: %s: %lu/%lu (0x%lx)\n",var->name,pos,(unsigned long)var->data.vardata.size,var->data.vardata.memory);
}
#endif
    *offsetp = offset;
done:
    return ret;
}

static int
delimitAtomicVar(NCD4meta* compiler, NCD4node* var, void** offsetp)
{
    int ret = NC_NOERR;
    void* offset;
    int typesize;
    d4size_t i;
    d4size_t dimproduct = NCD4_dimproduct(var);
    nc_type tid;
    NCD4node* truetype;

    if(var->basetype->subsort == NC_OPAQUE)
        return delimitOpaqueVar(compiler,var,offsetp);

    offset = *offsetp;

    var->data.vardata.memory = offset;

    truetype = var->basetype;    
    if(truetype->subsort == NC_ENUM)
        truetype = var->basetype->basetype;

    tid = truetype->subsort;
    typesize = NCD4_typesize(tid);
    if(tid != NC_STRING) {
        offset += (typesize*dimproduct);
    } else if(tid == NC_STRING) { /* walk the counts */
        unsigned long long count;
        for(i=0;i<dimproduct;i++) {
            /* Get string count */
            count = GETCOUNTER(offset);
            SKIPCOUNTER(offset);
            /* skip count bytes */
            offset += count;
        }
    }
    var->data.vardata.size = (d4size_t)(offset - var->data.vardata.memory);
    *offsetp = offset;
    return ret;
}

static int
delimitOpaqueVar(NCD4meta* compiler,  NCD4node* var, void** offsetp)
{
    int ret = NC_NOERR;
    void* offset;
    d4size_t i;
    unsigned long long count;
    d4size_t dimproduct = NCD4_dimproduct(var);

    offset = *offsetp;
    var->data.vardata.memory = offset;
    for(i=0;i<dimproduct;i++) {
        /* Walk the instances */
        count = GETCOUNTER(offset);
        SKIPCOUNTER(offset);
        offset += count;
    }
    var->data.vardata.size = (d4size_t)(offset - var->data.vardata.memory);
    *offsetp = offset;
    return ret;
}

static int
delimitStructArray(NCD4meta* compiler, NCD4node* var,  void** offsetp)
{
    int ret = NC_NOERR;
    void* offset;
    d4size_t i;
    d4size_t dimproduct = NCD4_dimproduct(var);

    offset = *offsetp;
    var->data.vardata.memory = offset;
    var->data.vardata.size = 0;
    for(i=0;i<dimproduct;i++) {
        if((ret=delimitStruct(compiler,var,&offset))) goto done;
    }
    var->data.vardata.size = (offset - var->data.vardata.memory);
    *offsetp = offset;
done:
    return ret;
}

static int
delimitStruct(NCD4meta* compiler, NCD4node* var, void** offsetp)
{
    int ret = NC_NOERR;
    int i;
    void* offset;

    offset = *offsetp;
    /* The fields are associated with the basetype struct */
    for(i=0;i<nclistlength(var->basetype->vars);i++) {
        NCD4node* field = (NCD4node*)nclistget(var->basetype->vars,i);
        switch (field->subsort) {
	default:
            if((ret=delimitAtomicVar(compiler,field,&offset))) goto done;
            break;
        case NC_STRUCT: /* recurse */
            if((ret=delimitStructArray(compiler,field,&offset))) goto done;
            break;
        case NC_SEQ:
            if((ret=delimitSeqArray(compiler,field,&offset))) goto done;
            break;
        }
    }
    *offsetp = offset;
done:
    return ret;
}

static int
delimitSeqArray(NCD4meta* compiler, NCD4node* var,  void** offsetp)
{
    int ret = NC_NOERR;
    void* offset;
    d4size_t i;
    d4size_t dimproduct = NCD4_dimproduct(var);

    offset = *offsetp;
    var->data.vardata.memory = offset;
    for(i=0;i<dimproduct;i++) {
        /* Getsize, possibly recursively, the single seq pointed to by offset*/
        if((ret=delimitSeq(compiler,var,&offset))) goto done;
    }
    var->data.vardata.size = (offset - var->data.vardata.memory);
    *offsetp = offset;
done:
    return ret;
}

static int
delimitSeq(NCD4meta* compiler, NCD4node* var, void** offsetp)
{
    int ret = NC_NOERR;
    int i;
    void* offset;
    d4size_t recordcount;

    offset = *offsetp;

    /* Get and getsize the record count */
    recordcount = GETCOUNTER(offset);
    SKIPCOUNTER(offset);

    for(i=0;i<recordcount;i++) {
        /* We can treat each record like a structure instance */
        if((ret=delimitStruct(compiler,var,&offset))) goto done;
    }
    *offsetp = offset;
done:
    return ret;
}

/**************************************************/

int
NCD4_fixstr(NCD4meta* compiler, NCD4node* var)
{
    int i,ret = NC_NOERR;
    d4size_t dimproduct = NCD4_dimproduct(var);
    void* offset = var->data.vardata.memory;
    char** p = (char**)offset;

    assert(var->container->sort == NCD4_GROUP); /* must be toplevel */    

    if(compiler->blobs == NULL)
        compiler->blobs = nclistnew();

    for(i=0;i<dimproduct;i++) {
        char* q;
	d4size_t count;
        /* Get string count (remember, it is already properly swapped) */
        count = GETCOUNTER(offset);
        SKIPCOUNTER(offset);
        /* Transfer out of band */
        q = (char*)malloc(count+1);
        memcpy(q,offset,count);
        q[count] = '\0';
        PUSH(compiler->blobs,q);
        /* Write the pointer to the string */
        *p++ = q;
        /* skip to next string */
        offset += count;
    }
    /* Now compress out any extra space */
    {
        /* we compress from the top-level variable */
        NCD4node* top = toplevel(var);
        char* dst = (char*)p;
        char* src = (char*)offset;
        ptrdiff_t delta = (src - dst); /* remember, we are moving down */
        ptrdiff_t used = (dst - (char*)top->data.vardata.memory);
        size_t tomove = (size_t)(top->data.vardata.size - used);
        if(tomove > 0)
            d4memmove(dst,src,tomove);
        /* fix up the variable size */
        top->data.vardata.size -= delta;
    }
    return ret;
}

int
NCD4_fixopfixed(NCD4meta* compiler, NCD4node* var)
{
    int ret = NC_NOERR;
    int i;
    d4size_t count;
    d4size_t dimproduct = NCD4_dimproduct(var);
    d4size_t opaquesize = var->basetype->opaque.size;
    void* offset = var->data.vardata.memory;
    void* q;

    q = offset;
    for(i=0;i<dimproduct;i++) {
        /* Get opaque count */
        count = GETCOUNTER(offset);
        SKIPCOUNTER(offset);
        /* verify that it is the correct size */
        if(count != opaquesize) {
            FAIL(NC_EVARSIZE,"Expected opaque size to be %lld; found %lld",opaquesize,count);
        }
        /* Compress out */
        memcpy(q,offset,count);
        q += count;
        /* skip to next opaque */
        offset += count;
    }
    /* Now compress out any extra space out of the variable*/
    {
        /* we compress from the top-level variable */
        NCD4node* top = toplevel(var);
        char* dst = (char*)q;
        char* src = (char*)offset;
        ptrdiff_t delta = (src - dst);
        ptrdiff_t used = (dst - (char*)top->data.vardata.memory);
        size_t tomove = (size_t)(top->data.vardata.size - used);
        if(tomove > 0)
            d4memmove(dst,src,tomove);
        /* fix up the variable size */
        top->data.vardata.size -= delta;
    }
done:
    return ret;
}

/*
Move a dap4 variable length opaque out of band.
This is very tricky because we need to replace each
opaque (plus its count) with an instance of nc_vlen_t.
But the total size of the opaque might be less than
sizeof(nc_vlen_t). In this case, we have to make
room in the variable to hold the vlens. There is one
more complication: it might be that any single opaque
instance is too small, but the aggregate set of opaques
is big enough (because some opaque instances are large).
In this case, we can, with some care, overwrite the opaque
data with the necessary vlens. For simplicity, we start
by separately allocating the vlens and then fiddle with the
variable's data space in order to store those vlens.
Ugh!
*/

int
NCD4_fixopvar(NCD4meta* compiler, NCD4node* var)
{
    int ret = NC_NOERR;
    d4size_t i;
    unsigned long long count;
    nc_vlen_t* vlens;
    ptrdiff_t needed, totalsize;
    size_t vlenspace;
    NCD4node* top;
    d4size_t dimproduct = NCD4_dimproduct(var);
    void* offset = var->data.vardata.memory;

    /* We will store the nc_vlen_t instances temporarily in
       a separate chunk of memory
    */
    vlenspace = (dimproduct*sizeof(nc_vlen_t));
    vlens = (nc_vlen_t*)malloc(vlenspace);
    if(vlens == NULL) {
        FAIL(NC_ENOMEM,"out of space");
    }

    /* Make a pass to move the opaques and to create the vlens for them */
    for(i=0;i<dimproduct;i++) {
        char* q;
        /* Get opaque count */
        count = GETCOUNTER(offset);
        SKIPCOUNTER(offset);
        /* Transfer out of band */
        q = (char*)malloc(count);
        memcpy(q,offset,count);
        PUSH(compiler->blobs,q);
        vlens[i].len = (size_t)count;
        vlens[i].p = q;
        /* skip to next opaque */
        offset += count;
    }
    /* How much space to do have inline to work with? */
    totalsize = (offset - var->data.vardata.memory);
    /* Do we have enough room to store the vlens inline? */
    needed = vlenspace - totalsize;
    top = toplevel(var); /* may need to reallocate */
    if((ret=resize(compiler,var,needed,top->data.vardata.memory+vlenspace,&offset))) goto done;
    /* Move inline */
    d4memmove(var->data.vardata.memory,vlens,vlenspace);
done:
    return ret;
}

/* Locate the top-level variable containing this field or var */
static NCD4node*
toplevel(NCD4node* field)
{
    NCD4node* top = field;
    while(top->container->sort != NCD4_GROUP)
        top = top->container;
    return top;
}

/**************************************************/

/* 
Goal here is to do a depth first search
for Sequence vars/fields. 
For each instance of such, we oob its records,
and replace with a single nc_vlen_t instance.
As we unwind the recursion, we to this for intermediate
sequence typed fields.
*/

int
NCD4_fixsequences(NCD4meta* compiler, NCD4node* cmpd)
{
    int ret = NC_NOERR;
    void* offset;
    offset = cmpd->data.vardata.memory;
    ret = fixseqWalk(compiler,cmpd,cmpd,&offset);
    return ret;
}

/*
Do depth first search lookinng for sequence typed fields.
We will change them as we unwind the recursion.
*/
static int
fixseqWalk(NCD4meta* compiler, NCD4node* top, NCD4node* cmpd, void** offsetp)
{
    int i, ret = NC_NOERR;
    void* offset = *offsetp;
    void* saveoffset = offset;

    /* Search the fields looking for those which transitively
       have sequences (remember: the fields are attached to the type).
       We also need to capture the offset of the true start of the sequence
       instance.
    */
    if(cmpd->subsort == NC_SEQ && (cmpd->data.flags & LEAFSEQ) == LEAFSEQ) {
	/* Short circuit the case of a leaf seq */
        if((ret=fixseqSeq(compiler,top,cmpd,&offset))) goto done;
    } else for(i=0;i<nclistlength(cmpd->basetype->vars);i++) {
	NCD4node* field = (NCD4node*)nclistget(cmpd->basetype->vars,i);
	/* walk down to the innermost sequence: one that has no contained sequence */
	switch (field->subsort) {
	case NC_SEQ:
	    if((ret=fixseqWalk(compiler,top,field,&offset))) goto done;
	    break;
	case NC_STRUCT: /* recurse*/
	    if((field->data.flags & HASSEQ)) {
	        if((ret=fixseqWalk(compiler,top,field,&offset))) goto done;
	    } else
	        offset += computeInstanceSize(compiler,field,HASSEQ,&offset);
	    break;
	default:
	    offset += computeInstanceSize(compiler,field,HASSEQ,&offset);
	    break;
	}
        if(cmpd->subsort == NC_SEQ) {
	    offset = saveoffset;
	    /* If we get here, we know that all inner sequences have been fixed */
            if((ret=fixseqSeq(compiler,top,cmpd,&offset))) goto done;
        }
    }
    *offsetp = offset;
done:
    return ret;
}

/*
We have a sequence that is a leaf sequence or whose
contained sequences have been fixed
*/
static int
fixseqSeq(NCD4meta* compiler, NCD4node* top, NCD4node* var, void** offsetp)
{
	int i, ret = NC_NOERR;
	d4size_t dimproduct = NCD4_dimproduct(var);
	void* offset;
	nc_vlen_t* vlens;
	size_t vlensize, recordspace;
	void* recorddata;
	void* newrecorddata;
	ptrdiff_t needed;
	void* saveoffset;
   
    /* Allocate the vlens, 1 per sequence instance */
    vlensize = dimproduct * sizeof(nc_vlen_t);
    vlens = (nc_vlen_t*) malloc(vlensize);
    if(vlens == NULL) {FAIL(NC_ENOMEM,"out of memory");}

    offset = *offsetp;
    recorddata = offset;
    for(i=0;i<dimproduct;i++) {
	    size_t recordcount;
 	saveoffset = recorddata;
	/* Process a single sequence object */
        recordcount = GETCOUNTER(recorddata); /* get the record count */
	/* Walk the complete set of records to get the offset to the next set of records */
	/* We want the size of one instance, so use basetype */
        (void)computeInstanceSize(compiler,var->basetype,HASSEQ,&recorddata);
        /* OOB the records */
        recordspace = (recorddata - saveoffset);
        newrecorddata = malloc(recordspace - COUNTSIZE); /*do not include recordcount */
        if(newrecorddata == NULL) {FAIL(NC_ENOMEM,"out of memory");}
	PUSH(compiler->blobs,newrecorddata);
        memcpy(newrecorddata,offset+COUNTSIZE,recordspace - COUNTSIZE);
	vlens[i].len = recordcount;
	vlens[i].p = newrecorddata;
    }
    /* By how much space do we need to resize? */
    needed = (vlensize - recordspace);
    saveoffset = offset; /* This is where we need to move the vlens */
    /* Compute the start of the space just past the vlens */
    offset += vlensize;
    if((ret=resize(compiler,top,needed,offset,recorddata+recordspace))) goto done;
    /* Move the vlens into place */
    memcpy(saveoffset,vlens,vlensize);
    *offsetp = offset;
done:
    return ret;
}

static int
resize(NCD4meta* compiler, NCD4node* top,
     ptrdiff_t needed,
     void* empty, /* pointer to the start of the space to be overwritten */
     void* suffixdata /* in: pointer to the start of the data to move up or down */
     )
{
    int ret = NC_NOERR;
    void* offset = empty;
    ptrdiff_t delta;

    if(needed > 0) { /* need more space than we have */
        void* newdata;
        /* Need to make room by reallocating top */
        /* Relativize dst */
        ptrdiff_t off = offset - top->data.vardata.memory;
        newdata = malloc(top->data.vardata.size+needed);
        if(newdata == NULL) {
            FAIL(NC_ENOMEM,"out of space");
        }
        top->data.vardata.memory = newdata;
        top->data.vardata.size += needed;
        /* Make offset be absolute again */
        offset = (top->data.vardata.memory + off);
	delta = needed;
        needed = 0;
    } else if(needed < 0) { /* Have too much space */
        char* src = (char*)suffixdata;
        char* dst = (char*)offset;
        ptrdiff_t used = (dst - (char*)top->data.vardata.memory);
        size_t tomove = (size_t)(top->data.vardata.size - used);
        if(tomove > 0)
            d4memmove(dst,src,tomove);
	delta = - tomove; /* we compressed */
    }
    /* Adjust the top size (delta might be negative)*/
    top->data.vardata.size += delta;
done:
    return ret;
}


/* Given a pointer to the start of an object
(var or field) compute its actual size
as represented in the dap data.
Notes:
1. this may differ from the type size.
2. flags determine what has been oob'd out.
3. Dimproduct is applied
*/

static d4size_t
computeInstanceSize(NCD4meta* builder, NCD4node* var, int flags, void** offsetp)
{
    int i;
    unsigned long long size = 0;
    NCD4node* type = (var->sort == NCD4_TYPE ? var : var->basetype);
    void* offset = *offsetp;
    d4size_t dimproduct = NCD4_dimproduct(var);

    switch (type->subsort) {
    default: /* Fixed size atomic */
	size = NCD4_typesize(type->subsort);
	size *= dimproduct;
        offset += size;
	break;
    case NC_STRING:
	if((flags & HASSTR) == 0) {/* oob'd out */
            size = sizeof(char*);
	    size *= dimproduct;
	    offset += size;
	} else {
	    int i;
	    for(i=0;i<dimproduct;i++) {
 	        d4size_t count = GETCOUNTER(offset);
	        SKIPCOUNTER(offset);
	        offset += count;
		size += COUNTSIZE+count;
	    }
	}
  	break;
    case NC_OPAQUE:
	if((flags & (HASOPFIX|HASOPVAR)) == 0) {/* oob'd out */
            size = (var->opaque.size == 0 ? sizeof(nc_vlen_t) : var->opaque.size);
	    size *= dimproduct;
	    offset += size;
	} else {
	    for(i=0;i<dimproduct;i++) {
 	        d4size_t count = GETCOUNTER(offset);
	        SKIPCOUNTER(offset);
	        offset += count;
		size += COUNTSIZE+count;
	    }
	}
  	break;
    case NC_ENUM:
	size = NCD4_typesize(type->basetype->subsort);
	size *= dimproduct;
	offset += size;
        break;
    case NC_STRUCT:
        for(i=0;i<nclistlength(var->vars);i++) {
            NCD4node* field = (NCD4node*)nclistget(var->vars,i);
	    d4size_t fieldsize = 0;	
	    void* fieldoffset = offset;
	    fieldsize = computeInstanceSize(builder,field,flags,&fieldoffset);
	    size += fieldsize;
	    offset = fieldoffset;
        }
        break;
    case NC_SEQ:
	if((flags & HASSEQ) == 0) {/* oob'd out */
            size = sizeof(nc_vlen_t);
	    size *= dimproduct;
	    offset += size;
	} else {
	    int j,k;
	    d4size_t nrecords;
	    d4size_t seqsize = 0;
	    void* seqpos = offset;
	    for(i=0;i<dimproduct;i++) {	    
 	        nrecords = GETCOUNTER(seqpos);
	        SKIPCOUNTER(seqpos);
	        seqsize += COUNTSIZE;
	        for(j=0;j<nrecords;j++) {
                    for(k=0;k<nclistlength(type->vars);k++) {
			NCD4node* field = (NCD4node*)nclistget(type->vars,k);
		        d4size_t fieldsize = 0;	
	                void* fieldoffset = seqpos;
	                fieldsize = computeInstanceSize(builder,field,flags,&fieldoffset);
	                seqsize += fieldsize;
	                seqpos = fieldoffset;
		    }
		}
	    }
	    size = seqsize;
	    offset = seqpos;
        }
	break;
    }
    assert((offset - *offsetp) == size);
    *offsetp = offset;
    return size;
}
