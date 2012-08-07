SET(CMAKE_INCLUDE_CURRENT_DIR ON)

INCLUDE_DIRECTORIES(".")
# Some extra tests
SET(EXTRA_TSTS4 tst_dims tst_dims2 tst_dims3 tst_files tst_files4 tst_vars tst_varms tst_unlim_vars tst_converts tst_converts2 tst_grps tst_grps2
tst_compounds tst_compounds2 tst_compounds3 tst_opaques tst_strings
tst_strings2 tst_interops tst_interops4 tst_interops5 tst_interops6
tst_enums tst_coords tst_coords2 tst_coords3 tst_vars3 tst_vars4
tst_chunks tst_chunks2 tst_utf8 tst_fills tst_fills2 tst_fillbug
tst_xplatform tst_xplatform2 tst_h_atts2 tst_endian_fill 
t_type cdm_sea_soundings tst_camrun tst_vl 
tst_vars2 tst_files5 tst_files6 tst_sync tst_h_strbug)

# Copy some test files from current source dir to out-of-tree build dir.
FILE(GLOB COPY_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.nc)
FILE(COPY ${COPY_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)

FOREACH(CTEST ${EXTRA_TSTS4})
  ADD_EXECUTABLE(${CTEST} ${CTEST}.c)
  TARGET_LINK_LIBRARIES(${CTEST} netcdf)
  ADD_TEST(${CTEST} ${EXECUTABLE_OUTPUT_PATH}/${CTEST})
ENDFOREACH()






