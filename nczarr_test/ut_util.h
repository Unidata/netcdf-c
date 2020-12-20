/*
 *	Copyright 2018, University Corporation for Atmospheric Research
 *      See netcdf/COPYRIGHT file for copying and redistribution conditions.
 */

#ifndef UT_UTIL_H
#define UT_UTIL_H

extern int parseslices(const char* s0, int* nslicesp, NCZSlice* slices);
extern int parsedimdef(const char* s0, Dimdef** defp);
extern int parsevardef(const char* s0, NClist* dimdefs, Vardef** varp);
extern int parsestringvector(const char* s0, int stopchar, char*** namesp);
extern int parseintvector(const char* s0, int typelen, void** vectorp);
extern void freedimdefs(NClist* defs);
extern void freevardefs(NClist* defs);
extern void freeranges(NCZChunkRange* ranges);
extern void freeslices(NCZSlice* slices);
extern void freestringvec(char** vec);
extern void freeprojvector(int rank, NCZProjection** vec);
extern int ut_typesize(nc_type t);
extern nc_type ut_typeforname(const char* tname);
extern NCZM_IMPL kind2impl(const char* kind);
extern const char* impl2kind(NCZM_IMPL impl);
extern size64_t computelinearoffset(int R, const size64_t* indices, const size64_t* max, size64_t* productp);
extern void slices2vector(int rank, NCZSlice* slices, size64_t** startp, size64_t** stopp, size64_t** stridep, size64_t** maxp);
extern void printoptions(struct UTOptions* opts);
extern int hasdriveletter(const char* f);
extern void ut_sortlist(NClist* l);
extern void fillcommon(struct Common* common, Vardef* var);

#endif /*UT_UTIL_H*/
