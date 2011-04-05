#undef FILL
#undef COMPARE

static Enumtest*
create_enumtest(ast_runtime* rt)
{
    Enumtest* input = (Enumtest*)ast_alloc(rt,sizeof(Enumtest));
    input->renum = ECON1;
    input->oenum.defined = 1;
    input->oenum.value = ECON2;
    input->penum.count = 2;
    input->penum.values = (Testenum*)ast_alloc(rt,input->penum.count*sizeof(Testenum));
    input->penum.values[0] = ECON3;
    input->penum.values[1] = ECON4;
    input->ppenum.count = 4;
    input->ppenum.values = (Testenum*)ast_alloc(rt,input->ppenum.count*sizeof(Testenum));
    input->ppenum.values[0] = ECON3;
    input->ppenum.values[1] = ECON4;
    input->ppenum.values[1] = ECON5;
    input->ppenum.values[1] = ECON6;
#ifdef IGNORE
#endif
    return input;
}

static int
compare_enumtest(Enumtest* input, Enumtest* output)
{
    int i;
    if(input->renum != output->renum) return 0;
    if(input->oenum.defined != output->oenum.defined) return 0;
    if(input->oenum.defined
       && input->oenum.value != output->oenum.value) return 0;
    if(input->penum.count != output->penum.count) return 0;
    for(i=0;i<input->penum.count;i++)
	if(input->penum.values[i] != output->penum.values[i]) return 0;
    if(input->ppenum.count != output->ppenum.count) return 0;
    for(i=0;i<input->ppenum.count;i++)
	if(input->ppenum.values[i] != output->ppenum.values[i]) return 0;
#ifdef IGNORE
#endif
    return 1;
}

static int
testenum1()
{
    char* testname = "testenum1";
    int status = AST_NOERR;
    Enumtest* input = NULL;
    Enumtest* output = NULL;
    ast_runtime* rt;
    bytes_t encoded;

    status = ast_byteio_new(AST_WRITE,NULL,0,&rt);
    if(status != AST_NOERR) goto fail;

    input = create_enumtest(rt);

    /* Write the input into the runtime buffer */
    status = Enumtest_write(rt,input);
    if(status != AST_NOERR) goto fail;

    /* Extract the written contents */
    status = ast_byteio_content(rt, &encoded);
    if(status != AST_NOERR) goto fail;
        
    status = ast_reclaim(rt);
    if(status != AST_NOERR) goto fail;

    status = ast_byteio_new(AST_READ,encoded.bytes,encoded.nbytes,&rt);
    if(status != AST_NOERR) goto fail;

    /* Reconstruct a Enumtest instance from contents */
    output = NULL;
    status = Enumtest_read(rt,&output);
    if(status != AST_NOERR) goto fail;

    status = ast_reclaim(rt);
    if(status != AST_NOERR) goto fail;

    /* Compare */
    if(!compare_enumtest(input,output)) {
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
