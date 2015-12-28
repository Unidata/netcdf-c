#include <stdio.h>
#include <stdlib.h>
#include <netcdf.h>

#define RANK_y3 1
#define RANK_y3d 1
#define FILENAME "cdf5_test.nc"

void
check_err(const int stat, const int line, const char *file) {
    if (stat != NC_NOERR) {
      (void)fprintf(stderr,"[err: %d] line %d of %s: %s\n", stat, line, file, nc_strerror(stat));
        fflush(stderr);
        exit(1);
    }
}

int
main() {/* create cdf5_test */

    int  stat;  /* return status */
    int  ncid;  /* netCDF id */

    /* dimension ids */
    int D3_dim;

    /* dimension lengths */
    size_t D3_len = 3;

    /* variable ids */
    int y3_id;
    int y3d_id;
    int i = 0;
    int y3_dims[RANK_y3];
    int y3d_dims[RANK_y3d];

    /* Lets do a bit of other
       testing here too. */
    {
      int xx = 0;
      signed char *tp = (signed char*)malloc(sizeof(signed char)*3);
      signed char *tph = tp;
      unsigned char *xp = (unsigned char *)malloc(sizeof(unsigned char)*3);
      unsigned char *xph = xp;
      tp[0] = -2;
      tp[1] = 255;
      tp[2] = -1;

      for(i = 0; i < 3; i++)
        *xp++ = (unsigned char)*tp++;

      xx = 1;
    }

    /* enter define mode */
    stat = nc_create(FILENAME, NC_CLOBBER | NC_64BIT_DATA, &ncid);
    check_err(stat,__LINE__,__FILE__);

    stat = nc_def_dim(ncid, "D3", D3_len, &D3_dim);
    check_err(stat,__LINE__,__FILE__);

    /* define variables */

    //y3_dims[0] = D3_dim;
    //stat = nc_def_var(ncid, "y3", NC_UBYTE, RANK_y3, y3_dims, &y3_id);
    //check_err(stat,__LINE__,__FILE__);

    y3d_dims[0] = D3_dim;
    stat = nc_def_var(ncid, "y3d", NC_UBYTE, RANK_y3d, y3d_dims, &y3d_id);
    check_err(stat,__LINE__,__FILE__);


    /* assign global attributes */

    {
    stat = nc_put_att_text(ncid, NC_GLOBAL, "Gc", 1, "€");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const signed char nc_test_cdf5_Gb_att[2] = {-128, 127} ;
    stat = nc_put_att_schar(ncid, NC_GLOBAL, "Gb", NC_BYTE, 2, nc_test_cdf5_Gb_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const short nc_test_cdf5_Gs_att[3] = {3333, 3333, 3332} ;
    stat = nc_put_att_short(ncid, NC_GLOBAL, "Gs", NC_SHORT, 3, nc_test_cdf5_Gs_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const int nc_test_cdf5_Gi_att[4] = {2222, 2222, 2221, 2223} ;
    stat = nc_put_att_int(ncid, NC_GLOBAL, "Gi", NC_INT, 4, nc_test_cdf5_Gi_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float nc_test_cdf5_Gf_att[5] = {((float)300), ((float)300), ((float)300), ((float)300), ((float)531)} ;
    stat = nc_put_att_float(ncid, NC_GLOBAL, "Gf", NC_FLOAT, 5, nc_test_cdf5_Gf_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const double nc_test_cdf5_Gd_att[6] = {((double)1000), ((double)1000), ((double)-1), ((double)1), ((double)660), ((double)650)} ;
    stat = nc_put_att_double(ncid, NC_GLOBAL, "Gd", NC_DOUBLE, 6, nc_test_cdf5_Gd_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const unsigned char nc_test_cdf5_Gy_att[7] = {0U, 255U, 255U, 0U, 36U, 38U, 40U} ;
    stat = nc_put_att_ubyte(ncid, NC_GLOBAL, "Gy", NC_UBYTE, 7, nc_test_cdf5_Gy_att);    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const unsigned short nc_test_cdf5_Gt_att[8] = {0U, 65535U, 65535U, 0U, 195U, 200U, 205U, 210U} ;
    stat = nc_put_att_ushort(ncid, NC_GLOBAL, "Gt", NC_USHORT, 8, nc_test_cdf5_Gt_att);    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const unsigned long long nc_test_cdf5_Gu_att[9] = {0ULL, 4294967295ULL, 4294967295ULL, 0ULL, 2880ULL, 2900ULL, 2920ULL, 2940ULL, 2960ULL} ;
    stat = nc_put_att_ulonglong(ncid, NC_GLOBAL, "Gu", NC_UINT64, 9, nc_test_cdf5_Gu_att);    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const signed long long nc_test_cdf5_Gx_att[10] = {-2147483776LL, 2147483775LL, -1LL, 1LL, 2720LL, 2700LL, 2680LL, 2660LL, 2640LL, 2620LL} ;
    stat = nc_put_att_longlong(ncid, NC_GLOBAL, "Gx", NC_INT64, 10, nc_test_cdf5_Gx_att);    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const unsigned long long nc_test_cdf5_Gz_att[11] = {0ULL, 4294967423ULL, 18446744073709551615ULL, 1ULL, 2880ULL, 2900ULL, 2920ULL, 2940ULL, 2960ULL, 2980ULL, 3000ULL} ;
    stat = nc_put_att_ulonglong(ncid, NC_GLOBAL, "Gz", NC_UINT64, 11, nc_test_cdf5_Gz_att);    check_err(stat,__LINE__,__FILE__);
    }


    /* leave define mode */
    stat = nc_enddef (ncid);
    check_err(stat,__LINE__,__FILE__);

    /* assign variable data */
    {
      double y3d_data[3] = {-2, 255, -1} ;
      size_t y3d_startset[1] = {0} ;
      size_t y3d_countset[1] = {3};
      nc_put_vara_double(ncid,y3d_id,y3d_startset,y3d_countset,y3d_data);
      //stat = nc_put_vara(ncid, y3d_id, y3d_startset, y3d_countset, y3d_data);
      check_err(stat,__LINE__,__FILE__);
    }
    /*
    {
      double y3_data[3] = {0U, 255U, 255U} ;
      size_t y3_startset[1] = {0} ;
      size_t y3_countset[1] = {3};
      stat = nc_put_vara_double(ncid, y3_id, y3_startset, y3_countset, y3_data);
      check_err(stat,__LINE__,__FILE__);
    }
    */
    stat = nc_close(ncid);
    check_err(stat,__LINE__,__FILE__);
    return 0;
}
