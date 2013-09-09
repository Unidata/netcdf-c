## This file should be placed in the root directory of your project.
## Then modify the CMakeLists.txt file in the root directory of your
## project to incorporate the testing dashboard.
##
## # The following are required to submit to the CDash dashboard:
##   ENABLE_TESTING()
##   INCLUDE(CTest)

IF(USE_REMOTE_CDASH)
	set(CTEST_PROJECT_NAME "netcdf-c")
	set(CTEST_NIGHTLY_START_TIME "00:00:00 EST")
	set(CTEST_SITE "Unidata OSX")
	set(CTEST_DROP_METHOD "http")
	set(CTEST_DROP_SITE "my.cdash.org")
	set(CTEST_DROP_LOCATION "/submit.php?project=netcdf-c")
	set(CTEST_DROP_SITE_CDASH TRUE)
ELSEIF(BUILDING_TRUNK)
	SET (CTEST_DROP_METHOD "http")
	SET (CTEST_DROP_SITE "yakov/")
	SET (CTEST_DROP_LOCATION "CDash/submit.php?project=netcdf-NCF-266-dev")
	SET (CTEST_DROP_SITE_CDASH TRUE)
ELSEIF(WORKING_FROM_HOME)
	SET (CTEST_DROP_METHOD "http")
	SET (CTEST_DROP_SITE "localhost:4111/")
	SET (CTEST_DROP_LOCATION "CDash/submit.php?project=netcdf-NCF-266-dev")
	SET (CTEST_DROP_SITE_CDASH TRUE)
ELSE()
	SET (CTEST_PROJECT_NAME "netcdf-NCF-266-devmake")
	SET (CTEST_DROP_METHOD "http")
	SET (CTEST_DROP_SITE "yakov/")
	SET (CTEST_DROP_LOCATION "CDash/submit.php?project=netcdf-NCF-266-dev")
	SET (CTEST_DROP_SITE_CDASH TRUE)
ENDIF()
