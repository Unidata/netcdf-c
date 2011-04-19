#undef FILL
#undef COMPARE

#define RFILLS(input,val) input->f_string = (val)
#define RFILLB(input,val) input->f_bytes.nbytes = strlen(val); input->f_bytes.bytes = (uint8_t*)(val)

#define RCOMPARES(input,output) if(strcmp(input->f_string,output->f_string)!=0) return n_string
#define RCOMPAREB(input,output) {\
   if(input->f_bytes.nbytes != output->f_bytes.nbytes) return 0; \
   if(memcmp((void*)input->f_bytes.bytes,output->f_bytes.bytes,input->f_bytes.nbytes) != 0) return n_bytes; \
}

#define OFILLS(input,ex,val) \
	input->f_string.defined = ex; input->f_string.value = val

#define OFILLB(input,ex,val) \
    input->f_bytes.defined = (ex); \
        input->f_bytes.value.nbytes = strlen(val); \
        input->f_bytes.value.bytes = (uint8_t*)(val)

#define OCOMPARES(input,output) \
	if(input->f_string.defined \
	    && (strcmp(input->f_string.value,output->f_string.value)!=0)) \
	    return n_string

#define OCOMPAREB(input,output) \
	if(input->f_bytes.defined \
	    && (input->f_bytes.value.nbytes != input->f_bytes.value.nbytes \
	        || memcmp(input->f_bytes.value.bytes,output->f_bytes.value.bytes,input->f_bytes.value.nbytes)!=0)) return n_bytes

#define PFILLS(input,cnt,val) \
    input->f_string.count = cnt; \
    input->f_string.values = (char**)calloc(cnt,sizeof(char*)); \
    for(i=0;i<cnt;i++) {input->f_string.values[i] = val[i];}

#define PFILLB(input,cnt,val) \
    input->f_bytes.count = cnt; \
    input->f_bytes.values = (bytes_t*)calloc(cnt,sizeof(bytes_t)); \
    for(i=0;i<cnt;i++) { \
	input->f_bytes.values[i].nbytes = strlen(val[i]); \
	input->f_bytes.values[i].bytes = (uint8_t*)val[i]; \
    }

#define PCOMPARES(input,output) \
    if(input->f_string.count != output->f_string.count) return n_string; \
    for(i=0;i<input->f_string.count;i++) {\
	if(strcmp(input->f_string.values[i],output->f_string.values[i])!=0) return n_string;\
    }

#define PCOMPAREB(input,output) \
    if(input->f_bytes.count != input->f_bytes.count) return n_bytes; \
    for(i=0;i<input->f_bytes.count;i++) {\
	if(input->f_bytes.values[i].nbytes != output->f_bytes.values[i].nbytes) return n_bytes; \
        if(memcmp(input->f_bytes.values[i].bytes,output->f_bytes.values[i].bytes,input->f_bytes.values[i].nbytes)!=0) return n_bytes; \
    }

static char*
testprim4_rcompare(BytesRequired* input, BytesRequired* output)
{
    RCOMPARES(input,output);
    RCOMPAREB(input,output);
    return NULL;
}

static char*
testprim4_ocompare(BytesOptional* input, BytesOptional* output)
{
    OCOMPARES(input,output);
    OCOMPAREB(input,output);
    return NULL;
}

static char*
testprim4_pcompare(BytesRepeated* input, BytesRepeated* output)
{
    int i;
    PCOMPARES(input,output);
    PCOMPAREB(input,output);
    return NULL;
}

static int
testprim4r()
{
    int status = AST_NOERR;
    BytesRequired* input = (BytesRequired*)malloc(sizeof(BytesRequired));
    BytesRequired* output = NULL;
    ast_runtime* rt = NULL;
    bytes_t encoded;
    char* fieldname = NULL;

    memset((void*)input,0,sizeof(BytesRequired));
    RFILLS(input,"1");
    RFILLB(input,"\x01");

    status = ast_byteio_new(AST_WRITE,NULL,0,&rt);
    /* Write the input into the runtime buffer */
    status = BytesRequired_write(rt,input);
    if(status != AST_NOERR) goto done;
    /* Reconstruct instance from contents */
    output = NULL;
    /* Extract the written contents */
    status = ast_byteio_content(rt, &encoded);
    /* Compare */
    status = ast_byteio_new(AST_READ,encoded.bytes,encoded.nbytes,&rt);
    status = BytesRequired_read(rt,&output);
    if(status != AST_NOERR) goto done;
    fieldname = testprim4_rcompare(input,output);
    if(fieldname != NULL) {
	fprintf(stderr,"*** FAIL: content mismatch: %s: field f_%s\n",testname,fieldname);
	status = AST_EFAIL;
	goto done;
    }

done:
    return status;
}

static int
testprim4o()
{
    int status = AST_NOERR;
    BytesOptional* input = (BytesOptional*)malloc(sizeof(BytesOptional));
    BytesOptional* output;
    ast_runtime* rt = NULL;
    bytes_t encoded;
    char* fieldname = NULL;

    memset((void*)input,0,sizeof(BytesOptional));
    OFILLS(input,1,"1");
    OFILLB(input,1,"\x01");

    status = ast_byteio_new(AST_WRITE,NULL,0,&rt);
    /* Write the input into the runtime buffer */
    status = BytesOptional_write(rt,input);
    if(status != AST_NOERR) goto done;

    /* Reconstruct instance from contents */
    output = NULL;
    /* Extract the written contents */
    status = ast_byteio_content(rt, &encoded);

    /* Compare */
    status = ast_byteio_new(AST_READ,encoded.bytes,encoded.nbytes,&rt);
    status = BytesOptional_read(rt,&output);
    if(status != AST_NOERR) goto done;
    fieldname = testprim4_ocompare(input,output);
    if(fieldname != NULL) {
	fprintf(stderr,"*** FAIL: content mismatch: %s: field f_%s\n",testname,fieldname);
	status = AST_EFAIL;
	goto done;
    }

done:
    return status;
}

static int
testprim4p()
{
    int status = AST_NOERR;
    int i;

    BytesRepeated* input = (BytesRepeated*)malloc(sizeof(BytesRepeated));
    BytesRepeated* output = NULL;
    ast_runtime* rt = NULL;
    bytes_t encoded;
    char* fieldname = NULL;

    memset((void*)input,0,sizeof(BytesRepeated));
    {
	char* svals[] = {"1","2","3"};
	char* bvals[] = {"0x01","0x02","0x03"};
        PFILLS(input,3,svals);
        PFILLB(input,3,bvals);
    }

    status = ast_byteio_new(AST_WRITE,NULL,0,&rt);
    /* Write the input into the runtime buffer */
    status = BytesRepeated_write(rt,input);
    if(status != AST_NOERR) goto done;

    /* Reconstruct instance from contents */
    output = NULL;
    /* Extract the written contents */
    status = ast_byteio_content(rt, &encoded);

    /* Compare */
    status = ast_byteio_new(AST_READ,encoded.bytes,encoded.nbytes,&rt);
    status = BytesRepeated_read(rt,&output);
    if(status != AST_NOERR) goto done;
    fieldname = testprim4_pcompare(input,output);
    if(fieldname != NULL) {
	fprintf(stderr,"*** FAIL: content mismatch: %s: field f_%s\n",testname,fieldname);
	status = AST_EFAIL;
	goto done;
    }

done:
    return status;
}

static int
testprim4()
{
    char* testname = "testprim4";
    int status = AST_NOERR;

    status = testprim4r();
    if(status) goto fail;
    status = testprim4o();
    if(status) goto fail;
    status = testprim4p();
    if(status) goto fail;

    fprintf(stderr,"*** PASS: %s\n",testname);
    return status;

fail:
    fprintf(stderr,"*** FAIL: %s  status=%d\n",testname,status);
    return status;
}
