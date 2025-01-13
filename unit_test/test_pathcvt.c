/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

/**
Test ncbytes.h
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "netcdf.h"
#include "ncbytes.h"

#define DEBUG

#define BUFSIZE 4096
#define MAXSTR 2048

typedef struct Test {
    char* test;
    char* expected;
} Test;

typedef enum Operand {OR_NULL=0,OR_INT=1,OR_CHR=2,OR_STR=3} Operand;
typedef struct Arg {
    Operand sort;
    union {
        size_t i;
	char c;
	char s[MAXSTR];
    } u;
} Arg;

typedef struct Inst {
    char op;
    Arg args[3];
} Inst;

static const char* code2txt[] = {
"0 free",
"1 new",
NULL
};

static void
reclaiminst(Inst* inst)
{
    size_t i;
    for(i=0;i<3;i++) {
	if(inst->args[i].sort == OR_STR || inst->args[i].u.s != NULL) free(inst->args[i].u.s);
    }
}

static void
reclaimprog(Inst* prog)
{
    size_t i;
    for(i=0;;i++) {
	reclaiminst(&prog[i]);
    }
}

static size_t
decodearg(const char* code, size_t ap, Arg* arg)
{
    char c;
    int count;

    /* Discriminate argument type */
    c = code[ap];
    if(strchr("0123456789",c)!=NULL) {
	arg->sort = OR_INT;
	count = -1;
	sscanf(&code[ap],"%zu%n",&arg->u.i,&count);
	assert(count > 0);
	ap += (size_t)count;
	goto done;
    } else if(c == '\'') {
	arg->sort = OR_CHR;
	ap++;
	c = code[ap];
	assert(c != '\0');
	arg->u.c = c;	
	ap++;
	goto done;
    } else if(c == '|') {
	char* p;
	ptrdiff_t slen;
	arg->sort = OR_STR;
	ap++;
	p = strchr(&code[ap],'|');
	assert(p != NULL);
	slen = (p - &code[ap]);
	assert(slen >= 0);
	assert(slen < MAXSTR);
	memcpy(arg->u.s,&code[ap],(size_t)slen);
	arg->u.s[slen] = '\0';
	ap += (size_t)slen;
	goto done;
    } else
	{abort();}
done:
    return ap;
}

static int
parseprog(const char* code, Inst** progp)
{
    int stat = NC_NOERR;
    size_t i,n,ap,ip;
    Inst* prog = NULL;

    if(code == NULL) return NC_EINVAL;
    n = strlen(code);      
    if((prog = calloc(n,sizeof(Inst)))==NULL) return NC_ENOMEM;
    ap = code; ip = prog;
    for(ap=0;ap<n;ap++) {
	char op = code[ap];
	prog[ip].op = op;
	
	/* Zero arg instructions */
	case '0':
	case '1':
	case 'd':
	case 'e':
	case 'n':
	case 'C':
	case 'D':
	case 'X':
	    break;
	
	/* One arg instructions */
	case 'a':
	case 'c':
	case 'f':
	case 'g':
	case 'r':
	case 'v':
	case 'x':
	case 'A':
	case 'L':
	    ap = decodearg(code,ap,&prog[ip].arg[0];
	    break;
	
	/* Two arg instructions */
	case 's':
	case 'S':
	    ap = decodearg(code,ap,&prog[ip].arg[0];
    	    ap = decodearg(code,ap,&prog[ip].arg[1];
	    break;

	/* Three arg instructions */
	case 'i':
	    ap = decodearg(code,ap,&prog[ip].arg[0];
    	    ap = decodearg(code,ap,&prog[ip].arg[1];
       	    ap = decodearg(code,ap,&prog[ip].arg[2];
	    break;

	default:
	    fprintf("Illegal operator[%zu] '%c'\n",ap,c); 
	    stat = NCEINTERNAL;
	    goto done;
	}

    }
}







static size_t
contentize(const char* s, char* buf)
{
    size_t used = blen;
    size_t slen, blen;
    int elided = 0;
    char* p = buf;

    if(s == NULL) return used;
    slen = strlen(s);
    if(slen > MAXSTR) {slen = (MAXSTR - strlen("...")); elided = 1;}

    blen = strlen(buf);
    b = &buf[blen];
    memcpy(b,s,slen);
    b += slen;
    blen += slen;
    if(elided) {
        memcpy(b,"...",3);
	b += 3;
	blen += 3;
    }
    return blen;
}

static const char*
ncbytesdump(NCbyte* bb)
{
static char* buf[BUFSIZE];
static const char* template1 = "{extendible=%c alloc=%zu len=%zu content=|";
static const char* template2 = "|}";
    size_t blen;

    sprintf(buf,sizeof(buf),template1,
    		(bb->extendible?"1":"0"),
		bb->alloc,
		bb->length);
    blen = contentize(bb->content,buf);
    memcpy(&buf[blen],template2,sizeof(template2));
    blen += sizeof(template2);
    buf[blen++] = '\0';
    assert(blen <= BUFSIZE);
    return buf;
}

static int
interpret(const char* prog, char** resultp)
{
    int stat = NC_NOERR;
    size_t ap, progsize
    NCbytes* bb = NULL;

    progsize = strlen(prog);
    for(ap=0;ap<=progsize;ap++) {
	const char c = prog[ap];
	switch (c) {

	case '1':
	    assert(bb == NULL);
	    bb = ncbytesnew();
	    printf("[%zu] %c: %s\n",ap,c,ncbytesdump(bb));
	    break;
	case '0':
	    ncbytesfree(bb); bb = NULL;
	    printf("[%zu] %c\n",ap,c);
	    break;
	case 'A':
	    assert(bb != NULL);
	    ncbytessetalloc(bb = ncbytesnew();
	    printf("[%zu] %c: %s\n",ap,c,ncbytesdump(bb));
	    break;
EXTERNL int ncbytessetalloc(NCbytes*,unsigned long);
case 'L':
EXTERNL int ncbytessetlength(NCbytes*,unsigned long);
case 'f':
EXTERNL int ncbytesfill(NCbytes*, char fill);
case 'd':
EXTERNL char* ncbytesdup(NCbytes*);
case 'e':
EXTERNL char* ncbytesextract(NCbytes*);
case 'g':
EXTERNL int ncbytesget(NCbytes*,unsigned long);
case 's':
EXTERNL int ncbytesset(NCbytes*,unsigned long,char);
case 'a':
EXTERNL int ncbytesappend(NCbytes*,char); /* Add at Tail */
EXTERNL int ncbytesappendn(NCbytes*,const void*,unsigned long); /* Add at Tail */
case 'n':
EXTERNL int ncbytesnull(NCbytes*);
case 'r':
EXTERNL int ncbytesremove(NCbytes*,unsigned long);
case 'c':
EXTERNL int ncbytescat(NCbytes*,const char*);
case 'i':
EXTERNL int ncbytesinsert(NCbytes*,size_t pos, size_t n, const char*);
case 'S':
EXTERNL int ncbytessetcontents(NCbytes*, void*, unsigned long);
case 'C':
#define ncbytescontents(bb) (((bb)!=NULL && (bb)->content!=NULL)?(bb)->content:(char*)"")
case 'x':
#define ncbytesextend(bb,len) ncbytessetalloc((bb),(len)+(bb->alloc))
case 'D':
#define ncbytesclear(bb) ((bb)!=NULL?(bb)->length=0:0)
case 'V':
#define ncbytesavail(bb,n) ((bb)!=NULL?((bb)->alloc - (bb)->length) >= (n):0)
case 'X':
#define ncbytesextendible(bb) ((bb)!=NULL?((bb)->nonextendible?0:1):0)

	default:
	    fprintf("Illegal code[%zu] 'c'\n",ap,c); 
	    stat = NCEINTERNAL;
	    goto done;
	}


    }
}




int
main(int argc, char** argv)
{
    Test* test;
    int failcount = 0;
    char* unescaped = NULL;
    char* expanded = NULL;
    int k;
    int drive = 'c';

    nc_initialize();

    setmacros();

    /* Test localkind X path-kind */
    for(test=PATHTESTS;test->test;test++) {
#ifdef DEBUG
	int inputkind = NCgetinputpathkind(test->test);
#endif
        /* Iterate over the test paths */
        for(k=0;k<NKINDS;k++) {
	    int kind = kinds[k];
	    /* Compare output for the localkind */
            if(test->expected[k] == NULL) {
#ifdef DEBUG
	        fprintf(stderr,"TEST input=%s target=%s: %s ignored\n",kind2string(inputkind),kind2string(kind),test->test);
#endif
	        continue;
	    }
	    /* ensure that NC_shellUnescape does not affect result */
	    unescaped = NC_shellUnescape(test->test);	
	    expanded = expand(test->expected[k]);
#ifdef DEBUG
	    fprintf(stderr,">>> unescaped=|%s| expanded=|%s|\n",unescaped,expanded);
#endif
#ifdef DEBUG
	    fprintf(stderr,"TEST input=%s target=%s: input: |%s|\n",
			kind2string(inputkind),kind2string(kind),test->test);
#endif
   	    cvt = NCpathcvt_test(unescaped,kind,drive);
#ifdef DEBUG
	    fprintf(stderr,"\texpected=|%s| actual=|%s|: ",expanded,cvt);
#endif
	    fflush(stderr); fflush(stdout);
	    if(cvt == NULL) {
#ifdef DEBUG
		fprintf(stderr," ILLEGAL");
#endif
		failcount++;
	    } else if(strcmp(cvt,expanded) != 0) {
#ifdef DEBUG
		fprintf(stderr," FAIL");
#endif
	        failcount++;
	    } else {
#ifdef DEBUG
		fprintf(stderr," PASS");
#endif
	    }
#ifdef DEBUG
	    fprintf(stderr,"\n");
#endif	    
	    nullfree(unescaped); unescaped = NULL;
	    nullfree(expanded); expanded = NULL;
	    nullfree(cvt); cvt = NULL;
	}
    }
    nullfree(cvt); nullfree(unescaped);
    fprintf(stderr,"%s test_pathcvt\n",failcount > 0 ? "***FAIL":"***PASS");

    reclaimmacros();

    nc_finalize();
    return (failcount > 0 ? 1 : 0);
}

