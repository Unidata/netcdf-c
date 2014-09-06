#ifndef NCBZIP2_H
#define NCBZIP2_H

/* Registered id for bzip2 filters */
#define H5Z_FILTER_BZIP2 307
#define H5Z_FILTER_BZIP2_DEFAULT_LEVEL 9

/*
Turn on bzip for a variable's plist
*/
extern int ncbzip2_set(hid_t plistid, const unsigned int level);

/* 
Register bzip2 filter with the library
*/
extern int ncbzip2_register(void);

#endif /*NCBZIP2_H*/
