/* Copyright 2009, UCAR/Unidata and OPeNDAP, Inc.
   See the COPYRIGHT file for more information. */

#ifndef NCBYTES_H
#define NCBYTES_H 1

typedef struct NCbytes {
  int nonextendible; /* 1 => fail if an attempt is made to extend this buffer*/
  unsigned long alloc;
  unsigned long length;
  char* content;
} NCbytes;

#if defined(_CPLUSPLUS_) || defined(__CPLUSPLUS__) || defined(__CPLUSPLUS)
extern "C" {
#endif

extern NCbytes* ncbytesnew(void);
extern void ncbytesfree(NCbytes*);
extern int ncbytessetalloc(NCbytes*,unsigned long);
extern int ncbytessetlength(NCbytes*,unsigned long);
extern int ncbytesfill(NCbytes*, char fill);

/* Produce a duplicate of the contents*/
extern char* ncbytesdup(NCbytes*);
/* Extract the contents and leave buffer empty */
extern char* ncbytesextract(NCbytes*);

/* Return the ith byte; -1 if no such index */
extern int ncbytesget(NCbytes*,unsigned long);
/* Set the ith byte */
extern int ncbytesset(NCbytes*,unsigned long,char);

/* Append one byte */
extern int ncbytesappend(NCbytes*,char); /* Add at Tail */
/* Append n bytes */
extern int ncbytesappendn(NCbytes*,const void*,unsigned long); /* Add at Tail */

/* Null terminate the byte string without extending its length (for debugging) */
extern int ncbytesnull(NCbytes*);

/* Remove char at position i */
extern int ncbytesremove(NCbytes*,unsigned long);

/* Concatenate a null-terminated string to the end of the buffer */
extern int ncbytescat(NCbytes*,const char*);

/* Set the contents of the buffer; mark the buffer as non-extendible */
extern int ncbytessetcontents(NCbytes*, char*, unsigned long);

/* Following are always "in-lined"*/
#define ncbyteslength(bb) ((bb)!=NULL?(bb)->length:0)
#define ncbytesalloc(bb) ((bb)!=NULL?(bb)->alloc:0)
#define ncbytescontents(bb) (((bb)!=NULL && (bb)->content!=NULL)?(bb)->content:(char*)"")
#define ncbytesextend(bb,len) ncbytessetalloc((bb),(len)+(bb->alloc))
#define ncbytesclear(bb) ((bb)!=NULL?(bb)->length=0:0)
#define ncbytesavail(bb,n) ((bb)!=NULL?((bb)->alloc - (bb)->length) >= (n):0)

#if defined(_CPLUSPLUS_) || defined(__CPLUSPLUS__) || defined(__CPLUSPLUS)
}
#endif

#endif /*NCBYTES_H*/
