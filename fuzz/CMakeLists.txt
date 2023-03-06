# This is part of the netCDF package.
# Copyright 2022 University Corporation for Atmospheric Research/Unidata
# See COPYRIGHT file for conditions of use.
#
# Added for fuzzing in https://github.com/Unidata/netcdf-c/pull/1972
#
# @authors Ward Fisher

if(DEFINED ENV{LIB_FUZZING_ENGINE})
  add_executable(fuzz_open fuzz_open.c)
  target_link_libraries(fuzz_open netcdf $ENV{LIB_FUZZING_ENGINE})
endif()
