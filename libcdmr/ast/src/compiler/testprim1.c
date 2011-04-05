#undef FILL
#undef COMPARE

#define FILL(input,typ,val) input->f_##typ = val
#define COMPARE(input,output,typ) if(input->f_##typ != output->f_##typ) return 0


static void
testprim1_setup(PrimRequired* input)
{
    /* Fill in input */
    memset((void*)input,0,sizeof(PrimRequired));
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
testprim1_compare(PrimRequired* input, PrimRequired* output)
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
    if(input->f_int64 != output->f_int64) return 0;
    if(input->f_uint32 != output->f_uint32) return 0;
    if(input->f_uint64 != output->f_uint64) return 0;
    if(input->f_sint32 != output->f_sint32) return 0;
    if(input->f_sint64 != output->f_sint64) return 0;
    if(input->f_fixed32 != output->f_fixed32) return 0;
    if(input->f_fixed64 != output->f_fixed64) return 0;
    if(input->f_sfixed32 != output->f_sfixed32) return 0;
    if(input->f_sfixed64 != output->f_sfixed64) return 0;
    if(input->f_double != output->f_double) return 0;
    if(input->f_float != output->f_float) return 0;
    return 1;
}

static int
testprim1()
{
    char* testname = "testprim1";
    int status = AST_NOERR;
    PrimRequired input;
    PrimRequired* output = NULL;
    ast_runtime* rt;
    bytes_t encoded;

    testprim1_setup(&input);

    status = ast_byteio_new(AST_WRITE,NULL,0,&rt);
    if(status != AST_NOERR) goto fail;

    /* Write the input into the runtime buffer */
    status = PrimRequired_write(rt,&input);
    if(status != AST_NOERR) goto fail;

    /* Extract the written contents */
    status = ast_byteio_content(rt, &encoded);
    if(status != AST_NOERR) goto fail;
        
    status = ast_reclaim(rt);
    if(status != AST_NOERR) goto fail;

    status = ast_byteio_new(AST_READ,encoded.bytes,encoded.nbytes,&rt);
    if(status != AST_NOERR) goto fail;

    /* Reconstruct a PrimRequired instance from contents */
    output = NULL;
    status = PrimRequired_read(rt,&output);
    if(status != AST_NOERR) goto fail;

    status = ast_reclaim(rt);
    if(status != AST_NOERR) goto fail;

    /* Compare */
    if(!testprim1_compare(&input,output)) {
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
