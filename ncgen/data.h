/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

#ifndef DATA_H
#define DATA_H 1

#ifndef NO_STDARG
#  include <stdarg.h>
#else
#  include <varargs.h>
#endif

/* nmemonics*/
#define TOPLEVEL 1
#define DEEP 0

/* Forward types */
struct Datalist;
struct Symbol;
struct Dimset;

/* any one possible value*/
typedef union Constvalue {
    struct Datalist* compoundv; /* NC_COMPOUND*/
    char charv;                 /* NC_CHAR*/
    signed char int8v;          /* NC_BYTE*/
    unsigned char uint8v;       /* NC_UBYTE*/
    short int16v;               /* NC_SHORT*/
    unsigned short uint16v;     /* NC_USHORT*/
    int int32v;                 /* NC_INT*/
    unsigned int uint32v;       /* NC_UINT*/
    long long int64v;           /* NC_INT64*/
    unsigned long long uint64v; /* NC_UINT64*/
    float floatv;               /* NC_FLOAT*/
    double doublev;             /* NC_DOUBLE*/
    struct Stringv {		/* NC_STRING*/
	int len;
	char* stringv; 
    } stringv;
    struct Opaquev {     /* NC_OPAQUE*/
	int len; /* length as originally written (rounded to even number)*/
	char* stringv; /*as  constant was written*/
		      /* (padded to even # chars >= 16)*/
		      /* without leading 0x*/
    } opaquev;
    struct Symbol* enumv;   /* NC_ECONST*/
} Constvalue;

typedef struct NCConstant {
    nc_type 	  nctype; /* NC_INT,... */
    nc_type 	  subtype; /* NC_DIM | NC_NAT */
    int		  lineno;
    Constvalue    value;
    int           filled; /* was this originally NC_FILLVALUE? */
} NCConstant;

typedef struct Datalist {
    int           readonly; /* data field is shared with another Datalist*/
    size_t  length; /* |data| */
    size_t  alloc;  /* track total allocated space for data field*/
    NCConstant**     data; /* actual list of constants constituting the datalist*/
    /* Track various values associated with the datalist*/
    /* (used to be in Constvalue.compoundv)*/
} Datalist;


extern List* alldatalists;

/* from: data.c */
extern Datalist* builddatalist(int initialize);
extern void capture(Datalist* dl);
extern void dlappend(Datalist*, NCConstant*);
extern void dlinsert(Datalist* dl, size_t pos, Datalist* insertion);
extern void dlset(Datalist*, size_t, NCConstant*);
extern NCConstant* dlremove(Datalist*, size_t);
extern NCConstant* builddatasublist(Datalist* dl);
extern Datalist* builddatasubset(Datalist* dl, size_t start, size_t count);
extern void dlextend(Datalist* dl);
extern void dlsetalloc(Datalist* dl,size_t);
extern Datalist* clonedatalist(Datalist* dl);
extern void reclaimalldatalists(void);
extern void reclaimdatalist(Datalist*);
extern void reclaimconstant(NCConstant*);
extern void freedatalist(Datalist* list);
extern Datalist* flatten(Datalist* list,int rank);

int       datalistline(Datalist*);
#define   datalistith(dl,i) ((dl)==NULL?NULL:((i) >= (dl)->length?NULL:(dl)->data[i]))
#define   datalistlen(dl) ((dl)==NULL?0:(dl)->length)
#define   datalistclear(dl) {if((dl)!=NULL) {dl->length=0;}}

NCConstant* list2const(Datalist*);
Datalist* const2list(NCConstant* con);

int isstringable(nc_type nctype);

#define islistconst(con) ((con)!=NULL && ((con)->nctype == NC_COMPOUND))
#define isfillconst(con) ((con)!=NULL && (con)->nctype == NC_FILLVALUE)
#define constline(con) (con==NULL?0:(con)->lineno)
#define consttype(con) (con==NULL?NC_NAT:(con)->nctype)

#define isnilconst(con) ((con)!=NULL && (con)->nctype == NC_NIL)
#define compoundfor(con) ((con)==NULL?NULL:(con)->value.compoundv)
#define setsubtype(con,type) {if((con)!=NULL){(con)->subtype=(type);}}

NCConstant* emptycompoundconst(int lineno);

NCConstant* emptystringconst(int);

NCConstant* cloneconstant(NCConstant* con); /* deep clone*/
void clearconstant(NCConstant* con); /* deep clear*/
#define freeconst(con) freeconstant(con,DEEP);
void freeconstant(NCConstant* con, int shallow);

extern NCConstant* nullconst(void);
extern NCConstant nullconstant;
extern NCConstant fillconstant;
extern NCConstant nilconstant;
extern Datalist* filldatalist;

#endif /*DATA_H*/

