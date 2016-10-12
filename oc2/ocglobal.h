/* Copyright 2009, UCAR/Unidata and OPeNDAP, Inc.
   See the COPYRIGHT file for more information. */

#ifndef OCGLOBAL_H
#define OCGLOBAL_H

#include "config.h"

/* Collect global state info in one place */
typedef struct OCGLOBALSTATE {
    int initialized;
    struct {
        int proto_file;
        int proto_https;
    } curl;
    char* tempdir; /* track a usable temp dir */
    char* home; /* track $HOME for use in creating $HOME/.oc dir */
    struct {
	int ignore; /* if 1, then do not use any rc file */
	int loaded;
        struct OCTriplestore daprc; /* the rc file triple store fields*/
        char* rcfile; /* specified rcfile; overrides anything else */
    } rc;
} OCGLOBALSTATE;

/*
Define all constant global data.
Basically, once initialized (under locking),
access to this data does not need to be locked.
*/
typedef struct OCGLOBALCONST {
} OCGLOBALCONST;

extern OCGLOBALSTATE* ocglobal;
extern OCGLOBALCONST* occonst;

extern int oc_global_init();

#ifdef ENABLE_THREADSAFE
/* dmutex.c */
extern int NC_mutex_initialize(void);
extern void NC_lock(void);
extern void NC_unlock(void);
#define NCLOCK() NC_lock();
#define NCUNLOCK() NC_unlock()
#else
#define NCLOCK()
#define NCUNLOCK()
#endif

#endif /*OCGLOBAL_H*/
