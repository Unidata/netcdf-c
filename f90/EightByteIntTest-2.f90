program EightBytIntTest
   use typeSizes
   implicit none

   integer (kind = EightByteInt) :: Eight
   integer                       :: defaultInt

   print *, "Kind parameter for EightByteInt is", EightByteInt
   print *, "Bit size of eight byte int is",      bit_size(Eight)
contains
   subroutine EightByteIntProcedure(input)
     integer (kind = EightByteInt), intent( in) :: input

     print *, int(input)
   end subroutine EightByteIntProcedure
end program EightBytIntTest
