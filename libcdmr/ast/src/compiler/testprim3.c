#undef FILL
#define FILL(input,typ,val) \
    input->f_##typ.count = val; \
        input->f_##typ.values = (typ##_t*)calloc(val,sizeof(typ##_t)); \
        for(i=0;i<val;i++) {input->f_##typ.values[i] = ((val<<8)+i);}

#undef COMPARE
#define COMPARE(input,output,typ) \
    if(input->f_##typ.count != output->f_##typ.count) return n_##typ; \
    for(i=0;i<input->f_##typ.count;i++) {\
	if(input->f_##typ.values[i] != output->f_##typ.values[i]) return n_##typ;\
    }

static void
testprim3_setup(PrimRepeated* input)
{
    int i;
    /* Fill in input */
    memset((void*)input,0,sizeof(PrimRepeated));
    FILL(input,int32,1);
    FILL(input,int64,2);
    FILL(input,uint32,3);
    FILL(input,uint64,4);
    FILL(input,sint32,5);
    FILL(input,sint64,6);
    FILL(input,fixed32,7);
    FILL(input,fixed64,8);
    FILL(input,sfixed32,9);
    FILL(input,sfixed64,10);
    FILL(input,double,11);
    FILL(input,float,12);
}

static char*
testprim3_compare(PrimRepeated* input, PrimRepeated* output)
{
    int i;
    COMPARE(input,output,int32);
    COMPARE(input,output,int64);
    COMPARE(input,output,uint32);
    COMPARE(input,output,uint64);
    COMPARE(input,output,sint32);
    COMPARE(input,output,sint64);
    COMPARE(input,output,fixed32);
    COMPARE(input,output,fixed64);
    COMPARE(input,output,sfixed32);
    COMPARE(input,output,sfixed64);
    COMPARE(input,output,double);
    COMPARE(input,output,float);
    return NULL;
}

static int
testprim3()
{
    char* testname = "testprim3";
    int status = AST_NOERR;
    PrimRepeated input;
    PrimRepeated* output = NULL;
    ast_runtime* rt;
    bytes_t encoded;
    char* fieldname = NULL;

    testprim3_setup(&input);

    status = ast_byteio_new(AST_WRITE,NULL,0,&rt);
    if(status != AST_NOERR) goto fail;

    /* Write the input into the runtime buffer */
    status = PrimRepeated_write(rt,&input);
    if(status != AST_NOERR) goto fail;

    /* Extract the written contents */
    status = ast_byteio_content(rt, &encoded);
    if(status != AST_NOERR) goto fail;
        
    status = ast_reclaim(rt);
    if(status != AST_NOERR) goto fail;

    status = ast_byteio_new(AST_READ,encoded.bytes,encoded.nbytes,&rt);
    if(status != AST_NOERR) goto fail;

    /* Reconstruct a PrimRepeated instance from contents */
    output = NULL;
    status = PrimRepeated_read(rt,&output);
    if(status != AST_NOERR) goto fail;

    status = ast_reclaim(rt);
    if(status != AST_NOERR) goto fail;

    /* Compare */
    fieldname = testprim3_compare(&input,output);
    if(fieldname != NULL) {
	fprintf(stderr,"*** FAIL: content mismatch: %s: field f_%s\n",testname,fieldname);
	status = AST_EFAIL;
	goto fail;
    }

    fprintf(stderr,"*** PASS: %s\n",testname);
    return status;

fail:
    fprintf(stderr,"*** FAIL: %s  status=%d\n",testname,status);
    return status;
}
