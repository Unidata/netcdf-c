# Sources that are specific to Zarr V3 testing
set(V3FILES run_nan.sh run_mud.sh)
set(V3DATA ref_nczarr2zarr.cdl ref_purezarr.cdl ref_xarray.cdl ref_misc2.cdl ref_jsonconvention.cdl ref_jsonconvention.zmap ref_nulls_zarr.baseline ref_string_zarr.baseline ref_string_nczarr.baseline ref_zarr_test_data_2d.cdl.gz ref_groups_regular.cdl ref_filtered.cdl ref_any.cdl ref_multi.cdl ref_tst_nans.dmp ref_bzip2.cdl ref_tst_mud4-bc.cdl ref_tst_mud4.cdl ref_tst_mud4_chars.cdl)

# Shell scripts that are copies of same files from nczarr_test
SET(TESTFILES_NCZARR_SH test_nczarr.sh run_chunkcases.sh run_corrupt.sh run_external.sh run_fillonlyz.sh run_filter.sh run_filterinstall.sh run_filter_misc.sh run_filter_vlen.sh run_interop.sh run_jsonconvention.sh run_misc.sh run_nccopy5.sh run_nccopyz.sh run_ncgen4.sh run_nczarr_fill.sh run_nczfilter.sh run_newformat.sh run_notzarr.sh run_nulls.sh run_perf_chunks1.sh run_purezarr.sh run_quantize.sh run_scalar.sh run_specific_filters.sh run_strings.sh run_unknown.sh run_unlim_io.sh run_ut_map.sh run_ut_mapapi.sh run_ut_misc.sh)

# Program files
set(TESTFILES_NCZARR_C test_chunking.c test_filter_vlen.c test_h5_endians.c test_put_vars_two_unlim_dim.c test_quantize.c test_unlim_vars.c tst_pure_awssdk.cpp)

# Data files
set(TESTDATA_NCZARR ref_nulls_nczarr.baseline ref_zarr_test_data.cdl.gz ref_avail1.cdl ref_byte.cdl ref_byte_fill_value_null.cdl ref_fillonly.cdl ref_misc1.cdl ref_ndims.cdl ref_newformatpure.cdl ref_nulls.cdl ref_oldformat.cdl ref_perdimspecs.cdl ref_power_901_constants.cdl ref_purezarr_base.cdl ref_quotes.cdl ref_rem.cdl ref_scalar.cdl ref_skip.cdl ref_skipw.cdl ref_string.cdl ref_t_meta_dim1.cdl ref_t_meta_var1.cdl ref_ut_mapapi_create.cdl ref_ut_mapapi_data.cdl ref_ut_mapapi_meta.cdl ref_ut_map_create.cdl ref_ut_map_writedata.cdl ref_ut_map_writemeta.cdl ref_ut_map_writemeta2.cdl ref_ut_testmap_create.cdl ref_whole.cdl ref_avail1.dmp ref_misc1.dmp ref_ndims.dmp ref_rem.dmp ref_noshape.file.zip ref_groups.h5 ref_notzarr.tar.gz ref_avail1.txt ref_skip.txt ref_ut_json_build.txt ref_ut_json_parse.txt ref_ut_mapapi_search.txt ref_ut_map_readmeta.txt ref_ut_map_readmeta2.txt ref_ut_map_search.txt ref_ut_proj.txt ref_whole.txt ref_byte.zarr.zip ref_byte_fill_value_null.zarr.zip ref_oldformat.zip ref_power_901_constants_orig.zip ref_quotes_orig.zip)

macro(v3_setup)
# Add command to update the test source from nczarr_test
  foreach(u ${TESTFILES_NCZARR_C} ${TESTDATA_NCZARR})
    set(SRCPATH "${CMAKE_SOURCE_DIR}/nczarr_test/${u}")
    set(DSTPATH "${CMAKE_BINARY_DIR}/v3_nczarr_test/${u}")
    file(REMOVE ${DSTPATH})
    file(COPY ${SRCPATH} DESTINATION ${DSTPATH})
  endforeach()

  foreach(u ${TESTFILES_NCZARR_SH})
      set(SRCPATH "${CMAKE_SOURCE_DIR}/nczarr_test/${u}")
      set(DSTPATH "${CMAKE_BINARY_DIR}/v3_nczarr_test/${u}")
      file(REMOVE ${DSTPATH})
      file(COPY ${SRCPATH} DESTINATION ${DSTPATH})
  endforeach()
endmacro()
