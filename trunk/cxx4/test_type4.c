#include <stdio.h>
#include <stdlib.h>
#include <netcdf.h>

#define MEMSIZE3 10000

typedef struct compoundType_3 {
    double member3[MEMSIZE3];
} compoundType_3;


void
check_err(const int stat, const int line, const char *file) {
    if (stat != NC_NOERR) {
        (void)fprintf(stderr,"line %d of %s: %s\n", line, file, nc_strerror(stat));
        fflush(stderr);
        exit(1);
    }
}

int
main() {/* create aa.nc */

    int  stat;  /* return status */
    int  ncid;  /* netCDF id */

    /* group ids */
    int root_grp;

    /* type ids */
    int compoundType_3_typ;

    /* dimension ids */
    int dim3_dim;

    /* dimension lengths */
    size_t dim3_len = 2;

    /* variable ids */
    int var_3_id;

    /* rank (number of dimensions) for each variable */
#   define RANK_var_3 1

    /* variable shapes */
    int var_3_dims[RANK_var_3];

    /* enter define mode */
    stat = nc_create("firstFile.cdf", NC_CLOBBER|NC_NETCDF4, &ncid);
    check_err(stat,__LINE__,__FILE__);
    root_grp = ncid;

    stat = nc_def_compound(root_grp, sizeof(compoundType_3), "compoundType_3", &compoundType_3_typ);
    check_err(stat,__LINE__,__FILE__);
    {
    static int member3_dims[1] = {MEMSIZE3};
    stat = nc_insert_array_compound(root_grp, compoundType_3_typ, "member3", NC_COMPOUND_OFFSET(compoundType_3,member3), NC_DOUBLE, 1, member3_dims);
    check_err(stat,__LINE__,__FILE__);
    }


    /* define dimensions */
    stat = nc_def_dim(root_grp, "dim3", dim3_len, &dim3_dim);
    check_err(stat,__LINE__,__FILE__);

    /* define variables */

    var_3_dims[0] = dim3_dim;
    stat = nc_def_var(root_grp, "var_3", compoundType_3_typ, RANK_var_3, var_3_dims, &var_3_id);
    check_err(stat,__LINE__,__FILE__);

    /* assign per-variable attributes */
    { /* _FillValue */
      static compoundType_3 var_3__FillValue_att[1];
      int j=0;
      for (j; j < MEMSIZE3; j++) var_3__FillValue_att[0].member3[j]=0;
      
      stat = nc_put_att(root_grp, var_3_id, "_FillValue", compoundType_3_typ, 1, var_3__FillValue_att);
      check_err(stat,__LINE__,__FILE__);
    }


    /* leave define mode */
    stat = nc_enddef (root_grp);
    check_err(stat,__LINE__,__FILE__);

    /* assign variable data */
    {
      compoundType_3 var_3_data[1];
      int j=0;
      for(j; j < MEMSIZE3; j++) var_3_data[0].member3[j]=j*j+1;
    
      size_t var_3_startset[1] = {0} ;
      size_t var_3_countset[1] = {1} ;
      stat = nc_put_vara(root_grp, var_3_id, var_3_startset, var_3_countset, var_3_data);
      check_err(stat,__LINE__,__FILE__);
    }

    stat = nc_close(root_grp);
    check_err(stat,__LINE__,__FILE__);
    return 0;
}
