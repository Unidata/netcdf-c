!     This is part of the netCDF package.
!     Copyright 2006 University Corporation for Atmospheric Research/Unidata.
!     See COPYRIGHT file for conditions of use.

!     This program tests netCDF-4 fill values.

!     $Id: f90tst_parallel_fill.f90,v 1.1 2009/12/10 16:44:51 ed Exp $

program f90tst_parallel_fill
  use typeSizes
  use netcdf
  implicit none
  include 'mpif.h'
  
  ! This is the name of the data file we will create.
  character (len = *), parameter :: FILE_NAME = "f90tst_parallel_fill.nc"
  integer, parameter :: MAX_DIMS = 2
  integer, parameter :: NX = 16, NY = 16
  integer, parameter :: HALF_NX = NX / 2, HALF_NY = NY / 2
  integer, parameter :: QUARTER_NX = NX / 4, QUARTER_NY = NY / 4
  integer, parameter :: NUM_VARS = 8
  character (len = *), parameter :: var_name(NUM_VARS) = &
       (/ 'byte', 'short', 'int', 'float', 'double', 'ubyte', 'ushort', 'uint' /)
  integer :: ncid, varid(NUM_VARS), dimids(MAX_DIMS)
  integer :: var_type(NUM_VARS) = (/ nf90_byte, nf90_short, nf90_int, &
       nf90_float, nf90_double, nf90_ubyte, nf90_ushort, nf90_uint /)
  integer :: x_dimid, y_dimid
  integer :: byte_out(QUARTER_NY, QUARTER_NX), byte_in(NY, NX)
  integer :: short_out(QUARTER_NY, QUARTER_NX), short_in(NY, NX)
  integer :: int_out(QUARTER_NY, QUARTER_NX), int_in(NY, NX)
  real :: areal_out(QUARTER_NY, QUARTER_NX), areal_in(NY, NX)
  real :: double_out(QUARTER_NY, QUARTER_NX), double_in(NY, NX)
  integer :: ubyte_out(QUARTER_NY, QUARTER_NX), ubyte_in(NY, NX)
  integer :: ushort_out(QUARTER_NY, QUARTER_NX), ushort_in(NY, NX)
  integer (kind = EightByteInt) :: uint_out(QUARTER_NY, QUARTER_NX), uint_in(NY, NX)
  integer :: nvars, ngatts, ndims, unlimdimid, file_format
  integer :: x, y, v
  integer :: start_out(MAX_DIMS), count_out(MAX_DIMS)
  integer :: start_in(MAX_DIMS), count_in(MAX_DIMS)
  integer :: p, my_rank, ierr

  call MPI_Init(ierr)
  call MPI_Comm_rank(MPI_COMM_WORLD, my_rank, ierr)
  call MPI_Comm_size(MPI_COMM_WORLD, p, ierr)

  if (my_rank .eq. 0) then
     print *,
     print *, '*** Testing fill values with unlimited dimension and parallel I/O.'
  endif

  ! There must be 4 procs for this test.
  if (p .ne. 4) then
     print *, 'Sorry, this test program must be run on four processors.'
     stop 1
  endif

  ! Create some pretend data.
  do x = 1, QUARTER_NX
     do y = 1, QUARTER_NY
        byte_out(y, x) = -1
        short_out(y, x) =  -2
        int_out(y, x) = -4
        areal_out(y, x) = 2.5
        double_out(y, x) = -4.5
        ubyte_out(y, x) = 1
        ushort_out(y, x) = 2
        uint_out(y, x) = 4
     end do
  end do

  ! Create the netCDF file. 
  call check(nf90_create(FILE_NAME, nf90_netcdf4, ncid, comm = MPI_COMM_WORLD, &
       info = MPI_INFO_NULL))

  ! Define the dimensions.
  call check(nf90_def_dim(ncid, "x", NX, x_dimid))
  call check(nf90_def_dim(ncid, "y", NY, y_dimid))
  dimids =  (/ y_dimid, x_dimid /)

  ! Define the variables. 
  do v = 1, NUM_VARS
     call check(nf90_def_var(ncid, var_name(v), var_type(v), dimids, varid(v)))
     call check(nf90_var_par_access(ncid, varid(v), nf90_collective))     
  end do

  ! This will be the last collective operation.
  call check(nf90_enddef(ncid))

  ! Determine what part of the variable will be written/read for this
  ! processor. It's a checkerboard decomposition. Only the third
  ! quadrant of data will be written, so each processor writes a
  ! quarter of the quadrant, or 1/16th of the total array. (And
  ! processor 0 doesn't write anyway.)
  count_out = (/ QUARTER_NX, QUARTER_NY /)
  if (my_rank .eq. 0) then 
     start_out = (/ HALF_NX + 1, HALF_NY + 1 /)
  else if (my_rank .eq. 1) then
     start_out = (/ HALF_NX + 1, HALF_NY + 1 + QUARTER_NY /)
  else if (my_rank .eq. 2) then
     start_out = (/ HALF_NX + 1 + QUARTER_NX, HALF_NY + 1 /)
  else if (my_rank .eq. 3) then
     start_out = (/ HALF_NX + 1 + QUARTER_NX, HALF_NY + 1 + QUARTER_NY /)
  endif
  print *, my_rank, start_out, count_out

  ! Write this processor's data, except for processor zero.
  if (my_rank .ne. 0) then
     call check(nf90_put_var(ncid, varid(1), byte_out, start = start_out, count = count_out))
!      call check(nf90_put_var(ncid, varid(2), short_out, start = start_out, count = count_out))
!      call check(nf90_put_var(ncid, varid(3), int_out, start = start_out, count = count_out))
!      call check(nf90_put_var(ncid, varid(4), areal_out, start = start_out, count = count_out))
!      call check(nf90_put_var(ncid, varid(5), double_out, start = start_out, count = count_out))
!      call check(nf90_put_var(ncid, varid(6), ubyte_out, start = start_out, count = count_out))
!      call check(nf90_put_var(ncid, varid(7), ushort_out, start = start_out, count = count_out))
!      call check(nf90_put_var(ncid, varid(8), uint_out, start = start_out, count = count_out))
  endif

  ! Close the file. 
  call check(nf90_close(ncid))

  ! Reopen the file.
!   call check(nf90_open(FILE_NAME, nf90_nowrite, ncid, comm = MPI_COMM_WORLD, &
!        info = MPI_INFO_NULL))
  
!   ! Check some stuff out.
!   call check(nf90_inquire(ncid, ndims, nvars, ngatts, unlimdimid, file_format))
!   if (ndims /= 2 .or. nvars /= NUM_VARS .or. ngatts /= 0 .or. unlimdimid /= 1 .or. &
!        file_format /= nf90_format_netcdf4) stop 2

!   ! Now each processor will read one quarter of the whole array.
!   count_in = (/ HALF_NX, HALF_NY /)
!   if (my_rank .eq. 0) then 
!      start_in = (/ 1, 1 /)
!   else if (my_rank .eq. 1) then
!      start_in = (/ HALF_NX + 1, 1 /)
!   else if (my_rank .eq. 2) then
!      start_in = (/ 1, HALF_NY + 1 /)
!   else if (my_rank .eq. 3) then
!      start_in = (/ HALF_NX + 1, HALF_NY + 1 /)
!   endif

!   ! Read this processor's data.
!   call check(nf90_get_var(ncid, varid(1), byte_in, start = start_in, count = count_in))
!   call check(nf90_get_var(ncid, varid(2), short_in, start = start_in, count = count_in))
!   call check(nf90_get_var(ncid, varid(3), int_in, start = start_in, count = count_in))
!   call check(nf90_get_var(ncid, varid(4), areal_in, start = start_in, count = count_in))
!   call check(nf90_get_var(ncid, varid(5), double_in, start = start_in, count = count_in))
!   call check(nf90_get_var(ncid, varid(6), ubyte_in, start = start_in, count = count_in))
!   call check(nf90_get_var(ncid, varid(7), ushort_in, start = start_in, count = count_in))
!   call check(nf90_get_var(ncid, varid(8), uint_in, start = start_in, count = count_in))

!   ! Check the data. All the data from the processor zero are fill
!   ! value.
!   do x = 1, NX
!      do y = 1, NY
!         if (my_rank .eq. 0) then
!            if (byte_in(y, x) .ne. -1) stop 13
!            if (short_in(y, x) .ne. -2) stop 14
!            if (int_in(y, x) .ne. -4) stop 15
!            if (areal_in(y, x) .ne. 2.5) stop 16
!            if (double_in(y, x) .ne. -4.5) stop 17
!            if (ubyte_in(y, x) .ne. 1) stop 18
!            if (ushort_in(y, x) .ne. 2) stop 19
!            if (uint_in(y, x) .ne. 4) stop 20
!         else 
!            if (byte_in(y, x) .ne. nf90_fill_byte) stop 3
!            if (short_in(y, x) .ne. nf90_fill_short) stop 4
!            if (int_in(y, x) .ne. nf90_fill_int) stop 5
!            if (areal_in(y, x) .ne. nf90_fill_real) stop 6
!            if (double_in(y, x) .ne. nf90_fill_double) stop 7
!            if (ubyte_in(y, x) .ne. nf90_fill_ubyte) stop 8
!            if (ushort_in(y, x) .ne. nf90_fill_ushort) stop 9
!            if (uint_in(y, x) .ne. nf90_fill_uint) stop 10
!         endif
!      end do
!   end do

!   ! Close the file. 
!   call check(nf90_close(ncid))

  call MPI_Finalize(ierr)

  if (my_rank .eq. 0) print *,'*** SUCCESS!'

contains
!     This subroutine handles errors by printing an error message and
!     exiting with a non-zero status.
  subroutine check(errcode)
    use netcdf
    implicit none
    integer, intent(in) :: errcode
    
    if(errcode /= nf90_noerr) then
       print *, 'Error: ', trim(nf90_strerror(errcode))
       stop 99
    endif
  end subroutine check
end program f90tst_parallel_fill

