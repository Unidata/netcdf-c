/*********************************************************************
 *   Copyright 2009, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#include "includes.h"
#include "nc_iter.h"
#include "nclog.h"

#ifdef ENABLE_BINARY

static const char zeros[] =
    "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";

/* It is helpful to have a structure that contains memory and an offset */
typedef struct Reclaim {char* memory; ptrdiff_t offset;} Reclaim;

/* Forward */

static ptrdiff_t read_align(ptrdiff_t offset, size_t alignment);
static void write_align(int alignment, Bytebuffer* buf);
static void genbin2_defineattr(Symbol* asym);
static int genbin2_definevardata(Symbol* vsym);
static void genbin2_deftype(Symbol* tsym);
static void genbin2_definespecialattributes(Symbol* var);
static int genbin2_generate_data(Datalist* data, Symbol* tsym, Datalist* fillvalue, Bytebuffer* databuf);
static int genbin2_generate_data_r(NCConstant* instance, Symbol* tsym, Datalist* fillvalue, Bytebuffer* databuf);

static int genbin2_reclaim_data(Symbol* tsym, void* memory, size_t count);
static int genbin2_reclaim_datar(Symbol* tsym, Reclaim* reclaim);
static int genbin2_reclaim_usertype(Symbol* tsym, Reclaim* reclaim);
static int genbin2_reclaim_compound(Symbol* tsym, Reclaim* reclaim);
static int genbin2_reclaim_vlen(Symbol* tsym, Reclaim* reclaim);
static int genbin2_reclaim_enum(Symbol* tsym, Reclaim* reclaim);
static int genbin2_reclaim_opaque(Symbol* tsym, Reclaim* reclaim);

/**************************************************/

/**
Walk a Symbol* tree representing the metadata and data
and invoke the corresponding API calls to create the file

@return netcdf error code
*/

void
genbin2_netcdf(void)
{
    int stat, ncid;
    int idim, ivar, iatt;
    int ndims, nvars, natts, ngatts;

#ifdef USE_NETCDF4
    int ntyps, ngrps, igrp;
#endif

    ndims = listlength(dimdefs);
    nvars = listlength(vardefs);
    natts = listlength(attdefs);
    ngatts = listlength(gattdefs);
#ifdef USE_NETCDF4
    ntyps = listlength(typdefs);
    ngrps = listlength(grpdefs);
#endif /*USE_NETCDF4*/

#ifdef LOGGING
    /* Turn on logging */
    nc_set_log_level(ncloglevel);
#endif

    /* create netCDF file, uses NC_CLOBBER mode */
    cmode_modifier |= NC_CLOBBER;
#ifdef USE_NETCDF4
    if(!usingclassic)
        cmode_modifier |= NC_NETCDF4;
#endif

    stat = nc_create(rootgroup->file.filename, cmode_modifier, &ncid);
    check_err(stat,__LINE__,__FILE__);

    /* ncid created above is also root group*/
    rootgroup->nc_id = ncid;

#ifdef USE_NETCDF4
    /* Define the group structure */
    /* walking grdefs list will do a preorder walk of all defined groups*/
    for(igrp=0;igrp<ngrps;igrp++) {
        Symbol* gsym = (Symbol*)listget(grpdefs,igrp);
        if(gsym == rootgroup) continue; /* ignore root group*/
        stat = nc_def_grp(gsym->container->nc_id,gsym->name,&gsym->nc_id);
        check_err(stat,__LINE__,__FILE__);
    }
#endif

#ifdef USE_NETCDF4
    /* Define the types*/
    if (ntyps > 0) {
        int ityp;
        for(ityp = 0; ityp < ntyps; ityp++) {
            Symbol* tsym = (Symbol*)listget(typdefs,ityp);
            genbin2_deftype(tsym);
        }
    }
#endif

    /* define dimensions from info in dims array */
    if (ndims > 0) {
        for(idim = 0; idim < ndims; idim++) {
            Symbol* dsym = (Symbol*)listget(dimdefs,idim);
            stat = nc_def_dim(dsym->container->nc_id,
                              dsym->name,
                              (dsym->dim.isunlimited?NC_UNLIMITED:dsym->dim.declsize),
                              &dsym->nc_id);
            check_err(stat,__LINE__,__FILE__);
       }
    }

    /* define variables from info in vars array */
    if (nvars > 0) {
        for(ivar = 0; ivar < nvars; ivar++) {
            Symbol* vsym = (Symbol*)listget(vardefs,ivar);
            if (vsym->typ.dimset.ndims > 0) {   /* a dimensioned variable */
                /* construct a vector of dimension ids*/
                int dimids[NC_MAX_VAR_DIMS];
                for(idim=0;idim<vsym->typ.dimset.ndims;idim++)
                    dimids[idim] = vsym->typ.dimset.dimsyms[idim]->nc_id;
                stat = nc_def_var(vsym->container->nc_id,
                                  vsym->name,
                                  vsym->typ.basetype->nc_id,
                                  vsym->typ.dimset.ndims,
                                  dimids,
                                  &vsym->nc_id);
            } else { /* a scalar */
                stat = nc_def_var(vsym->container->nc_id,
                                  vsym->name,
                                  vsym->typ.basetype->nc_id,
                                  vsym->typ.dimset.ndims,
                                  NULL,
                                  &vsym->nc_id);
            }
            check_err(stat,__LINE__,__FILE__);
        }
    }

#ifdef USE_NETCDF4
    /* define special variable properties */
    if(nvars > 0) {
        for(ivar = 0; ivar < nvars; ivar++) {
            Symbol* var = (Symbol*)listget(vardefs,ivar);
            genbin2_definespecialattributes(var);
        }
    }
#endif /*USE_NETCDF4*/

/* define global attributes */
    if(ngatts > 0) {
        for(iatt = 0; iatt < ngatts; iatt++) {
            Symbol* gasym = (Symbol*)listget(gattdefs,iatt);
            genbin2_defineattr(gasym);
        }
    }

    /* define per-variable attributes */
    if(natts > 0) {
        for(iatt = 0; iatt < natts; iatt++) {
            Symbol* asym = (Symbol*)listget(attdefs,iatt);
            genbin2_defineattr(asym);
        }
    }

    if (nofill_flag) {
        stat = nc_set_fill(rootgroup->nc_id, NC_NOFILL, 0);
        check_err(stat,__LINE__,__FILE__);
    }

    /* leave define mode */
    stat = nc_enddef(rootgroup->nc_id);
    check_err(stat,__LINE__,__FILE__);

    if(!header_only) {
        /* Load values into those variables with defined data */
        if(nvars > 0) {
            for(ivar = 0; ivar < nvars; ivar++) {
                Symbol* vsym = (Symbol*)listget(vardefs,ivar);
                if(vsym->data != NULL) {
                    genbin2_definevardata(vsym);
                }
            }
        }
    }
}

void
genbin2_close(void)
{
    int stat;
    stat = nc_close(rootgroup->nc_id);
    check_err(stat,__LINE__,__FILE__);
}

static void
genbin2_defineattr(Symbol* asym)
{
    int stat = NC_NOERR;
    Bytebuffer* databuf = bbNew();
    char* data;
    size_t len = datalistlen(asym->data);
    int grpid = asym->container->nc_id;
    int varid = (asym->att.var == NULL?NC_GLOBAL : asym->att.var->nc_id);
    int typid = asym->typ.basetype->nc_id;
    stat = genbin2_generate_data(asym->data,asym->typ.basetype,NULL,databuf);
    check_err(stat,__LINE__,__FILE__);
    data = (char*)bbContents(databuf);
    stat = nc_put_att(grpid,varid,asym->name,typid,len,(void*)data);
    check_err(stat,__LINE__,__FILE__);
    stat = genbin2_reclaim_data(asym->typ.basetype,data,len);
    check_err(stat,__LINE__,__FILE__);
    bbFree(databuf);
}

static int
genbin2_definevardata(Symbol* vsym)
{
    int stat = NC_NOERR;
    Bytebuffer* databuf = bbNew();
    char* data;
    size_t len = datalistlen(vsym->data);
    int grpid = vsym->container->nc_id;
    int varid = vsym->nc_id;
    stat = genbin2_generate_data(vsym->data,vsym->typ.basetype,NULL,databuf);
    check_err(stat,__LINE__,__FILE__);
    data = (char*)bbContents(databuf);
    stat = nc_put_var(grpid,varid,(void*)data);
    check_err(stat,__LINE__,__FILE__);
    stat = genbin2_reclaim_data(vsym->typ.basetype,data,len);
    check_err(stat,__LINE__,__FILE__);

    bbFree(databuf);
    return stat;
}

#ifdef USE_NETCDF4

/*
Generate type definitions
*/
static void
genbin2_deftype(Symbol* tsym)
{
    unsigned long i;
    int stat;

    ASSERT(tsym->objectclass == NC_TYPE);
    switch (tsym->subclass) {
    case NC_PRIM: break; /* these are already taken care of*/
    case NC_OPAQUE:
        stat = nc_def_opaque(tsym->container->nc_id,
                             tsym->typ.size,
                             tsym->name,
                             &tsym->nc_id);
        check_err(stat,__LINE__,__FILE__);
        break;
    case NC_ENUM:
      {
        stat = nc_def_enum(tsym->container->nc_id,
                           tsym->typ.basetype->nc_id,
                           tsym->name,
                           &tsym->nc_id);
        check_err(stat,__LINE__,__FILE__);
        for(i=0;i<listlength(tsym->subnodes);i++) {
          Symbol* econst = (Symbol*)listget(tsym->subnodes,i);
          Bytebuffer* datum = bbNew();
   	  Datalist* ecdl = const2list(cloneconstant(econst->typ.econst));

          ASSERT(econst->subclass == NC_ECONST);
          stat = nc_insert_enum(tsym->container->nc_id,
                                tsym->nc_id,
                                econst->name,
                                bbContents(datum));
          check_err(stat,__LINE__,__FILE__);
	  bbFree(datum);
	  reclaimdatalist(ecdl);
        }
      }
      break;
    case NC_VLEN:
        stat = nc_def_vlen(tsym->container->nc_id,
                           tsym->name,
                           tsym->typ.basetype->nc_id,
                           &tsym->nc_id);
        check_err(stat,__LINE__,__FILE__);
        break;
    case NC_COMPOUND:
        stat = nc_def_compound(tsym->container->nc_id,
                               tsym->typ.size,
                               tsym->name,
                               &tsym->nc_id);
        check_err(stat,__LINE__,__FILE__);
        for(i=0;i<listlength(tsym->subnodes);i++) {
            Symbol* efield = (Symbol*)listget(tsym->subnodes,i);
            ASSERT(efield->subclass == NC_FIELD);
            if(efield->typ.dimset.ndims == 0){
                stat = nc_insert_compound(
                                tsym->container->nc_id,
                                tsym->nc_id,
                                efield->name,
                                efield->typ.offset,
                                efield->typ.basetype->nc_id);
            } else {
                int j;
                int dimsizes[NC_MAX_VAR_DIMS]; /* int because inside compound */
                /* Generate the field dimension constants*/
                for(j=0;j<efield->typ.dimset.ndims;j++) {
                     unsigned int size = efield->typ.dimset.dimsyms[j]->dim.declsize;
                     dimsizes[j] = size;
                }
                stat = nc_insert_array_compound(
                                tsym->container->nc_id,
                                tsym->nc_id,
                                efield->name,
                                efield->typ.offset,
                                efield->typ.basetype->nc_id,
                                efield->typ.dimset.ndims,
                                dimsizes);
            }
            check_err(stat,__LINE__,__FILE__);
        }
        break;
    default: panic("definectype: unexpected type subclass");
    }
}

static void
genbin2_definespecialattributes(Symbol* var)
{
    int stat = NC_NOERR;
    Specialdata* special = var->var.special;
    if(special->flags & _STORAGE_FLAG) {
	if(special->_Storage == NC_CONTIGUOUS) {
	    stat = nc_def_var_chunking(var->container->nc_id, var->nc_id, NC_CONTIGUOUS, NULL);
	} else { /* chunked */
	    if(special->nchunks == 0 || special->_ChunkSizes == NULL)
	        derror("NC_CHUNKED requested, but no chunksizes specified");
            stat = nc_def_var_chunking(var->container->nc_id, var->nc_id, NC_CHUNKED, special->_ChunkSizes);
	}
        check_err(stat,__LINE__,__FILE__);
    }
    if(special->flags & _FLETCHER32_FLAG) {
        stat = nc_def_var_fletcher32(var->container->nc_id,
                                     var->nc_id,
                                     special->_Fletcher32);
        check_err(stat,__LINE__,__FILE__);
    }
    if(special->flags & (_DEFLATE_FLAG | _SHUFFLE_FLAG)) {
        stat = nc_def_var_deflate(var->container->nc_id,
                                  var->nc_id,
                                  (special->_Shuffle == 1?1:0),
                                  (special->_DeflateLevel >= 0?1:0),
                                  (special->_DeflateLevel >= 0?special->_DeflateLevel
                                                              :0));
        check_err(stat,__LINE__,__FILE__);
    }
    if(special->flags & _ENDIAN_FLAG) {
        stat = nc_def_var_endian(var->container->nc_id,
                                 var->nc_id,
                                 (special->_Endianness == NC_ENDIAN_LITTLE?
                                        NC_ENDIAN_LITTLE
                                       :NC_ENDIAN_BIG));
        check_err(stat,__LINE__,__FILE__);
    }
    if(special->flags & _NOFILL_FLAG) {
        stat = nc_def_var_fill(var->container->nc_id,
                                 var->nc_id,
                                 (special->_Fill?NC_FILL:NC_NOFILL),
                                 NULL);
        check_err(stat,__LINE__,__FILE__);
    }
    if(special->flags & _FILTER_FLAG) {
        /* Special check for alternate way to specify _Deflate */
        if(special->_FilterID == ZIP_ID) {
            unsigned int level;
            if(special->nparams == 0 || special->_FilterParams == NULL)
                level = 9; /* default */
            else
                level = special->_FilterParams[0];
            if(level > 9)
                derror("Illegal deflate level");
            else {
                stat = nc_def_var_deflate(var->container->nc_id,
                        var->nc_id,
                        (special->_Shuffle == 1?1:0),
                        (level > 0?1:0),
                        level);
            }
        } else {
            stat = nc_def_var_filter(var->container->nc_id,
                        var->nc_id,
                        special->_FilterID,
                        special->nparams,
                        special->_FilterParams
                        );
        }
        check_err(stat,__LINE__,__FILE__);
    }
}

#endif /*USE_NETCDF4*/


/**************************************************/


/**
Walk a datalist using a type symbol as template. Generate
the corresponding binary data into the databuf.

@param data -- (Datalist*) to use to generate the binary data
@param tsym -- (Symbol*)  the top-level type for which instances are to be generated
@param fillvalue -- (Datalist*) the fillvalue for the toplevel type
@param databuf -- (Bytebuffer*) the buffer into which instances are to be stored
@return netcdf error code
*/

static int
genbin2_generate_data(Datalist* data, Symbol* tsym, Datalist* fillvalue, Bytebuffer* databuf)
{
    int stat = NC_NOERR;
    size_t count = data->length;
    size_t i;

    bbClear(databuf);
    for(i=0;i<count;i++) {
	NCConstant* instance = datalistith(data,i);
	if((stat = genbin2_generate_data_r(instance, tsym, fillvalue, databuf))) goto done;
    }
done:
    return stat;
}

/* Recursive helper that does the bulk of the work */
static int
genbin2_generate_data_r(NCConstant* instance, Symbol* tsym, Datalist* fillvalue, Bytebuffer* databuf)
{
    int stat = NC_NOERR;

    if(instance->nctype == NC_FILLVALUE) {
        /* replace with fillvalue for the type */
	Datalist* filllist = (fillvalue == NULL ? getfiller(tsym) : fillvalue);
	ASSERT(datalistlen(filllist)==1)
	instance = datalistith(filllist,0);
    }

    switch (tsym->subclass) {
    case NC_PRIM: {
	switch (tsym->nc_id) {
        case NC_CHAR: {
            char* p = NULL;
            NCConstant* tmp = nullconst();
            tmp->nctype = NC_CHAR;
            convert1(instance,tmp);
            p = &tmp->value.charv;;
            bbAppendn(databuf,p,sizeof(char));
            reclaimconstant(tmp);
            } break;
        case NC_BYTE: {
            signed char* p = NULL;
            NCConstant* tmp = nullconst();
            tmp->nctype = NC_BYTE;
            convert1(instance,tmp);
            p = &tmp->value.int8v;
            bbAppendn(databuf,p,sizeof(signed char));
            reclaimconstant(tmp);
            } break;
        case NC_UBYTE: {
            unsigned char* p = NULL;
            NCConstant* tmp = nullconst();
            tmp->nctype = NC_UBYTE;
            convert1(instance,tmp);
            p = &tmp->value.uint8v;
            bbAppendn(databuf,p,sizeof(unsigned char));
            reclaimconstant(tmp);
            } break;
        case NC_SHORT: {
            short* p = NULL;
            NCConstant* tmp = nullconst();
            tmp->nctype = NC_SHORT;
            convert1(instance,tmp);
            p = &tmp->value.int16v;
            bbAppendn(databuf,p,sizeof(short));
            reclaimconstant(tmp);
            } break;
        case NC_USHORT: {
            unsigned short* p = NULL;
            NCConstant* tmp = nullconst();
            tmp->nctype = NC_USHORT;
            convert1(instance,tmp);
            p = &tmp->value.uint16v;
            bbAppendn(databuf,p,sizeof(unsigned short));
            reclaimconstant(tmp);
            } break;
        case NC_INT: {
            int* p = NULL;
            NCConstant* tmp = nullconst();
            tmp->nctype = NC_INT;
            convert1(instance,tmp);
            p = &tmp->value.int32v;
            bbAppendn(databuf,p,sizeof(int));
            reclaimconstant(tmp);
            } break;
        case NC_UINT: {
            unsigned int* p = NULL;
            NCConstant* tmp = nullconst();
            tmp->nctype = NC_UINT;
            convert1(instance,tmp);
            p = &tmp->value.uint32v;
            bbAppendn(databuf,p,sizeof(unsigned int));
            reclaimconstant(tmp);
            } break;
        case NC_INT64: {
            long long* p = NULL;
            NCConstant* tmp = nullconst();
            tmp->nctype = NC_INT64;
            convert1(instance,tmp);
            p = &tmp->value.int64v;
            bbAppendn(databuf,p,sizeof(long long));
            reclaimconstant(tmp);
            } break;
        case NC_UINT64: {
            unsigned long long* p = NULL;
            NCConstant* tmp = nullconst();
            tmp->nctype = NC_UINT64;
            convert1(instance,tmp);
            p = &tmp->value.uint64v;
            bbAppendn(databuf,p,sizeof(unsigned long long));
            reclaimconstant(tmp);
            } break;
        case NC_FLOAT: {
            float* p = NULL;
            NCConstant* tmp = nullconst();
            tmp->nctype = NC_FLOAT;
            convert1(instance,tmp);
            p = &tmp->value.floatv;
            bbAppendn(databuf,p,sizeof(float));
            reclaimconstant(tmp);
            } break;
        case NC_DOUBLE: {
            double* p = NULL;
            NCConstant* tmp = nullconst();
            tmp->nctype = NC_DOUBLE;
            convert1(instance,tmp);
            p = &tmp->value.doublev;
            bbAppendn(databuf,p,sizeof(double));
            reclaimconstant(tmp);
            } break;
        case NC_STRING: {
            char* p = NULL;
            NCConstant* tmp = nullconst();
            tmp->nctype = NC_STRING;
            convert1(instance,tmp);
            p = emalloc(tmp->value.stringv.len+1);
	    memcpy(p,tmp->value.stringv.stringv,tmp->value.stringv.len);
	    p[tmp->value.stringv.len] = '\0';
            bbAppendn(databuf,&p,sizeof(char*));
            reclaimconstant(tmp);
            } break;
	default: stat = NC_EINTERNAL; goto done; /* Should never happen */
	} break; /*switch*/
	} break; /*NC_PRIM*/
	
    case NC_ENUM: {
	Symbol* basetype = tsym->typ.basetype;
	/* Pretend */
	stat = genbin2_generate_data_r(instance,basetype,fillvalue,databuf);
        } break;
    case NC_OPAQUE: {
	unsigned char* bytes = NULL;
	size_t len = 0;
	if(instance->nctype != NC_OPAQUE)
	    {stat = NC_EBADTYPE; goto done;}
	/* Assume the opaque string has been normalized */
        bytes=makebytestring(instance->value.opaquev.stringv,&len);
	if(bytes == NULL) {stat = NC_ENOMEM; goto done;}
        bbAppendn(databuf,(void*)bytes,len);
	free(bytes);
        } break;
    case NC_VLEN: {
	Datalist* sublist = NULL;
	Bytebuffer* vlendata = NULL;
	nc_vlen_t p;
	if(instance->nctype != NC_COMPOUND) {
	    nclog(NCLOGERR,"Translating vlen: expected sublist");
	    stat = NC_EBADTYPE; goto done;
	}
	sublist = instance->value.compoundv;
	vlendata = bbNew();
	if((stat = genbin2_generate_data(sublist,tsym->typ.basetype,NULL,vlendata))) goto done;
	p.len = datalistlen(sublist);
	p.p = bbContents(vlendata);
        bbAppendn(databuf,(char*)&p,sizeof(nc_vlen_t));
        } break;
    case NC_COMPOUND: { /* The really hard one */
	size_t nfields, fid, i;
	Datalist* cmpd = instance->value.compoundv;
        write_align(tsym->typ.cmpdalign,databuf);
        /* Get info about each field in turn and build it*/
        nfields = listlength(tsym->subnodes);
        for(fid=0;fid<nfields;fid++) {
	    Symbol* field = listget(tsym->subnodes,fid);
	    NCConstant* fieldinstance = datalistith(cmpd,fid);
	    int ndims = field->typ.dimset.ndims;
	    size_t arraycount;
	    if(ndims == 0) {
	        ndims=1; /* fake the scalar case */
	    }
  	    /* compute the total number of elements in the field array */
	    arraycount = 1;
	    for(i=0;i<ndims;i++) arraycount *= field->typ.dimset.dimsyms[i]->dim.declsize;
	    write_align(field->typ.alignment,databuf);
	    /* Write the instances */
	    for(i=0;i<arraycount;i++) {
	        if((stat = genbin2_generate_data_r(fieldinstance, field->typ.basetype, NULL, databuf))) goto done;
	    }
	}		
        } break;

    default: stat = NC_EINTERNAL; goto done; /* Should never happen */
    }
done:
    return stat;
}

/**************************************************/
/**
Internal equivalent of ncaux_reclaim_data.
*/

static int
genbin2_reclaim_data(Symbol* tsym, void* memory, size_t count)
{
    int stat = NC_NOERR;
    size_t i;
    Reclaim reclaimer;
    
    if(tsym == NULL
       || (memory == NULL && count > 0))
        {stat = NC_EINVAL; goto done;}
    if(memory == NULL || count == 0)
        goto done; /* ok, do nothing */
    reclaimer.offset = 0;
    reclaimer.memory = memory;
    for(i=0;i<count;i++) {
	if((stat=genbin2_reclaim_datar(tsym,&reclaimer))) /* reclaim one instance */
	    break;
    }
done:
    return stat;
}

/* Recursive type walker: reclaim a single instance */
static int
genbin2_reclaim_datar(Symbol* tsym, Reclaim* reclaimer)
{
    int stat = NC_NOERR;
    
    switch  (tsym->nc_id) {
    case NC_CHAR: case NC_BYTE: case NC_UBYTE:
    case NC_SHORT: case NC_USHORT:
    case NC_INT: case NC_UINT: case NC_FLOAT:
    case NC_INT64: case NC_UINT64: case NC_DOUBLE:
        reclaimer->offset += tsym->typ.size;
	break;
#ifdef USE_NETCDF4
    case NC_STRING: {
	char** sp = (char**)(reclaimer->memory+reclaimer->offset);
        /* Need to reclaim string */
	if(*sp != NULL) efree(*sp);
	reclaimer->offset += tsym->typ.size;
	} break;
    default:
    	/* reclaim a user type */
	stat = genbin2_reclaim_usertype(tsym,reclaimer);
#else
    default:
	stat = NC_ENOTNC4;
#endif
	break;
    }
    return stat;
}
	
static int
genbin2_reclaim_usertype(Symbol* tsym, Reclaim* reclaimer)
{
    int stat = NC_NOERR;

    /* Get info about the xtype */
    switch (tsym->subclass) {
    case NC_OPAQUE: stat = genbin2_reclaim_opaque(tsym,reclaimer); break;
    case NC_ENUM: stat = genbin2_reclaim_enum(tsym,reclaimer); break;
    case NC_VLEN: stat = genbin2_reclaim_vlen(tsym,reclaimer); break;
    case NC_COMPOUND: stat = genbin2_reclaim_compound(tsym,reclaimer); break;
    default:
        stat = NC_EINVAL;
	break;
    }
    return stat;
}

static int
genbin2_reclaim_vlen(Symbol* tsym, Reclaim* reclaimer)
{
    int stat = NC_NOERR;
    size_t i;
    Symbol* basetype = tsym->typ.basetype;
    nc_vlen_t* vl = (nc_vlen_t*)(reclaimer->memory+reclaimer->offset);

    /* Free up each entry in the vlen list */
    if(vl->p != NULL) {
	Reclaim vreclaimer;
	vreclaimer.memory = vl->p;
	vreclaimer.offset = 0;
        for(i=0;i<vl->len;i++) {
	    vreclaimer.offset = read_align(vreclaimer.offset,basetype->typ.alignment);
	    if((stat = genbin2_reclaim_datar(basetype,&vreclaimer))) goto done;
	    vreclaimer.offset += basetype->typ.size;
	}
	reclaimer->offset += tsym->typ.size;
	efree(vl->p);
    }
done:
    return stat;
}

static int
genbin2_reclaim_enum(Symbol* tsym, Reclaim* reclaimer)
{
    return genbin2_reclaim_datar(tsym->typ.basetype,reclaimer);
}

static int
genbin2_reclaim_opaque(Symbol* tsym, Reclaim* reclaimer)
{
    /* basically a fixed size sequence of bytes */
    reclaimer->offset += tsym->typ.size;
    return NC_NOERR;
}

static int
genbin2_reclaim_compound(Symbol* tsym, Reclaim* reclaimer)
{
    int stat = NC_NOERR;
    int nfields;
    size_t fid, i, arraycount;
    ptrdiff_t saveoffset;

    reclaimer->offset = read_align(reclaimer->offset,tsym->typ.cmpdalign);
    saveoffset = reclaimer->offset;

    /* Get info about each field in turn and reclaim it */
    nfields = listlength(tsym->subnodes);
    for(fid=0;fid<nfields;fid++) {
	Symbol* field = listget(tsym->subnodes,fid);
	int ndims = field->typ.dimset.ndims;
	/* compute the total number of elements in the field array */
	for(i=0;i<ndims;i++) arraycount *= field->typ.dimset.dimsyms[i]->dim.declsize;
	reclaimer->offset = read_align(reclaimer->offset,field->typ.alignment);
	for(i=0;i<arraycount;i++) {
	    if((stat = genbin2_reclaim_datar(field->typ.basetype, reclaimer))) goto done;
	}		
    }
    reclaimer->offset = saveoffset;
    reclaimer->offset += tsym->typ.size;
done:
    return stat;
}

static ptrdiff_t
read_align(ptrdiff_t offset, size_t alignment)
{
    size_t delta = (offset % alignment);
    if(delta == 0) return offset;
    return offset + (alignment - delta);
}

static void
write_align(int alignment, Bytebuffer* buf)
{
    int pad = 0;
    ptrdiff_t offset = bbLength(buf);
    pad = getpadding(offset,alignment);
    if(pad > 0) {
	bbAppendn(buf,(void*)zeros,pad);
    }
}

#endif /*ENABLE_BINARY*/

