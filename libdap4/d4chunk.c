/*********************************************************************
 *   Copyright 2016, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#include "d4includes.h"
#include "d4chunk.h"

/**************************************************/

/* Header flags */
#define LAST_CHUNK (1<<0)
#define ERR_CHUNK (1<<1)
#define LITTLE_ENDIAN_CHUNK (1<<2)

/**************************************************/

/*
Given a packet as read from the wire via http (or a file), convert in
place from chunked format to a single continguous set of bytes. If an
error packet is recovered, then make that available to the caller and
return an error. Also return whether the data was big endian encoded.
Notes:
*/

/* Define a local struct for convenience */
struct HDR {unsigned int flags;	unsigned int count;};

/* Forward */
static void* getheader(void* p, struct HDR* hdr, int hostbigendian);

/**************************************************/

int
NCD4_dechunk(NCD4meta* metadata)
{
    unsigned char* p;
    unsigned char* q;
    int firstchunk;

    metadata->serial.errdata = NULL;
    metadata->serial.dmr = NULL;
    metadata->serial.dap = NULL;
    metadata->serial.hostbigendian = NCD4_isBigEndian();
    metadata->serial.remotebigendian = 0; /* do not actually know yet */

    p = metadata->serial.rawdata;
    firstchunk = 1;
    for(;;firstchunk=0) {
	struct HDR hdr;
	p = getheader(p,&hdr,metadata->serial.hostbigendian);
	if(hdr.flags & ERR_CHUNK) {
	    metadata->serial.errdata = (char*)malloc(hdr.count+1);
	    if(metadata->serial.errdata == NULL) return NC_ENOMEM;
	    memcpy(metadata->serial.errdata,p,hdr.count);
	    metadata->serial.errdata[hdr.count] = '\0';
	    return NC_ENODATA; /* slight lie */
	}
	if(firstchunk) { /* extract 1st (dmr) chunk */
	    metadata->serial.remotebigendian = ((hdr.flags & LITTLE_ENDIAN_CHUNK) ? 0 : 1);
	    metadata->serial.dmr = p;
	    metadata->serial.dmr[hdr.count-1] = '\0';
	    metadata->serial.dmrsize = hdr.count;
	    p += hdr.count;
	    metadata->serial.dap = p;
	    q = p;
	} else {
   	    /* data chunk; possibly last */
	    d4memmove(q,p,hdr.count); /* will overwrite the header */
	    p += hdr.count;
	    q += hdr.count;
	}
	if(hdr.flags & LAST_CHUNK) break;
    }
    metadata->serial.dapsize = (size_t)(((void*)p) - metadata->serial.dap);
    return NC_NOERR;    
}

void
NCD4_setdmr(NCD4meta* meta, const char* dmr)
{
    meta->serial.dmr = strdup(dmr);
    meta->serial.dmrsize = strlen(dmr);
}

int
NCD4_isdmr(const void* data)
{
    /* See if we have just a raw dmr */
    return (memcmp(data,"<Dataset",strlen("<Dataset"))==0);
}

static void*
getheader(void* p, struct HDR* hdr, int hostbigendian)
{
    unsigned char bytes[4];
    memcpy(bytes,p,sizeof(bytes));
    p += 4; /* on-the-wire hdr is 4 bytes */
    hdr->flags = bytes[0]; /* big endian => flags are in byte 0 */
    bytes[0] = 0; /* so we can do byte swap to get count */
    if(!hostbigendian)
        swapinline32(bytes); /* host is little endian */
    hdr->count = *(unsigned int*)bytes; /* get count */
    return p;
}
