/*********************************************************************
 *   Copyright 1993, UCAR/Unidata
 *   See netcdf/COPYRIGHT file for copying and redistribution conditions.
 * @author Ed Hartnett
 * @date 10/10/17
 *********************************************************************/

/* WARNING: Order of mpi.h, nc.h, and pio.h is important */
#include "config.h"
#include <stdlib.h>
#include <mpi.h>
#include "nc.h"
#include "ncdispatch.h"
#include <pio.h>
#include <pio_internal.h>

int current_iosysid;

/* Must follow netcdf.h */
/* #include <pnetcdf.h> */

/* typedef struct PIO_INFO */
/* { */
/*     /\* pnetcdf_file will be true if the file is created/opened with the */
/*      * parallel-netcdf library. pnetcdf_access_mode keeps track of */
/*      * whether independpent or collective mode is */
/*      * desired. pnetcdf_ndims keeps track of how many dims each var */
/*      * has, which I need to know to convert start, count, and stride */
/*      * arrays from size_t to MPI_Offset. (I can't use an inq function */
/*      * to find out the number of dims, because these are collective in */
/*      * pnetcdf.) *\/ */
/*     int pnetcdf_access_mode; */
/* } PIO_INFO; */

/* Define accessors for the dispatchdata */
/* #define PIO_DATA(nc) ((PIO_INFO*)(nc)->dispatchdata) */
/* #define PIO_DATA_SET(nc, data) ((nc)->dispatchdata = (void*)(data)) */

/* Cannot have NC_MPIPOSIX flag, ignore NC_MPIIO as PnetCDF use MPIIO */
static const int LEGAL_CREATE_FLAGS = (NC_NOCLOBBER | NC_64BIT_OFFSET | NC_CLASSIC_MODEL | NC_NETCDF4 |
                                       NC_SHARE | NC_LOCK | NC_64BIT_DATA | NC_MPIIO | NC_PIO);

static const int LEGAL_OPEN_FLAGS = (NC_WRITE | NC_NOCLOBBER | NC_SHARE | NC_LOCK | NC_NETCDF4 |
                                     NC_CLASSIC_MODEL | NC_64BIT_OFFSET | NC_64BIT_DATA | NC_MPIIO | NC_PIO);

/**************************************************/

/**
 * Set the current iosysid.
 *
 * The IO system ID is required for nc_create() and nc_open() to
 * support PIO. This function sets the value of the current IO System
 * ID. Call this function before creating or opening a file to change
 * the IO system ID for that file.
 *
 * @param iosysid the new IO system ID.
 *
 * @returns 0 for success.
 * @author Ed Hartnett
 */
int nc_set_iosysid(int iosysid)
{
   current_iosysid = iosysid;
   return NC_NOERR;
}

/**
 * Get the current iosysid.
 *
 * The IO system ID is required for nc_create() and nc_open() to
 * support PIO. This function gets the value of the current IO System
 * ID.
 *
 * @param iosysidp a pointer that gets the current IO system
 * ID. Ignored if NULL.
 *
 * @returns 0 for success.
 * @author Ed Hartnett
 */
int nc_get_iosysid(int *iosysidp)
{
   if (iosysidp)
      *iosysidp = current_iosysid;
   return NC_NOERR;
}

static int
PIO_create(const char *path, int cmode, size_t initialsz, int basepe, size_t *chunksizehintp,
           int use_parallel, void* mpidata, struct NC_Dispatch* table, NC* nc)
{
   int res, default_format;
   /* PIO_INFO *nc5; */
   /* MPI_Comm comm = MPI_COMM_WORLD; */
   /* MPI_Info info = MPI_INFO_NULL; */

   LOG((1, "PIO_create path %s cmode %d use_parallel %d", path, cmode, use_parallel));

   /* Check the cmode for only valid flags*/
   if (cmode & ~LEGAL_CREATE_FLAGS)
      return NC_EINVAL;

   /* Cannot have both MPIO flags */
   if ((cmode & (NC_MPIIO|NC_MPIPOSIX)) == (NC_MPIIO|NC_MPIPOSIX))
      return NC_EINVAL;

   /* Cannot have both NC_64BIT_OFFSET & NC_64BIT_DATA */
   if ((cmode & (NC_64BIT_OFFSET|NC_64BIT_DATA)) == (NC_64BIT_OFFSET|NC_64BIT_DATA))
      return NC_EINVAL;

   default_format = nc_get_default_format();

   /* if (default_format == NC_FORMAT_CLASSIC) then we respect the format set in cmode */
   if (default_format == NC_FORMAT_64BIT_OFFSET)
      cmode |= NC_64BIT_OFFSET;
   else if (default_format == NC_FORMAT_CDF5)
      cmode |= NC_64BIT_DATA;
   else if (default_format == NC_NETCDF4)
      cmode |= NC_NETCDF4;

   /* No MPI environment initialized */
   /* if (mpidata == NULL) */
   /*   return NC_ENOPAR; */

   /* comm = ((NC_MPI_INFO *)mpidata)->comm; */
   /* info = ((NC_MPI_INFO *)mpidata)->info; */

   /* Link nc5 and nc */
   /* if (!(nc->dispatchdata = calloc(1, sizeof(PIO_INFO)))) */
   /*   return NC_ENOMEM; */
   /* LOG((2, "allocated nc->dispatchdata %d", nc->dispatchdata)); */

   /* Fix up the cmode by keeping only essential flags;
      these are the flags that are the same in netcf.h and pnetcdf.h
   */
   /* /\* PnetCDF recognizes the flags below for create and ignores NC_LOCK and  NC_SHARE *\/ */
   /* cmode &= (NC_WRITE | NC_NOCLOBBER | NC_SHARE | NC_64BIT_OFFSET | NC_64BIT_DATA); */

   int iotype = PIO_IOTYPE_NETCDF;
   if (cmode & NC_NETCDF4)
   {
      if (cmode & NC_SHARE)
      {
         iotype = PIO_IOTYPE_NETCDF4P;
      }
      else
         iotype = PIO_IOTYPE_NETCDF4C;
   }
   else if (cmode & NC_PNETCDF)
      iotype = PIO_IOTYPE_PNETCDF;

   res = PIOc_createfile_int3(current_iosysid, &nc->ext_ncid, &iotype, path, cmode,
                              use_parallel, mpidata, table, nc, false);
   LOG((2, "PIOc_createfile_int3 called res %d nc->ext_ncid %d", res, nc->ext_ncid));

   /* Free this extra memory if there was a problem with create. */
   /* if (res && nc5) */
   /*   free(nc5); /\* reclaim allocated space *\/ */

   return res;
}

static int
PIO_open(const char *path, int cmode, int basepe, size_t *chunksizehintp,
         int use_parallel, void* mpidata, struct NC_Dispatch* table, NC* nc)
{
   /* MPI_Comm comm = MPI_COMM_WORLD; */
   /* MPI_Info info = MPI_INFO_NULL; */
   int res;

   /* Check the cmode for only valid flags*/
   if (cmode & ~LEGAL_OPEN_FLAGS)
      return NC_EINVAL;

   /* Cannot have both MPIO flags */
   if ((cmode & (NC_MPIIO|NC_MPIPOSIX)) == (NC_MPIIO|NC_MPIPOSIX))
      return NC_EINVAL;

   /* if (mpidata) */
   /* { */
   /*     comm = ((NC_MPI_INFO *)mpidata)->comm; */
   /*     info = ((NC_MPI_INFO *)mpidata)->info; */
   /* } */

   /* PnetCDF recognizes the flags NC_WRITE and NC_NOCLOBBER for file open
    * and ignores NC_LOCK, NC_SHARE, NC_64BIT_OFFSET, and NC_64BIT_DATA.
    * Ignoring the NC_64BIT_OFFSET and NC_64BIT_DATA flags is because the
    * file is already in one of the CDF-formats, and setting these 2 flags
    * will not change the format of that file.
    */
   /* cmode &= (NC_WRITE | NC_NOCLOBBER); */

   int ncid;
   int iotype = PIO_IOTYPE_NETCDF;
   if (cmode & NC_NETCDF4)
   {
      if (cmode & NC_SHARE)
         iotype = PIO_IOTYPE_NETCDF4P;
      else
         iotype = PIO_IOTYPE_NETCDF4C;
   }
   else if (cmode & NC_PNETCDF)
      iotype = PIO_IOTYPE_PNETCDF;
   res = PIOc_openfile_retry3(current_iosysid, &ncid, &iotype, path, cmode, 0, table, nc, false);

   return res;
}

static int
PIO_redef(int ncid)
{
   NC* nc;
   int status = NC_check_id(ncid, &nc);
   if (status != NC_NOERR) return status;
   return PIOc_redef(nc->ext_ncid);
}

static int
PIO__enddef(int ncid, size_t h_minfree, size_t v_align, size_t v_minfree, size_t r_align)
{
   int status;
   NC* nc;
   /* PIO_INFO* nc5; */
   /* MPI_Offset mpi_h_minfree = h_minfree; */
   /* MPI_Offset mpi_v_align   = v_align; */
   /* MPI_Offset mpi_v_minfree = v_minfree; */
   /* MPI_Offset mpi_r_align   = r_align; */

   status = NC_check_id(ncid, &nc);
   if (status != NC_NOERR)
      return status;

   /* nc5 = PIO_DATA(nc); */
   /* assert(nc5); */

   status = PIOc_enddef(nc->ext_ncid);

   if (!status) {
      /* if (nc5->pnetcdf_access_mode == NC_INDEPENDENT) */
      /* status = PIOc_begin_indep_data(nc->ext_ncid); */
   }
   return status;
}

static int
PIO_sync(int ncid)
{
   NC* nc;
   int status = NC_check_id(ncid, &nc);
   if (status != NC_NOERR) return status;
   return PIOc_sync(nc->ext_ncid);
}

static int
PIO_abort(int ncid)
{
   NC* nc;
   /* PIO_INFO* nc5; */
   int status;

   if ((status = NC_check_id(ncid, &nc)))
      return status;

   /* status = PIOc_abort(nc->ext_ncid); */

/* done: */
   /* nc5 = PIO_DATA(nc); */
   /* if (nc5 != NULL) free(nc5); /\* reclaim allocated space *\/ */
   return status;
}

static int
PIO_close(int ncid)
{
   NC* nc;
   int status;

   LOG((2, "PIO_close ncid %d", ncid));
   if ((status = NC_check_id(ncid, &nc)))
      return status;

   status = PIOc_closefile(nc->ext_ncid);

   /* LOG((2, "nc->dispatchdata %d", nc->dispatchdata)); */
   /* if (nc->dispatchdata) */
   /*   free(nc->dispatchdata); /\* reclaim allocated space *\/ */

   return status;
}

static int
PIO_set_fill(int ncid, int fillmode, int *old_mode_ptr)
{
   NC* nc;
   int status = NC_check_id(ncid, &nc);
   if (status != NC_NOERR)
      return status;
   return PIOc_set_fill(nc->ext_ncid, fillmode, old_mode_ptr);
}

static int
PIO_inq_base_pe(int ncid, int* pep)
{
   if (pep) *pep = 0;
   return NC_NOERR;
}

static int
PIO_set_base_pe(int ncid, int pe)
{
   return NC_NOERR;
}

static int
PIO_inq_format(int ncid, int* formatp)
{
   NC* nc;
   int status = NC_check_id(ncid, &nc);
   if (status != NC_NOERR) return status;
   status = PIOc_inq_format(nc->ext_ncid,formatp);
   return status;
}

static int
PIO_inq_format_extended(int ncid, int* formatp, int *modep)
{
   NC* nc;
   int status = NC_check_id(ncid, &nc);
   if (status != NC_NOERR) return status;
   if (modep) *modep = nc->mode;
   /* Note that we do not use NC_FORMAT_CDF5 because PNETCDF has a dispatch table */
   if (formatp) *formatp = NC_FORMATX_PNETCDF;
   return NC_NOERR;
}

static int
PIO_inq(int ncid,
        int *ndimsp,
        int *nvarsp,
        int *nattsp,
        int *unlimp)
{
   NC* nc;
   int status = NC_check_id(ncid, &nc);
   if (status != NC_NOERR) return status;
   return PIOc_inq(nc->ext_ncid,ndimsp,nvarsp,nattsp,unlimp);
}


static int
PIO_inq_type(int ncid, nc_type typeid, char* name, size_t* size)
{
   if (typeid < NC_BYTE || typeid >= NC_STRING)
      return NC_EBADTYPE;
   if (name)
      strcpy(name, NC_atomictypename(typeid));
   if (size)
      *size = NC_atomictypelen(typeid);
   return NC_NOERR;
}

static int
PIO_def_dim(int ncid, const char* name, size_t len, int* idp)
{
   int status;
   NC* nc;

   status = NC_check_id(ncid, &nc);
   if (status != NC_NOERR)
      return status;

   return PIOc_def_dim(nc->ext_ncid, name, len, idp);
}

static int
PIO_inq_dimid(int ncid, const char *name, int *idp)
{
   NC* nc;
   int status;

   if ((status = NC_check_id(ncid, &nc)))
      return status;
   return PIOc_inq_dimid(nc->ext_ncid, name, idp);
}

static int
PIO_inq_dim(int ncid, int dimid, char *name, size_t* lenp)
{
   int status;
   NC* nc;
   MPI_Offset mpilen;
   if ((status = NC_check_id(ncid, &nc)))
      return status;
   status = PIOc_inq_dim(nc->ext_ncid, dimid, name, &mpilen);
   if (lenp)
      *lenp = mpilen;
   return status;
}

static int
PIO_inq_unlimdim(int ncid,  int *unlimdimidp)
{
   NC* nc;
   int status = NC_check_id(ncid, &nc);
   if (status != NC_NOERR) return status;
   return PIOc_inq_unlimdim(nc->ext_ncid,unlimdimidp);
}

static int
PIO_rename_dim(int ncid, int dimid, const char* newname)
{
   NC* nc;
   int status = NC_check_id(ncid, &nc);
   if (status != NC_NOERR) return status;
   return PIOc_rename_dim(nc->ext_ncid,dimid,newname);
}

static int
PIO_inq_att(int ncid, int varid, const char* name, nc_type* xtypep, size_t* lenp)
{
   NC* nc;
   MPI_Offset mpilen;
   int status = NC_check_id(ncid, &nc);
   if (status != NC_NOERR) return status;
   status = PIOc_inq_att(nc->ext_ncid,varid,name,xtypep,&mpilen);
   if (status != NC_NOERR) return status;
   if (lenp) *lenp = mpilen;
   return status;
}

static int
PIO_inq_attid(int ncid, int varid, const char *name, int *idp)
{
   NC* nc;
   int status = NC_check_id(ncid, &nc);
   if (status != NC_NOERR) return status;
   return PIOc_inq_attid(nc->ext_ncid,varid,name,idp);
}

static int
PIO_inq_attname(int ncid, int varid, int attnum, char *name)
{
   NC* nc;
   int status = NC_check_id(ncid, &nc);
   if (status != NC_NOERR) return status;
   return PIOc_inq_attname(nc->ext_ncid,varid,attnum,name);

}

static int
PIO_rename_att(int ncid, int varid, const char *name,
               const char *newname)
{
   NC* nc;
   int status = NC_check_id(ncid, &nc);
   if (status != NC_NOERR) return status;
   return PIOc_rename_att(nc->ext_ncid,varid,name,newname);
}

static int
PIO_del_att(int ncid, int varid, const char *name)
{
   NC* nc;
   int status = NC_check_id(ncid, &nc);
   if (status != NC_NOERR) return status;
   return PIOc_del_att(nc->ext_ncid,varid,name);
}

int
PIO_get_att(
   int ncid,
   int varid,
   const char *name,
   void *ip,
   nc_type memtype)
{
   NC* nc;
   int status;
   nc_type xtype;

   status = NC_check_id(ncid, &nc);
   if (status != NC_NOERR) return status;

   status = PIO_inq_att(ncid,varid,name,&xtype,NULL);
   if (status != NC_NOERR) return status;

   if (memtype == NC_NAT) memtype = xtype;

   switch (memtype) {
   case NC_CHAR:
      return PIOc_get_att_text(nc->ext_ncid, varid, name, (char*)ip);
   case NC_BYTE:
      return PIOc_get_att_schar(nc->ext_ncid, varid, name, (signed char*)ip);
   case NC_SHORT:
      return PIOc_get_att_short(nc->ext_ncid, varid, name, (short*)ip);
   case NC_INT:
      return PIOc_get_att_int(nc->ext_ncid, varid, name, (int*)ip);
   case NC_FLOAT:
      return PIOc_get_att_float(nc->ext_ncid, varid, name, (float*)ip);
   case NC_DOUBLE:
      return PIOc_get_att_double(nc->ext_ncid, varid, name, (double*)ip);
   case NC_UBYTE:
      return PIOc_get_att_uchar(nc->ext_ncid, varid, name, (unsigned char*)ip);
   case NC_USHORT:
      return PIOc_get_att_ushort(nc->ext_ncid, varid, name, (unsigned short*)ip);
   case NC_UINT:
      return PIOc_get_att_uint(nc->ext_ncid, varid, name, (unsigned int*)ip);
   case NC_INT64:
      return PIOc_get_att_longlong(nc->ext_ncid, varid, name, (long long*)ip);
   case NC_UINT64:
      return PIOc_get_att_ulonglong(nc->ext_ncid, varid, name, (unsigned long long*)ip);
   default:
      break;
   }
   return NC_EBADTYPE;
}

int
PIO_put_att(int ncid, int varid, const char *name, nc_type xtype, size_t len,
            const void *ip, nc_type memtype)
{
   NC *nc;
   int status;
   MPI_Offset mpilen;

   /* check if ncid is valid */
   if ((status = NC_check_id(ncid, &nc)))
      return status;

   /* check if varid is valid */
   if ((status = PIOc_inq_varnatts(nc->ext_ncid, varid, NULL)))
      return status;

   if (!name || (strlen(name) > NC_MAX_NAME))
      return NC_EBADNAME;

   /* The length needs to be positive (cast needed for braindead
      systems with signed size_t). */
   if (((unsigned long) len) > X_INT_MAX)
      return NC_EINVAL;

   mpilen = len;
   return PIOc_put_att_tc(nc->ext_ncid, varid, name, xtype, mpilen, memtype, ip);
}

static int
PIO_def_var(int ncid, const char *name, nc_type xtype,
            int ndims, const int *dimidsp, int *varidp)
{
   NC* nc;
   int status;

   if ((status = NC_check_id(ncid, &nc)) != NC_NOERR)
      return status;

   return PIOc_def_var(nc->ext_ncid, name, xtype, ndims, dimidsp, varidp);
}

static int
PIO_inq_varid(int ncid, const char *name, int *varidp)
{
   NC* nc;
   int status = NC_check_id(ncid, &nc);
   if (status != NC_NOERR)
      return status;
   return PIOc_inq_varid(nc->ext_ncid,name,varidp);
}

static int
PIO_rename_var(int ncid, int varid, const char *name)
{
   NC* nc;
   int status = NC_check_id(ncid, &nc);
   if (status != NC_NOERR) return status;
   return PIOc_rename_var(nc->ext_ncid,varid,name);
}

static int
PIO_get_vara(int ncid, int varid, const size_t* startp, const size_t* countp,
             void* ip, nc_type memtype)
{
   NC* nc;
   MPI_Offset mpi_start[NC_MAX_VAR_DIMS], mpi_count[NC_MAX_VAR_DIMS];
   int d;
   int rank = 0;
   int status;

   if ((status = NC_check_id(ncid, &nc)))
      return status;

   /* get variable's rank */
   if ((status= PIOc_inq_varndims(nc->ext_ncid, varid, &rank)))
      return status;

   /* We must convert the start and count arrays to MPI_Offset type. */
   for (d = 0; d < rank; d++)
   {
      mpi_start[d] = startp[d];
      mpi_count[d] = countp[d];
   }

   /* Call the PIO function to do a get_vara. */
   if ((status == PIOc_get_vara_tc(nc->ext_ncid, varid, mpi_start, mpi_count, memtype, ip)))
      return status;

   return NC_NOERR;
}

static int
PIO_put_vara(int ncid,
             int varid,
             const size_t* startp,
             const size_t* countp,
             const void*ip,
             nc_type memtype)
{
   NC* nc;
   /* PIO_INFO* nc5; */
   int status;
   MPI_Offset mpi_start[NC_MAX_VAR_DIMS], mpi_count[NC_MAX_VAR_DIMS];
   int d;
   int rank;

   status = NC_check_id(ncid, &nc);
   if (status != NC_NOERR) return status;

   /* nc5 = PIO_DATA(nc); */
   /* assert(nc5); */

   /* get variable's rank */
   status = PIOc_inq_varndims(nc->ext_ncid, varid, &rank);
   if (status) return status;

   /* We must convert the start and count arrays to MPI_Offset type. */
   for (d = 0; d < rank; d++) {
      mpi_start[d] = startp[d];
      mpi_count[d] = countp[d];
   }

   if (memtype == NC_NAT) {
      status = PIOc_inq_vartype(nc->ext_ncid, varid, &memtype);
      if (status) return status;
   }

   switch(memtype)
   {
   case NC_BYTE:
      status = PIOc_put_vara_schar(nc->ext_ncid, varid, mpi_start, mpi_count, ip); break;
   case NC_CHAR:
      status = PIOc_put_vara_text(nc->ext_ncid, varid, mpi_start, mpi_count, ip); break;
   case NC_SHORT:
      status = PIOc_put_vara_short(nc->ext_ncid, varid, mpi_start, mpi_count, ip); break;
   case NC_INT:
      status = PIOc_put_vara_int(nc->ext_ncid, varid, mpi_start, mpi_count, ip); break;
   case NC_FLOAT:
      status = PIOc_put_vara_float(nc->ext_ncid, varid, mpi_start, mpi_count, ip); break;
   case NC_DOUBLE:
      status = PIOc_put_vara_double(nc->ext_ncid, varid, mpi_start, mpi_count, ip); break;
   case NC_UBYTE:
      status = PIOc_put_vara_uchar(nc->ext_ncid, varid, mpi_start, mpi_count, ip); break;
   case NC_USHORT:
      status = PIOc_put_vara_ushort(nc->ext_ncid, varid, mpi_start, mpi_count, ip); break;
   case NC_UINT:
      status = PIOc_put_vara_uint(nc->ext_ncid, varid, mpi_start, mpi_count, ip); break;
   case NC_INT64:
      status = PIOc_put_vara_longlong(nc->ext_ncid, varid, mpi_start, mpi_count, ip); break;
   case NC_UINT64:
      status = PIOc_put_vara_ulonglong(nc->ext_ncid, varid, mpi_start, mpi_count, ip); break;
   default:
      status = NC_EBADTYPE;
   }
   return status;
}

static int
PIO_get_vars(int ncid,
             int varid,
             const size_t* startp,
             const size_t* countp,
             const ptrdiff_t* stridep,
             void* ip,
             nc_type memtype)
{
   NC* nc;
   /* PIO_INFO* nc5; */
   int status;
   MPI_Offset mpi_start[NC_MAX_VAR_DIMS], mpi_count[NC_MAX_VAR_DIMS], mpi_stride[NC_MAX_VAR_DIMS];
   int d;
   int rank = 0;

   status = NC_check_id(ncid, &nc);
   if (status != NC_NOERR) return status;

   /* nc5 = PIO_DATA(nc); */
   /* assert(nc5); */

   /* get variable's rank */
   status= PIOc_inq_varndims(nc->ext_ncid, varid, &rank);
   if (status) return status;

   /* We must convert the start, count, and stride arrays to MPI_Offset type. */
   for (d = 0; d < rank; d++) {
      mpi_start[d] = startp[d];
      mpi_count[d] = countp[d];
      mpi_stride[d] = stridep[d];
   }

   if (memtype == NC_NAT) {
      status = PIOc_inq_vartype(nc->ext_ncid, varid, &memtype);
      if (status) return status;
   }

   switch(memtype) {
   case NC_BYTE:
      status=PIOc_get_vars_schar(nc->ext_ncid, varid, mpi_start, mpi_count, mpi_stride, ip); break;
   case NC_CHAR:
      status=PIOc_get_vars_text(nc->ext_ncid, varid, mpi_start, mpi_count, mpi_stride, ip); break;
   case NC_SHORT:
      status=PIOc_get_vars_short(nc->ext_ncid, varid, mpi_start, mpi_count, mpi_stride, ip); break;
   case NC_INT:
      status=PIOc_get_vars_int(nc->ext_ncid, varid, mpi_start, mpi_count, mpi_stride, ip); break;
   case NC_FLOAT:
      status=PIOc_get_vars_float(nc->ext_ncid, varid, mpi_start, mpi_count, mpi_stride, ip); break;
   case NC_DOUBLE:
      status=PIOc_get_vars_double(nc->ext_ncid, varid, mpi_start, mpi_count, mpi_stride, ip); break;
   case NC_UBYTE:
      status=PIOc_get_vars_uchar(nc->ext_ncid, varid, mpi_start, mpi_count, mpi_stride, ip); break;
   case NC_USHORT:
      status=PIOc_get_vars_ushort(nc->ext_ncid, varid, mpi_start, mpi_count, mpi_stride, ip); break;
   case NC_UINT:
      status=PIOc_get_vars_uint(nc->ext_ncid, varid, mpi_start, mpi_count, mpi_stride, ip); break;
   case NC_INT64:
      status=PIOc_get_vars_longlong(nc->ext_ncid, varid, mpi_start, mpi_count, mpi_stride, ip); break;
   case NC_UINT64:
      status=PIOc_get_vars_ulonglong(nc->ext_ncid, varid, mpi_start, mpi_count, mpi_stride, ip); break;
   default:
      status = NC_EBADTYPE;
   }
   return status;
}

static int
PIO_put_vars(int ncid,
             int varid,
             const size_t* startp,
             const size_t* countp,
             const ptrdiff_t* stridep,
             const void*ip,
             nc_type memtype)
{
   NC* nc;
   /* PIO_INFO* nc5; */
   int status;
   MPI_Offset mpi_start[NC_MAX_VAR_DIMS], mpi_count[NC_MAX_VAR_DIMS], mpi_stride[NC_MAX_VAR_DIMS];
   int d;
   int rank;

   status = NC_check_id(ncid, &nc);
   if (status != NC_NOERR) return status;

   /* nc5 = PIO_DATA(nc); */
   /* assert(nc5); */

   /* get variable's rank */
   status = PIOc_inq_varndims(nc->ext_ncid, varid, &rank);
   if (status) return status;

   /* We must convert the start, count, and stride arrays to MPI_Offset type. */
   for (d = 0; d < rank; d++) {
      mpi_start[d] = startp[d];
      mpi_count[d] = countp[d];
      mpi_stride[d] = stridep[d];
   }

   if (memtype == NC_NAT) {
      status = PIOc_inq_vartype(nc->ext_ncid, varid, &memtype);
      if (status) return status;
   }

   switch(memtype) {
   case NC_BYTE:
      status = PIOc_put_vars_schar(nc->ext_ncid, varid, mpi_start, mpi_count, mpi_stride, ip); break;
   case NC_CHAR:
      status = PIOc_put_vars_text(nc->ext_ncid, varid, mpi_start, mpi_count, mpi_stride, ip); break;
   case NC_SHORT:
      status = PIOc_put_vars_short(nc->ext_ncid, varid, mpi_start, mpi_count, mpi_stride, ip); break;
   case NC_INT:
      status = PIOc_put_vars_int(nc->ext_ncid, varid, mpi_start, mpi_count, mpi_stride, ip); break;
   case NC_FLOAT:
      status = PIOc_put_vars_float(nc->ext_ncid, varid, mpi_start, mpi_count, mpi_stride, ip); break;
   case NC_DOUBLE:
      status = PIOc_put_vars_double(nc->ext_ncid, varid, mpi_start, mpi_count, mpi_stride, ip); break;
   case NC_UBYTE:
      status = PIOc_put_vars_uchar(nc->ext_ncid, varid, mpi_start, mpi_count, mpi_stride, ip); break;
   case NC_USHORT:
      status = PIOc_put_vars_ushort(nc->ext_ncid, varid, mpi_start, mpi_count, mpi_stride, ip); break;
   case NC_UINT:
      status = PIOc_put_vars_uint(nc->ext_ncid, varid, mpi_start, mpi_count, mpi_stride, ip); break;
   case NC_INT64:
      status = PIOc_put_vars_longlong(nc->ext_ncid, varid, mpi_start, mpi_count, mpi_stride, ip); break;
   case NC_UINT64:
      status = PIOc_put_vars_ulonglong(nc->ext_ncid, varid, mpi_start, mpi_count, mpi_stride, ip); break;
   default:
      status = NC_EBADTYPE;
   }
   return status;
}

static int
PIO_get_varm(int ncid,
             int varid,
             const size_t* startp,
             const size_t* countp,
             const ptrdiff_t* stridep,
             const ptrdiff_t* imapp,
             void* ip,
             nc_type memtype)
{
   /* NC* nc; */
   /* PIO_INFO* nc5; */
   /* int status; */
   /* MPI_Offset mpi_start[NC_MAX_VAR_DIMS], mpi_count[NC_MAX_VAR_DIMS], mpi_stride[NC_MAX_VAR_DIMS], mpi_imap[NC_MAX_VAR_DIMS]; */
   /* int d; */
   /* int rank = 0; */

   /* status = NC_check_id(ncid, &nc); */
   /* if (status != NC_NOERR) return status; */

   /* nc5 = PIO_DATA(nc); */
   /* assert(nc5); */

   /* /\* get variable's rank *\/ */
   /* status= PIOc_inq_varndims(nc->ext_ncid, varid, &rank); */
   /* if (status) return status; */

   /* /\* We must convert the start, count, stride, and imap arrays to MPI_Offset type. *\/ */
   /* for (d = 0; d < rank; d++) { */
   /*      mpi_start[d] = startp[d]; */
   /*      mpi_count[d] = countp[d]; */
   /*      mpi_stride[d] = stridep[d]; */
   /*      mpi_imap[d] = imapp[d]; */
   /* } */

   /* if (memtype == NC_NAT) { */
   /*     status = PIOc_inq_vartype(nc->ext_ncid, varid, &memtype); */
   /*     if (status) return status; */
   /* } */

   /*     switch(memtype) { */
   /*     case NC_BYTE: */
   /*         status=PIOc_get_varm_schar(nc->ext_ncid, varid, mpi_start, mpi_count, mpi_stride, mpi_imap, ip); break; */
   /*     case NC_CHAR: */
   /*         status=PIOc_get_varm_text(nc->ext_ncid, varid, mpi_start, mpi_count, mpi_stride, mpi_imap, ip); break; */
   /*     case NC_SHORT: */
   /*         status=PIOc_get_varm_short(nc->ext_ncid, varid, mpi_start, mpi_count, mpi_stride, mpi_imap, ip); break; */
   /*     case NC_INT: */
   /*         status=PIOc_get_varm_int(nc->ext_ncid, varid, mpi_start, mpi_count, mpi_stride, mpi_imap, ip); break; */
   /*     case NC_FLOAT: */
   /*         status=PIOc_get_varm_float(nc->ext_ncid, varid, mpi_start, mpi_count, mpi_stride, mpi_imap, ip); break; */
   /*     case NC_DOUBLE: */
   /*         status=PIOc_get_varm_double(nc->ext_ncid, varid, mpi_start, mpi_count, mpi_stride, mpi_imap, ip); break; */
   /*     case NC_UBYTE: */
   /*         status=PIOc_get_varm_uchar(nc->ext_ncid, varid, mpi_start, mpi_count, mpi_stride, mpi_imap, ip); break; */
   /*     case NC_USHORT: */
   /*         status=PIOc_get_varm_ushort(nc->ext_ncid, varid, mpi_start, mpi_count, mpi_stride, mpi_imap, ip); break; */
   /*     case NC_UINT: */
   /*         status=PIOc_get_varm_uint(nc->ext_ncid, varid, mpi_start, mpi_count, mpi_stride, mpi_imap, ip); break; */
   /*     case NC_INT64: */
   /*         status=PIOc_get_varm_longlong(nc->ext_ncid, varid, mpi_start, mpi_count, mpi_stride, mpi_imap, ip); break; */
   /*     case NC_UINT64: */
   /*         status=PIOc_get_varm_ulonglong(nc->ext_ncid, varid, mpi_start, mpi_count, mpi_stride, mpi_imap, ip); break; */
   /*     default: */
   /*         status = NC_EBADTYPE; */
   /*     } */
   /* return status; */
   return PIO_NOERR;
}

static int
PIO_put_varm(int ncid,
             int varid,
             const size_t* startp,
             const size_t* countp,
             const ptrdiff_t* stridep,
             const ptrdiff_t* imapp,
             const void*ip,
             nc_type memtype)
{
   /* NC* nc; */
   /* PIO_INFO* nc5; */
   /* int status; */
   /* MPI_Offset mpi_start[NC_MAX_VAR_DIMS], mpi_count[NC_MAX_VAR_DIMS], mpi_stride[NC_MAX_VAR_DIMS], mpi_imap[NC_MAX_VAR_DIMS]; */
   /* int d; */
   /* int rank; */

   /* status = NC_check_id(ncid, &nc); */
   /* if (status != NC_NOERR) return status; */

   /* nc5 = PIO_DATA(nc); */
   /* assert(nc5); */

   /* /\* get variable's rank *\/ */
   /* status = PIOc_inq_varndims(nc->ext_ncid, varid, &rank); */
   /* if (status) return status; */

   /* /\* We must convert the start, count, stride, and imap arrays to MPI_Offset type. *\/ */
   /* for (d = 0; d < rank; d++) { */
   /*      mpi_start[d] = startp[d]; */
   /*      mpi_count[d] = countp[d]; */
   /*      mpi_stride[d] = stridep[d]; */
   /*      mpi_imap[d] = imapp[d]; */
   /* } */

   /* if (memtype == NC_NAT) { */
   /*     status = PIOc_inq_vartype(nc->ext_ncid, varid, &memtype); */
   /*     if (status) return status; */
   /* } */

   /* if (nc5->pnetcdf_access_mode == NC_INDEPENDENT) { */
   /*     switch(memtype) { */
   /*     case NC_BYTE: */
   /*         status = PIOc_put_varm_schar(nc->ext_ncid, varid, mpi_start, mpi_count, mpi_stride, mpi_imap, ip); break; */
   /*     case NC_CHAR: */
   /*         status = PIOc_put_varm_text(nc->ext_ncid, varid, mpi_start, mpi_count, mpi_stride, mpi_imap, ip); break; */
   /*     case NC_SHORT: */
   /*         status = PIOc_put_varm_short(nc->ext_ncid, varid, mpi_start, mpi_count, mpi_stride, mpi_imap, ip); break; */
   /*     case NC_INT: */
   /*         status = PIOc_put_varm_int(nc->ext_ncid, varid, mpi_start, mpi_count, mpi_stride, mpi_imap, ip); break; */
   /*     case NC_FLOAT: */
   /*         status = PIOc_put_varm_float(nc->ext_ncid, varid, mpi_start, mpi_count, mpi_stride, mpi_imap, ip); break; */
   /*     case NC_DOUBLE: */
   /*         status = PIOc_put_varm_double(nc->ext_ncid, varid, mpi_start, mpi_count, mpi_stride, mpi_imap, ip); break; */
   /*     case NC_UBYTE: */
   /*         status = PIOc_put_varm_uchar(nc->ext_ncid, varid, mpi_start, mpi_count, mpi_stride, mpi_imap, ip); break; */
   /*     case NC_USHORT: */
   /*         status = PIOc_put_varm_ushort(nc->ext_ncid, varid, mpi_start, mpi_count, mpi_stride, mpi_imap, ip); break; */
   /*     case NC_UINT: */
   /*         status = PIOc_put_varm_uint(nc->ext_ncid, varid, mpi_start, mpi_count, mpi_stride, mpi_imap, ip); break; */
   /*     case NC_INT64: */
   /*         status = PIOc_put_varm_longlong(nc->ext_ncid, varid, mpi_start, mpi_count, mpi_stride, mpi_imap, ip); break; */
   /*     case NC_UINT64: */
   /*         status = PIOc_put_varm_ulonglong(nc->ext_ncid, varid, mpi_start, mpi_count, mpi_stride, mpi_imap, ip); break; */
   /*     default: */
   /*         status = NC_EBADTYPE; */
   /*     } */
   /*   } else { */
   /*     switch(memtype) { */
   /*     case NC_BYTE: */
   /*         status = PIOc_put_varm_schar_all(nc->ext_ncid, varid, mpi_start, mpi_count, mpi_stride, mpi_imap, ip); break; */
   /*     case NC_CHAR: */
   /*         status = PIOc_put_varm_text_all(nc->ext_ncid, varid, mpi_start, mpi_count, mpi_stride, mpi_imap, ip); break; */
   /*     case NC_SHORT: */
   /*         status = PIOc_put_varm_short_all(nc->ext_ncid, varid, mpi_start, mpi_count, mpi_stride, mpi_imap, ip); break; */
   /*     case NC_INT: */
   /*         status = PIOc_put_varm_int_all(nc->ext_ncid, varid, mpi_start, mpi_count, mpi_stride, mpi_imap, ip); break; */
   /*     case NC_FLOAT: */
   /*         status = PIOc_put_varm_float_all(nc->ext_ncid, varid, mpi_start, mpi_count, mpi_stride, mpi_imap, ip); break; */
   /*     case NC_DOUBLE: */
   /*         status = PIOc_put_varm_double_all(nc->ext_ncid, varid, mpi_start, mpi_count, mpi_stride, mpi_imap, ip); break; */
   /*     case NC_UBYTE: */
   /*         status = PIOc_put_varm_uchar_all(nc->ext_ncid, varid, mpi_start, mpi_count, mpi_stride, mpi_imap, ip); break; */
   /*     case NC_USHORT: */
   /*         status = PIOc_put_varm_ushort_all(nc->ext_ncid, varid, mpi_start, mpi_count, mpi_stride, mpi_imap, ip); break; */
   /*     case NC_UINT: */
   /*         status = PIOc_put_varm_uint_all(nc->ext_ncid, varid, mpi_start, mpi_count, mpi_stride, mpi_imap, ip); break; */
   /*     case NC_INT64: */
   /*         status = PIOc_put_varm_longlong_all(nc->ext_ncid, varid, mpi_start, mpi_count, mpi_stride, mpi_imap, ip); break; */
   /*     case NC_UINT64: */
   /*         status = PIOc_put_varm_ulonglong_all(nc->ext_ncid, varid, mpi_start, mpi_count, mpi_stride, mpi_imap, ip); break; */
   /*     default: */
   /*         status = NC_EBADTYPE; */
   /*     } */
   /* } */
   /* return status; */
   return PIO_NOERR;
}

static int
PIO_inq_var_all(int ncid, int varid, char *name, nc_type *xtypep, int *ndimsp, int *dimidsp,
                int *nattsp, int *shufflep, int *deflatep, int *deflate_levelp, int *fletcher32p,
                int *contiguousp, size_t *chunksizesp, int *no_fill, void *fill_valuep,
                int *endiannessp, unsigned int* idp, size_t *nparamsp, unsigned int *paramsp)
{
   int status;
   NC *nc;

   if ((status = NC_check_id(ncid, &nc)))
      return status;

   return PIOc_inq_var_all(nc->ext_ncid, varid, name, xtypep, ndimsp, dimidsp, nattsp, shufflep,
                           deflatep, deflate_levelp, fletcher32p, contiguousp, chunksizesp,
                           no_fill, fill_valuep, endiannessp, idp, nparamsp, paramsp);
}

static int
PIO_var_par_access(int ncid, int varid, int par_access)
{
   NC *nc;
   /* PIO_INFO* nc5; */
   int status;

   if (par_access != NC_INDEPENDENT && par_access != NC_COLLECTIVE)
      return NC_EINVAL;

   status = NC_check_id(ncid, &nc);
   if (status != NC_NOERR) return status;

   /* nc5 = PIO_DATA(nc); */
   /* assert(nc5); */

   /* if (par_access == nc5->pnetcdf_access_mode) */
   /*     return NC_NOERR; */
   /* nc5->pnetcdf_access_mode = par_access; */
   /* if (par_access == NC_INDEPENDENT) */
   /*     return PIOc_begin_indep_data(nc->ext_ncid); */
   /* else */
   /*     return PIOc_end_indep_data(nc->ext_ncid); */
   return PIO_NOERR;
}

#ifdef USE_NETCDF4

static int
PIO_show_metadata(int ncid)
{
   return NC_NOERR;
}

static int
PIO_inq_unlimdims(int ncid, int *ndimsp, int *unlimdimidsp)
{
   int retval;
   int unlimid;

   if ((retval = PIO_inq_unlimdim(ncid, &unlimid)))
      return retval;
   if (unlimid != -1) {
      if (ndimsp) *ndimsp = 1;
      if (unlimdimidsp)
         unlimdimidsp[0] = unlimid;
   } else
      if (ndimsp) *ndimsp = 0;
   return NC_NOERR;
}

static int
PIO_inq_type_equal(int ncid1, nc_type typeid1, int ncid2, nc_type typeid2, int* equalp)
{
   /* Check input. */
   if (equalp == NULL) return NC_NOERR;

   if (typeid1 <= NC_NAT || typeid2 <= NC_NAT)
      return NC_EINVAL;

   *equalp = 0; /* assume */

   /* If one is atomic, and the other user-defined, the types are not equal */
   if ((typeid1 <= NC_STRING && typeid2 > NC_STRING) ||
       (typeid2 <= NC_STRING && typeid1 > NC_STRING)) {
      if (equalp) *equalp = 0;
      return NC_NOERR;
   }

   /* If both are atomic types, the answer is easy. */
   if (typeid1 <= ATOMICTYPEMAX5) {
      if (equalp) {
         if (typeid1 == typeid2)
            *equalp = 1;
         else
            *equalp = 0;
      }
      return NC_NOERR;
   }
   return NC_NOERR;
}

static int
PIO_def_grp(int parent_ncid, const char *name, int *new_ncid)
{
   return NC_ENOTNC4;
}

static int
PIO_rename_grp(int ncid, const char *name)
{
   return NC_ENOTNC4;
}

static int
PIO_inq_ncid(int ncid, const char *name, int *grp_ncid)
{
   if (grp_ncid) *grp_ncid = ncid;
   return NC_NOERR;
}

static int
PIO_inq_grps(int ncid, int *numgrps, int *ncids)
{
   if (numgrps)
      *numgrps = 0;
   return NC_NOERR;
}

static int
PIO_inq_grpname(int ncid, char *name)
{
   if (name)
      strcpy(name, "/");
   return NC_NOERR;
}

static int
PIO_inq_grpname_full(int ncid, size_t *lenp, char *full_name)
{
   if (full_name)
      strcpy(full_name, "/");
   if (lenp) *lenp = 1;
   return NC_NOERR;
}

static int
PIO_inq_grp_parent(int ncid, int *parent_ncid)
{
   return NC_ENOGRP;
}

static int
PIO_inq_grp_full_ncid(int ncid, const char *full_name, int *grp_ncid)
{
   return NC_ENOGRP;
}

static int
PIO_inq_varids(int ncid, int *nvarsp, int *varids)
{
   int retval,v,nvars;
   /* This is, effectively, a netcdf-3 file, there is only one group, the root
      group, and its vars have ids 0 thru nvars - 1. */
   if ((retval = PIO_inq(ncid, NULL, &nvars, NULL, NULL)))
      return retval;
   if (nvarsp) *nvarsp = nvars;
   if (varids)
      for (v = 0; v < nvars; v++)
         varids[v] = v;
   return NC_NOERR;
}

static int
PIO_inq_dimids(int ncid, int *ndimsp, int *dimids, int include_parents)
{
   int retval,d,ndims;
   /* If this is like a netcdf-3 file, then the dimids are going to be 0
      thru ndims-1, so just provide them. */
   if ((retval = PIO_inq(ncid, &ndims,  NULL, NULL, NULL)))
      return retval;
   if (ndimsp) *ndimsp = ndims;
   if (dimids)
      for (d = 0; d < ndims; d++)
         dimids[d] = d;
   return NC_NOERR;
}

static int
PIO_inq_typeid(int ncid, const char *name, nc_type *typeidp)
{
   int i;
   for (i = 0; i <= ATOMICTYPEMAX5; i++)
      if (!strcmp(name, NC_atomictypename(i))) {
         if (typeidp) *typeidp = i;
         return NC_NOERR;
      }
   return NC_ENOTNC4;
}

static int
PIO_inq_typeids(int ncid, int *ntypes, int *typeids)
{
   if (ntypes) *ntypes = 0;
   return NC_NOERR;
}

static int
PIO_inq_user_type(int ncid, nc_type typeid, char *name, size_t *size,
                  nc_type *base_nc_typep, size_t *nfieldsp, int *classp)
{
   return NC_ENOTNC4;
}

static int
PIO_def_compound(int ncid, size_t size, const char *name, nc_type *typeidp)
{
   return NC_ENOTNC4;
}

static int
PIO_insert_compound(int ncid, nc_type typeid, const char *name, size_t offset,
                    nc_type field_typeid)
{
   return NC_ENOTNC4;
}

static int
PIO_insert_array_compound(int ncid, nc_type typeid, const char *name,
                          size_t offset, nc_type field_typeid,
                          int ndims, const int *dim_sizes)
{
   return NC_ENOTNC4;
}


static int
PIO_inq_compound_field(int ncid, nc_type typeid, int fieldid, char *name,
                       size_t *offsetp, nc_type *field_typeidp, int *ndimsp,
                       int *dim_sizesp)
{
   return NC_ENOTNC4;
}

static int
PIO_inq_compound_fieldindex(int ncid, nc_type typeid, const char *name, int *fieldidp)
{
   return NC_ENOTNC4;
}

static int
PIO_def_opaque(int ncid, size_t datum_size, const char *name, nc_type* xtypep)
{
   return NC_ENOTNC4;
}

static int
PIO_def_vlen(int ncid, const char *name, nc_type base_typeid, nc_type* xtypep)
{
   return NC_ENOTNC4;
}

static int
PIO_def_enum(int ncid, nc_type base_typeid, const char *name,
             nc_type *typeidp)
{
   return NC_ENOTNC4;
}

static int
PIO_inq_enum_ident(int ncid, nc_type xtype, long long value, char *identifier)
{
   return NC_ENOTNC4;
}

static int
PIO_inq_enum_member(int ncid, nc_type typeid, int idx, char *identifier,
                    void *value)
{
   return NC_ENOTNC4;
}

static int
PIO_insert_enum(int ncid, nc_type typeid, const char *identifier,
                const void *value)
{
   return NC_ENOTNC4;
}

static int
PIO_put_vlen_element(int ncid, int typeid, void *vlen_element,
                     size_t len, const void *data)
{
   return NC_ENOTNC4;
}

static int
PIO_get_vlen_element(int ncid, int typeid, const void *vlen_element,
                     size_t *len, void *data)
{
   return NC_ENOTNC4;
}

static int
PIO_set_var_chunk_cache(int ncid, int varid, size_t size, size_t nelems, float preemption)
{
   return NC_ENOTNC4;
}

static int
PIO_get_var_chunk_cache(int ncid, int varid, size_t *sizep, size_t *nelemsp, float *preemptionp)
{
   return NC_ENOTNC4;
}

static int
PIO_def_var_deflate(int ncid, int varid, int shuffle, int deflate,
                    int deflate_level)
{
   return NC_ENOTNC4;
}

static int
PIO_def_var_fletcher32(int ncid, int varid, int fletcher32)
{
   return NC_ENOTNC4;
}

static int
PIO_def_var_chunking(int ncid, int varid, int contiguous, const size_t *chunksizesp)
{
   return NC_ENOTNC4;
}

static int
PIO_def_var_fill(int ncid, int varid, int no_fill, const void *fill_value)
{
   return NC_ENOTNC4;
}

static int
PIO_def_var_endian(int ncid, int varid, int endianness)
{
   return NC_ENOTNC4;
}

static int
PIO_def_var_filter(int ncid, int varid, unsigned int id, size_t nparams, const unsigned int* parms)
{
   return NC_ENOTNC4;
}

#endif /*USE_NETCDF4*/

/**************************************************/
/* Pnetcdf Dispatch table */

NC_Dispatch PIO_dispatcher = {

   NC_FORMATX_PNETCDF,

   PIO_create,
   PIO_open,

   PIO_redef,
   PIO__enddef,
   PIO_sync,
   PIO_abort,
   PIO_close,
   PIO_set_fill,
   PIO_inq_base_pe,
   PIO_set_base_pe,
   PIO_inq_format,
   PIO_inq_format_extended,

   PIO_inq,
   PIO_inq_type,

   PIO_def_dim,
   PIO_inq_dimid,
   PIO_inq_dim,
   PIO_inq_unlimdim,
   PIO_rename_dim,

   PIO_inq_att,
   PIO_inq_attid,
   PIO_inq_attname,
   PIO_rename_att,
   PIO_del_att,
   PIO_get_att,
   PIO_put_att,

   PIO_def_var,
   PIO_inq_varid,
   PIO_rename_var,
   PIO_get_vara,
   PIO_put_vara,
   PIO_get_vars,
   PIO_put_vars,
   PIO_get_varm,
   PIO_put_varm,

   PIO_inq_var_all,

   PIO_var_par_access,

#ifdef USE_NETCDF4
   PIO_show_metadata,
   PIO_inq_unlimdims,

   PIO_inq_ncid,
   PIO_inq_grps,
   PIO_inq_grpname,
   PIO_inq_grpname_full,
   PIO_inq_grp_parent,
   PIO_inq_grp_full_ncid,
   PIO_inq_varids,
   PIO_inq_dimids,
   PIO_inq_typeids,
   PIO_inq_type_equal,
   PIO_def_grp,
   PIO_rename_grp,
   PIO_inq_user_type,
   PIO_inq_typeid,

   PIO_def_compound,
   PIO_insert_compound,
   PIO_insert_array_compound,
   PIO_inq_compound_field,
   PIO_inq_compound_fieldindex,
   PIO_def_vlen,
   PIO_put_vlen_element,
   PIO_get_vlen_element,
   PIO_def_enum,
   PIO_insert_enum,
   PIO_inq_enum_member,
   PIO_inq_enum_ident,
   PIO_def_opaque,
   PIO_def_var_deflate,
   PIO_def_var_fletcher32,
   PIO_def_var_chunking,
   PIO_def_var_fill,
   PIO_def_var_endian,
   PIO_def_var_filter,
   PIO_set_var_chunk_cache,
   PIO_get_var_chunk_cache,
#endif /*USE_NETCDF4*/

};

NC_Dispatch* PIO_dispatch_table = NULL; /* moved here from ddispatch.c */

int
PIO_initialize(void)
{
   PIO_dispatch_table = &PIO_dispatcher;
   return NC_NOERR;
}

int
PIO_finalize(void)
{
   return NC_NOERR;
}
