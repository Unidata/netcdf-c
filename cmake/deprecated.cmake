#######
# Check for deprecated arguments, provide warning, and set the
# corresponding option.
#
# This file is being added in support of https://github.com/Unidata/netcdf-c/pull/2895 and may eventually
# no longer be necessary and removed in the future. 
# 
#######



function(check_depr_opt arg)

    if(DEFINED ${arg})

        set(val ${${arg}})
        message(WARNING "${arg} is deprecated and will be removed. Please use NETCDF_${arg} in the future")
        set(NETCDF_${arg} ${val} PARENT_SCOPE)
        set(DEPR_OPT "${DEPR_OPT}\n\to ${arg} --> NETCDF_${arg}" PARENT_SCOPE)
    endif() 

endfunction()

message(STATUS "Checking for Deprecated Options")
list(APPEND opts BUILD_UTILITIES ENABLE_BENCHMARKS ENABLE_BYTERANGE ENABLE_CDF5 ENABLE_CONVERSION_WARNINGS)
list(APPEND opts ENABLE_DAP ENABLE_DAP2 ENABLE_DAP4 ENABLE_DISKLESS ENABLE_DOXYGEN ENABLE_ERANGE_FILL)
list(APPEND opts ENABLE_EXAMPLES ENABLE_EXAMPLES_TESTS ENABLE_EXTREME_NUMBERS ENABLE_FFIO ENABLE_FILTER_BLOSC)
list(APPEND opts ENABLE_FILTER_BZ2 ENABLE_FILTER_SZIP ENABLE_FILTER_TESTING ENABLE_FILTER_ZSTD ENABLE_FSYNC)
list(APPEND opts ENABLE_HDF4 ENABLE_HDF5 ENABLE_LARGE_FILE_SUPPORT ENABLE_LARGE_FILE_TESTS ENABLE_LIBXML2)
list(APPEND opts ENABLE_LOGGING ENABLE_METADATA_PERF_TESTS ENABLE_MMAP ENABLE_NCZARR ENABLE_NCZARR_FILTERS)
list(APPEND opts ENABLE_NCZARR_S3 ENABLE_NCZARR_ZIP ENABLE_NETCDF_4 ENABLE_PARALLEL4 ENABLE_PARALLEL_TESTS)
list(APPEND opts ENABLE_PLUGINS ENABLE_PNETCDF ENABLE_QUANTIZE ENABLE_REMOTE_FUNCTIONALITY ENABLE_S3 ENABLE_S3_AWS)
list(APPEND opts ENABLE_S3_INTERNAL ENABLE_STDIO ENABLE_STRICT_NULL_BYTE_HEADER_PADDING ENABLE_TESTS ENABLE_UNIT_TESTS)
list(APPEND opts FIND_SHARED_LIBS LIB_NAME ENABLE_HDF4_FILE_TESTS)

foreach(opt ${opts})
    #MESSAGE(STATUS "Option: ${opt}")
    check_depr_opt(${opt})
endforeach()
