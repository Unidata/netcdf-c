if(DEFINED ENV{LIB_FUZZING_ENGINE})
  add_executable(fuzz_open fuzz_open.c)
  target_link_libraries(fuzz_open netcdf $ENV{LIB_FUZZING_ENGINE})
endif()
