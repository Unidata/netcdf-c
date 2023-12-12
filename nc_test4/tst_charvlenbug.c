#define AFIRST

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "netcdf.h"

#define AFIRST

#undef DEBUG

#define FILE "tst_charvlenbug.nc"

void checkErrorCode(int status, const char* message){
    if (status != NC_NOERR){
        fprintf(stderr,"Error code: %d: %s\n",status,message);
    }
}

int
main(int argc, const char * argv[])
{
    int ncid;
    int retval;
    int dimid_a;
    int dimid_b;
    int dimids[2];
    int varid;
    int typeid;
    int ndims, dimids_read[NC_MAX_VAR_DIMS];
    size_t dimlen;
    size_t num_items;
    ptrdiff_t stride[2] = {1, 1};
    nc_vlen_t vlenPointers[4];
    nc_vlen_t* readVlenPointers;
    int i;
    
#ifdef AFIRST
    size_t start[2] = {1, 0};
    size_t count[2] = {2, 2};
#else
    size_t start[2] = {0, 1};
    size_t count[2] = {2, 2};
#endif

    // set up data
     
    vlenPointers[0].len = 1; vlenPointers[0].p = "a";
    vlenPointers[1].len = 2; vlenPointers[1].p = "aa";
    vlenPointers[2].len = 1; vlenPointers[2].p = "b";
    vlenPointers[3].len = 2; vlenPointers[3].p = "bb";

    // -- WRITE --
    
    retval = nc_create(FILE, NC_NETCDF4, &ncid);
    checkErrorCode(retval, "nc_create");
    
    // Define dimensions
    retval = nc_def_dim(ncid, "a", NC_UNLIMITED, &dimid_a);
    checkErrorCode(retval, "nc_def_dim for 'a'");
    
    retval = nc_def_dim(ncid, "b", 4, &dimid_b);
    checkErrorCode(retval, "nc_def_dim for 'b'");

    /* Define VLEN type */
    retval = nc_def_vlen(ncid,"str",NC_CHAR,&typeid);
    checkErrorCode(retval, "nc_def_vlen");
    
    // Define variable
#ifdef AFIRST
    dimids[0] = dimid_a;
    dimids[1] = dimid_b;
#else
    dimids[0] = dimid_b;
    dimids[1] = dimid_a;
#endif
    retval = nc_def_var(ncid, "var", typeid, 2, dimids, &varid);
    checkErrorCode(retval, "nc_def_var");
    
    // Put variable

    retval = nc_put_vars(ncid, varid, start, count, stride, vlenPointers);
//    retval = nc_put_vara_string(ncid, varid, start, count, vlenPointers);
    checkErrorCode(retval, "nc_put_vars_string");

    retval = nc_close(ncid);
    checkErrorCode(retval, "nc_close(1)");
    
    // -- READ --
    retval = nc_open(FILE, NC_NOWRITE, &ncid);
    checkErrorCode(retval, "nc_open");
    
    // get dimensions
    retval = nc_inq_var(ncid, varid, NULL, NULL, &ndims, dimids_read, NULL);
    checkErrorCode(retval, "nc_inq_var");

    // calculate num elements to read
    dimlen = 0;
    num_items = 1;
    for (i = 0; i < ndims; i++) {
        retval = nc_inq_dimlen(ncid, dimids_read[i], &dimlen);
        checkErrorCode(retval, "nc_inq_dimlen");
        num_items *= dimlen;
    }
    
    // get var
    readVlenPointers = (nc_vlen_t*)malloc(sizeof(nc_vlen_t)*num_items);
    retval = nc_get_var(ncid, varid, readVlenPointers);
    checkErrorCode(retval, "nc_get_var");

#ifdef DEBUG
    for(i=0;i<num_items;i++) {
	int j;
	char* s;
	nc_vlen_t* v = &readVlenPointers[i];
	fprintf(stderr,"readVlenPointers[%d] = ",i);
	fprintf(stderr,"(%d,%p)",(int)v->len,v->p);
	fprintf(stderr," \"");
	s = (char*)v->p;
	for(j=0;j<v->len;j++) fprintf(stderr,"%c",s[j]);
	fprintf(stderr,"\"\n");
    }
#endif

    if((retval = nc_reclaim_data(ncid,typeid,readVlenPointers,num_items))) goto done;
    free(readVlenPointers);

    retval = nc_close(ncid);
    checkErrorCode(retval, "nc_close(2)");

done:
    return retval;
}
