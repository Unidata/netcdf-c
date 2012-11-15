#####
# Some Test/System Setup
#####
set (CTEST_PROJECT_NAME "netCDF-C")

# Get Hostname
find_program(HOSTNAME_CMD NAMES hostname)
exec_program(${HOSTNAME_CMD} ARGS "-f" OUTPUT_VARIABLE HOSTNAME)
set(CTEST_SITE "${HOSTNAME}")

# Get system configuration
find_program(UNAME NAMES uname)
macro(getuname name flag)
	exec_program("${UNAME}" ARGS "${flag}" OUTPUT_VARIABLE "${name}")
endmacro(getuname)
getuname(osname -s)
getuname(osrel  -r)
getuname(cpu    -m)
set(CTEST_BUILD_NAME        "${osname}-${osrel}-${cpu}")


# Set locations of src/build
set (CTEST_DASHBOARD_ROOT /Users/wfisher/Desktop/netcdf-cmake/Dashboards)
SET (CTEST_SOURCE_DIRECTORY "${CTEST_SCRIPT_DIRECTORY}/src-snapshot")
SET (CTEST_BINARY_DIRECTORY "${CTEST_DASHBOARD_ROOT}/build-cont")

#####
# End Test/System Setup
#####

set (CTEST_CONTINUOUS_DURATION 300)
set (CTEST_CONTINUOUS_MINIMUM_INTERVAL 10)

set (CTEST_CMAKE_GENERATOR "Unix Makefiles")
set (CTEST_SVN_COMMAND "/opt/local/bin/svn")
set (CTEST_SVN_CHECKOUT "\"${CTEST_SVN_COMMAND} co https://sub.unidata.ucar.edu/netcdf/branches/netcdf-cmake ${CTEST_SOURCE_DIRECTORY}")
set (CTEST_COMMAND "\"${CTEST_EXECUTABLE_NAME} -D Continuous -A \"${CTEST_SCRIPT_DIRECTORY}/${CTEST_SCRIPT_NAME}\"")

set (CTEST_CHECKOUT_COMMAND "svn co https://sub.unidata.ucar.edu/netcdf/branches/netcdf-cmake ${CTEST_SOURCE_DIRECTORY}")
set (CTEST_UPDATE_COMMAND "svn")
set (CTEST_START_WITH_EMPTY_BINARY_DIRECTORY TRUE)

## Kick off the test
ctest_empty_binary_directory(${CTEST_BINARY_DIRECTORY})


while (${CTEST_ELAPSED_TIME} LESS 36000)
	set (START_TIME ${CTEST_ELAPSED_TIME})
	ctest_start("Continuous")
	ctest_update(RETURN_VALUE count)
	message("Count: ${count}")
	if (count GREATER 0)
		message("Count ${count} > 0, running analysis.")
		ctest_configure()
		ctest_build()
		ctest_test()
		ctest_submit()
	endif()
	ctest_sleep( ${START_TIME} 60 ${CTEST_ELAPSED_TIME})
endwhile()
