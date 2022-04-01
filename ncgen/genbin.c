/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *   $Header: /upc/share/CVS/netcdf-3/ncgen/genbin.c,v 1.4 2010/05/27 21:34:17 dmh Exp $
 *********************************************************************/

#include "includes.h"
#include <ctype.h>      /* for isprint() */
#include "netcdf_aux.h"
#include "netcdf_filter.h"

#ifdef ENABLE_BINARY

#undef TRACE

/* Forward*/
static int genbin_defineattr(Symbol* asym);
static int genbin_definevardata(Symbol* vsym);
static int  genbin_write(Generator*,Symbol*,Bytebuffer*,int,size_t*,size_t*);
static int genbin_writevar(Generator*,Symbol*,Bytebuffer*,int,size_t*,size_t*);
static int genbin_writeattr(Generator*,Symbol*,Bytebuffer*,int,size_t*,size_t*);
#ifdef USE_NETCDF4
static int genbin_deftype(Symbol* tsym);
static int genbin_definespecialattributes(Symbol* var);
#endif

/*
 * Generate C code for creating netCDF from in-memory structure.
 */
void
genbin_netcdf(void)
{
    int stat, ncid;
    int idim, ivar, iatt;
    int ndims, nvars, natts, ngatts;
    const char* filename = rootgroup->file.filename;

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

    /* Turn on logging */
#ifdef LOGGING
    nc_set_log_level(ncloglevel);
#endif

    /* create netCDF file, uses NC_CLOBBER mode */
    cmode_modifier |= NC_CLOBBER;
#ifdef USE_NETCDF4
    if(!usingclassic)
        cmode_modifier |= NC_NETCDF4;
#endif

    stat = nc_create(filename, cmode_modifier, &ncid);
    CHECK_ERR(stat);

    /* ncid created above is also root group*/
    rootgroup->nc_id = ncid;

    if (nofill_flag) {
        stat = nc_set_fill(rootgroup->nc_id, NC_NOFILL, 0);
        CHECK_ERR(stat);
    }

#ifdef USE_NETCDF4
    /* Define the group structure */
    /* walking grdefs list will do a preorder walk of all defined groups*/
    for(igrp=0;igrp<ngrps;igrp++) {
        Symbol* gsym = (Symbol*)listget(grpdefs,igrp);
        if(gsym == rootgroup) continue; /* ignore root group*/
        stat = nc_def_grp(gsym->container->nc_id,gsym->name,&gsym->nc_id);
        CHECK_ERR(stat);
    }
#endif

#ifdef USE_NETCDF4
    /* Define the types*/
    if (ntyps > 0) {
        int ityp;
        for(ityp = 0; ityp < ntyps; ityp++) {
            Symbol* tsym = (Symbol*)listget(typdefs,ityp);
            genbin_deftype(tsym);
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
            CHECK_ERR(stat);
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
            CHECK_ERR(stat);
        }
    }

#ifdef USE_NETCDF4
    /* define special variable properties */
    if(nvars > 0) {
        for(ivar = 0; ivar < nvars; ivar++) {
            Symbol* var = (Symbol*)listget(vardefs,ivar);
            genbin_definespecialattributes(var);
        }
    }
#endif /*USE_NETCDF4*/

    /* define global attributes */
    if(ngatts > 0) {
        for(iatt = 0; iatt < ngatts; iatt++) {
            Symbol* gasym = (Symbol*)listget(gattdefs,iatt);
            genbin_defineattr(gasym);
        }
    }

    /* define per-variable attributes */
    if(natts > 0) {
        for(iatt = 0; iatt < natts; iatt++) {
            Symbol* asym = (Symbol*)listget(attdefs,iatt);
            genbin_defineattr(asym);
        }
    }

    if (nofill_flag) {
        stat = nc_set_fill(rootgroup->nc_id, NC_NOFILL, 0);
        CHECK_ERR(stat);
    }

    /* leave define mode */
    stat = nc_enddef(rootgroup->nc_id);
    CHECK_ERR(stat);

    if(!header_only) {
        /* Load values into those variables with defined data */
        if(nvars > 0) {
            for(ivar = 0; ivar < nvars; ivar++) {
                Symbol* vsym = (Symbol*)listget(vardefs,ivar);
                if(vsym->data != NULL) {
                    genbin_definevardata(vsym);
                }
            }
        }
    }
}

#ifdef USE_NETCDF4

#if 0
Turn off for now.

static void
genbin_defineglobalspecials(void)
{
    int stat = NC_NOERR;
    const char* format = NULL;
    if(usingclassic) return;
    if(!/*Main.*/format_attribute) return;
    /* Watch out, this is a global Attribute */
    format = kind_string(/*Main.*/format_flag);
    stat = nc_put_att_text(rootgroup->nc_id,NC_GLOBAL,"_Format",strlen(format),format);
    CHECK_ERR(stat);
}
#endif /*0*/

static int
genbin_definespecialattributes(Symbol* var)
{
    int stat = NC_NOERR;
    Specialdata* special = &var->var.special;
    if(special->flags & _STORAGE_FLAG) {
	if(special->_Storage == NC_CONTIGUOUS
	   || special->_Storage == NC_COMPACT) {
	    stat = nc_def_var_chunking(var->container->nc_id, var->nc_id, special->_Storage, NULL);
	} else { /* chunked */
	    if(special->nchunks == 0 || special->_ChunkSizes == NULL)
	        derror("NC_CHUNKED requested, but no chunksizes specified");
            stat = nc_def_var_chunking(var->container->nc_id, var->nc_id, NC_CHUNKED, special->_ChunkSizes);
	}
        CHECK_ERR(stat);
    }
    if(special->flags & _FLETCHER32_FLAG) {
        stat = nc_def_var_fletcher32(var->container->nc_id,
                                     var->nc_id,
                                     special->_Fletcher32);
        CHECK_ERR(stat);
    }
    if(special->flags & (_DEFLATE_FLAG | _SHUFFLE_FLAG)) {
        stat = nc_def_var_deflate(var->container->nc_id,
                                  var->nc_id,
                                  (special->_Shuffle == 1?1:0),
                                  (special->_DeflateLevel >= 0?1:0),
                                  (special->_DeflateLevel >= 0?special->_DeflateLevel
                                                              :0));
        CHECK_ERR(stat);
    }
    if(special->flags & _ENDIAN_FLAG) {
        stat = nc_def_var_endian(var->container->nc_id,
                                 var->nc_id,
                                 (special->_Endianness == NC_ENDIAN_LITTLE?
                                        NC_ENDIAN_LITTLE
                                       :NC_ENDIAN_BIG));
        CHECK_ERR(stat);
    }
    if(special->flags & _NOFILL_FLAG) {
        stat = nc_def_var_fill(var->container->nc_id,
                                 var->nc_id,
                                 (special->_Fill?NC_FILL:NC_NOFILL),
                                 NULL);
        CHECK_ERR(stat);
    }
    if(special->flags & _FILTER_FLAG) {
	int k;
	for(k=0;k<special->nfilters;k++) {
	    NC_H5_Filterspec* nfs = special->_Filters[k];
	    /* See if the filter is available */
	    stat = nc_inq_filter_avail(var->container->nc_id, nfs->filterid);
	    switch (stat) {
	    case NC_NOERR: break;
	    case NC_ENOFILTER:
		derror("Filter id=%u; filter not available",nfs->filterid);
		/* fall thru */
	    default: CHECK_ERR(stat); break;
	    }
            stat = nc_def_var_filter(var->container->nc_id,
                        var->nc_id,
			nfs->filterid,
                        nfs->nparams,
                        nfs->params
                        );
        }
        CHECK_ERR(stat);
    }
    if(special->flags & (_QUANTIZEBG_FLAG | _QUANTIZEGBR_FLAG | _QUANTIZEBR_FLAG)) {
        stat = nc_def_var_quantize(var->container->nc_id,
                                 var->nc_id, special->_Quantizer, special->_NSD);
        CHECK_ERR(stat);
    }
    return stat;
}
#endif /*USE_NETCDF4*/

void
genbin_close(void)
{
    int stat;
    stat = nc_close(rootgroup->nc_id);
    CHECK_ERR(stat);
}

#ifdef USE_NETCDF4
/*
Generate type definitions
*/
static int
genbin_deftype(Symbol* tsym)
{
    unsigned long i;
    int stat = 0;

    ASSERT(tsym->objectclass == NC_TYPE);
    switch (tsym->subclass) {
    case NC_PRIM: break; /* these are already taken care of*/
    case NC_OPAQUE:
        stat = nc_def_opaque(tsym->container->nc_id,
                             tsym->typ.size,
                             tsym->name,
                             &tsym->nc_id);
        CHECK_ERR(stat);
        break;
    case NC_ENUM:
      {
        Bytebuffer* datum;
        stat = nc_def_enum(tsym->container->nc_id,
                           tsym->typ.basetype->nc_id,
                           tsym->name,
                           &tsym->nc_id);
        CHECK_ERR(stat);
        datum = bbNew();
        for(i=0;i<listlength(tsym->subnodes);i++) {
          Symbol* econst = (Symbol*)listget(tsym->subnodes,i);
          ASSERT(econst->subclass == NC_ECONST);
          generator_reset(bin_generator,NULL);
          bbClear(datum);
          generate_basetype(econst->typ.basetype,econst->typ.econst,datum,NULL,bin_generator);
          stat = nc_insert_enum(tsym->container->nc_id,
                                tsym->nc_id,
                                econst->name,
                                bbContents(datum));
          CHECK_ERR(stat);
        }
        bbFree(datum);
      }
      break;
    case NC_VLEN:
        stat = nc_def_vlen(tsym->container->nc_id,
                           tsym->name,
                           tsym->typ.basetype->nc_id,
                           &tsym->nc_id);
        CHECK_ERR(stat);
        break;
    case NC_COMPOUND:
        stat = nc_def_compound(tsym->container->nc_id,
                               tsym->typ.size,
                               tsym->name,
                               &tsym->nc_id);
        CHECK_ERR(stat);
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
            CHECK_ERR(stat);
        }
        break;
    default: panic("definectype: unexpected type subclass");
    }
    return stat;
}
#endif /*USE_NETCDF4*/

static int
genbin_defineattr(Symbol* asym)
{
    int stat = NC_NOERR;
    Bytebuffer* databuf = bbNew();
    generator_reset(bin_generator,NULL);
    generate_attrdata(asym,bin_generator,(Writer)genbin_write,databuf);
    stat = nc_reclaim_data(asym->container->nc_id,asym->typ.basetype->nc_id,bbContents(databuf),datalistlen(asym->data));
    bbFree(databuf);
    return stat;
}


/* Following is patterned after the walk functions in semantics.c */
static int
genbin_definevardata(Symbol* vsym)
{
    int stat = NC_NOERR;
    Bytebuffer* databuf = NULL;
    if(vsym->data == NULL) goto done;
    databuf = bbNew();
    generator_reset(bin_generator,NULL);
    generate_vardata(vsym,bin_generator,(Writer)genbin_write,databuf);
    stat = nc_reclaim_data_all(vsym->container->nc_id,vsym->typ.basetype->nc_id,bbExtract(databuf),datalistlen(vsym->data));
done:
    bbFree(databuf);
    return stat;
}

static int
genbin_write(Generator* generator, Symbol* sym, Bytebuffer* memory,
             int rank, size_t* start, size_t* count)
{
    if(sym->objectclass == NC_ATT)
        return genbin_writeattr(generator,sym,memory,rank,start,count);
    else if(sym->objectclass == NC_VAR)
        return genbin_writevar(generator,sym,memory,rank,start,count);
    else
        PANIC("illegal symbol for genbin_write");
    return NC_EINVAL;
}

static int
genbin_writevar(Generator* generator, Symbol* vsym, Bytebuffer* memory,
                int rank, size_t* start, size_t* count)
{
    int stat = NC_NOERR;
    int i;
    char* data = bbContents(memory);
    size_t nelems;

    /* Compute total number of elements */
    for(nelems=1,i=0;i<rank;i++) nelems *= count[i];

#ifdef GENDEBUG
    {
    int i;
    fprintf(stderr,"startset = [");
    for(i=0;i<rank;i++)
        fprintf(stderr,"%s%lu",(i>0?", ":""),(unsigned long)start[i]);
    fprintf(stderr,"] ");
    fprintf(stderr,"countset = [");
    for(i=0;i<rank;i++)
        fprintf(stderr,"%s%lu",(i>0?", ":""),(unsigned long)count[i]);
    fprintf(stderr,"]\n");
    fflush(stderr);
    }
#endif

    if(rank == 0) {
        size_t count[1] = {1};
        stat = nc_put_var1(vsym->container->nc_id, vsym->nc_id, count, data);
    } else {
        stat = nc_put_vara(vsym->container->nc_id, vsym->nc_id, start, count, data);
    }
    CHECK_ERR(stat);
#if 0
    /* Reclaim the data */
    stat = nc_reclaim_data(vsym->container->nc_id, vsym->typ.basetype->nc_id, data, nelems);
    CHECK_ERR(stat);
    bbClear(memory); /* reclaim top-level memory */
#endif
    return stat;
}

static int
genbin_writeattr(Generator* generator, Symbol* asym, Bytebuffer* databuf,
           int rank, size_t* start, size_t* count)
{
    int stat = NC_NOERR;
    size_t len;
    Datalist* list;
    int varid, grpid, typid;
    Symbol* basetype = asym->typ.basetype;

    grpid = asym->container->nc_id;
    varid = (asym->att.var == NULL?NC_GLOBAL : asym->att.var->nc_id);
    typid = basetype->nc_id;

    list = asym->data;
    len = list->length;

    /* Use the specialized put_att_XX routines if possible*/
    if(isprim(typid)) {
      switch (basetype->typ.typecode) {
      case NC_BYTE: {
        signed char* data = (signed char*)bbContents(databuf);
        stat = nc_put_att_schar(grpid,varid,asym->name,typid,len,data);
        CHECK_ERR(stat);
      } break;
      case NC_CHAR: {
        char* data = (char*)bbContents(databuf);
                size_t slen = bbLength(databuf);
                /* Revise length if slen == 0 */
                if(slen == 0) {
          bbAppend(databuf,'\0');
          /* bbAppend frees the memory pointed to by char* data,
             so re-assign.  See Coverity issue: 1265731.*/
          data = (char*)bbContents(databuf);
          slen++;
        }
        stat = nc_put_att_text(grpid,varid,asym->name,slen,data);
        CHECK_ERR(stat);
      } break;
      case NC_SHORT: {
        short* data = (short*)bbContents(databuf);
        stat = nc_put_att_short(grpid,varid,asym->name,typid,len,data);
        CHECK_ERR(stat);
        } break;
        case NC_INT: {
            int* data = (int*)bbContents(databuf);
            stat = nc_put_att_int(grpid,varid,asym->name,typid,len,data);
            CHECK_ERR(stat);
        } break;
        case NC_FLOAT: {
            float* data = (float*)bbContents(databuf);
            stat = nc_put_att_float(grpid,varid,asym->name,typid,len,data);
            CHECK_ERR(stat);
        } break;
        case NC_DOUBLE: {
            double* data = (double*)bbContents(databuf);
            stat = nc_put_att_double(grpid,varid,asym->name,typid,len,data);
            CHECK_ERR(stat);
        } break;
        case NC_STRING: {
            const char** data;
            data = (const char**)bbContents(databuf);
            stat = nc_put_att_string(grpid,varid,asym->name,
                                     bbLength(databuf)/sizeof(char*),
                                     data);
            } break;
        case NC_UBYTE: {
            unsigned char* data = (unsigned char*)bbContents(databuf);
            stat = nc_put_att_uchar(grpid,varid,asym->name,typid,len,data);
            CHECK_ERR(stat);
        } break;
        case NC_USHORT: {
            unsigned short* data = (unsigned short*)bbContents(databuf);
            stat = nc_put_att_ushort(grpid,varid,asym->name,typid,len,data);
            CHECK_ERR(stat);
        } break;
        case NC_UINT: {
            unsigned int* data = (unsigned int*)bbContents(databuf);
            stat = nc_put_att_uint(grpid,varid,asym->name,typid,len,data);
            CHECK_ERR(stat);
        } break;
        case NC_INT64: {
            long long* data = (long long*)bbContents(databuf);
            stat = nc_put_att_longlong(grpid,varid,asym->name,typid,len,data);
            CHECK_ERR2(stat,asym->lineno);
        } break;
        case NC_UINT64: {
            unsigned long long* data = (unsigned long long*)bbContents(databuf);
            stat = nc_put_att_ulonglong(grpid,varid,asym->name,typid,len,data);
            CHECK_ERR(stat);
        } break;
	default: PANIC1("genbin_defineattr: unexpected basetype: %d",basetype->typ.typecode);
        }
    } else { /* use the generic put_attribute for user defined types*/
        const char* data;
        data = (const char*)bbContents(databuf);
        stat = nc_put_att(grpid,varid,asym->name,typid,
                                len,(void*)data);
        CHECK_ERR(stat);
#ifdef GENDEBUG
        {
            char out[4096];
            memset(out,0x77,sizeof(out));
            stat = nc_get_att(grpid,varid,asym->name,&out);
            CHECK_ERR(stat);
        }
#endif
    }
    return stat;
}

#endif /*ENABLE_BINARY*/
