#ifndef RENAME4_H
#define RENAME4_H

#ifdef USE_DISPATCH

#define RENAME3(name) NC3_##name

#ifndef USE_DAP
#define RENAME4(name) NC4_##name
#define EXPORT4
#define RENAME44(name) NC4_##name
#endif

#else

/* When using dispatch, we want to forcibly rename everything */

#define RENAME3(name) nc3_##name

#ifndef USE_DAP
#define RENAME4(name) nc_##name
#define EXPORT4 static
#define RENAME44(name) nc4_##name
#endif

#ifdef USE_DAP
#include "netcdf3.h"
#include "netcdf4l.h"
#define RENAME4(name) l4##nc_##name
#define RENAME44(name) l4##nc4_##name
#define EXPORT4
/* Add a declaration for each use of EXPORT4 */
extern int
nc_create_file(const char*,int,size_t,int,size_t*,MPI_Comm,MPI_Info,int*);
extern int
nc_open_file(const char*,int,int,size_t*,int,MPI_Comm,MPI_Info,int*);
#endif

#endif /*USE_DISPATCH*/

#endif /*RENAME4_H*/

