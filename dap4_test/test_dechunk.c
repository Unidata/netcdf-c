/*********************************************************************
 *   Copyright 2026, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

/**
Feed NCD4_dechunk() DAP responses whose chunk headers claim more
bytes than were actually received, and check that they are rejected
instead of being read past the end of the response buffer.
*/

#include "d4includes.h"

#define DMR "<Dataset name=\"x\"/>"

static int failures = 0;

/* Wrap the bytes in a response and run NCD4_dechunk on it */
static void
check(const char* name, const unsigned char* bytes, size_t len, int expected)
{
    int ret;
    NCD4INFO* info = NULL;
    NCD4response* resp = NULL;

    if((info = (NCD4INFO*)calloc(1,sizeof(NCD4INFO))) == NULL) exit(1);
    info->platform.hostlittleendian = NC_isLittleEndian();
    if((ret = NCD4_newResponse(info,&resp))) exit(1);
    /* exact-size copy so any overread lands outside the allocation */
    if((resp->raw.memory = malloc(len)) == NULL) exit(1);
    memcpy(resp->raw.memory,bytes,len);
    resp->raw.size = len;
    resp->mode = NCD4_DAP;

    ret = NCD4_dechunk(resp);
    if(ret != expected) {
	fprintf(stderr,"***Fail: %s: expected %d got %d\n",name,expected,ret);
	failures++;
    } else
	fprintf(stderr,"pass: %s\n",name);
    NCD4_reclaimResponse(resp);
    free(info);
}

#define HDR(flags,count) (unsigned char)(flags), \
    (unsigned char)(((count)>>16)&0xff), \
    (unsigned char)(((count)>>8)&0xff), \
    (unsigned char)((count)&0xff)

#define DMRBYTES '<','D','a','t','a','s','e','t',' ','n','a','m','e','=','"','x','"','/','>'

int
main(void)
{
    /* well formed: dmr chunk followed by one last data chunk */
    static const unsigned char good[] = {
	HDR(0,sizeof(DMR)-1), DMRBYTES,
	HDR(NCD4_LAST_CHUNK,4), 1,2,3,4};
    /* dmr chunk count larger than the response */
    static const unsigned char bigdmr[] = {
	HDR(0,0x10000), DMRBYTES, 0,0,0,0};
    /* error chunk in place of the dmr, count larger than the response */
    static const unsigned char bigdmrerr[] = {
	HDR(NCD4_ERR_CHUNK,0x10000), DMRBYTES};
    /* data chunk count larger than the response */
    static const unsigned char bigdata[] = {
	HDR(0,sizeof(DMR)-1), DMRBYTES,
	HDR(0,0x10000), 1,2,3,4};
    /* data error chunk count larger than the response */
    static const unsigned char bigdataerr[] = {
	HDR(0,sizeof(DMR)-1), DMRBYTES,
	HDR(NCD4_ERR_CHUNK,0x10000), 'e','r','r','!'};
    /* response ends before a last chunk is seen */
    static const unsigned char truncated[] = {
	HDR(0,sizeof(DMR)-1), DMRBYTES,
	HDR(0,4), 1,2,3,4, 0,0};

    check("good",good,sizeof(good),NC_NOERR);
    check("bigdmr",bigdmr,sizeof(bigdmr),NC_EDMR);
    check("bigdmrerr",bigdmrerr,sizeof(bigdmrerr),NC_EDMR);
    check("bigdata",bigdata,sizeof(bigdata),NC_EDATADAP);
    check("bigdataerr",bigdataerr,sizeof(bigdataerr),NC_EDATADAP);
    check("truncated",truncated,sizeof(truncated),NC_EDATADAP);

    return (failures ? EXIT_FAILURE : EXIT_SUCCESS);
}
