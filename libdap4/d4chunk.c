/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#include "d4includes.h"
#include "d4chunk.h"

#define CHECKSUMFLAG

/**************************************************/

/**************************************************/

/*
Given a packet as read from the wire via http (or a file), convert in
place from chunked format to a single contiguous set of bytes. If an
error packet is recovered, then make that available to the caller and
return an error. Also return whether the data was big endian encoded
and whether it has checksums.
Notes:
*/

/* Forward */
static int processerrchunk(NCD4meta* metadata, void* errchunk, unsigned int count);

/**************************************************/

void
NCD4_resetSerial(NCD4serial* serial, size_t rawsize, void* rawdata)
{
    nullfree(serial->errdata);
    nullfree(serial->dmr);
    nullfree(serial->dap);
    nullfree(serial->rawdata);
    /* clear all fields */
    memset(serial,0,sizeof(NCD4serial));
    /* Reset fields */
    serial->hostlittleendian = NCD4_isLittleEndian();
    serial->rawsize = rawsize;
    serial->rawdata = rawdata;
}

int
NCD4_dechunk(NCD4meta* metadata)
{
    unsigned char *praw, *phdr, *pdap;
    NCD4HDR hdr;

#ifdef D4DUMPRAW
    NCD4_tagdump(metadata->serial.rawsize,metadata->serial.rawdata,0,"RAW");
#endif

    /* Access the returned raw data */
    praw = metadata->serial.rawdata;

    if(metadata->mode == NCD4_DSR) {
	return THROW(NC_EDMR);
    } else if(metadata->mode == NCD4_DMR) {
        /* Verify the mode; assume that the <?xml...?> is optional */
        if(memcmp(praw,"<?xml",strlen("<?xml"))==0
           || memcmp(praw,"<Dataset",strlen("<Dataset"))==0) {
	    size_t len = 0;
	    /* setup as dmr only */
            /* Avoid strdup since rawdata might contain nul chars */
	    len = metadata->serial.rawsize;
            if((metadata->serial.dmr = malloc(len+1)) == NULL)
                return THROW(NC_ENOMEM);    
            memcpy(metadata->serial.dmr,praw,len);
            metadata->serial.dmr[len] = '\0';
            /* Suppress nuls */
            (void)NCD4_elidenuls(metadata->serial.dmr,len);
            return THROW(NC_NOERR); 
	}
    } else if(metadata->mode != NCD4_DAP)
    	return THROW(NC_EDAP);

    /* We must be processing a DAP mode packet */
    praw = (metadata->serial.dap = metadata->serial.rawdata);
    metadata->serial.rawdata = NULL;

    /* If the raw data looks like xml, then we almost certainly have an error */
    if(memcmp(praw,"<?xml",strlen("<?xml"))==0
           || memcmp(praw,"<!doctype",strlen("<!doctype"))==0) {
	/* Set up to report the error */
	int stat = NCD4_seterrormessage(metadata, metadata->serial.rawsize, metadata->serial.rawdata);
        return THROW(stat); /* slight lie */
    }

    /* Get the DMR chunk header*/
    phdr = NCD4_getheader(praw,&hdr,metadata->serial.hostlittleendian);
    if(hdr.count == 0)
        return THROW(NC_EDMR);
    if(hdr.flags & NCD4_ERR_CHUNK) {
        return processerrchunk(metadata, (void*)phdr, hdr.count);
    }

#ifdef CHECKSUMHACK
    /* Temporary hack; We mistakenly thought that bit 3 of the flags
       of the first header indicated that checksumming was not in force.
       Test for it, and propagate the _DAP4_Checksum_CRC32 attribute later */
    metadata->serial.checksumhack = ((hdr.flags & NCD4_NOCHECKSUM_CHUNK) ? 1 : 0);
#endif
    metadata->serial.remotelittleendian = ((hdr.flags & NCD4_LITTLE_ENDIAN_CHUNK) ? 1 : 0);
    /* Again, avoid strxxx operations on dmr */
    if((metadata->serial.dmr = malloc(hdr.count+1)) == NULL)
        return THROW(NC_ENOMEM);        
    memcpy(metadata->serial.dmr,phdr,hdr.count);
    metadata->serial.dmr[hdr.count-1] = '\0';
    /* Suppress nuls */
    (void)NCD4_elidenuls(metadata->serial.dmr,hdr.count);

    if(hdr.flags & NCD4_LAST_CHUNK)
        return THROW(NC_ENODATA);

    /* Read and concat together the data chunks */
    phdr = phdr + hdr.count; /* point to data chunk header */
    /* Do a sanity check in case the server has shorted us with no data */
    if((hdr.count + CHUNKHDRSIZE) >= metadata->serial.rawsize) {
        /* Server only sent the DMR part */
        metadata->serial.dapsize = 0;
        return THROW(NC_EDATADDS);
    }
    pdap = metadata->serial.dap; 
    for(;;) {
        phdr = NCD4_getheader(phdr,&hdr,metadata->serial.hostlittleendian);
        if(hdr.flags & NCD4_ERR_CHUNK) {
            return processerrchunk(metadata, (void*)phdr, hdr.count);
        }
        /* data chunk; possibly last; possibly empty */
        if(hdr.count > 0) {
            d4memmove(pdap,phdr,hdr.count); /* will overwrite the header */
            phdr += hdr.count;
	    pdap += hdr.count;
        }
        if(hdr.flags & NCD4_LAST_CHUNK) break;
    }
    metadata->serial.dapsize = (size_t)DELTA(pdap,metadata->serial.dap);

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
