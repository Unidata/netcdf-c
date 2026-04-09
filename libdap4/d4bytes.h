/* Copyright 2018, UCAR/Unidata and OPeNDAP, Inc.
   See the COPYRIGHT file for more information. */

/** @file d4bytes.h
 * @brief Growable byte-buffer type for the DAP4 client.
 *
 * Declares D4bytes, a simple heap-allocated buffer that grows on demand,
 * and the functions and macros for allocating, writing, and resetting it.
 * @author Dennis Heimbigner
 */

#ifndef D4BYTES_H
#define D4BYTES_H 1

/** Growable heap buffer used for assembling DAP4 byte streams. */
typedef struct D4bytes {
  size_t alloc;  /**< Total allocated capacity in bytes. */
  size_t used;   /**< Number of bytes currently written. */
  void* memory;  /**< Pointer to the allocated buffer. */
} D4bytes;

#if defined(_CPLUSPLUS_) || defined(__CPLUSPLUS__) || defined(__CPLUSPLUS)
extern "C" {
#endif

/** Allocate and initialise a new empty D4bytes buffer. */
extern D4bytes* d4bytesnew(void);
/** Free a D4bytes buffer and its contents. */
extern void d4bytesfree(D4bytes* b);
/**
 * Ensure at least @p n bytes are available and return a pointer to the
 * next write position; advances the used count by @p n.
 */
extern void* d4bytesalloc(D4bytes* b, size_t n);
/**
 * Like d4bytesalloc() but zero-fills the allocated region.
 */
extern void* d4byteszero(D4bytes* b, size_t n);
/** Append the contents of @p src onto @p dst and return @p dst. */
extern D4bytes* d4bytesconcat(D4bytes* dst, D4bytes* src);

/** Return the number of bytes written into @p d4. */
#define d4byteslength(d4) ((d4)->used)
/** Return a pointer to the start of the buffer in @p d4. */
#define d4bytesmemory(d4) ((d4)->memory)
/** Reset the used count of @p d4 to zero without freeing memory. */
#define d4bytesreset(d4) {(d4)->used = 0;}

#if defined(_CPLUSPLUS_) || defined(__CPLUSPLUS__) || defined(__CPLUSPLUS)
}
#endif

#endif /*D4BYTES_H*/
