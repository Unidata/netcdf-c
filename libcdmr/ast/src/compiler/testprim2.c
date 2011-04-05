#undef FILL
#undef COMPARE

#define FILL(input,typ,val) \
    input->f_##typ.defined = (val % 2); \
        input->f_##typ.value = val;

#define COMPARE(input,output,typ) if(input->f_##typ.defined) {if(input->f_##typ.value != output->f_##typ.value) return 0;}

static void
testprim2_setup(PrimOptional* input)
{
    /* Fill in input */
    memset((void*)input,0,sizeof(PrimOptional));
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

static int
testprim2_compare(PrimOptional* input, PrimOptional* output)
{
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
    return 1;
}

static int
testprim2()
{
    char* testname = "testprim2";
    int status = AST_NOERR;
    PrimOptional input;
    PrimOptional* output = NULL;
    ast_runtime* rt;
    bytes_t encoded;

    testprim2_setup(&input);

    status = ast_byteio_new(AST_WRITE,NULL,0,&rt);
    if(status != AST_NOERR) goto fail;

    /* Write the input into the runtime buffer */
    status = PrimOptional_write(rt,&input);
    if(status != AST_NOERR) goto fail;

    /* Extract the written contents */
    status = ast_byteio_content(rt, &encoded);
    if(status != AST_NOERR) goto fail;
        
    status = ast_reclaim(rt);
    if(status != AST_NOERR) goto fail;

    status = ast_byteio_new(AST_READ,encoded.bytes,encoded.nbytes,&rt);
    if(status != AST_NOERR) goto fail;

    /* Reconstruct a PrimOptional instance from contents */
    output = NULL;
    status = PrimOptional_read(rt,&output);
    if(status != AST_NOERR) goto fail;

    status = ast_reclaim(rt);
    if(status != AST_NOERR) goto fail;

    /* Compare */
    if(!testprim2_compare(&input,output)) {
	fprintf(stderr,"*** FAIL: content mismatch: %s\n",testname);
	status = AST_EFAIL;
	goto fail;
    }

    fprintf(stderr,"*** PASS: %s\n",testname);
    return status;

fail:
    fprintf(stderr,"*** FAIL: %s  status=%d\n",testname,status);
    return status;
}
