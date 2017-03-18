/*
 *	Copyright 2017, University Corporation for Atmospheric Research
 *      See netcdf/COPYRIGHT file for copying and redistribution conditions.
 */

#ifndef NCUTF8_H
#define NCUTF8_H 1

/* Provide a wrapper around whatever utf8 library we use. */

/*
 * Check validity of a UTF8 encoded null-terminated byte string.
 * Return codes:
 * NC_NOERR -- string is valid utf8
 * NC_ENOMEM -- out of memory
 * NC_EBADNAME-- not valid utf8
 */
extern int nc_utf8_validate(const unsigned char * name);

/*
 * Apply NFC normalization to a string.
 * Returns a pointer to newly allocated memory of an NFC
 * normalized version of the null-terminated string 'str'.
 * Pointer to normalized string is returned in normalp argument;
 * caller must free.
 * Return codes:
 * NC_NOERR -- success
 * NC_ENOMEM -- out of memory
 * NC_EBADNAME -- other failure
 */
extern int nc_utf8_normalize(const unsigned char* str, unsigned char** normalp);

#endif /*NCUTF8_H*/

