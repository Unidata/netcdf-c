/* This is part of the netCDF package.
   Copyright 2018 University Corporation for Atmospheric Research/Unidata
   See COPYRIGHT file for conditions of use.
    
   Test that netCDF provides proper error messages if broken Files are supplied.
*/

#include <config.h>
#include <stdio.h>
#include <nc_tests.h>
#include "err_macros.h"
#include "netcdf.h"
#include "netcdf_mem.h"

#include <string.h>

#define FILE_NAME "tst_broken_files.nc"
#define TRUNCATED_FILE_CONTENT "\x89HDF\r\n\x1a\n"

int
main() {
    printf("\n*** Testing NetCDF-4 with truncated (broken) sample file.\n");
    {
        printf("*** testing via file on file-system ...\n");
#if defined _WIN32 || defined __MINGW32__
        FILE *fp = fopen(FILE_NAME, "wb");
#else
	        FILE *fp = fopen(FILE_NAME, "w");
#endif
        if(!fp) ERR;
        if(fwrite(TRUNCATED_FILE_CONTENT, sizeof(char), sizeof(TRUNCATED_FILE_CONTENT), fp) != sizeof(TRUNCATED_FILE_CONTENT)) ERR;
        fclose(fp);


        int  ncid, stat;
        stat = nc_open(FILE_NAME, 0, &ncid);
        if (stat != NC_EHDFERR && stat != NC_ENOFILTER && stat != NC_ENOTNC) ERR;

    }

    {
        printf("*** testing via in-memory access ...\n");
        int  ncid;
        if (nc_open_mem(FILE_NAME, 0, sizeof(TRUNCATED_FILE_CONTENT), TRUNCATED_FILE_CONTENT, &ncid) != NC_EHDFERR) ERR;
    }
    SUMMARIZE_ERR;
    FINAL_RESULTS;
}
