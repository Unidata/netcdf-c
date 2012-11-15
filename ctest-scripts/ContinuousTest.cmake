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
set (CTEST_DASHBOARD_ROOT "${CTEST_SCRIPT_DIRECTORY}/Dashboards")
SET (CTEST_SOURCE_DIRECTORY "${CTEST_DASHBOARD_ROOT}/netcdf-src")
SET (CTEST_BINARY_DIRECTORY "${CTEST_DASHBOARD_ROOT}/builds/build-cont")

#####
# End Test/System Setup
#####

#set (CTEST_CONTINUOUS_DURATION 300)
#set (CTEST_CONTINUOUS_MINIMUM_INTERVAL 10)

set (CTEST_CMAKE_GENERATOR "Unix Makefiles")
set (CTEST_SVN_COMMAND "/opt/local/bin/svn")
set (CTEST_SVN_CHECKOUT "\"${CTEST_SVN_COMMAND} co https://sub.unidata.ucar.edu/netcdf/branches/netcdf-cmake ${CTEST_SOURCE_DIRECTORY}")
set (CTEST_COMMAND "\"${CTEST_EXECUTABLE_NAME} -D Continuous -A \"${CTEST_SCRIPT_DIRECTORY}/${CTEST_SCRIPT_NAME}\"")

set (CTEST_CHECKOUT_COMMAND "svn co https://sub.unidata.ucar.edu/netcdf/branches/netcdf-cmake ${CTEST_SOURCE_DIRECTORY}")
set (CTEST_UPDATE_COMMAND "svn")
set (CTEST_START_WITH_EMPTY_BINARY_DIRECTORY TRUE)

## Kick off the test
SET (CTEST_START_WITH_EMPTY_BINARY_DIRECTORY_ONCE 1)
ctest_start("Continuous")

while (${CTEST_ELAPSED_TIME} LESS 36000)
	set (START_TIME ${CTEST_ELAPSED_TIME})

	ctest_update(RETURN_VALUE count)
	message("Count: ${count}")
	if (count GREATER 0)
		message("Count ${count} > 0, running analysis.")
		ctest_configure()
		message("Configuring")
		ctest_build()
		message("Building")
		ctest_test()
		message("Testing")
		ctest_submit()
		message("Submitting")
		message("Analysis complete.")
	endif()
	ctest_sleep( ${START_TIME} 60 ${CTEST_ELAPSED_TIME})
endwhile()
