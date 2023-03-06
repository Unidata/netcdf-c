/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#ifndef GENERATE_H
#define GENERATE_H 1

typedef struct Generator Generator;

void alignbuffer(struct NCConstant* prim, Bytebuffer* buf);

/* Code dump support procedures */
void bbindent(Bytebuffer*,const int);
void bbprintf(Bytebuffer*,const char *fmt, ...); /* append */
void bbprintf0(Bytebuffer*,const char *fmt, ...); /* clear, then append*/
/* Following dump to codebuffer */
void codeprintf(const char *fmt, ...);
void codedump(Bytebuffer*);
void codepartial(const char*);
void codeline(const char*);
void codelined(int n,const char*);
void codeflush(void); /* flush codebuffer to stdout */

void commify(Bytebuffer* buf);
char* word(char* p, Bytebuffer* buf);

/* Provide buffers for language based generators */
extern Bytebuffer* codebuffer; /* buffer over the std output */
extern Bytebuffer* stmt; /* single stmt text generation */

/* From genchar.c */
void gen_charattr(Datalist*, Bytebuffer*);
void gen_charseq(Datalist*, Bytebuffer*);
void gen_chararray(Dimset*, int, Datalist*, Bytebuffer*, Datalist*);

typedef enum ListClass {
    LISTDATA, LISTATTR, LISTVLEN, LISTCOMPOUND, LISTFIELDARRAY
} ListClass;

struct Generator {
    void* globalstate; /* per-generator; per list state is in the method args where needed */
    int (*charconstant)(Generator*,struct Symbol*,Bytebuffer*,...);
    int (*constant)(Generator*,struct Symbol*,NCConstant*,Bytebuffer*,...);
    int (*listbegin)(Generator*,struct Symbol*,void*,ListClass,size_t,Bytebuffer*,int*,...);
    int (*list)(Generator*,struct Symbol*,void*,ListClass,int,size_t,Bytebuffer*,...);
    int (*listend)(Generator*,struct Symbol*,void*,ListClass,int,size_t,Bytebuffer*,...);
    int (*vlendecl)(Generator*,struct Symbol*,Bytebuffer*,int,size_t,...);
    int (*vlenstring)(Generator*,struct Symbol*,Bytebuffer*,int*,size_t*,...);
};

extern int generator_getstate(Generator*,void**);
extern int generator_reset(Generator*,void*);

typedef int (*Writer)(Generator*,struct Symbol*,Bytebuffer*,int,const size_t*,const size_t*);

extern void generate_attrdata(struct Symbol*, Generator*, Writer writer, Bytebuffer*);
extern void generate_vardata(struct Symbol*, Generator*, Writer writer,Bytebuffer*);
extern void generate_basetype(struct Symbol*,NCConstant*,Bytebuffer*,Datalist*,Generator*);

#endif /*DATA_H*/

