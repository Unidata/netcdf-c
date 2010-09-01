program testEightByteGet
  use typeSizes
  use netcdf
  implicit none
  
  ! netcdf related variables
  integer :: ncFileID, dimId, varId
             
  ! Local variables
  integer, parameter :: numLats = 10
  character (len = *), parameter :: fileName = "eightByteIntExample.nc"
  integer            :: counter
  !integer(kind = FourByteInt), dimension(numLats) :: lats  
  integer(kind = EightByteInt), dimension(numLats) :: lats  

! -----  
  call check(nf90_create(path = trim(fileName), cmode = nf90_clobber, ncid = ncFileID))
  
  ! Define the dimensions
  call check(nf90_def_dim(ncid = ncFileID, name = "lat", len = numLats, dimid = dimID))
  ! Define the variable
  call check(nf90_def_var(ncFileID, "lat", nf90_int, dimids = dimID, varID = varID) )
  ! Leave define mode
  call check(nf90_enddef(ncfileID))
  ! Write the variable
  lats(:) = (/ (counter, counter = 1, numLats) /)
  print *, lats
  call check(nf90_put_var(ncFileID, varId, lats ))
  call check(nf90_close(ncFileID))

  ! Now read it back in... 
  lats(:) = 0
  call check(nf90_open(path = trim(fileName), mode = nf90_nowrite, ncid = ncFileID))
  call check(nf90_inq_varid(ncFileId, "lat", varId))
  call check(nf90_get_var(ncFileID, varId, lats))
  print *, lats
  call check(nf90_close(ncFileID))  
  
contains
  ! Internal subroutine - checks error status after each netcdf, prints out text message each time
  !   an error code is returned. 
  subroutine check(status)
    integer, intent ( in) :: status
    
    if(status /= nf90_noerr) then 
      print *, trim(nf90_strerror(status))
    end if
  end subroutine check  
end program testEightByteGet
