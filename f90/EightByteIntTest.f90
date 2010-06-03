program EightBytIntTest
   use typeSizes
   implicit none

   integer (kind = EightByteInt) :: Eight
   integer                       :: defaultInt

   print *, "Bit size of default int is",         bit_size(defaultInt)
   print *, "Kind parameter for EightByteInt is", EightByteInt
   print *, "Bit size of eight byte int is",      bit_size(Eight)
end program EightBytIntTest
