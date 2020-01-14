/* Copyright 2018, University Corporation for Atmospheric
 * Research. See COPYRIGHT file for copying and redistribution
 * conditions. */

/**
 * @file @internal The netCDF-4 functions which control NCZ
 * caching. These caching controls allow the user to change the cache
 * sizes of ZARR before opening files.
 *
 * @author Dennis Heimbigner, Ed Hartnett
 */

#include "zincludes.h"

/* These are the default chunk cache sizes for ZARR files created or
 * opened with netCDF-4. */
size_t ncz_chunk_cache_size = 0;
size_t ncz_chunk_cache_nelems = 0;
float ncz_chunk_cache_preemption = 0;

/**
 * Set chunk cache size. Only affects files opened/created *after* it
 * is called.
 *
 * @param size Size in bytes to set cache.
 * @param nelems Number of elements to hold in cache.
 * @param preemption Premption stragety (between 0 and 1).
 *
 * @return ::NC_NOERR No error.
 * @return ::NC_EINVAL Bad preemption.
 * @author Dennis Heimbigner, Ed Hartnett
 */
int
NCZ_set_chunk_cache(size_t size, size_t nelems, float preemption)
{
    if (preemption < 0 || preemption > 1)
        return NC_EINVAL;
    ncz_chunk_cache_size = size;
    ncz_chunk_cache_nelems = nelems;
    ncz_chunk_cache_preemption = preemption;
    return NC_NOERR;
}

/**
 * Get chunk cache size. Only affects files opened/created *after* it
 * is called.
 *
 * @param sizep Pointer that gets size in bytes to set cache.
 * @param nelemsp Pointer that gets number of elements to hold in cache.
 * @param preemptionp Pointer that gets premption stragety (between 0 and 1).
 *
 * @return ::NC_NOERR No error.
 * @author Dennis Heimbigner, Ed Hartnett
 */
int
NCZ_get_chunk_cache(size_t *sizep, size_t *nelemsp, float *preemptionp)
{
    if (sizep)
        *sizep = ncz_chunk_cache_size;

    if (nelemsp)
        *nelemsp = ncz_chunk_cache_nelems;

    if (preemptionp)
        *preemptionp = ncz_chunk_cache_preemption;
    return NC_NOERR;
}

/**
 * @internal Set the chunk cache. Required for fortran to avoid size_t
 * issues.
 *
 * @param size Cache size.
 * @param nelems Number of elements.
 * @param preemption Preemption * 100.
 *
 * @return NC_NOERR No error.
 * @author Dennis Heimbigner, Ed Hartnett
 */
int
ncz_set_chunk_cache_ints(int size, int nelems, int preemption)
{
    if (size <= 0 || nelems <= 0 || preemption < 0 || preemption > 100)
        return NC_EINVAL;
    ncz_chunk_cache_size = size;
    ncz_chunk_cache_nelems = nelems;
    ncz_chunk_cache_preemption = (float)preemption / 100;
    return NC_NOERR;
}

/**
 * @internal Get the chunk cache settings. Required for fortran to
 * avoid size_t issues.
 *
 * @param sizep Pointer that gets cache size.
 * @param nelemsp Pointer that gets number of elements.
 * @param preemptionp Pointer that gets preemption * 100.
 *
 * @return NC_NOERR No error.
 * @author Dennis Heimbigner, Ed Hartnett
 */
int
ncz_get_chunk_cache_ints(int *sizep, int *nelemsp, int *preemptionp)
{
    if (sizep)
        *sizep = (int)ncz_chunk_cache_size;
    if (nelemsp)
        *nelemsp = (int)ncz_chunk_cache_nelems;
    if (preemptionp)
        *preemptionp = (int)(ncz_chunk_cache_preemption * 100);

    return NC_NOERR;
}

int
ncz_adjust_var_cache(NC_GRP_INFO_T* grp, NC_VAR_INFO_T* var)
{
    return NC_NOERR;
}
