/*********************************************************************
 *   Copyright 2026, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

/**
Feed NCD4_parcelvars() DAP responses whose serialized string, opaque
and array data claim more bytes than the response holds, and check
that they are rejected instead of walking past the end of the data.
*/

#include "d4includes.h"

#define DMRHEAD "<Dataset name=\"x\" dapVersion=\"4.0\" dmrVersion=\"1.0\"" \
    " xmlns=\"http://xml.opendap.org/ns/DAP/4.0#\">"
#define DMRTAIL "</Dataset>\r\n"

static int failures = 0;

static void
puthdr(unsigned char* p, int flags, size_t count)
{
    p[0] = (unsigned char)flags;
    p[1] = (unsigned char)((count>>16)&0xff);
    p[2] = (unsigned char)((count>>8)&0xff);
    p[3] = (unsigned char)(count&0xff);
}

/* Wrap the DMR and the serialized data in a little-endian DAP
   response and run it through the same steps as NCD4_get_vara */
static void
check(const char* name, const char* dmr, const unsigned char* data, size_t datalen, int expected)
{
    int ret;
    size_t dmrlen = strlen(dmr);
    size_t len = 4 + dmrlen + 4 + datalen;
    unsigned char* raw = NULL;
    NCD4INFO* info = NULL;
    NCD4response* resp = NULL;
    NCD4meta* meta = NULL;

    if((info = (NCD4INFO*)calloc(1,sizeof(NCD4INFO))) == NULL) exit(1);
    info->platform.hostlittleendian = NC_isLittleEndian();
    if((ret = NCD4_newResponse(info,&resp))) exit(1);
    if((raw = malloc(len)) == NULL) exit(1);
    puthdr(raw,NCD4_LITTLE_ENDIAN_CHUNK,dmrlen);
    memcpy(raw+4,dmr,dmrlen);
    puthdr(raw+4+dmrlen,NCD4_LITTLE_ENDIAN_CHUNK|NCD4_LAST_CHUNK,datalen);
    memcpy(raw+4+dmrlen+4,data,datalen);
    resp->raw.memory = raw;
    resp->raw.size = len;
    resp->mode = NCD4_DAP;

    if((ret = NCD4_dechunk(resp))) goto done;
    if((ret = NCD4_newMeta(info,&meta))) goto done;
    if((ret = NCD4_parse(meta,resp,1))) goto done;
    if((ret = NCD4_inferChecksums(meta,resp))) goto done;
    ret = NCD4_parcelvars(meta,resp);
done:
    if(ret != expected) {
	fprintf(stderr,"***Fail: %s: expected %d got %d\n",name,expected,ret);
	failures++;
    } else
	fprintf(stderr,"pass: %s\n",name);
    if(meta) NCD4_reclaimMeta(meta);
    NCD4_reclaimResponse(resp);
    free(info);
}

/* 8 byte little-endian count */
#define COUNT(n) (unsigned char)((n)&0xff),(unsigned char)(((n)>>8)&0xff), \
    (unsigned char)(((n)>>16)&0xff),(unsigned char)(((n)>>24)&0xff), \
    0,0,0,0
#define HUGE 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff

int
main(void)
{
    static const char strdmr[] = DMRHEAD "<String name=\"s\"/>" DMRTAIL;
    static const char opqdmr[] = DMRHEAD "<Opaque name=\"o\"/>" DMRTAIL;
    static const char arrdmr[] = DMRHEAD "<Dimension name=\"d\" size=\"4\"/>"
	"<Int32 name=\"v\"><Dim name=\"/d\"/></Int32>" DMRTAIL;

    static const unsigned char goodstr[] = {COUNT(4), 'a','b','c','d'};
    static const unsigned char bigstr[] = {COUNT(0x10000), 'a','b','c','d'};
    static const unsigned char hugestr[] = {HUGE, 'a','b','c','d'};
    static const unsigned char shortcount[] = {4,0,0,0};
    static const unsigned char goodopq[] = {COUNT(2), 1,2};
    static const unsigned char bigopq[] = {COUNT(0x10000), 1,2};
    static const unsigned char goodarr[] = {1,0,0,0, 2,0,0,0, 3,0,0,0, 4,0,0,0};
    static const unsigned char shortarr[] = {1,0,0,0, 2,0,0,0};

    check("goodstr",strdmr,goodstr,sizeof(goodstr),NC_NOERR);
    check("bigstr",strdmr,bigstr,sizeof(bigstr),NC_EDATADAP);
    check("hugestr",strdmr,hugestr,sizeof(hugestr),NC_EDATADAP);
    check("shortcount",strdmr,shortcount,sizeof(shortcount),NC_EDATADAP);
    check("goodopq",opqdmr,goodopq,sizeof(goodopq),NC_NOERR);
    check("bigopq",opqdmr,bigopq,sizeof(bigopq),NC_EDATADAP);
    check("goodarr",arrdmr,goodarr,sizeof(goodarr),NC_NOERR);
    check("shortarr",arrdmr,shortarr,sizeof(shortarr),NC_EDATADAP);

    return (failures ? EXIT_FAILURE : EXIT_SUCCESS);
}
