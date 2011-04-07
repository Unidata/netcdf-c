#undef FILL
#undef COMPARE

static Submsg*
create_submsg(ast_runtime* rt,int32_t val)
{
    Submsg* submsg = (Submsg*)ast_alloc(rt,sizeof(Submsg));
    submsg->f_int32 = val;
    return submsg;
}

static Msgtest*
create_msgtest(ast_runtime* rt)
{
    Msgtest* input = (Msgtest*)ast_alloc(rt,sizeof(Msgtest));
    input->rmsg = create_submsg(rt,17);
    input->omsg.defined = 1;
    input->omsg.value = create_submsg(rt,19);
    input->pmsg.count = 2;
    input->pmsg.values = (Submsg**)ast_alloc(rt,input->pmsg.count*sizeof(Submsg*));
    input->pmsg.values[0] = create_submsg(rt,23);
    input->pmsg.values[1] = create_submsg(rt,29);
    return input;
}

static int
compare_submsg(Submsg* input, Submsg* output)
{
    if(input->f_int32 != output->f_int32) return 0;
    return 1;
}

static int
compare_msgtest(Msgtest* input, Msgtest* output)
{
    int i;
    if(!compare_submsg(input->rmsg,output->rmsg)) return 0;
    if(input->omsg.defined != output->omsg.defined) return 0;
    if(input->omsg.defined
       && !compare_submsg(input->omsg.value,output->omsg.value)) return 0;
    if(input->pmsg.count != output->pmsg.count) return 0;
    for(i=0;i<input->pmsg.count;i++)
        if(!compare_submsg(input->pmsg.values[i],output->pmsg.values[i])) return 0;
    return 1;
}

static int
testmsg1()
{
    char* testname = "testmsg1";
    int status = AST_NOERR;
    Msgtest* input = NULL;
    Msgtest* output = NULL;
    ast_runtime* rt;
    bytes_t encoded;

    status = ast_byteio_new(AST_WRITE,NULL,0,&rt);
    if(status != AST_NOERR) goto fail;

    input = create_msgtest(rt);

    /* Write the input into the runtime buffer */
    status = Msgtest_write(rt,input);
    if(status != AST_NOERR) goto fail;

    /* Extract the written contents */
    status = ast_byteio_content(rt, &encoded);
    if(status != AST_NOERR) goto fail;
        
    status = ast_reclaim(rt);
    if(status != AST_NOERR) goto fail;

    status = ast_byteio_new(AST_READ,encoded.bytes,encoded.nbytes,&rt);
    if(status != AST_NOERR) goto fail;

    /* Reconstruct a Msgtest instance from contents */
    output = NULL;
    status = Msgtest_read(rt,&output);
    if(status != AST_NOERR) goto fail;

    status = ast_reclaim(rt);
    if(status != AST_NOERR) goto fail;

    /* Compare */
    if(!compare_msgtest(input,output)) {
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
