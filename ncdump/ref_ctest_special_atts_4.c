#include <stdio.h>
#include <stdlib.h>
#include <netcdf.h>


typedef struct obs_t {
    signed char obs_t_PERIOD_day;
    short obs_t_PERIOD_elev;
    int obs_t_PERIOD_count;
    float obs_t_PERIOD_relhum;
    double obs_t_PERIOD_time;
    unsigned char obs_t_PERIOD_category;
    unsigned short obs_t_PERIOD_id;
    unsigned int obs_t_PERIOD_particularity;
    signed long long obs_t_PERIOD_attention_span;
} obs_t;

static size_t var2_chunksizes[2] = {6, 7} ;
static size_t var3_chunksizes[3] = {6, 7, 8} ;
static size_t var4_chunksizes[3] = {6, 7, 8} ;
static size_t var5_chunksizes[1] = {6} ;

void
check_err(const int stat, int line, const char* file, const char* func) {
    if (stat != NC_NOERR) {
        (void)fprintf(stderr,"line %d of %s.%s: %s\n", line, file, func, nc_strerror(stat));
        fflush(stderr);
        exit(1);
    }
}

#define CHECK_ERR(err) check_err(err, __LINE__, __FILE__, __func__)

int
main() {/* create ref_tst_special_atts.nc */

    int  stat;  /* return status */
    int  ncid;  /* netCDF id */

    /* group ids */
    int tst_special_atts_grp;

    /* type ids */
    int obs_t_typ;

    /* dimension ids */
    int dim1_dim;
    int dim2_dim;
    int dim3_dim;

    /* dimension lengths */
    size_t dim1_len = 10;
    size_t dim2_len = 20;
    size_t dim3_len = 30;

    /* variable ids */
    int var1_id;
    int var2_id;
    int var3_id;
    int var4_id;
    int var5_id;
    int var6_id;
    int var7_id;

    /* rank (number of dimensions) for each variable */
#   define RANK_var1 1
#   define RANK_var2 2
#   define RANK_var3 3
#   define RANK_var4 3
#   define RANK_var5 1
#   define RANK_var6 1
#   define RANK_var7 0

    /* variable shapes */
    int var1_dims[RANK_var1];
    int var2_dims[RANK_var2];
    int var3_dims[RANK_var3];
    int var4_dims[RANK_var4];
    int var5_dims[RANK_var5];
    int var6_dims[RANK_var6];

    /* enter define mode */
    stat = nc_create("ref_tst_special_atts.nc", NC_CLOBBER|NC_NETCDF4, &ncid);
    CHECK_ERR(stat);
    tst_special_atts_grp = ncid;

    stat = nc_def_compound(tst_special_atts_grp, sizeof(obs_t), "obs_t", &obs_t_typ);    CHECK_ERR(stat);
    {
    stat = nc_insert_compound(tst_special_atts_grp, obs_t_typ, "day", NC_COMPOUND_OFFSET(obs_t,obs_t_PERIOD_day), NC_BYTE);    CHECK_ERR(stat);
    stat = nc_insert_compound(tst_special_atts_grp, obs_t_typ, "elev", NC_COMPOUND_OFFSET(obs_t,obs_t_PERIOD_elev), NC_SHORT);    CHECK_ERR(stat);
    stat = nc_insert_compound(tst_special_atts_grp, obs_t_typ, "count", NC_COMPOUND_OFFSET(obs_t,obs_t_PERIOD_count), NC_INT);    CHECK_ERR(stat);
    stat = nc_insert_compound(tst_special_atts_grp, obs_t_typ, "relhum", NC_COMPOUND_OFFSET(obs_t,obs_t_PERIOD_relhum), NC_FLOAT);    CHECK_ERR(stat);
    stat = nc_insert_compound(tst_special_atts_grp, obs_t_typ, "time", NC_COMPOUND_OFFSET(obs_t,obs_t_PERIOD_time), NC_DOUBLE);    CHECK_ERR(stat);
    stat = nc_insert_compound(tst_special_atts_grp, obs_t_typ, "category", NC_COMPOUND_OFFSET(obs_t,obs_t_PERIOD_category), NC_UBYTE);    CHECK_ERR(stat);
    stat = nc_insert_compound(tst_special_atts_grp, obs_t_typ, "id", NC_COMPOUND_OFFSET(obs_t,obs_t_PERIOD_id), NC_USHORT);    CHECK_ERR(stat);
    stat = nc_insert_compound(tst_special_atts_grp, obs_t_typ, "particularity", NC_COMPOUND_OFFSET(obs_t,obs_t_PERIOD_particularity), NC_UINT);    CHECK_ERR(stat);
    stat = nc_insert_compound(tst_special_atts_grp, obs_t_typ, "attention_span", NC_COMPOUND_OFFSET(obs_t,obs_t_PERIOD_attention_span), NC_INT64);    CHECK_ERR(stat);
    }


    /* define dimensions */
    stat = nc_def_dim(tst_special_atts_grp, "dim1", dim1_len, &dim1_dim);
    CHECK_ERR(stat);
    stat = nc_def_dim(tst_special_atts_grp, "dim2", dim2_len, &dim2_dim);
    CHECK_ERR(stat);
    stat = nc_def_dim(tst_special_atts_grp, "dim3", dim3_len, &dim3_dim);
    CHECK_ERR(stat);

    /* define variables */

    var1_dims[0] = dim1_dim;
    stat = nc_def_var(tst_special_atts_grp, "var1", NC_INT, RANK_var1, var1_dims, &var1_id);
    CHECK_ERR(stat);
    stat = nc_def_var_chunking(tst_special_atts_grp, var1_id, NC_CONTIGUOUS, NULL);
    CHECK_ERR(stat);
    stat = nc_def_var_endian(tst_special_atts_grp, var1_id, NC_ENDIAN_LITTLE);
    CHECK_ERR(stat);

    var2_dims[0] = dim1_dim;
    var2_dims[1] = dim2_dim;
    stat = nc_def_var(tst_special_atts_grp, "var2", NC_INT, RANK_var2, var2_dims, &var2_id);
    CHECK_ERR(stat);
    stat = nc_def_var_chunking(tst_special_atts_grp, var2_id, NC_CHUNKED, var2_chunksizes);
    CHECK_ERR(stat);
    stat = nc_def_var_fletcher32(tst_special_atts_grp, var2_id, 1);
    CHECK_ERR(stat);
    stat = nc_def_var_endian(tst_special_atts_grp, var2_id, NC_ENDIAN_BIG);
    CHECK_ERR(stat);

    var3_dims[0] = dim1_dim;
    var3_dims[1] = dim2_dim;
    var3_dims[2] = dim3_dim;
    stat = nc_def_var(tst_special_atts_grp, "var3", NC_INT, RANK_var3, var3_dims, &var3_id);
    CHECK_ERR(stat);
    stat = nc_def_var_chunking(tst_special_atts_grp, var3_id, NC_CHUNKED, var3_chunksizes);
    CHECK_ERR(stat);
    stat = nc_def_var_deflate(tst_special_atts_grp, var3_id, NC_NOSHUFFLE, 1, 2);
    CHECK_ERR(stat);
    stat = nc_def_var_endian(tst_special_atts_grp, var3_id, NC_ENDIAN_LITTLE);
    CHECK_ERR(stat);

    var4_dims[0] = dim1_dim;
    var4_dims[1] = dim2_dim;
    var4_dims[2] = dim3_dim;
    stat = nc_def_var(tst_special_atts_grp, "var4", NC_INT, RANK_var4, var4_dims, &var4_id);
    CHECK_ERR(stat);
    stat = nc_def_var_chunking(tst_special_atts_grp, var4_id, NC_CHUNKED, var4_chunksizes);
    CHECK_ERR(stat);
    stat = nc_def_var_deflate(tst_special_atts_grp, var4_id, NC_SHUFFLE, 1, 2);
    CHECK_ERR(stat);
    stat = nc_def_var_endian(tst_special_atts_grp, var4_id, NC_ENDIAN_LITTLE);
    CHECK_ERR(stat);
    stat = nc_def_var_fill(tst_special_atts_grp, var4_id, NC_NOFILL, NULL);
    CHECK_ERR(stat);

    var5_dims[0] = dim1_dim;
    stat = nc_def_var(tst_special_atts_grp, "var5", obs_t_typ, RANK_var5, var5_dims, &var5_id);
    CHECK_ERR(stat);
    stat = nc_def_var_chunking(tst_special_atts_grp, var5_id, NC_CHUNKED, var5_chunksizes);
    CHECK_ERR(stat);
    stat = nc_def_var_fletcher32(tst_special_atts_grp, var5_id, 1);
    CHECK_ERR(stat);
    stat = nc_def_var_deflate(tst_special_atts_grp, var5_id, NC_SHUFFLE, 1, 2);
    CHECK_ERR(stat);

    var6_dims[0] = dim1_dim;
    stat = nc_def_var(tst_special_atts_grp, "var6", NC_INT, RANK_var6, var6_dims, &var6_id);
    CHECK_ERR(stat);
    stat = nc_def_var_chunking(tst_special_atts_grp, var6_id, NC_COMPACT, NULL);
    CHECK_ERR(stat);
    stat = nc_def_var_endian(tst_special_atts_grp, var6_id, NC_ENDIAN_LITTLE);
    CHECK_ERR(stat);

    stat = nc_def_var(tst_special_atts_grp, "var7", NC_INT, RANK_var7, 0, &var7_id);
    CHECK_ERR(stat);
    stat = nc_def_var_chunking(tst_special_atts_grp, var7_id, NC_COMPACT, NULL);
    CHECK_ERR(stat);
    stat = nc_def_var_endian(tst_special_atts_grp, var7_id, NC_ENDIAN_LITTLE);
    CHECK_ERR(stat);

    /* leave define mode */
    stat = nc_enddef (tst_special_atts_grp);
    CHECK_ERR(stat);

    /* assign variable data */

    stat = nc_close(tst_special_atts_grp);
    CHECK_ERR(stat);
    return 0;
}
