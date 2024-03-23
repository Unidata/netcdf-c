cmake_minimum_required(VERSION 3.11)
project(NetCDFExample)

include(FetchContent)

# Fetch NetCDF
FetchContent_Declare(
    netcdf
    GIT_REPOSITORY https://github.com/Unidata/netcdf-c
    GIT_TAG main
)

set(ENABLE_TESTS OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(netcdf)

# Add executable
add_executable(netcdf_example main.c)

# Link against NetCDF
target_link_libraries(netcdf_example PUBLIC netcdf)

# Add test
enable_testing()
add_test(NAME netcdf_example_test COMMAND netcdf_example)
