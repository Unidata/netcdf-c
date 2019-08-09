/*********************************************************************
   Copyright 2018, UCAR/Unidata See netcdf/COPYRIGHT file for
   copying and redistribution conditions.
*********************************************************************/
/**
 * @file
 *
 * Functions to manage the list of NC structs. There is one NC struct
 * for each open file.
 *
 * @author Dennis Heimbigner
*/

#include "config.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "ncdispatch.h"

/** This shift is applied to the ext_ncid in order to get the index in
 * the array of NC. */
#define ID_SHIFT (16)

/** This is the length of the NC list - the number of files that can
 * be open at one time. We use 2^16 = 65536 entries in the array, but
 * slot 0 is not used, so only 65535 files may be open at one
 * time.. */
#define NCFILELISTLENGTH 0x10000

/** This is the pointer to the array of NC, one for each open file. */
static NC** nc_filelist = NULL;

/** The number of files currently open. */
static int numfiles = 0;

/**
 * How many files are currently open?
 *
 * @return number of open files.
 * @author Dennis Heimbigner
 */
int
count_NCList(void)
{
    return numfiles;
}

/**
 * Free an empty NCList. @note If list is not empty, function will
 * silently exit.
 *
 * @author Dennis Heimbigner
 */
void
free_NCList(void)
{
    if(numfiles > 0) return; /* not empty */
    if(nc_filelist != NULL) free(nc_filelist);
    nc_filelist = NULL;
}

/**
 * Add an already-allocated NC to the list. It will be assigned an
 * ncid in this function.
 *
 * If this is the first file to be opened, the nc_filelist will be
 * allocated and set to all 0.
 *
 * The ncid is assigned by finding the first open index in the
 * nc_filelist array (skipping index 0). The ncid is this index
 * left-shifted ID_SHIFT bits. This puts the file ID in the first two
 * bytes of the 4-byte integer, and leaves the last two bytes for
 * group IDs for netCDF-4 files.
 *
 * @param ncp Pointer to already-allocated and initialized NC struct.
 *
 * @return ::NC_NOERR No error.
 * @return ::NC_ENOMEM Out of memory.
 * @author Dennis Heimbigner
 */
int
add_to_NCList(NC* ncp)
{
    int i;
    int new_id;
    if(nc_filelist == NULL) {
        if (!(nc_filelist = calloc(1, sizeof(NC*)*NCFILELISTLENGTH)))
            return NC_ENOMEM;
        numfiles = 0;
    }

    new_id = 0; /* id's begin at 1 */
    for(i=1; i < NCFILELISTLENGTH; i++) {
        if(nc_filelist[i] == NULL) {new_id = i; break;}
    }
    if(new_id == 0) return NC_ENOMEM; /* no more slots */
    nc_filelist[new_id] = ncp;
    numfiles++;
    new_id = (new_id << ID_SHIFT);
    ncp->ext_ncid = new_id;
    return NC_NOERR;
}

/**
 * Delete an NC struct from the list. This happens when the file is
 * closed. Relies on all memory in the NC already being deallocated.
 *
 * @note If the file list is empty, or this NC can't be found in the
 * list, this function will silently exit.
 *
 * @param ncp Pointer to NC to be removed from list.
 *
 * @author Dennis Heimbigner
 */
void
del_from_NCList(NC* ncp)
{
    unsigned int ncid = ((unsigned int)ncp->ext_ncid) >> ID_SHIFT;
    if(numfiles == 0 || ncid == 0 || nc_filelist == NULL) return;
    if(nc_filelist[ncid] != ncp) return;

    nc_filelist[ncid] = NULL;
    numfiles--;

    /* If all files have been closed, release the filelist memory. */
    if (numfiles == 0)
        free_NCList();
}

/**
 * Find an NC in the list, given an ncid.
 *
 * @param ext_ncid The ncid of the file to find.
 *
 * @return pointer to NC or NULL if not found.
 * @author Dennis Heimbigner
 */
NC *
find_in_NCList(int ext_ncid)
{
    NC* f = NULL;
    unsigned int ncid = ((unsigned int)ext_ncid) >> ID_SHIFT;
    if(numfiles > 0 && nc_filelist != NULL && ncid < NCFILELISTLENGTH)
        f = nc_filelist[ncid];

    /* for classic files, ext_ncid must be a multiple of (1<<ID_SHIFT) */
    if (f != NULL && f->model->impl == NC_FORMATX_NC3 && (ext_ncid % (1<<ID_SHIFT)))
        return NULL;

    return f;
}

/**
 * Find an NC in the list using the file name.
 *
 * @param path Name of the file.
 *
 * @return pointer to NC or NULL if not found.
 * @author Dennis Heimbigner
 */
NC*
find_in_NCList_by_name(const char* path)
{
    int i;
    NC* f = NULL;
    if(nc_filelist == NULL)
        return NULL;
    for(i=1; i < NCFILELISTLENGTH; i++) {
        if(nc_filelist[i] != NULL) {
            if(strcmp(nc_filelist[i]->path,path)==0) {
                f = nc_filelist[i];
                break;
            }
        }
    }
    return f;
}

/**
 * Find an NC in list based on its index. The index is ((unsigned
 * int)ext_ncid) >> ID_SHIFT.
 *
 * @param index The index in the NC list.
 * @param ncp Pointer that gets pointer to the next NC. Igored if
 * NULL.
 *
 * @return ::NC_NOERR No error.
 * @return ::NC_ERANGE Index out of range.
 * @author Dennis Heimbigner
 */
int
iterate_NCList(int index, NC** ncp)
{
    /* Walk from 0 ...; 0 return => stop */
    if(index < 0 || index >= NCFILELISTLENGTH)
        return NC_ERANGE;
    if(ncp) *ncp = nc_filelist[index];
    return NC_NOERR;
}
