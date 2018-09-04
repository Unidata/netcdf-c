#include "d4includes.h"
#include "d4curlfunctions.h"
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#include "ncwinpath.h"

/* Do conversion if this code was compiled via Vis. Studio or Mingw */

/*Forward*/
static int readpacket(NCD4INFO* state, NCURI*, NCbytes*, NCD4mode, long*);
static int readfile(const NCURI*, const char* suffix, NCbytes* packet);
static int readfiletofile(const NCURI*, const char* suffix, FILE* stream, d4size_t*);

int
NCD4_readDMR(NCD4INFO* state)
{
    int stat = NC_NOERR;
    long lastmodified = -1;

    stat = readpacket(state,state->uri,state->curl->packet,NCD4_DMR,&lastmodified);
    if(stat == NC_NOERR)
	state->data.dmrlastmodified = lastmodified;
    return THROW(stat);
}

int
NCD4_readDAP(NCD4INFO* state, int flags)
{
    int stat = NC_NOERR;
    long lastmod = -1;

    if((flags & NCF_ONDISK) == 0) {
        stat = readpacket(state,state->uri,state->curl->packet,NCD4_DAP,&lastmod);
        if(stat == NC_NOERR)
            state->data.daplastmodified = lastmod;
    } else { /*((flags & NCF_ONDISK) != 0) */
        NCURI* url = state->uri;
        int fileprotocol = (strcmp(url->protocol,"file")==0);
        if(fileprotocol) {
            stat = readfiletofile(url, ".dap", state->data.ondiskfile, &state->data.datasize);
        } else {
	    char* readurl = NULL;
            int flags = 0;
            if(!fileprotocol) flags |= NCURIQUERY;
            flags |= NCURIENCODE;
	    flags |= NCURIPWD;
#ifdef FIX
            ncurisetconstraints(url,state->constraint);
#endif
	    readurl = ncuribuild(url,NULL,".dods",NCURISVC);
	    if(readurl == NULL)
		return THROW(NC_ENOMEM);
            if (state->debug > 0)
                {fprintf(stderr, "fetch url=%s\n", readurl);fflush(stderr);}
            stat = NCD4_fetchurl_file(state->curl, readurl, state->data.ondiskfile,
                                   &state->data.datasize, &lastmod);
            nullfree(readurl);
            if(stat == NC_NOERR)
                state->data.daplastmodified = lastmod;
            if (state->debug > 0)
                {fprintf(stderr,"fetch complete\n"); fflush(stderr);}
        }
    }
    return THROW(stat);
}

static const char*
dxxextension(int dxx)
{
    switch(dxx) {
    case NCD4_DMR: return ".dmr";
    case NCD4_DAP: return ".dap";
    default: break;
    }
    return NULL;
}

static int
readpacket(NCD4INFO* state, NCURI* url, NCbytes* packet, NCD4mode dxx, long* lastmodified)
{
    int stat = NC_NOERR;
    int fileprotocol = 0;
    const char* suffix = dxxextension(dxx);
    CURL* curl = state->curl->curl;

    fileprotocol = (strcmp(url->protocol,"file")==0);

    if(fileprotocol) {
	/* Short circuit file://... urls*/
	/* We do this because the test code always needs to read files*/
	stat = readfile(url,suffix,packet);
    } else {
        char* fetchurl = NULL;
	int flags = NCURIBASE;
	if(!fileprotocol) flags |= NCURIQUERY;
	flags |= NCURIENCODE;
        fetchurl = ncuribuild(url,NULL,suffix,flags);
	MEMCHECK(fetchurl);
	if(state->debug > 0)
            {fprintf(stderr,"fetch url=%s\n",fetchurl); fflush(stderr);}
        stat = NCD4_fetchurl(curl,fetchurl,packet,lastmodified);
        nullfree(fetchurl);
	if(stat) goto fail;
	if(state->debug > 0)
            {fprintf(stderr,"fetch complete\n"); fflush(stderr);}
    }
#ifdef D4DEBUG
  {
fprintf(stderr,"readpacket: packet.size=%lu\n",
		(unsigned long)ncbyteslength(packet));
  }
#endif
fail:
    return THROW(stat);
}

static int
readfiletofile(const NCURI* uri, const char* suffix, FILE* stream, d4size_t* sizep)
{
    int stat = NC_NOERR;
    NCbytes* packet = ncbytesnew();
    size_t len;
    stat = readfile(uri,suffix,packet);
#ifdef D4DEBUG
fprintf(stderr,"readfiletofile: packet.size=%lu\n",
		(unsigned long)ncbyteslength(packet));
#endif
    if(stat != NC_NOERR) goto unwind;
    len = nclistlength(packet);
    if(stat == NC_NOERR) {
	size_t written;
        fseek(stream,0,SEEK_SET);
	written = fwrite(ncbytescontents(packet),1,len,stream);
	if(written != len) {
#ifdef D4DEBUG
fprintf(stderr,"readfiletofile: written!=length: %lu :: %lu\n",
	(unsigned long)written,(unsigned long)len);
#endif
	    stat = NC_EIO;
	}
    }
    if(sizep != NULL) *sizep = len;
unwind:
    ncbytesfree(packet);
    return THROW(stat);
}

static int
readfile(const NCURI* uri, const char* suffix, NCbytes* packet)
{
    int stat = NC_NOERR;
    NCbytes* tmp = ncbytesnew();
    char* filename = NULL;

    ncbytescat(tmp,uri->path);
    if(suffix != NULL) ncbytescat(tmp,suffix);
    ncbytesnull(tmp);
    filename = ncbytesextract(tmp);
    ncbytesfree(tmp);

    stat = NC_readfile(filename,packet);
    return THROW(stat);
}
