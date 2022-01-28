#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "netcdf.h"

#undef DEBUG

#ifdef DEBUG
static char* buf = NULL;
#endif

#define totalNumStrings 13

static const char* charPointers[totalNumStrings] = 
	{"banana", "kiwi", "apple", "grape", "apple", "cherry", "lemon", "melon", "watermelon", "strawberry", "peach", "raspberry", "pineapple"};
    
#define first_size 2
#define second_size 1
#define third_size 5
#define fourth_size 3
#define fifth_size 0
#define sixth_size 2

#define attlength 6

void
checkErrorCode(int status, const char* message)
{
    if (status != NC_NOERR){
	fprintf(stderr,"Error code: %d from %s\n",status,message);
        fprintf(stderr,"\t%s\n\n",nc_strerror(status));
    }
}


void
writeVariable(int dimlength, int ncid, nc_type vlen_typeID)
{
    int retval = NC_NOERR;
    int dimid;
    int varid;
    
    /* Setup data */
    nc_vlen_t* data = calloc(sizeof(nc_vlen_t),dimlength);

    /* create six variable length arrays of strings */
    int stringIndex = 0;

    data[0].len = first_size;
    data[0].p = charPointers+stringIndex;
    stringIndex += first_size;
    
    data[1].len = second_size;
    data[1].p = charPointers+stringIndex;
    stringIndex += second_size;
    
    data[2].len = third_size;
    data[2].p = charPointers+stringIndex;
    stringIndex += third_size;
    
    data[3].len = fourth_size;
    data[3].p = charPointers+stringIndex;
    stringIndex += fourth_size;
    
    data[4].len = fifth_size;
    data[4].p = charPointers+stringIndex;
    stringIndex += fifth_size;
    
    data[5].len = sixth_size;
    data[5].p = charPointers+stringIndex;
    stringIndex += sixth_size;
    
    /* Write vlen variable named Fruits */
    
    /* Define dimension */
    retval = nc_def_dim(ncid, "D1", dimlength, &dimid);
    checkErrorCode(retval, "nc_def_dim");
    
    /* Define variable */
    retval = nc_def_var(ncid, "Fruits", vlen_typeID, 1, &dimid, &varid);
    checkErrorCode(retval, "nc_def_var");
    
    /* Write variable */
    retval = nc_put_var(ncid, varid, data);
    checkErrorCode(retval, "nc_put_var");
    
    free(data);
}

void
writeAttribute(int len, int ncid, nc_type vlen_typeID)
{
    int retval = NC_NOERR;
    
    /* Setup data */
    nc_vlen_t* data = calloc(sizeof(nc_vlen_t),len);

    /* create six variable length arrays of strings */
    int stringIndex = 0;

    data[0].len = first_size;
    data[0].p = charPointers+stringIndex;
    stringIndex += first_size;
    
    data[1].len = second_size;
    data[1].p = charPointers+stringIndex;
    stringIndex += second_size;
    
    data[2].len = third_size;
    data[2].p = charPointers+stringIndex;
    stringIndex += third_size;
    
    data[3].len = fourth_size;
    data[3].p = charPointers+stringIndex;
    stringIndex += fourth_size;
    
    data[4].len = fifth_size;
    data[4].p = charPointers+stringIndex;
    stringIndex += fifth_size;
    
    data[5].len = sixth_size;
    data[5].p = charPointers+stringIndex;
    stringIndex += sixth_size;
        
#ifdef DEBUG
    if(buf) {free(buf); buf = NULL;}
    nc_dump_data(ncid,vlen_typeID,data,len,&buf);
    fprintf(stderr,">>> attribute = %s\n",buf);
#endif

    /* Write vlen attribute named Fruits */
    retval = nc_put_att(ncid, NC_GLOBAL, "Fruit", vlen_typeID, len, data);
    checkErrorCode(retval, "nc_put_att");
    
    free(data);
}

int
main(int argc, const char * argv[])
{
    /* Open file */
    int ncid;
    int retval;
    nc_type vlen_typeID;
    
    retval = nc_create("tst_vlenstr.nc", NC_NETCDF4, &ncid);
    checkErrorCode(retval, "nc_create");
    
    /* Define vlen type named MY_VLEN_STRING */
    retval = nc_def_vlen(ncid, "MY_VLEN_STRING", NC_STRING, &vlen_typeID);
    checkErrorCode(retval, "nc_def_vlen");
    
    /* write variable - will be able to read it back fine */
    writeVariable(attlength, ncid, vlen_typeID);
    
    /* write attribute - will read back garbled values */
    writeAttribute(attlength, ncid, vlen_typeID);
    
    retval = nc_close(ncid);
    checkErrorCode(retval, "nc_close");
    
#ifdef DEBUG
    if(buf) free(buf);
#endif
    return retval;
}
