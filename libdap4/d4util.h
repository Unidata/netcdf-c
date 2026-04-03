/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

/** @file d4util.h
 * @brief Basic types and byte-swap macros for the DAP4 client.
 *
 * Defines d4size_t, D4blob, and the inline byte-swap macros used
 * throughout libdap4 for endian conversion.
 * @author Dennis Heimbigner
 */

#ifndef D4UTIL_H
#define D4UTIL_H 1

/**
 * Unsigned integer type large enough to hold any file offset or
 * in-memory buffer size used by the DAP4 client.
 */
typedef unsigned long long d4size_t;

/** A (size, pointer) pair describing a contiguous memory buffer. */
typedef struct D4blob {
    d4size_t size; /**< Byte length of the buffer. */
    void* memory;  /**< Pointer to the first byte of the buffer. */
} D4blob;

/** Zero-initialise a D4blob in place. */
#define NULLBLOB(blob) {blob.size = 0; blob.memory = NULL;}

/** Initialise a D4blob from an NCD4offset (covers base..limit). */
#define OFFSET2BLOB(blob,offset) do{(blob).size = (d4size_t)((offset)->limit - (offset)->base); (blob).memory = (offset)->base; }while(0)
/** Initialise an NCD4offset to walk the bytes of a D4blob. */
#define BLOB2OFFSET(offset,blob) do{\
(offset)->base = (blob).memory; \
(offset)->limit = ((char*)(blob).memory) + (blob).size; \
(offset)->offset = (offset)->base; \
} while(0)

/**************************************************/

/** Byte-swap a 16-bit value at @p ip in place. */
#define swapinline16(ip) \
{ \
    char b[2]; \
    char* src = (char*)(ip); \
    b[0] = src[1]; \
    b[1] = src[0]; \
    memcpy(ip, b, 2); \
}

/** Byte-swap a 32-bit value at @p ip in place. */
#define swapinline32(ip) \
{ \
    char b[4]; \
    char* src = (char*)(ip); \
    b[0] = src[3]; \
    b[1] = src[2]; \
    b[2] = src[1]; \
    b[3] = src[0]; \
    memcpy(ip, b, 4); \
}

/** Byte-swap a 64-bit value at @p ip in place. */
#define swapinline64(ip) \
{ \
    char b[8]; \
    char* src = (char*)(ip); \
    b[0] = src[7]; \
    b[1] = src[6]; \
    b[2] = src[5]; \
    b[3] = src[4]; \
    b[4] = src[3]; \
    b[5] = src[2]; \
    b[6] = src[1]; \
    b[7] = src[0]; \
    memcpy(ip, b, 8); \
}

/***************************************************/
/** @defgroup ncd4_data_flags NCD4node Data Content Flags
 * Bitmask flags recording what complex sub-types a variable transitively
 * contains.  Used to select the appropriate data-copy path.
 * @{
 */
#define HASNIL   (0)  /**< No special sub-types. */
#define HASSEQ   (1)  /**< Transitively contains one or more Sequences. */
#define HASSTR   (2)  /**< Transitively contains one or more Strings. */
#define HASOPFIX (4)  /**< Transitively contains fixed-size Opaques. */
#define HASOPVAR (8)  /**< Transitively contains variable-size Opaques. */
#define LEAFSEQ  (16) /**< This node is a leaf Sequence. */
#define HASANY   (HASNIL|HASSEQ|HASSTR|HASOPFIX|HASOPVAR) /**< Mask of all content flags. */
/** @} */
/***************************************************/

/**
 * Test whether @p path looks like a DAP4 URL.
 * @param path      Path or URL string to test.
 * @param basename  If non-NULL and the path is a URL, receives the
 *                  basename of the URL path (without extension);
 *                  caller must free.
 * @return NC_NOERR if @p path is a valid URL, NC_EURL otherwise.
 */
extern int ncd4__testurl(const char* path, char** basename);

#endif /*D4UTIL_H*/
