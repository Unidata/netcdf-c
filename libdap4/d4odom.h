/*********************************************************************
 *   Copyright 2018, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 *********************************************************************/

/** @file d4odom.h
 * @brief Multi-dimensional odometer for iterating over DAP4 array slices.
 *
 * A D4odometer tracks the current index position across up to
 * NC_MAX_VAR_DIMS dimensions and advances through a hyperslab defined
 * by start, count, stride, and declared size vectors.
 * @author Dennis Heimbigner
 */

#ifndef D4ODOM_H
#define D4ODOM_H 1

/**
 * Multi-dimensional odometer for iterating over a hyperslab of an array.
 */
typedef struct D4odometer {
    size_t rank;                        /**< Number of active dimensions. */
    size_t index[NC_MAX_VAR_DIMS];      /**< Current index in each dimension. */
    size_t start[NC_MAX_VAR_DIMS];      /**< Start index in each dimension. */
    size_t stride[NC_MAX_VAR_DIMS];     /**< Stride in each dimension. */
    size_t stop[NC_MAX_VAR_DIMS];       /**< Exclusive stop index in each dimension. */
    size_t declsize[NC_MAX_VAR_DIMS];   /**< Declared (total) size of each dimension. */
} D4odometer;

/** Allocate a scalar odometer that iterates exactly once. */
extern D4odometer* d4scalarodom_new(void);

/**
 * Allocate an odometer for a hyperslab.
 * @param rank   Number of dimensions.
 * @param start  Start indices (NULL means all zeros).
 * @param count  Element counts per dimension (NULL means full extent).
 * @param stride Strides per dimension (NULL means 1).
 * @param size   Declared dimension sizes (NULL means derived from start+count*stride).
 * @return Newly allocated D4odometer, or NULL on allocation failure.
 */
extern D4odometer* d4odom_new(size_t rank,
                               const size_t* start, const size_t* count,
                               const ptrdiff_t* stride, const size_t* size);

/** Free a D4odometer allocated by d4odom_new() or d4scalarodom_new(). */
extern void d4odom_free(D4odometer* odom);

/** Return non-zero while the odometer has more positions to visit. */
extern int d4odom_more(D4odometer* odom);
/**
 * Advance the odometer to the next position.
 * @return The linear offset of the position *before* advancing.
 */
extern d4size_t d4odom_next(D4odometer* odom);

/** Return the current linear element offset for the odometer's index vector. */
extern d4size_t d4odom_offset(D4odometer* odom);

/** Return the total number of elements in the odometer's index space. */
extern d4size_t d4odom_nelements(D4odometer* odom);

/** Return non-zero if the odometer covers the entire declared index space with stride 1. */
extern int d4odom_isWhole(D4odometer* odom);

#endif /*D4ODOM_H*/
