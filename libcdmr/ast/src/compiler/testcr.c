#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
 
#include <curl/curl.h>


#include <ast_runtime.h>
#include <ast_internal.h>
#include <ast_byteio.h>
#include <ast_curl.h>

#include "ncStream.h"
#include "nccr.h"


/* Getopt values */

#define URLPREFIX "http://motherlode.ucar.edu:8080/thredds/cdmremote/nws/upperair/ncdecoded/files/Upperair_20110223_0000.nc"

//#define CONSTRAINT1 "wmoStaNum(0:10)"
//#define CONSTRAINT1 "staLat(0:10)"
#define CONSTRAINT1 "staName(0:10,0:5)"

static char* url = URLPREFIX;
static char* req = "header";

static int debug = 1;

static ast_err test1(char*);
static ast_err test2(char*);

int
main(int argc, char** argv)
{
    int status = AST_NOERR;
    int c;
    opterr = 0;
 
    while ((c = getopt (argc, argv, "ur")) != -1)
    switch (c) {
    case 'f':
	url = strdup(optarg);
	break;
    case 'r':
	req = strdup(optarg);
	break;
    case '?':
	if (optopt == 'c')
           fprintf (stderr, "Option -%c requires an argument.\n", optopt);
        else if (isprint (optopt))
            fprintf (stderr, "Unknown option `-%c'.\n", optopt);
        else
            fprintf (stderr,
                     "Unknown option character `\\x%x'.\n",
                     optopt);
          return 1;
    default:
        abort ();
    }

    if(url == NULL) {
	if(optind < argc) {
	    url = strdup(argv[optind]);
	} else {
	    fprintf(stderr,"No url specified\n");
	    exit(1);
	}
    }

    if(debug) printf("Using url: %s\n",url);

    status = test1(url);
    if(status != AST_NOERR) goto done;
    status = test2(url);
    if(status != AST_NOERR) goto done;

done:
    exit(0);
}

static ast_err
fetchheaderdata(char* baseurl, bytes_t* packet)
{
    ast_err status = AST_NOERR;
    CURL* curl = NULL;
    long filetime = 0;
    char* fullurl;

    fullurl = (char*)malloc(strlen(baseurl)+strlen("?req=header")+1);
    strcpy(fullurl,baseurl);
    strcat(fullurl,"?req=header");

    status = ast_curlopen(&curl);
    if(status != AST_NOERR) goto done;

    status = ast_fetchurl(curl, fullurl, packet, &filetime);
    if(status != AST_NOERR) goto done;

    status = ast_curlclose(curl);
    if(status != AST_NOERR) goto done;

done:
    return status;
}


static ast_err
skiptoheader(bytes_t* packet)
{
    ast_err status = AST_NOERR;
    unsigned long long vlen;
    size_t size;

    /* Check the structure of the resulting data */
    if(packet->nbytes < (strlen(MAGIC_HEADER) + strlen(MAGIC_HEADER))) {
	fprintf(stderr,"*** FAIL: curl data too short: %d\n",packet->nbytes);
	status = AST_EFAIL;
	goto done;
    }
    if(memcmp(packet->bytes,MAGIC_HEADER,strlen(MAGIC_HEADER)) != 0) {
	fprintf(stderr,"*** FAIL: MAGIC_HEADER missing\n");
	status = AST_EFAIL;
	goto done;
    }
    packet->nbytes -= strlen(MAGIC_HEADER);
    packet->bytes += strlen(MAGIC_HEADER);
    /* Extract the proposed count as a varint */
    vlen = varint_decode(10,packet->bytes,&size);
    packet->nbytes -= size;
    packet->bytes += size;
    if(vlen != packet->nbytes) {
	fprintf(stderr,"*** FAIL: curl data size mismatch\n");
	status = AST_EFAIL;
	goto done;
    }

done:
    return status;    
}

static ast_err
decodeheaderdata(bytes_t* packet, Header** hdrp)
{
    ast_err status = AST_NOERR;
    ast_runtime* rt = NULL;
    Header* protohdr = NULL;

    /* Now decode the buffer */
    status = ast_byteio_new(AST_READ,packet->bytes,packet->nbytes,&rt);
    if(status != AST_NOERR) goto done;

    status = Header_read(rt,&protohdr);
    if(status != AST_NOERR)
	fprintf(stderr,"*** FAIL: Header decode failed\n");

    status = ast_reclaim(rt);
    if(status != AST_NOERR) goto done;

    if(hdrp) *hdrp = protohdr;

done:
    return status;
}

static ast_err
fetchheader(char* url, Header** hdrp)
{
    ast_err status = AST_NOERR;
    bytes_t data;

    status = fetchheaderdata(url,&data);
    if(status != AST_NOERR) goto done;

    status = skiptoheader(&data);
    if(status != AST_NOERR) goto done;

    status = decodeheaderdata(&data,hdrp);
    if(status != AST_NOERR) goto done;

done:
    return status;    
}


/*
This test verifies that we can read a cdmremote url
header into memory using libcurl
*/

static ast_err
test1(char* url)
{
    ast_err status = AST_NOERR;
    bytes_t packet;
    Header* protohdr;

    status = fetchheaderdata(url,&packet);
    if(status != AST_NOERR) goto done;

    status = skiptoheader(&packet);
    if(status != AST_NOERR) goto done;

    status = decodeheaderdata(&packet,&protohdr);
    if(status != AST_NOERR) goto done;

    /* Dump the result */
    status = nccr_dumpheader(protohdr);
    if(status != AST_NOERR) goto done;

    if(status == AST_NOERR)
	fprintf(stderr,"*** PASS\n");

done:
    return status;
}

static Variable*
locatevariable(Header* protohdr, char* varname)
{
    /* Temp: assume no groups */
    int i;
    Group* root = protohdr->root;
    for(i=0;i<root->vars.count;i++) {
	if(strcmp(root->vars.values[i]->name,varname)==0) return root->vars.values[i];
    }    
    return NULL;
}

/*
This test verifies that we can read a cdmremote url
content into memory using libcurl
*/

static ast_err
test2(char* url)
{
    ast_err status = AST_NOERR;
    CURL* curl = NULL;
    long filetime = 0;
    unsigned long long vlen;
    size_t size;
    ast_runtime* rt;
    char* dataurl;
    bytes_t packet;
    Header* protohdr;
    Data* protodata;
    bytes_t data;
    int bigendian = 1;

    status = fetchheader(url,&protohdr);
    if(status != AST_NOERR) goto done;

    /* Fetch the data */
    dataurl = (char*)malloc(strlen(url)+1+strlen(CONSTRAINT1)+1);
    strcpy(dataurl,url);
    strcat(dataurl,"?");
    strcat(dataurl,CONSTRAINT1);

    status = ast_curlopen(&curl);
    if(status != AST_NOERR) goto done;

    status = ast_fetchurl(curl, dataurl, &packet, &filetime);
    if(status != AST_NOERR) goto done;

    status = ast_curlclose(curl);
    if(status != AST_NOERR) goto done;

    /* Check the structure of the resulting data */
    if(packet.nbytes < (strlen(MAGIC_DATA) + strlen(MAGIC_DATA))) {
	fprintf(stderr,"*** FAIL: curl data too short: %d\n",packet.nbytes);
	status = AST_EFAIL;
	goto done;
    }

    if(memcmp(packet.bytes,MAGIC_DATA,strlen(MAGIC_DATA)) != 0) {
	fprintf(stderr,"*** FAIL: MAGIC_DATA missing\n");
	status = AST_EFAIL;
	goto done;
    }

    packet.nbytes -= strlen(MAGIC_DATA);
    packet.bytes += strlen(MAGIC_DATA);

    /* Extract the proposed count as a varint */
    vlen = varint_decode(10,packet.bytes,&size);
    packet.nbytes -= size;
    packet.bytes += size;
 
    /* Now decode the buffer */
    status = ast_byteio_new(AST_READ,packet.bytes,packet.nbytes,&rt);
    if(status != AST_NOERR) goto done;
    ast_mark(rt,vlen);

    status = Data_read(rt,&protodata);
    if(status != AST_NOERR) {
	fprintf(stderr,"*** FAIL: Data decode failed\n");
	goto done;
    }

    ast_unmark(rt);

    /* track the position */
    packet.nbytes -= vlen;
    packet.bytes += vlen;

    /* Next is another count */
    vlen = varint_decode(10,packet.bytes,&size);
    packet.nbytes -= size;
    packet.bytes += size;
    if(vlen != packet.nbytes) {
	fprintf(stderr,"*** FAIL: curl packet size mismatch\n");
	status = AST_EFAIL;
	goto done;
    }

    /* Finally is the data itself */
    data.nbytes = packet.nbytes;
    data.bytes = (uint8_t*)malloc(data.nbytes);
    memcpy(data.bytes,packet.bytes,data.nbytes);

    /* Locate the variable in the header */
    Variable* var = locatevariable(protohdr,protodata->varName);
    if(var == NULL) {
	fprintf(stderr,"*** FAIL: cannot locate data variable: %s\n",protodata->varName);
	status = AST_EFAIL;
	goto done;
    }

    /* Verify DataType */
    if(var->dataType != protodata->dataType) {
	fprintf(stderr,"*** FAIL: datatype mismatch\n");
	status = AST_EFAIL;
	goto done;
    }

    if(protodata->bigend.defined) bigendian = (protodata->bigend.value?1:0);
    if(protodata->compress.defined && protodata->compress.value == DEFLATE) {
	fprintf(stderr,"*** FAIL: compression not supported\n");
	status = AST_EFAIL;
	goto done;
    }
    if(protodata->crc32.defined) {
	/* Do nothing until we find a stand-alone crc32 calculator */
    }

    switch (protodata->dataType) {
    case ENUM1:
    case ENUM2:
    case ENUM4:
    case STRUCTURE:
    case SEQUENCE:
	fprintf(stderr,"*** FAIL: unsupported DataType: %d\n",(int)protodata->dataType);
	status = AST_EFAIL;
	goto done;
    default:
	/* simple array of data */
	nccr_data_dump(protodata, var, bigendian, &data);
	break;
    }


    status = ast_reclaim(rt);
    if(status != AST_NOERR) goto done;

#ifdef IGNORE
    /* Dump the result */
    status = nccr_data_dum(protodata, );
    if(status != AST_NOERR) goto done;
#endif

    if(status == AST_NOERR)
	fprintf(stderr,"*** PASS\n");

done:
    return status;

#ifdef IGNORE
dump:
    fprintf(stderr,"DUMP:\n");
    fprintf(stderr,packet.bytes);
    return AST_EFAIL;
#endif
}

void
dump(bytes_t* packet)
{
    fprintf(stderr,"DUMP:\n");
    fprintf(stderr,packet->bytes);
    fflush(stderr);
}
