/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#include "d4includes.h"
#include "d4chunk.h"

#define CHECKSUMFLAG

/**************************************************/

/* Header flags */
#define LAST_CHUNK          (1)
#define ERR_CHUNK           (2)
#define LITTLE_ENDIAN_CHUNK (4)
#ifdef CHECKSUMHACK
#define NOCHECKSUM_CHUNK    (8)
#else
#define NOCHECKSUM_CHUNK    (0)
#endif

#define ALL_CHUNK_FLAGS (LAST_CHUNK|ERR_CHUNK|LITTLE_ENDIAN_CHUNK|NOCHECKSUM_CHUNK)

/**************************************************/

/*
Given a packet as read from the wire via http (or a file), convert in
place from chunked format to a single contiguous set of bytes. If an
error packet is recovered, then make that available to the caller and
return an error. Also return whether the data was big endian encoded
and whether it has checksums.
Notes:
*/

/* Define a local struct for convenience */
struct HDR {unsigned int flags; unsigned int count;};

/* Forward */
static void* getheader(void* p, struct HDR* hdr, int hostlittleendian);
static int processerrchunk(NCD4meta* metadata, void* errchunk, unsigned int count);

/**************************************************/

int
NCD4_dechunk(NCD4meta* metadata)
{
    unsigned char* p;
    unsigned char* q;
    struct HDR hdr;

    if(metadata->mode == NCD4_DSR) 
        return THROW(NC_EDMR);

    metadata->serial.errdata = NULL;
    metadata->serial.dmr = NULL;
    metadata->serial.dap = NULL;
    metadata->serial.hostlittleendian = NCD4_isLittleEndian();
    metadata->serial.remotelittleendian = 0; /* do not actually know yet */
#ifdef CHECKSUMHACK
    metadata->serial.checksumhack = 0; /* do not actually know yet */
#endif
    /* Assume proper mode has been inferred already. */

    /* Verify the mode; assume that the <?xml...?> is optional */
    q = metadata->serial.rawdata;
    if(memcmp(q,"<?xml",strlen("<?xml"))==0
       || memcmp(q,"<Dataset",strlen("<Dataset"))==0) {
        if(metadata->mode != NCD4_DMR) 
            return THROW(NC_EDMR);
        /* setup as dmr only */
        metadata->serial.dmr = (char*)metadata->serial.rawdata; /* temp */
        /* Avoid strdup since rawdata might contain nul chars */
        if((metadata->serial.dmr = malloc(metadata->serial.rawsize+1)) == NULL)
            return THROW(NC_ENOMEM);    
        memcpy(metadata->serial.dmr,metadata->serial.rawdata,metadata->serial.rawsize);
        metadata->serial.dmr[metadata->serial.rawsize-1] = '\0';
        /* Suppress nuls */
        (void)NCD4_elidenuls(metadata->serial.dmr,metadata->serial.rawsize);
        return THROW(NC_NOERR); 
    }

    /* We must be processing a DAP mode packet */
    p = metadata->serial.rawdata;
    metadata->serial.dap = p;

#ifdef D4DUMPRAW
    NCD4_tagdump(metadata->serial.rawsize,metadata->serial.rawdata,0,"RAW");
#endif

    /* Get the DMR chunk header*/
    p = getheader(p,&hdr,metadata->serial.hostlittleendian);
    if(hdr.count == 0)
        return THROW(NC_EDMR);
    if(hdr.flags & ERR_CHUNK) {
        return processerrchunk(metadata, (void*)p, hdr.count);
    }

#ifdef CHECKSUMHACK
    /* Temporary hack; We mistakenly thought that bit 3 of the flags
       of the first header indicated that checksumming was not in force.
       Test for it, and propagate the _DAP4_Checksum_CRC32 attribute later */
    metadata->serial.checksumhack = ((hdr.flags & NOCHECKSUM_CHUNK) ? 1 : 0);
fprintf(stderr,"checksumhack=%d\n",metadata->serial.checksumhack);
#endif
    metadata->serial.remotelittleendian = ((hdr.flags & LITTLE_ENDIAN_CHUNK) ? 1 : 0);
    /* Again, avoid strxxx operations on dmr */
    if((metadata->serial.dmr = malloc(hdr.count+1)) == NULL)
        return THROW(NC_ENOMEM);        
    memcpy(metadata->serial.dmr,p,hdr.count);
    metadata->serial.dmr[hdr.count-1] = '\0';
    /* Suppress nuls */
    (void)NCD4_elidenuls(metadata->serial.dmr,hdr.count);

    if(hdr.flags & LAST_CHUNK)
        return THROW(NC_ENODATA);
    /* Read and compress the data chunks */
    p = p + hdr.count; /* point to data chunk header */
    /* Do a sanity check in case the server has shorted us with no data */
    if((hdr.count + CHUNKHDRSIZE) >= metadata->serial.rawsize) {
        /* Server only sent the DMR part */
        metadata->serial.dapsize = 0;
        return THROW(NC_EDATADDS);
    }
    q = metadata->serial.dap; 
    for(;;) {
        p = getheader(p,&hdr,metadata->serial.hostlittleendian);
        if(hdr.flags & ERR_CHUNK) {
            return processerrchunk(metadata, (void*)p, hdr.count);
        }
        /* data chunk; possibly last; possibly empty */
        if(hdr.count > 0) {
            d4memmove(q,p,hdr.count); /* will overwrite the header */
            p += hdr.count;
	    q += hdr.count;
        }
        if(hdr.flags & LAST_CHUNK) break;
    }
    metadata->serial.dapsize = (size_t)DELTA(q,metadata->serial.dap);

#ifdef D4DUMPDMR
    fprintf(stderr,"%s\n",metadata->serial.dmr);
    fflush(stderr);
#endif
#ifdef D4DUMPDAP
    NCD4_tagdump(metadata->serial.dapsize,metadata->serial.dap,0,"DAP");
#endif
    return THROW(NC_NOERR);    
}

static int
processerrchunk(NCD4meta* metadata, void* errchunk, unsigned int count)
{
    metadata->serial.errdata = (char*)d4alloc(count+1);
    if(metadata->serial.errdata == NULL)
        return THROW(NC_ENOMEM);
    memcpy(metadata->serial.errdata,errchunk,count);
    metadata->serial.errdata[count] = '\0';
    return THROW(NC_ENODATA); /* slight lie */
}

static void*
getheader(void* p, struct HDR* hdr, int hostlittleendian)
{
    unsigned char bytes[4];
    memcpy(bytes,p,sizeof(bytes));
    p = INCR(p,4); /* on-the-wire hdr is 4 bytes */
    /* assume header is network (big) order */
    hdr->flags = bytes[0]; /* big endian => flags are in byte 0 */
    hdr->flags &= ALL_CHUNK_FLAGS; /* Ignore extraneous flags */
    bytes[0] = 0; /* so we can do byte swap to get count */
    if(hostlittleendian)
        swapinline32(bytes); /* host is little endian */
    hdr->count = *(unsigned int*)bytes; /* get count */
    return p;
}

/**
Given a raw response, attempt to infer the mode: DMR, DAP, DSR.
Since DSR is not standardizes, it becomes the default.
*/
int
NCD4_infermode(NCD4meta* meta)
{
    d4size_t size = meta->serial.rawsize;
    char* raw = meta->serial.rawdata;

    if(size < 16)
        return THROW(NC_EDAP); /* must have at least this to hold a hdr + partial dmr*/ 
    if(memcmp(raw,"<?xml",strlen("<?xml"))==0
       || memcmp(raw,"<Dataset",strlen("<Dataset"))==0) {
        meta->mode = NCD4_DMR;
        goto done;
    }
    raw += 4; /* Pretend we have a DAP hdr */
    if(memcmp(raw,"<?xml",strlen("<?xml"))==0
       || memcmp(raw,"<Dataset",strlen("<Dataset"))==0) {
        meta->mode = NCD4_DAP;
        goto done;
    }
    /* Default to DSR */
    meta->mode = NCD4_DSR;

done:
    return NC_NOERR;
}
