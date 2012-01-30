/* Copyright 2009, UCAR/Unidata and OPeNDAP, Inc.
   See the COPYRIGHT file for more information. */

#include "config.h"
#include "ocinternal.h"
#include "occontent.h"
#include "ocdata.h"
#include "ocdebug.h"

const char StartOfSequence = '\x5A';
const char EndOfSequence = '\xA5';

static int ocerrorstring(XXDR* xdrs);

#define LOCALMEMMAX 1024

/* Skip arbitrary object:
Cases:
astype = Grid|Structure: skip single set of fields
astype = Primitive array (or scalar): skip whole array.
astype = Sequence: skip whole sequence
astype = OC_NAT => use node's type
*/
OCerror
ocskipinstanceas(OCnode* node, XXDR* xdrs, OCtype astype)
{
    unsigned int i;
    int stat = OC_NOERR;
    char tmp[XDRUNIT];

    /* assume that xxdr_getpos points to start of the object (including leading counts) */

    /* Make sure astype is defined */
    if(astype == OC_NAT)
	astype = node->octype;

    switch (astype) {
	case OC_Dataset:
        case OC_Grid:
        case OC_Structure:
            if(node->skip.instancesize != OCINDETERMINATE
               && node->skip.count != OCINDETERMINATE) {
		/* skip directly past it */
	        xxdr_skip(xdrs,node->skip.totalsize);
	    } else {/* Walk instance by instance */
                for(i=0;i<oclistlength(node->subnodes);i++) {
                    OCnode* field = (OCnode*)oclistget(node->subnodes,i);
		    stat = ocskipinstanceas(field, xdrs, OC_NAT);
                    if(stat != OC_NOERR) {OCTHROWCHK(stat); break;}
                }
                if(stat != OC_NOERR) {OCTHROWCHK(stat); break;}
	    }
	    break;

        case OC_Sequence: 
	    /* skip whole sequence */
            for(i=0;;i++) {
	        /* extract the tag byte*/
        	if(!xxdr_getbytes(xdrs,tmp,sizeof(tmp))) return xdrerror();
	        if(tmp[0] == EndOfSequence) {
		    /* we are done */
		    break;
	        } else if(tmp[0] == StartOfSequence) {
		    /* skip record instance */
		    stat = ocskipinstanceas(node, xdrs, OC_Structure);
                    if(stat != OC_NOERR) {OCTHROWCHK(stat); break;}
		} else {stat = OCTHROW(OC_EXDR); break;}
	    }
	    break;

	case OC_Primitive:
            OCASSERT(node->skip.count != OCINDETERMINATE);
	    if(node->skip.instancesize != OCINDETERMINATE) {
		/* skip directly past it */
	        xxdr_skip(xdrs,node->skip.totalsize);
	    } else {/* Walk instance by instance */
		int skipstart = 0; /* worst case */
		OCASSERT(node->etype == OC_String || node->etype == OC_URL);
#ifdef IGNORE
		/* But first, check the cache, if any */
		if(node->cache.cacheable && node->cache.valid) {
		    /* use the cache index to shorten how much we need to skip */
		    OCASSERT(node->cache.index <= node->skip.count);
		    if(!xxdr_setpos(xdrs,node->cache.offset))
			return xdrerror();
		    skipstart = node->cache.index;
		}
#endif
                for(i=skipstart;i<node->skip.count;i++) {
		    /* read and skip the string */
		    unsigned int len;
		    /* read string size */
		    if(!xxdr_uint(xdrs,&len)) {stat = OCTHROW(OC_EXDR); break;}
		    /* round up to next XDRUNIT */
		    len = RNDUP(len);
		    if(!xxdr_skip(xdrs,(size_t)len)) {stat = OCTHROW(OC_EXDR); break;}
                }
                if(stat != OC_NOERR) {OCTHROWCHK(stat); break;}
	    }
	    break;

        default:
	    OCPANIC1("ocskipinstance: encountered unexpected node type: %x",astype);
	    break;
    }
    return OCTHROW(stat);
}

/*
Extract data from the xdr packet into a chunk of memory.
Normally, it is assumed that we are (at least virtually)
"at" a single instance in the xdr packet; which we read.
Virtually because for packed data, we need to point to
the beginning of the packed data and use the index to indicate
which packed element to get. Assume that in any case,
any leading counts have been passed.
*/
OCerror
ocxdrread(OCcontent* content, XXDR* xdrs, char* memory, size_t memsize,
          ocindex_t start, ocindex_t count)
{
    int stat = OC_NOERR;
    unsigned int i;
    size_t elemsize;
    size_t readsize;
    size_t skipsize;
    char localmem[LOCALMEMMAX];
    char* srcmem;    
    unsigned int* p;
    int packed;
    int scalar;
    OCtype octype,etype;
    ocindex_t localstart = start; /* will change if node is cacheing */
    OCnode* node;

    node = content->node;
    octype = node->octype;
    etype = node->etype;

    elemsize = octypesize(etype);

    scalar = (node->array.rank == 0 ? 1 : 0);

    /* check if the data is packed*/
    packed = (octype == OC_Primitive && !scalar
              && (etype == OC_Byte || etype == OC_UByte || etype == OC_Char));
	 
    /* validate memory space*/
    if(memsize < elemsize*count) return OCTHROW(OC_EINVAL);

#ifdef IGNORE
    if(!scalar && (!node->cache.cacheable || !node->cache.valid)) {
        unsigned int xdrcount0,xdrcount1;
	/* assume xdr position is correct */
        /* Read leading double count if ! scalar*/
        if(!xxdr_uint(xdrs,&xdrcount0)) goto shortxdr;
        if(!xxdr_uint(xdrs,&xdrcount1)) goto shortxdr;
        if(xdrcount0 != xdrcount1) return OCTHROW(OC_EXDR);
        if(xdrcount0 < start+count) goto shortxdr;
    }
#endif
 
    /* Handle packed data specially*/
    if(packed) {
	readsize = count*1; /* |OC_(Char,Byte,UByte)| == 1 */
	skipsize = start*1; /* |OC_(Char,Byte,UByte)| == 1 */
	/* skip to start of what we want to read */
	if(!xxdr_skip(xdrs,skipsize)) goto shortxdr;
	/* read data, keeping xdrs on XDRUNIT boundary */
	if(!xxdr_opaque(xdrs,memory,readsize))
	    goto shortxdr;
	return OCTHROW(OC_NOERR);
    }

    /* Not packed */

#ifdef IGNORE
    /* If this (primitive) object is cacheable and is valid cache,
       then modify start and set the xdr position accordingly
    */
    if(node->cache.cacheable && node->cache.valid) {
	if(node->cache.index <= start) {
	    localstart -= node->cache.index;
	    if(!xxdr_setpos(xdrs,node->cache.offset)) return xdrerror();
	}
    }
#endif

    /* Compute how much to skip based on the content's cache index */
    localstart = start - content->cache.index;
    if(localstart < 0) localstart = 0;

    /* extract count items; use xxdr_getbytes to speed up*/
    srcmem = memory;
    switch (etype) {
    case OC_Float64: case OC_Int64: case OC_UInt64:
	readsize = count*2*XDRUNIT;
	skipsize = localstart*2*XDRUNIT;
	/* skip to start of what we want to read */
	if(!xxdr_skip(xdrs,skipsize)) goto shortxdr;
	if(!xxdr_opaque(xdrs,(char*)srcmem,readsize)) goto shortxdr;
	if(etype == OC_Float64) {
	    double* dp;
	    for(dp=(double*)srcmem,i=0;i<count;i++,dp++) {
		double swap;
		xxdrntohdouble((char*)dp,&swap);
		*dp = swap;
	    }
	} else if(!xxdr_network_order) {
	    unsigned long long* llp;
	    for(llp=(unsigned long long*)srcmem,i=0;i<count;i++,p++) {
		swapinline64(llp);
	    }
	}
	break;

    case OC_String: case OC_URL: {
	/* Read string by string */
        char* s = NULL;
	char** pmem = (char**)srcmem;
	/* First skip to the starting string */
	for(i=0;i<localstart;i++) {
	    unsigned int slen;
            if(!xxdr_uint(xdrs,&slen)) goto shortxdr;
	    slen = RNDUP(slen);
            if(!xxdr_skip(xdrs,slen)) goto shortxdr;
        }
	/* Read count strings */
	for(i=0;i<count;i++) {
	    off_t slen;
	    /* xxdr_string will always alloc the space */	
            if(!xxdr_string(xdrs,&s,&slen)) 
		goto shortxdr;
	    pmem[i] = s;
	}
    } break;


    case OC_Char: case OC_Byte: case OC_UByte:
    case OC_Int16: case OC_UInt16:
	/* We need to store the xdr data locally until we can convert it out 
           because  elemsize < sizeof(int) */
	srcmem = localmem;
	if(count*elemsize > sizeof(localmem)) {
	    srcmem = (char*)ocmalloc(count*sizeof(unsigned int));
	    if(srcmem == NULL) {stat = OC_ENOMEM; goto done;}
	}
	/* fall thru */		
    case OC_Int32: case OC_UInt32:
    case OC_Float32:
        readsize = (count)*XDRUNIT;
        skipsize = (localstart)*XDRUNIT;
	if(!xxdr_skip(xdrs,skipsize)) goto shortxdr;
	if(!xxdr_opaque(xdrs,(char*)srcmem,readsize)) goto shortxdr;
	if(!xxdr_network_order) {
	    for(p=(unsigned int*)srcmem,i=0;i<count;i++,p++) {
		swapinline32(p);
	    }
	}
	break;

    default: OCPANIC("unexpected etype"); break;
    }

    /* Convert memory to right format */
    p = (unsigned int*)memory;
    switch (etype) {

    case OC_Char: case OC_Byte: case OC_UByte: {
	char* pmem = (char*)memory;
	p = (unsigned int*)srcmem;
	for(i=0;i<count;i++) {
	    unsigned int tmp = *p++;
	    *pmem++ = (unsigned char)tmp;
	}
    } break;

    case OC_Int16: case OC_UInt16: {
	unsigned short* pmem = (unsigned short*)memory;
	p = (unsigned int*)srcmem;
	for(i=0;i<count;i++) {
	    unsigned int tmp = *p++;
	    *pmem++ = (unsigned short)tmp;
	}
    } break;

    default: break; /* already handled above */
    }

    /* set cache */
    content->cache.index = start + count; /* should be our current index */
    content->cache.offset = xxdr_getpos(xdrs); /* should be our current position */

done:
    return OCTHROW(stat);

shortxdr:
    content->cache.valid = 0; /* no longer valid */
    if(!ocerrorstring(xdrs))
        oc_log(LOGERR,"DAP DATADDS packet is apparently too short");
    stat = OC_EDATADDS;
    goto done;    
}

int
occountrecords(OCnode* node, XXDR* xdrs, size_t* nrecordsp)
{
    int stat = OC_NOERR;
    size_t nrecords = 0;

    if(node->octype != OC_Sequence) return OCTHROW(OC_EINVAL);
    /* checkpoint the xdr position*/
    for(;;) { unsigned int i;
        /* pick up the sequence record begin marker*/
        char tmp[sizeof(unsigned int)];
        /* extract the tag byte*/
        if(!xxdr_getbytes(xdrs,tmp,sizeof(tmp))) return xdrerror();
        if(tmp[0] == StartOfSequence) {
            /* Walk each member field*/
            for(i=0;i<oclistlength(node->subnodes);i++) {
                OCnode* member = (OCnode*)oclistget(node->subnodes,i);
                stat = ocskipinstanceas(member,xdrs,OC_Structure);
                if(stat != OC_NOERR) break;
            }
	    nrecords++;
        } else if(tmp[0] == EndOfSequence) {
            break; /* we are done with the this sequence instance*/
        } else {
            oc_log(LOGERR,"missing/invalid begin/end record marker\n");
            stat = OC_EINVALCOORDS;
            break;
        }
        if(stat != OC_NOERR) break;
    }
    /* move to checkpoint position*/
    if(nrecordsp != NULL) *nrecordsp = nrecords;
    return OCTHROW(stat);
}



#define tag "Error {\n"

static int
ocerrorstring(XXDR* xdrs)
{
    /* Check to see if the xdrs contains "Error {\n'; assume it is at the beginning of data */
    off_t avail = xxdr_getavail(xdrs);
    char* data = (char*)malloc(avail);
    if(!xxdr_setpos(xdrs,0)) return 0;
    if(!xxdr_opaque(xdrs,data,avail)) return 0;
    /* check for error tag at front */
    if(ocstrncmp(data,tag,sizeof(tag))==0) {
	char* p;
        if((p=strchr(data,'}')) != NULL) *(++p)='\0';
        oc_log(LOGERR,"Server error: %s",data);
        /* Since important, report to stderr as well */
        fprintf(stderr,"Server error: %s",data);
	return 1;
    }
    return 0;
}
