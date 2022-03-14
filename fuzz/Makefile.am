# This is part of the netCDF package.
# Copyright 2022 University Corporation for Atmospheric Research/Unidata
# See COPYRIGHT file for conditions of use.
#
# @authors Ward Fisher

# Set AM_CPPFLAGS and AM_LDFLAGS based on user choices.
include $(top_srcdir)/lib_flags.am

#if(DEFINED ENV{LIB_FUZZING_ENGINE})
#  add_executable(fuzz_open fuzz_open.c)
#  target_link_libraries(fuzz_open netcdf $ENV{LIB_FUZZING_ENGINE})
#endif()

EXTRA_DIST = fuzz.c fuzz_open.c