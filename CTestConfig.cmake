# CTest/CDash configuration
IF(USE_REMOTE_CDASH)

	set(CTEST_PROJECT_NAME "NetCDF-C")
	set(CTEST_NIGHTLY_START_TIME "00:00:00 EST")
	
	set(CTEST_SITE "Unidata OSX")
	set(CTEST_DROP_METHOD "http")
	set(CTEST_DROP_SITE "my.cdash.org")
	set(CTEST_DROP_LOCATION "/submit.php?project=NetCDF-C")
	set(CTEST_DROP_SITE_CDASH TRUE)
ELSE()
	SET (CTEST_DROP_METHOD "http")
	SET (CTEST_DROP_SITE "yakov.unidata.ucar.edu/")
	SET (CTEST_DROP_LOCATION "CDash/submit.php?project=netCDF")
	SET (CTEST_DROP_SITE_CDASH TRUE)
ENDIF()
