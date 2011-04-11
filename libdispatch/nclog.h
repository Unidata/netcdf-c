/* Copyright 2009, UCAR/Unidata and OPeNDAP, INC.
   See the COPYRIGHT file for more information. */

#ifndef NCLOG_H
#define NCLOG_H

#define ENVFLAG "NCLOGFILE"

/* Suggested tag values */
#define NCLOGNOTE 0
#define NCLOGWARN 1
#define NCLOGERR 2
#define NCLOGDBG 3

extern void ncloginit(void);
extern void ncsetlogging(int tf);
extern void nclogopen(const char* file);
extern void nclogclose(void);

/* The tag value is an arbitrary integer */
extern void nclog(int tag,const char* fmt, ...);
extern void nclogtext(int tag, const char* text, size_t count);

extern void nclogsettags(char** tagset, char* dfalt);

#endif /*NCLOG_H*/
