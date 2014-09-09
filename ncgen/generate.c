/*********************************************************************
 *   Copyright 2009, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#include "includes.h"
#include "nc_iter.h"
#include "odom.h"
#include "offsets.h"

/**************************************************/
/* Code for generating data lists*/
/**************************************************/
/* For datalist constant rules: see the rules on the man page */

/* Forward*/
static void generate_array(Symbol*,Bytebuffer*,Datalist*,Generator*,Writer);
static void generate_arrayr(Symbol*,Bytebuffer*,Datalist*,Odometer*,int,Datalist*,Generator*);
static void generate_primdata(Symbol*, NCConstant*, Bytebuffer*, Datalist* fillsrc, Generator*);
static void generate_fieldarray(Symbol*, NCConstant*, Dimset*, Bytebuffer*, Datalist* fillsrc, Generator*);

/* Mnemonics */
#define VLENLIST1
#define FIELDARRAY 1

/**************************************************/
/* Generator general procedures */

int
generator_getstate(Generator* generator ,void** statep)
{
    if(statep) *statep = (void*)generator->state;
    return 1;
}

int generator_reset(Generator* generator, void* state)
{
    generator->state = state;
    return 1;
}

#ifdef IGNORe
static void
checkodom(Odometer* odom)
{
    int i;
    for(i=0;i<odom->rank;i++) {
	ASSERT(odom->index[i] == odom->start[i]+odom->count[i]);
    }
}
#endif

/**************************************************/

void
generate_attrdata(Symbol* asym, Generator* generator, Writer writer, Bytebuffer* codebuf)
{
    Symbol* basetype = asym->typ.basetype;
    nc_type typecode = basetype->typ.typecode;

   if(typecode == NC_CHAR) {
	gen_charattr(asym->data,codebuf);
    } else {
	int uid;
	size_t count;
        generator->listbegin(generator,LISTATTR,asym->data->length,codebuf,&uid);
        for(count=0;count<asym->data->length;count++) {
        NCConstant* con = datalistith(asym->data,count);
	    generator->list(generator,LISTATTR,uid,count,codebuf);
            generate_basetype(asym->typ.basetype,con,codebuf,NULL,generator);
	}
        generator->listend(generator,LISTATTR,uid,count,codebuf);
    }
    writer(generator,asym,codebuf,0,NULL,NULL);
}

void
generate_vardata(Symbol* vsym, Generator* generator, Writer writer, Bytebuffer* code)
{
    Dimset* dimset = &vsym->typ.dimset;
    int rank = dimset->ndims;
    Symbol* basetype = vsym->typ.basetype;
    Datalist* filler = getfiller(vsym);

    if(vsym->data == NULL) return;

    /* give the buffer a running start to be large enough*/
    if(!bbSetalloc(code, nciterbuffersize))
      return;

    if(rank == 0) {/*scalar case*/
        NCConstant* c0 = datalistith(vsym->data,0);
        generate_basetype(basetype,c0,code,filler,generator);
        writer(generator,vsym,code,0,NULL,NULL);
    } else {/*rank > 0*/
	generate_array(vsym,code,filler,generator,writer);
        writer(generator,vsym,code,0,NULL,NULL);
    }
}

static void
generate_array(Symbol* vsym,
               Bytebuffer* code,
               Datalist* filler,
               Generator* generator,
	       Writer writer
              )
{
    Dimset* dimset = &vsym->typ.dimset;
    int rank = dimset->ndims;
    Symbol* basetype = vsym->typ.basetype;
    nc_type typecode = basetype->typ.typecode;
    Odometer* odom;
    nciter_t iter;

    ASSERT(rank > 0);

    /* First, create an odometer using all of the dimensions */
    odom = newodometer(dimset,NULL,NULL);
    /* recurse using the helper function */
    generate_arrayr(vsym,code,vsym->data,odom,0,filler,generator);
}

static void
generate_arrayr(Symbol* vsym,
               Bytebuffer* code,
               Datalist* list,
	       Odometer* odom,
               int dimindex,
               Datalist* filler,
               Generator* generator
              )
{
    Dimset* dimset = &vsym->typ.dimset;
    int rank = dimset->ndims;
    Symbol* basetype = vsym->typ.basetype;
    nc_type typecode = basetype->typ.typecode;

    int lastunlim = findlastunlimited(dimset);
    /* If no last unlim, then use last dimension */
    if(lastunlim == rank)
	lastunlim = rank - 1;

    if(lastunlim > dimindex) {
	/* If we have not reached the last unlimited, then just
	   iterate and recurse
	*/
	/* Iterate over this odometer's dimension */
        Odometer* thisodom = newsubodometer(odom,dimset,dimindex,dimindex);
	while(odometermore(thisodom)) {
	    NCConstant* sublist;
	    size_t offset = odometeroffset(thisodom);
	    /* Get the offset'th datalist */
	    sublist = datalistith(list,offset);
	    if(isnilconst(sublist)) {
		/* list is too short */
		semerror(constline(sublist),"Datalist is shorter than corresponding dimension");
	        return;
	    }	    
	    if(!islistconst(sublist)) {
		semerror(constline(sublist),"Expected data list {...}, constant found");
		return;
	    }
            /* recurse using the helper function */
            generate_arrayr(vsym,code,const2list(sublist),odom,dimindex+1,filler,generator);
	    odometerincr(thisodom);	
	}
    } else if(typecode != NC_CHAR) {/* we are at last unlimited (or last dimension) and variable is not  char */
	int uid,i;
	/* build a special odometer to walk the last few dimensions */
        Odometer* slabodom = newsubodometer(odom,dimset,dimindex,rank);
	/* compute the starting offset in our datalist
	   (Assumes that slabodom->index[i] == slabodom->start[i])
        */
        generator->listbegin(generator,LISTDATA,list->length,code,&uid);
	for(i=0;odometermore(slabodom);i++) {
	    size_t offset = odometeroffset(slabodom);
            NCConstant* con = datalistith(list,offset);
#ifdef USE_NOFILL
	    if(nofill_flag && con == NULL)
		break;
#endif
            generator->list(generator,LISTDATA,uid,i,code);
            generate_basetype(basetype,con,code,filler,generator);
	    odometerincr(slabodom);
        }
        generator->listend(generator,LISTDATA,uid,i,code);
	odometerfree(slabodom);
    } else {/* we are at last unlimited (or last dim) and variable is char */
	Bytebuffer* charbuf = bbNew();
	/*
	Action: just concat/pad any char constants and fill to
	dimension cross product.
        */
	gen_leafchararray(dimset,lastunlim,list,charbuf,filler);
	/* generate a language specific character constant */
	generator->charconstant(generator,code,charbuf);
	bbFree(charbuf);
#if 0
	/* dump to output */
        odom = newodometer(dimset,NULL,NULL);
        writer(generator,vsym,code,odom->rank,odom->start,odom->count);
#endif
    }
}

/* Generate an instance of the basetype */
void
generate_basetype(Symbol* tsym, NCConstant* con, Bytebuffer* codebuf, Datalist* filler, Generator* generator)
{
    Datalist* data;

    switch (tsym->subclass) {

    case NC_ENUM:
    case NC_OPAQUE:
    case NC_PRIM:
	if(islistconst(con)) {
	    semerror(constline(con),"Expected primitive found {..}");
	}
	generate_primdata(tsym,con,codebuf,filler,generator);
	break;

    case NC_COMPOUND: {
	int i,uid, nfields, dllen;
	if(con == NULL || isfillconst(con)) {
	    Datalist* fill = (filler==NULL?getfiller(tsym):filler);
	    ASSERT(fill->length == 1);
	    con = &fill->data[0];
	    if(!islistconst(con))
	        semerror(con->lineno,"Compound data fill value is not enclosed in {..}");
	}
	if(!islistconst(con)) {/* fail on no compound*/
	    semerror(constline(con),"Compound data must be enclosed in {..}");
        }
	data = con->value.compoundv;
        nfields = listlength(tsym->subnodes);
	dllen = datalistlen(data);
	if(dllen > nfields) {
	    semerror(con->lineno,"Datalist longer than the number of compound fields");
	    break;
	}
	generator->listbegin(generator,LISTCOMPOUND,listlength(tsym->subnodes),codebuf,&uid);
        for(i=0;i<nfields;i++) {
            Symbol* field = (Symbol*)listget(tsym->subnodes,i);
	    con = datalistith(data,i);
	    generator->list(generator,LISTCOMPOUND,uid,i,codebuf);
            generate_basetype(field,con,codebuf,NULL,generator);
	}
	generator->listend(generator,LISTCOMPOUND,uid,i,codebuf);
	} break;

    case NC_VLEN: {
	Bytebuffer* vlenbuf;
        int uid;
	size_t count;

	if(con == NULL || isfillconst(con)) {
	    Datalist* fill = (filler==NULL?getfiller(tsym):filler);
	    ASSERT(fill->length == 1);
	    con = &fill->data[0];
	    if(con->nctype != NC_COMPOUND) {
	        semerror(con->lineno,"Vlen data fill value is not enclosed in {..}");
	    }
	}

	if(!islistconst(con)) {
	    semerror(constline(con),"Vlen data must be enclosed in {..}");
        }
        data = con->value.compoundv;
        /* generate the nc_vlen_t instance*/
	vlenbuf = bbNew();
	if(tsym->typ.basetype->typ.typecode == NC_CHAR) {
	    gen_charvlen(data,vlenbuf);
	    generator->vlenstring(generator,vlenbuf,&uid,&count);
	} else {
    	    generator->listbegin(generator,LISTVLEN,data->length,codebuf,&uid);
            for(count=0;count<data->length;count++) {
	      NCConstant* con;
   	        generator->list(generator,LISTVLEN,uid,count,vlenbuf);
		con = datalistith(data,count);
                generate_basetype(tsym->typ.basetype,con,vlenbuf,NULL,generator);
	    }
   	    generator->listend(generator,LISTVLEN,uid,count,codebuf,(void*)vlenbuf);
	}
	generator->vlendecl(generator,codebuf,tsym,uid,count,vlenbuf);
	bbFree(vlenbuf);
        } break;

    case NC_FIELD:
	if(tsym->typ.dimset.ndims > 0) {
	    /* Verify that we have a sublist (or fill situation) */
	    if(con != NULL && !isfillconst(con) && !islistconst(con))
		semerror(constline(con),"Dimensioned fields must be enclose in {...}");
            generate_fieldarray(tsym->typ.basetype,con,&tsym->typ.dimset,codebuf,filler,generator);
	} else {
	    generate_basetype(tsym->typ.basetype,con,codebuf,NULL,generator);
	}
	break;

    default: PANIC1("generate_basetype: unexpected subclass %d",tsym->subclass);
    }
}

/* Used only for structure field arrays*/
static void
generate_fieldarray(Symbol* basetype, NCConstant* con, Dimset* dimset,
		 Bytebuffer* codebuf, Datalist* filler, Generator* generator)
{
    int i;
    int chartype = (basetype->typ.typecode == NC_CHAR);
    Datalist* data;

    ASSERT(dimset->ndims > 0);

    if(con != NULL && !isfillconst(con))
        data = con->value.compoundv;
    else
	data = NULL;

    if(chartype) {
	/* Collect the char field in a separate buffer */
	Bytebuffer* charbuf = bbNew();
        gen_chararray(dimset,data,charbuf,filler);
	generator->charconstant(generator,codebuf,charbuf);
	bbFree(charbuf);
    } else {
	int uid;
	size_t xproduct = crossproduct(dimset,0,0); /* compute total number of elements */
        generator->listbegin(generator,LISTFIELDARRAY,xproduct,codebuf,&uid);
        for(i=0;i<xproduct;i++) {
	    con = (data == NULL ? NULL : datalistith(data,i));
	    generator->list(generator,LISTFIELDARRAY,uid,i,codebuf);
            generate_basetype(basetype,con,codebuf,NULL,generator);
	}
        generator->listend(generator,LISTFIELDARRAY,uid,i,codebuf);
    }
}


/* An opaque string value might not conform
   to the size of the opaque to which it is being
   assigned. Normalize it to match the required
   opaque length (in bytes).
   Note that the string is a sequence of nibbles (4 bits).
*/
static void
normalizeopaquelength(NCConstant* prim, unsigned long nbytes)
{
    int nnibs = 2*nbytes;
    ASSERT(prim->nctype==NC_OPAQUE);
    if(prim->value.opaquev.len == nnibs) {
        /* do nothing*/
    } else if(prim->value.opaquev.len > nnibs) { /* truncate*/
	prim->value.opaquev.stringv[nnibs] = '\0';
	prim->value.opaquev.len = nnibs;
    } else {/* prim->value.opaquev.len < nnibs => expand*/
        char* s;
	s = (char*)emalloc(nnibs+1);
	memset(s,'0',nnibs);    /* Fill with '0' characters */
	memcpy(s,prim->value.opaquev.stringv,prim->value.opaquev.len);
	s[nnibs] = '\0';
	efree(prim->value.opaquev.stringv);
	prim->value.opaquev.stringv=s;
	prim->value.opaquev.len = nnibs;
    }
}

static void
generate_primdata(Symbol* basetype, NCConstant* prim, Bytebuffer* codebuf,
		  Datalist* filler, Generator* generator)
{
    NCConstant target;
    int match;

    if(prim == NULL || isfillconst(prim)) {
	Datalist* fill = (filler==NULL?getfiller(basetype):filler);
	ASSERT(fill->length == 1);
	prim = datalistith(fill,0);
    }

    ASSERT(prim->nctype != NC_COMPOUND);

    /* Verify that the constant is consistent with the type */
    match = 1;
    switch (prim->nctype) {
    case NC_CHAR:
    case NC_BYTE:
    case NC_SHORT:
    case NC_INT:
    case NC_FLOAT:
    case NC_DOUBLE:
    case NC_UBYTE:
    case NC_USHORT:
    case NC_UINT:
    case NC_INT64:
    case NC_UINT64:
    case NC_STRING:
	match = (basetype->subclass == NC_PRIM ? 1 : 0);
	break;

#ifdef USE_NETCDF4
    case NC_NIL:
	match = (basetype->subclass == NC_PRIM && basetype->typ.typecode == NC_STRING ? 1 : 0);
	break;

    case NC_OPAQUE:
	/* OPAQUE is also consistent with numbers */
	match = (basetype->subclass == NC_OPAQUE
		 || basetype->subclass == NC_PRIM ? 1 : 0);
	break;
    case NC_ECONST:
	match = (basetype->subclass == NC_ENUM ? 1 : 0);
	if(match) {
	    /* Make sure this econst belongs to this enum */
	    Symbol* ec = prim->value.enumv;
	    Symbol* en = ec->container;
	    match = (en == basetype);
	}
	break;
#endif
    default:
	match = 0;
    }
    if(!match) {
        semerror(constline(prim),"Data value is not consistent with the expected type: %s",
		 basetype->name);
    }

    target.nctype = basetype->typ.typecode;

    if(target.nctype != NC_ECONST) {
	convert1(prim,&target);
    }

    switch (target.nctype) {
    case NC_ECONST:
        if(basetype->subclass != NC_ENUM) {
	    semerror(constline(prim),"Conversion to enum not supported (yet)");
	} break;
     case NC_OPAQUE:
	normalizeopaquelength(&target,basetype->typ.size);
	break;
    default:
	break;
    }
    generator->constant(generator,&target,codebuf);

    return;
}
