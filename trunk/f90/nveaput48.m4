   function NF90_AFUN`'(ncid, varid, values, start, count, stride, map)
     integer,                         intent( in) :: ncid, varid
     TYPE, dimension(COLONS), &
                                      intent(IN_OR_OUT) :: values
     integer, dimension(:), optional, intent( in) :: start, count, stride, map
     integer                                      :: NF90_AFUN
 
     integer, dimension(nf90_max_var_dims) :: localStart, localCount, localStride, localMap
     integer                               :: numDims, counter
 
     ! Set local arguments to default values
     numDims                 = size(shape(values))
     localStart (:         ) = 1
     localCount (:numDims  ) = shape(values)
     localCount (numDims+1:) = 1
     localStride(:         ) = 1
     localMap   (:numDims  ) = (/ 1, (product(localCount(:counter)), counter = 1, numDims - 1) /)
 
     if(present(start))  localStart (:size(start) )  = start(:)
     if(present(count))  localCount (:size(count) )  = count(:)
     if(present(stride)) localStride(:size(stride)) = stride(:)
     if(present(map))  then
       localMap   (:size(map))    = map(:)
       NF90_AFUN = &
          NF_MFUN`'(ncid, varid, localStart, localCount, localStride, localMap, int(values))
     else if(present(stride)) then
       NF90_AFUN = &
          NF_SFUN`'(ncid, varid, localStart, localCount, localStride, int(values))
     else
       NF90_AFUN = &
          NF_AFUN`'(ncid, varid, localStart, localCount, int(values))
     end if
   end function NF90_AFUN
