   function NF90_1FUN`'(ncid, varid, values, start)
     integer,                         intent( in) :: ncid, varid
     TYPE, intent(IN_OR_OUT) :: values
     integer, dimension(:), optional, intent( in) :: start
     integer                                      :: NF90_1FUN
 
     integer, dimension(nf90_max_var_dims) :: localIndex
     integer                               :: counter
     integer                               :: defaultInteger
     
     ! Set local arguments to default values
     localIndex(:) = 1
     if(present(start)) localIndex(:size(start)) = start(:)
 
     NF90_1FUN = NF_1FUN`'(ncid, varid, localIndex, defaultInteger)
     values = defaultInteger
   end function NF90_1FUN
