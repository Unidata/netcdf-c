/* Copyright 2018-2018 University Corporation for Atmospheric
   Research/Unidata. */
/**
 * @file This header file contains types (and type-related macros)
 * for the libzarr code.
 *
 * @author Dennis Heimbigner
*/

#ifndef NCZTYPES_H
#define NCZTYPES_H

typedef enum NCZ_Format { ZARRV1=1, ZARRV2=2, ZARRNC4=4 } NCZ_Format;

/* Define annotation data for NC_OBJ objects */


/* Annotation for NC dispatch data; includes the annotations
   that might normally be attached to the NC_FILE_INFO_T object.
*/
typdef struct NCZINFO {
    NC* controller;
    char* rawurltext; /* as given to NCZ_open */
    char* urltext;    /* as modified by NCZ_open */
    NCURI* uri;      /* parse of rawuritext */
    struct nczarr {
        NCZ_Format format = 0;
    } zarr;
    NCZMAP* map; /* implementation */
    NClist* controls;
    NCauth auth;
    NC_FILE_INFO_T* dataset; /* root of the dataset tree */
} NCZINFO;

#endif /*NCZTYPES_H*/
