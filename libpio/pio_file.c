/**
 * @file
 * PIO File Handling
 */
#include <config.h>
#include <pio.h>
#include <pio_internal.h>

/* This is the next ncid that will be used when a file is opened or
   created. We start at 16 so that it will be easy for us to notice
   that it's not netcdf (starts at 4), pnetcdf (starts at 0) or
   netCDF-4/HDF5 (starts at 65xxx). */
int pio_next_ncid = 513;

/**
 * Open an existing file using PIO library.
 *
 * If the open fails, try again as netCDF serial before giving
 * up. Input parameters are read on comp task 0 and ignored elsewhere.
 *
 * Note that the file is opened with default fill mode, NOFILL for
 * pnetcdf, and FILL for netCDF classic and netCDF-4 files.
 *
 * @param iosysid : A defined pio system descriptor (input)
 * @param ncidp : A pio file descriptor (output)
 * @param iotype : A pio output format (input)
 * @param filename : The filename to open
 * @param mode : The netcdf mode for the open operation
 * @return 0 for success, error code otherwise.
 * @ingroup PIO_openfile
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_openfile(int iosysid, int *ncidp, int *iotype, const char *filename,
                  int mode)
{
   return PIOc_openfile2(iosysid, ncidp, iotype, filename, mode);
}

/**
 * Open an existing file using PIO library.
 *
 * Note that the file is opened with default fill mode, NOFILL for
 * pnetcdf, and FILL for netCDF classic and netCDF-4 files.
 *
 * @param iosysid : A defined pio system descriptor (input)
 * @param ncidp : A pio file descriptor (output)
 * @param iotype : A pio output format (input)
 * @param filename : The filename to open
 * @param mode : The netcdf mode for the open operation
 * @return 0 for success, error code otherwise.
 * @ingroup PIO_openfile
 * @author Ed Hartnett
 */
int PIOc_openfile2(int iosysid, int *ncidp, int *iotype, const char *filename,
                   int mode)
{
   iosystem_desc_t *ios;  /* Pointer to io system information. */
   int ret;               /* Return code from function calls. */

   LOG((1, "PIOc_openfile2 iosysid %d *iotype %d filename %s mode %d", iosysid,
	iotype ? *iotype : 0, filename, mode));

   /* Get the IO system info from the id. */
   if (!(ios = pio_get_iosystem_from_id(iosysid)))
      return pio_err(NULL, NULL, PIO_EBADID, __FILE__, __LINE__);

   /* Set this as the current IO system ID. */
   if ((ret = nc_set_iosysid(iosysid)))
      return ret;

   /* Turn on PIO. */
   mode |= NC_PIO;

   /* Set the mode flag based on io type. */
   if (*iotype == PIO_IOTYPE_NETCDF4C)
      mode |= NC_NETCDF4;
   if (*iotype == PIO_IOTYPE_NETCDF4P)
      mode |= NC_NETCDF4 | NC_SHARE | NC_MPIIO;
   if (*iotype == PIO_IOTYPE_PNETCDF)
      mode |= NC_PNETCDF;
   
   if (*iotype == PIO_IOTYPE_PNETCDF || *iotype == PIO_IOTYPE_NETCDF4P)
      ret = nc_open_par(filename, mode, ios->io_comm, MPI_INFO_NULL, ncidp);
   else
      ret = nc_open(filename, mode, ncidp);

   return ret;
}

/**
 * Open an existing file using PIO library.
 *
 * Input parameters are read on comp task 0 and ignored elsewhere.
 *
 * @param iosysid A defined pio system descriptor
 * @param path The filename to open
 * @param mode The netcdf mode for the open operation
 * @param ncidp pointer to int where ncid will go
 * @return 0 for success, error code otherwise.
 * @ingroup PIO_openfile
 * @author Ed Hartnett
 */
int PIOc_open(int iosysid, const char *path, int mode, int *ncidp)
{
   int iotype = PIO_IOTYPE_NETCDF;

   if (mode & NC_PNETCDF)
      iotype = PIO_IOTYPE_PNETCDF;
   if (mode & NC_NETCDF4)
   {
      if (mode & NC_SHARE)
         iotype = PIO_IOTYPE_NETCDF4P;
      else
         iotype = PIO_IOTYPE_NETCDF4C;
   }
   
   return PIOc_openfile2(iosysid, ncidp, &iotype, path, mode);
}

/**
 * Create a new file using pio. Input parameters are read on comp task
 * 0 and ignored elsewhere. NOFILL mode will be turned on in all
 * cases.
 *
 * @param iosysid A defined pio system ID, obtained from
 * PIOc_InitIntercomm() or PIOc_InitAsync().
 * @param ncidp A pointer that gets the ncid of the newly created
 * file.
 * @param iotype A pointer to a pio output format. Must be one of
 * PIO_IOTYPE_PNETCDF, PIO_IOTYPE_NETCDF, PIO_IOTYPE_NETCDF4C, or
 * PIO_IOTYPE_NETCDF4P.
 * @param filename The filename to create.
 * @param mode The netcdf mode for the create operation.
 * @returns 0 for success, error code otherwise.
 * @ingroup PIO_createfile
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_createfile(int iosysid, int *ncidp, int *iotype, const char *filename,
                    int mode)
{
   iosystem_desc_t *ios;  /* Pointer to io system information. */
   int ret;               /* Return code from function calls. */

   /* Get the IO system info from the id. */
   if (!(ios = pio_get_iosystem_from_id(iosysid)))
      return pio_err(NULL, NULL, PIO_EBADID, __FILE__, __LINE__);

   /* Set this as the current IO system ID. */
   if ((ret = nc_set_iosysid(iosysid)))
      return ret;

   /* Based on the IO type, come up with create mode. */
   mode |= NC_PIO;
   if (*iotype == PIO_IOTYPE_NETCDF4C || *iotype == PIO_IOTYPE_NETCDF4P)
      mode |= NC_NETCDF4;
   if (*iotype == PIO_IOTYPE_NETCDF4P)
      mode |= NC_SHARE;

   /* Call nc_create/nc_create_par to create the file. */
   if (*iotype == PIO_IOTYPE_PNETCDF || *iotype == PIO_IOTYPE_NETCDF4P)
      ret = nc_create_par(filename, mode, ios->io_comm, MPI_INFO_NULL, ncidp);
   else
      ret = nc_create(filename, mode, ncidp);

   return ret;
}

/**
 * Open a new file using pio. The default fill mode will be used (FILL
 * for netCDF and netCDF-4 formats, NOFILL for pnetcdf.) Input
 * parameters are read on comp task 0 and ignored elsewhere.
 *
 * @param iosysid : A defined pio system descriptor (input)
 * @param mode : The netcdf mode for the create operation.
 * @param filename : The filename to open
 * @param ncidp : A pio file descriptor (output)
 * @return 0 for success, error code otherwise.
 * @ingroup PIO_create
 * @author Ed Hartnett
 */
int PIOc_create(int iosysid, const char *filename, int mode, int *ncidp)
{
   int iotype = PIO_IOTYPE_NETCDF;

   if (mode & NC_PNETCDF)
      iotype = PIO_IOTYPE_PNETCDF;
   if (mode & NC_NETCDF4)
   {
      if (mode & NC_SHARE)
         iotype = PIO_IOTYPE_NETCDF4P;
      else
         iotype = PIO_IOTYPE_NETCDF4C;
   }
   return PIOc_createfile(iosysid, ncidp, &iotype, filename, mode);
}

/**
 * Close a file previously opened with PIO.
 *
 * @param ncid: the file pointer
 * @returns PIO_NOERR for success, error code otherwise.
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_closefile(int ncid)
{
   iosystem_desc_t *ios;  /* Pointer to io system information. */
   file_desc_t *file;     /* Pointer to file information. */
   int ierr = PIO_NOERR;  /* Return code from function calls. */
   int mpierr = MPI_SUCCESS, mpierr2;  /* Return code from MPI function codes. */

   LOG((1, "PIOc_closefile ncid = %d", ncid));

   /* Find the info about this file. */
   if ((ierr = pio_get_file(ncid, &file)))
      return pio_err(NULL, NULL, ierr, __FILE__, __LINE__);
   ios = file->iosystem;

   /* Sync changes before closing on all tasks if async is not in
    * use, but only on non-IO tasks if async is in use. */
   if (!ios->async || !ios->ioproc)
      if (file->writable)
	 PIOc_sync(ncid);

   /* If async is in use and this is a comp tasks, then the compmaster
    * sends a msg to the pio_msg_handler running on the IO master and
    * waiting for a message. Then broadcast the ncid over the intercomm
    * to the IO tasks. */
   if (ios->async)
   {
      if (!ios->ioproc)
      {
	 int msg = PIO_MSG_CLOSE_FILE;

	 if (ios->compmaster == MPI_ROOT)
	    mpierr = MPI_Send(&msg, 1, MPI_INT, ios->ioroot, 1, ios->union_comm);

	 if (!mpierr)
	    mpierr = MPI_Bcast(&ncid, 1, MPI_INT, ios->compmaster, ios->intercomm);
      }

      /* Handle MPI errors. */
      if ((mpierr2 = MPI_Bcast(&mpierr, 1, MPI_INT, ios->comproot, ios->my_comm)))
	 return check_mpi(file, mpierr2, __FILE__, __LINE__);
      if (mpierr)
	 return check_mpi(file, mpierr, __FILE__, __LINE__);
   }

   /* If this is an IO task, then call the netCDF function. */
   if (ios->ioproc)
   {
      switch (file->iotype)
      {
#ifdef _NETCDF4
      case PIO_IOTYPE_NETCDF4P:
	 ierr = NC4_close(file->fh);
	 break;
      case PIO_IOTYPE_NETCDF4C:
	 if (ios->io_rank == 0)
            ierr = NC4_close(file->fh);
	 break;
#endif
      case PIO_IOTYPE_NETCDF:
	 if (ios->io_rank == 0)
	    ierr = NC3_close(file->fh);
	 break;
#ifdef _PNETCDF
      case PIO_IOTYPE_PNETCDF:
	 if (file->writable)
	    ierr = ncmpi_buffer_detach(file->fh);
	 ierr = ncmpi_close(file->fh);
	 break;
#endif
      default:
	 return pio_err(ios, file, PIO_EBADIOTYPE, __FILE__, __LINE__);
      }
   }

   /* Broadcast and check the return code. */
   if ((mpierr = MPI_Bcast(&ierr, 1, MPI_INT, ios->ioroot, ios->my_comm)))
      return check_mpi(file, mpierr, __FILE__, __LINE__);
   if (ierr)
      return check_netcdf(file, ierr, __FILE__, __LINE__);

   /* Delete file from our list of open files. */
   if ((ierr = pio_delete_file_from_list(ncid)))
      return pio_err(ios, file, ierr, __FILE__, __LINE__);

   return ierr;
}

/**
 * Delete a file.
 *
 * @param iosysid a pio system handle.
 * @param filename a filename.
 * @returns PIO_NOERR for success, error code otherwise.
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_deletefile(int iosysid, const char *filename)
{
   iosystem_desc_t *ios;  /* Pointer to io system information. */
   int ierr = PIO_NOERR;  /* Return code from function calls. */
   int mpierr = MPI_SUCCESS, mpierr2;  /* Return code from MPI function codes. */
   int msg = PIO_MSG_DELETE_FILE;
   size_t len;

   LOG((1, "PIOc_deletefile iosysid = %d filename = %s", iosysid, filename));

   /* Get the IO system info from the id. */
   if (!(ios = pio_get_iosystem_from_id(iosysid)))
      return pio_err(NULL, NULL, PIO_EBADID, __FILE__, __LINE__);

   /* If async is in use, send message to IO master task. */
   if (ios->async)
   {
      if (!ios->ioproc)
      {
	 if (ios->comp_rank==0)
	    mpierr = MPI_Send(&msg, 1,MPI_INT, ios->ioroot, 1, ios->union_comm);

	 len = strlen(filename);
	 if (!mpierr)
	    mpierr = MPI_Bcast(&len, 1, MPI_INT, ios->compmaster, ios->intercomm);
	 if (!mpierr)
	    mpierr = MPI_Bcast((void *)filename, len + 1, MPI_CHAR, ios->compmaster,
			       ios->intercomm);
	 LOG((2, "Bcast len = %d filename = %s", len, filename));
      }

      /* Handle MPI errors. */
      if ((mpierr2 = MPI_Bcast(&mpierr, 1, MPI_INT, ios->comproot, ios->my_comm)))
	 return check_mpi2(ios, NULL, mpierr2, __FILE__, __LINE__);
      if (mpierr)
	 return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
      LOG((3, "done hanlding errors mpierr = %d", mpierr));
   }

   /* If this is an IO task, then call the netCDF function. The
    * barriers are needed to assure that no task is trying to operate
    * on the file while it is being deleted. IOTYPE is not known, but
    * nc_delete() will delete any type of file. */
   if (ios->ioproc)
   {
      mpierr = MPI_Barrier(ios->io_comm);

      if (!mpierr && ios->io_rank == 0)
	 ierr = nc_delete(filename);

      if (!mpierr)
	 mpierr = MPI_Barrier(ios->io_comm);
   }
   LOG((2, "PIOc_deletefile ierr = %d", ierr));

   /* Broadcast and check the return code. */
   if ((mpierr = MPI_Bcast(&ierr, 1, MPI_INT, ios->ioroot, ios->my_comm)))
      return check_mpi2(ios, NULL, mpierr2, __FILE__, __LINE__);
   if (ierr)
      return check_netcdf2(ios, NULL, ierr, __FILE__, __LINE__);

   return ierr;
}

/**
 * PIO interface to nc_sync This routine is called collectively by all
 * tasks in the communicator ios.union_comm.
 *
 * Refer to the <A
 * HREF="http://www.unidata.ucar.edu/software/netcdf/docs/modules.html"
 * target="_blank"> netcdf </A> documentation.
 *
 * @param ncid the ncid of the file to sync.
 * @returns PIO_NOERR for success, error code otherwise.
 * @author Jim Edwards, Ed Hartnett
 */
int PIOc_sync(int ncid)
{
   iosystem_desc_t *ios;  /* Pointer to io system information. */
   file_desc_t *file;     /* Pointer to file information. */
   int mpierr = MPI_SUCCESS, mpierr2;  /* Return code from MPI function codes. */
   int ierr = PIO_NOERR;  /* Return code from function calls. */

   LOG((1, "PIOc_sync ncid = %d", ncid));

   /* Get the file info from the ncid. */
   if ((ierr = pio_get_file(ncid, &file)))
      return pio_err(NULL, NULL, ierr, __FILE__, __LINE__);
   ios = file->iosystem;

   /* Flush data buffers on computational tasks. */
   if (!ios->async || !ios->ioproc)
   {
      if (file->writable)
      {
	 wmulti_buffer *wmb, *twmb;

	 LOG((3, "PIOc_sync checking buffers"));
	 wmb = &file->buffer;
	 while (wmb)
	 {
	    /* If there are any data arrays waiting in the
	     * multibuffer, flush it. */
	    if (wmb->num_arrays > 0)
	       flush_buffer(ncid, wmb, true);
	    twmb = wmb;
	    wmb = wmb->next;
	    if (twmb == &file->buffer)
	    {
	       twmb->ioid = -1;
	       twmb->next = NULL;
	    }
	    else
	    {
	       brel(twmb);
	    }
	 }
      }
   }

   /* If async is in use, send message to IO master tasks. */
   if (ios->async)
   {
      if (!ios->ioproc)
      {
	 int msg = PIO_MSG_SYNC;

	 if (ios->compmaster == MPI_ROOT)
	    mpierr = MPI_Send(&msg, 1, MPI_INT, ios->ioroot, 1, ios->union_comm);

	 if (!mpierr)
	    mpierr = MPI_Bcast(&ncid, 1, MPI_INT, ios->compmaster, ios->intercomm);
      }

      /* Handle MPI errors. */
      if ((mpierr2 = MPI_Bcast(&mpierr, 1, MPI_INT, ios->comproot, ios->my_comm)))
	 check_mpi(file, mpierr2, __FILE__, __LINE__);
      if (mpierr)
	 return check_mpi(file, mpierr, __FILE__, __LINE__);
   }

   /* Call the sync function on IO tasks. */
   if (file->writable)
   {
      if (ios->ioproc)
      {
	 switch(file->iotype)
	 {
#ifdef _NETCDF4
	 case PIO_IOTYPE_NETCDF4P:
	    ierr = NC4_sync(file->fh);
	    break;
	 case PIO_IOTYPE_NETCDF4C:
	    if (ios->io_rank == 0)
	       ierr = NC4_sync(file->fh);
	    break;
#endif
	 case PIO_IOTYPE_NETCDF:
	    if (ios->io_rank == 0)
	       ierr = NC3_sync(file->fh);
	    break;
#ifdef _PNETCDF
	 case PIO_IOTYPE_PNETCDF:
	    flush_output_buffer(file, true, 0);
	    ierr = ncmpi_sync(file->fh);
	    break;
#endif
	 default:
	    return pio_err(ios, file, PIO_EBADIOTYPE, __FILE__, __LINE__);
	 }
      }
      LOG((2, "PIOc_sync ierr = %d", ierr));
   }

   /* Broadcast and check the return code. */
   if ((mpierr = MPI_Bcast(&ierr, 1, MPI_INT, ios->ioroot, ios->my_comm)))
      return check_mpi2(ios, NULL, mpierr, __FILE__, __LINE__);
   if (ierr)
      return check_netcdf2(ios, NULL, ierr, __FILE__, __LINE__);

   return ierr;
}
