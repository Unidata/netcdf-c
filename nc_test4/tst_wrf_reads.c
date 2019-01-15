/* This is part of the netCDF package. Copyright 2005-2018 University
   Corporation for Atmospheric Research/Unidata See COPYRIGHT file for
   conditions of use.

   This is a benchmarking program that depends on some KNMI files from
   the Unidata ftp site. The files are opened and read, and
   performance is timed.

   Ed Hartnett
*/

#include <nc_tests.h>
#include "err_macros.h"
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

#define FILE_NAME "tst_wrf_reads.nc"

/* Prototype from tst_utils.c. */
int nc4_timeval_subtract(struct timeval *result, struct timeval *x,
                         struct timeval *y);

extern const char* nc_strerror(int ncerr);

void
check_err(const int stat, const int line, const char *file) {
   if (stat != NC_NOERR) {
      (void)fprintf(stderr,"line %d of %s: %s\n", line, file, nc_strerror(stat));
      fflush(stderr);
      exit(1);
   }
}

int
create_wrfbdy_file(char *file_name, int cmode) {/* create wrfbdy.nc */

   int  stat;  /* return status */
   int  ncid;  /* netCDF id */

   /* dimension ids */
   int Time_dim;
   int DateStrLen_dim;
   int south_north_dim;
   int bottom_top_dim;
   int bdy_width_dim;
   int west_east_stag_dim;
   int south_north_stag_dim;
   int west_east_dim;
   int bottom_top_stag_dim;

   /* dimension lengths */
   size_t Time_len = NC_UNLIMITED;
   size_t DateStrLen_len = 19;
   size_t south_north_len = 60;
   size_t bottom_top_len = 32;
   size_t bdy_width_len = 5;
   size_t west_east_stag_len = 74;
   size_t south_north_stag_len = 61;
   size_t west_east_len = 73;
   size_t bottom_top_stag_len = 33;

   /* variable ids */
   int Times_id;
   int md___thisbdytimee_x_t_d_o_m_a_i_n_m_e_t_a_data__id;
   int md___nextbdytimee_x_t_d_o_m_a_i_n_m_e_t_a_data__id;
   int U_BXS_id;
   int U_BXE_id;
   int U_BYS_id;
   int U_BYE_id;
   int U_BTXS_id;
   int U_BTXE_id;
   int U_BTYS_id;
   int U_BTYE_id;
   int V_BXS_id;
   int V_BXE_id;
   int V_BYS_id;
   int V_BYE_id;
   int V_BTXS_id;
   int V_BTXE_id;
   int V_BTYS_id;
   int V_BTYE_id;
   int W_BXS_id;
   int W_BXE_id;
   int W_BYS_id;
   int W_BYE_id;
   int W_BTXS_id;
   int W_BTXE_id;
   int W_BTYS_id;
   int W_BTYE_id;
   int PH_BXS_id;
   int PH_BXE_id;
   int PH_BYS_id;
   int PH_BYE_id;
   int PH_BTXS_id;
   int PH_BTXE_id;
   int PH_BTYS_id;
   int PH_BTYE_id;
   int T_BXS_id;
   int T_BXE_id;
   int T_BYS_id;
   int T_BYE_id;
   int T_BTXS_id;
   int T_BTXE_id;
   int T_BTYS_id;
   int T_BTYE_id;
   int MU_BXS_id;
   int MU_BXE_id;
   int MU_BYS_id;
   int MU_BYE_id;
   int MU_BTXS_id;
   int MU_BTXE_id;
   int MU_BTYS_id;
   int MU_BTYE_id;
   int QVAPOR_BXS_id;
   int QVAPOR_BXE_id;
   int QVAPOR_BYS_id;
   int QVAPOR_BYE_id;
   int QCLOUD_BXS_id;
   int QCLOUD_BXE_id;
   int QCLOUD_BYS_id;
   int QCLOUD_BYE_id;
   int QRAIN_BXS_id;
   int QRAIN_BXE_id;
   int QRAIN_BYS_id;
   int QRAIN_BYE_id;
   int QICE_BXS_id;
   int QICE_BXE_id;
   int QICE_BYS_id;
   int QICE_BYE_id;
   int QSNOW_BXS_id;
   int QSNOW_BXE_id;
   int QSNOW_BYS_id;
   int QSNOW_BYE_id;
   int QGRAUP_BXS_id;
   int QGRAUP_BXE_id;
   int QGRAUP_BYS_id;
   int QGRAUP_BYE_id;
   int QVAPOR_BTXS_id;
   int QVAPOR_BTXE_id;
   int QVAPOR_BTYS_id;
   int QVAPOR_BTYE_id;
   int QCLOUD_BTXS_id;
   int QCLOUD_BTXE_id;
   int QCLOUD_BTYS_id;
   int QCLOUD_BTYE_id;
   int QRAIN_BTXS_id;
   int QRAIN_BTXE_id;
   int QRAIN_BTYS_id;
   int QRAIN_BTYE_id;
   int QICE_BTXS_id;
   int QICE_BTXE_id;
   int QICE_BTYS_id;
   int QICE_BTYE_id;
   int QSNOW_BTXS_id;
   int QSNOW_BTXE_id;
   int QSNOW_BTYS_id;
   int QSNOW_BTYE_id;
   int QGRAUP_BTXS_id;
   int QGRAUP_BTXE_id;
   int QGRAUP_BTYS_id;
   int QGRAUP_BTYE_id;
   int QNICE_BXS_id;
   int QNICE_BXE_id;
   int QNICE_BYS_id;
   int QNICE_BYE_id;
   int QNRAIN_BXS_id;
   int QNRAIN_BXE_id;
   int QNRAIN_BYS_id;
   int QNRAIN_BYE_id;
   int QNICE_BTXS_id;
   int QNICE_BTXE_id;
   int QNICE_BTYS_id;
   int QNICE_BTYE_id;
   int QNRAIN_BTXS_id;
   int QNRAIN_BTXE_id;
   int QNRAIN_BTYS_id;
   int QNRAIN_BTYE_id;
   int HT_SHAD_BXS_id;
   int HT_SHAD_BXE_id;
   int HT_SHAD_BYS_id;
   int HT_SHAD_BYE_id;
   int HT_SHAD_BTXS_id;
   int HT_SHAD_BTXE_id;
   int HT_SHAD_BTYS_id;
   int HT_SHAD_BTYE_id;
   int PC_BXS_id;
   int PC_BXE_id;
   int PC_BYS_id;
   int PC_BYE_id;
   int PC_BTXS_id;
   int PC_BTXE_id;
   int PC_BTYS_id;
   int PC_BTYE_id;

   /* rank (number of dimensions) for each variable */
#   define RANK_Times 2
#   define RANK_md___thisbdytimee_x_t_d_o_m_a_i_n_m_e_t_a_data_ 2
#   define RANK_md___nextbdytimee_x_t_d_o_m_a_i_n_m_e_t_a_data_ 2
#   define RANK_U_BXS 4
#   define RANK_U_BXE 4
#   define RANK_U_BYS 4
#   define RANK_U_BYE 4
#   define RANK_U_BTXS 4
#   define RANK_U_BTXE 4
#   define RANK_U_BTYS 4
#   define RANK_U_BTYE 4
#   define RANK_V_BXS 4
#   define RANK_V_BXE 4
#   define RANK_V_BYS 4
#   define RANK_V_BYE 4
#   define RANK_V_BTXS 4
#   define RANK_V_BTXE 4
#   define RANK_V_BTYS 4
#   define RANK_V_BTYE 4
#   define RANK_W_BXS 4
#   define RANK_W_BXE 4
#   define RANK_W_BYS 4
#   define RANK_W_BYE 4
#   define RANK_W_BTXS 4
#   define RANK_W_BTXE 4
#   define RANK_W_BTYS 4
#   define RANK_W_BTYE 4
#   define RANK_PH_BXS 4
#   define RANK_PH_BXE 4
#   define RANK_PH_BYS 4
#   define RANK_PH_BYE 4
#   define RANK_PH_BTXS 4
#   define RANK_PH_BTXE 4
#   define RANK_PH_BTYS 4
#   define RANK_PH_BTYE 4
#   define RANK_T_BXS 4
#   define RANK_T_BXE 4
#   define RANK_T_BYS 4
#   define RANK_T_BYE 4
#   define RANK_T_BTXS 4
#   define RANK_T_BTXE 4
#   define RANK_T_BTYS 4
#   define RANK_T_BTYE 4
#   define RANK_MU_BXS 3
#   define RANK_MU_BXE 3
#   define RANK_MU_BYS 3
#   define RANK_MU_BYE 3
#   define RANK_MU_BTXS 3
#   define RANK_MU_BTXE 3
#   define RANK_MU_BTYS 3
#   define RANK_MU_BTYE 3
#   define RANK_QVAPOR_BXS 4
#   define RANK_QVAPOR_BXE 4
#   define RANK_QVAPOR_BYS 4
#   define RANK_QVAPOR_BYE 4
#   define RANK_QCLOUD_BXS 4
#   define RANK_QCLOUD_BXE 4
#   define RANK_QCLOUD_BYS 4
#   define RANK_QCLOUD_BYE 4
#   define RANK_QRAIN_BXS 4
#   define RANK_QRAIN_BXE 4
#   define RANK_QRAIN_BYS 4
#   define RANK_QRAIN_BYE 4
#   define RANK_QICE_BXS 4
#   define RANK_QICE_BXE 4
#   define RANK_QICE_BYS 4
#   define RANK_QICE_BYE 4
#   define RANK_QSNOW_BXS 4
#   define RANK_QSNOW_BXE 4
#   define RANK_QSNOW_BYS 4
#   define RANK_QSNOW_BYE 4
#   define RANK_QGRAUP_BXS 4
#   define RANK_QGRAUP_BXE 4
#   define RANK_QGRAUP_BYS 4
#   define RANK_QGRAUP_BYE 4
#   define RANK_QVAPOR_BTXS 4
#   define RANK_QVAPOR_BTXE 4
#   define RANK_QVAPOR_BTYS 4
#   define RANK_QVAPOR_BTYE 4
#   define RANK_QCLOUD_BTXS 4
#   define RANK_QCLOUD_BTXE 4
#   define RANK_QCLOUD_BTYS 4
#   define RANK_QCLOUD_BTYE 4
#   define RANK_QRAIN_BTXS 4
#   define RANK_QRAIN_BTXE 4
#   define RANK_QRAIN_BTYS 4
#   define RANK_QRAIN_BTYE 4
#   define RANK_QICE_BTXS 4
#   define RANK_QICE_BTXE 4
#   define RANK_QICE_BTYS 4
#   define RANK_QICE_BTYE 4
#   define RANK_QSNOW_BTXS 4
#   define RANK_QSNOW_BTXE 4
#   define RANK_QSNOW_BTYS 4
#   define RANK_QSNOW_BTYE 4
#   define RANK_QGRAUP_BTXS 4
#   define RANK_QGRAUP_BTXE 4
#   define RANK_QGRAUP_BTYS 4
#   define RANK_QGRAUP_BTYE 4
#   define RANK_QNICE_BXS 4
#   define RANK_QNICE_BXE 4
#   define RANK_QNICE_BYS 4
#   define RANK_QNICE_BYE 4
#   define RANK_QNRAIN_BXS 4
#   define RANK_QNRAIN_BXE 4
#   define RANK_QNRAIN_BYS 4
#   define RANK_QNRAIN_BYE 4
#   define RANK_QNICE_BTXS 4
#   define RANK_QNICE_BTXE 4
#   define RANK_QNICE_BTYS 4
#   define RANK_QNICE_BTYE 4
#   define RANK_QNRAIN_BTXS 4
#   define RANK_QNRAIN_BTXE 4
#   define RANK_QNRAIN_BTYS 4
#   define RANK_QNRAIN_BTYE 4
#   define RANK_HT_SHAD_BXS 3
#   define RANK_HT_SHAD_BXE 3
#   define RANK_HT_SHAD_BYS 3
#   define RANK_HT_SHAD_BYE 3
#   define RANK_HT_SHAD_BTXS 3
#   define RANK_HT_SHAD_BTXE 3
#   define RANK_HT_SHAD_BTYS 3
#   define RANK_HT_SHAD_BTYE 3
#   define RANK_PC_BXS 3
#   define RANK_PC_BXE 3
#   define RANK_PC_BYS 3
#   define RANK_PC_BYE 3
#   define RANK_PC_BTXS 3
#   define RANK_PC_BTXE 3
#   define RANK_PC_BTYS 3
#   define RANK_PC_BTYE 3

   /* variable shapes */
   int Times_dims[RANK_Times];
   int md___thisbdytimee_x_t_d_o_m_a_i_n_m_e_t_a_data__dims[RANK_md___thisbdytimee_x_t_d_o_m_a_i_n_m_e_t_a_data_];
   int md___nextbdytimee_x_t_d_o_m_a_i_n_m_e_t_a_data__dims[RANK_md___nextbdytimee_x_t_d_o_m_a_i_n_m_e_t_a_data_];
   int U_BXS_dims[RANK_U_BXS];
   int U_BXE_dims[RANK_U_BXE];
   int U_BYS_dims[RANK_U_BYS];
   int U_BYE_dims[RANK_U_BYE];
   int U_BTXS_dims[RANK_U_BTXS];
   int U_BTXE_dims[RANK_U_BTXE];
   int U_BTYS_dims[RANK_U_BTYS];
   int U_BTYE_dims[RANK_U_BTYE];
   int V_BXS_dims[RANK_V_BXS];
   int V_BXE_dims[RANK_V_BXE];
   int V_BYS_dims[RANK_V_BYS];
   int V_BYE_dims[RANK_V_BYE];
   int V_BTXS_dims[RANK_V_BTXS];
   int V_BTXE_dims[RANK_V_BTXE];
   int V_BTYS_dims[RANK_V_BTYS];
   int V_BTYE_dims[RANK_V_BTYE];
   int W_BXS_dims[RANK_W_BXS];
   int W_BXE_dims[RANK_W_BXE];
   int W_BYS_dims[RANK_W_BYS];
   int W_BYE_dims[RANK_W_BYE];
   int W_BTXS_dims[RANK_W_BTXS];
   int W_BTXE_dims[RANK_W_BTXE];
   int W_BTYS_dims[RANK_W_BTYS];
   int W_BTYE_dims[RANK_W_BTYE];
   int PH_BXS_dims[RANK_PH_BXS];
   int PH_BXE_dims[RANK_PH_BXE];
   int PH_BYS_dims[RANK_PH_BYS];
   int PH_BYE_dims[RANK_PH_BYE];
   int PH_BTXS_dims[RANK_PH_BTXS];
   int PH_BTXE_dims[RANK_PH_BTXE];
   int PH_BTYS_dims[RANK_PH_BTYS];
   int PH_BTYE_dims[RANK_PH_BTYE];
   int T_BXS_dims[RANK_T_BXS];
   int T_BXE_dims[RANK_T_BXE];
   int T_BYS_dims[RANK_T_BYS];
   int T_BYE_dims[RANK_T_BYE];
   int T_BTXS_dims[RANK_T_BTXS];
   int T_BTXE_dims[RANK_T_BTXE];
   int T_BTYS_dims[RANK_T_BTYS];
   int T_BTYE_dims[RANK_T_BTYE];
   int MU_BXS_dims[RANK_MU_BXS];
   int MU_BXE_dims[RANK_MU_BXE];
   int MU_BYS_dims[RANK_MU_BYS];
   int MU_BYE_dims[RANK_MU_BYE];
   int MU_BTXS_dims[RANK_MU_BTXS];
   int MU_BTXE_dims[RANK_MU_BTXE];
   int MU_BTYS_dims[RANK_MU_BTYS];
   int MU_BTYE_dims[RANK_MU_BTYE];
   int QVAPOR_BXS_dims[RANK_QVAPOR_BXS];
   int QVAPOR_BXE_dims[RANK_QVAPOR_BXE];
   int QVAPOR_BYS_dims[RANK_QVAPOR_BYS];
   int QVAPOR_BYE_dims[RANK_QVAPOR_BYE];
   int QCLOUD_BXS_dims[RANK_QCLOUD_BXS];
   int QCLOUD_BXE_dims[RANK_QCLOUD_BXE];
   int QCLOUD_BYS_dims[RANK_QCLOUD_BYS];
   int QCLOUD_BYE_dims[RANK_QCLOUD_BYE];
   int QRAIN_BXS_dims[RANK_QRAIN_BXS];
   int QRAIN_BXE_dims[RANK_QRAIN_BXE];
   int QRAIN_BYS_dims[RANK_QRAIN_BYS];
   int QRAIN_BYE_dims[RANK_QRAIN_BYE];
   int QICE_BXS_dims[RANK_QICE_BXS];
   int QICE_BXE_dims[RANK_QICE_BXE];
   int QICE_BYS_dims[RANK_QICE_BYS];
   int QICE_BYE_dims[RANK_QICE_BYE];
   int QSNOW_BXS_dims[RANK_QSNOW_BXS];
   int QSNOW_BXE_dims[RANK_QSNOW_BXE];
   int QSNOW_BYS_dims[RANK_QSNOW_BYS];
   int QSNOW_BYE_dims[RANK_QSNOW_BYE];
   int QGRAUP_BXS_dims[RANK_QGRAUP_BXS];
   int QGRAUP_BXE_dims[RANK_QGRAUP_BXE];
   int QGRAUP_BYS_dims[RANK_QGRAUP_BYS];
   int QGRAUP_BYE_dims[RANK_QGRAUP_BYE];
   int QVAPOR_BTXS_dims[RANK_QVAPOR_BTXS];
   int QVAPOR_BTXE_dims[RANK_QVAPOR_BTXE];
   int QVAPOR_BTYS_dims[RANK_QVAPOR_BTYS];
   int QVAPOR_BTYE_dims[RANK_QVAPOR_BTYE];
   int QCLOUD_BTXS_dims[RANK_QCLOUD_BTXS];
   int QCLOUD_BTXE_dims[RANK_QCLOUD_BTXE];
   int QCLOUD_BTYS_dims[RANK_QCLOUD_BTYS];
   int QCLOUD_BTYE_dims[RANK_QCLOUD_BTYE];
   int QRAIN_BTXS_dims[RANK_QRAIN_BTXS];
   int QRAIN_BTXE_dims[RANK_QRAIN_BTXE];
   int QRAIN_BTYS_dims[RANK_QRAIN_BTYS];
   int QRAIN_BTYE_dims[RANK_QRAIN_BTYE];
   int QICE_BTXS_dims[RANK_QICE_BTXS];
   int QICE_BTXE_dims[RANK_QICE_BTXE];
   int QICE_BTYS_dims[RANK_QICE_BTYS];
   int QICE_BTYE_dims[RANK_QICE_BTYE];
   int QSNOW_BTXS_dims[RANK_QSNOW_BTXS];
   int QSNOW_BTXE_dims[RANK_QSNOW_BTXE];
   int QSNOW_BTYS_dims[RANK_QSNOW_BTYS];
   int QSNOW_BTYE_dims[RANK_QSNOW_BTYE];
   int QGRAUP_BTXS_dims[RANK_QGRAUP_BTXS];
   int QGRAUP_BTXE_dims[RANK_QGRAUP_BTXE];
   int QGRAUP_BTYS_dims[RANK_QGRAUP_BTYS];
   int QGRAUP_BTYE_dims[RANK_QGRAUP_BTYE];
   int QNICE_BXS_dims[RANK_QNICE_BXS];
   int QNICE_BXE_dims[RANK_QNICE_BXE];
   int QNICE_BYS_dims[RANK_QNICE_BYS];
   int QNICE_BYE_dims[RANK_QNICE_BYE];
   int QNRAIN_BXS_dims[RANK_QNRAIN_BXS];
   int QNRAIN_BXE_dims[RANK_QNRAIN_BXE];
   int QNRAIN_BYS_dims[RANK_QNRAIN_BYS];
   int QNRAIN_BYE_dims[RANK_QNRAIN_BYE];
   int QNICE_BTXS_dims[RANK_QNICE_BTXS];
   int QNICE_BTXE_dims[RANK_QNICE_BTXE];
   int QNICE_BTYS_dims[RANK_QNICE_BTYS];
   int QNICE_BTYE_dims[RANK_QNICE_BTYE];
   int QNRAIN_BTXS_dims[RANK_QNRAIN_BTXS];
   int QNRAIN_BTXE_dims[RANK_QNRAIN_BTXE];
   int QNRAIN_BTYS_dims[RANK_QNRAIN_BTYS];
   int QNRAIN_BTYE_dims[RANK_QNRAIN_BTYE];
   int HT_SHAD_BXS_dims[RANK_HT_SHAD_BXS];
   int HT_SHAD_BXE_dims[RANK_HT_SHAD_BXE];
   int HT_SHAD_BYS_dims[RANK_HT_SHAD_BYS];
   int HT_SHAD_BYE_dims[RANK_HT_SHAD_BYE];
   int HT_SHAD_BTXS_dims[RANK_HT_SHAD_BTXS];
   int HT_SHAD_BTXE_dims[RANK_HT_SHAD_BTXE];
   int HT_SHAD_BTYS_dims[RANK_HT_SHAD_BTYS];
   int HT_SHAD_BTYE_dims[RANK_HT_SHAD_BTYE];
   int PC_BXS_dims[RANK_PC_BXS];
   int PC_BXE_dims[RANK_PC_BXE];
   int PC_BYS_dims[RANK_PC_BYS];
   int PC_BYE_dims[RANK_PC_BYE];
   int PC_BTXS_dims[RANK_PC_BTXS];
   int PC_BTXE_dims[RANK_PC_BTXE];
   int PC_BTYS_dims[RANK_PC_BTYS];
   int PC_BTYE_dims[RANK_PC_BTYE];

   /* enter define mode */
   stat = nc_create(file_name, cmode|NC_CLOBBER, &ncid);
   check_err(stat,__LINE__,__FILE__);

   /* define dimensions */
   stat = nc_def_dim(ncid, "Time", Time_len, &Time_dim);
   check_err(stat,__LINE__,__FILE__);
   stat = nc_def_dim(ncid, "DateStrLen", DateStrLen_len, &DateStrLen_dim);
   check_err(stat,__LINE__,__FILE__);
   stat = nc_def_dim(ncid, "south_north", south_north_len, &south_north_dim);
   check_err(stat,__LINE__,__FILE__);
   stat = nc_def_dim(ncid, "bottom_top", bottom_top_len, &bottom_top_dim);
   check_err(stat,__LINE__,__FILE__);
   stat = nc_def_dim(ncid, "bdy_width", bdy_width_len, &bdy_width_dim);
   check_err(stat,__LINE__,__FILE__);
   stat = nc_def_dim(ncid, "west_east_stag", west_east_stag_len, &west_east_stag_dim);
   check_err(stat,__LINE__,__FILE__);
   stat = nc_def_dim(ncid, "south_north_stag", south_north_stag_len, &south_north_stag_dim);
   check_err(stat,__LINE__,__FILE__);
   stat = nc_def_dim(ncid, "west_east", west_east_len, &west_east_dim);
   check_err(stat,__LINE__,__FILE__);
   stat = nc_def_dim(ncid, "bottom_top_stag", bottom_top_stag_len, &bottom_top_stag_dim);
   check_err(stat,__LINE__,__FILE__);

   /* define variables */

   Times_dims[0] = Time_dim;
   Times_dims[1] = DateStrLen_dim;
   stat = nc_def_var(ncid, "Times", NC_CHAR, RANK_Times, Times_dims, &Times_id);
   check_err(stat,__LINE__,__FILE__);

   md___thisbdytimee_x_t_d_o_m_a_i_n_m_e_t_a_data__dims[0] = Time_dim;
   md___thisbdytimee_x_t_d_o_m_a_i_n_m_e_t_a_data__dims[1] = DateStrLen_dim;
   stat = nc_def_var(ncid, "md___thisbdytimee_x_t_d_o_m_a_i_n_m_e_t_a_data_", NC_CHAR, RANK_md___thisbdytimee_x_t_d_o_m_a_i_n_m_e_t_a_data_, md___thisbdytimee_x_t_d_o_m_a_i_n_m_e_t_a_data__dims, &md___thisbdytimee_x_t_d_o_m_a_i_n_m_e_t_a_data__id);
   check_err(stat,__LINE__,__FILE__);

   md___nextbdytimee_x_t_d_o_m_a_i_n_m_e_t_a_data__dims[0] = Time_dim;
   md___nextbdytimee_x_t_d_o_m_a_i_n_m_e_t_a_data__dims[1] = DateStrLen_dim;
   stat = nc_def_var(ncid, "md___nextbdytimee_x_t_d_o_m_a_i_n_m_e_t_a_data_", NC_CHAR, RANK_md___nextbdytimee_x_t_d_o_m_a_i_n_m_e_t_a_data_, md___nextbdytimee_x_t_d_o_m_a_i_n_m_e_t_a_data__dims, &md___nextbdytimee_x_t_d_o_m_a_i_n_m_e_t_a_data__id);
   check_err(stat,__LINE__,__FILE__);

   U_BXS_dims[0] = Time_dim;
   U_BXS_dims[1] = bdy_width_dim;
   U_BXS_dims[2] = bottom_top_dim;
   U_BXS_dims[3] = south_north_dim;
   stat = nc_def_var(ncid, "U_BXS", NC_FLOAT, RANK_U_BXS, U_BXS_dims, &U_BXS_id);
   check_err(stat,__LINE__,__FILE__);

   U_BXE_dims[0] = Time_dim;
   U_BXE_dims[1] = bdy_width_dim;
   U_BXE_dims[2] = bottom_top_dim;
   U_BXE_dims[3] = south_north_dim;
   stat = nc_def_var(ncid, "U_BXE", NC_FLOAT, RANK_U_BXE, U_BXE_dims, &U_BXE_id);
   check_err(stat,__LINE__,__FILE__);

   U_BYS_dims[0] = Time_dim;
   U_BYS_dims[1] = bdy_width_dim;
   U_BYS_dims[2] = bottom_top_dim;
   U_BYS_dims[3] = west_east_stag_dim;
   stat = nc_def_var(ncid, "U_BYS", NC_FLOAT, RANK_U_BYS, U_BYS_dims, &U_BYS_id);
   check_err(stat,__LINE__,__FILE__);

   U_BYE_dims[0] = Time_dim;
   U_BYE_dims[1] = bdy_width_dim;
   U_BYE_dims[2] = bottom_top_dim;
   U_BYE_dims[3] = west_east_stag_dim;
   stat = nc_def_var(ncid, "U_BYE", NC_FLOAT, RANK_U_BYE, U_BYE_dims, &U_BYE_id);
   check_err(stat,__LINE__,__FILE__);

   U_BTXS_dims[0] = Time_dim;
   U_BTXS_dims[1] = bdy_width_dim;
   U_BTXS_dims[2] = bottom_top_dim;
   U_BTXS_dims[3] = south_north_dim;
   stat = nc_def_var(ncid, "U_BTXS", NC_FLOAT, RANK_U_BTXS, U_BTXS_dims, &U_BTXS_id);
   check_err(stat,__LINE__,__FILE__);

   U_BTXE_dims[0] = Time_dim;
   U_BTXE_dims[1] = bdy_width_dim;
   U_BTXE_dims[2] = bottom_top_dim;
   U_BTXE_dims[3] = south_north_dim;
   stat = nc_def_var(ncid, "U_BTXE", NC_FLOAT, RANK_U_BTXE, U_BTXE_dims, &U_BTXE_id);
   check_err(stat,__LINE__,__FILE__);

   U_BTYS_dims[0] = Time_dim;
   U_BTYS_dims[1] = bdy_width_dim;
   U_BTYS_dims[2] = bottom_top_dim;
   U_BTYS_dims[3] = west_east_stag_dim;
   stat = nc_def_var(ncid, "U_BTYS", NC_FLOAT, RANK_U_BTYS, U_BTYS_dims, &U_BTYS_id);
   check_err(stat,__LINE__,__FILE__);

   U_BTYE_dims[0] = Time_dim;
   U_BTYE_dims[1] = bdy_width_dim;
   U_BTYE_dims[2] = bottom_top_dim;
   U_BTYE_dims[3] = west_east_stag_dim;
   stat = nc_def_var(ncid, "U_BTYE", NC_FLOAT, RANK_U_BTYE, U_BTYE_dims, &U_BTYE_id);
   check_err(stat,__LINE__,__FILE__);

   V_BXS_dims[0] = Time_dim;
   V_BXS_dims[1] = bdy_width_dim;
   V_BXS_dims[2] = bottom_top_dim;
   V_BXS_dims[3] = south_north_stag_dim;
   stat = nc_def_var(ncid, "V_BXS", NC_FLOAT, RANK_V_BXS, V_BXS_dims, &V_BXS_id);
   check_err(stat,__LINE__,__FILE__);

   V_BXE_dims[0] = Time_dim;
   V_BXE_dims[1] = bdy_width_dim;
   V_BXE_dims[2] = bottom_top_dim;
   V_BXE_dims[3] = south_north_stag_dim;
   stat = nc_def_var(ncid, "V_BXE", NC_FLOAT, RANK_V_BXE, V_BXE_dims, &V_BXE_id);
   check_err(stat,__LINE__,__FILE__);

   V_BYS_dims[0] = Time_dim;
   V_BYS_dims[1] = bdy_width_dim;
   V_BYS_dims[2] = bottom_top_dim;
   V_BYS_dims[3] = west_east_dim;
   stat = nc_def_var(ncid, "V_BYS", NC_FLOAT, RANK_V_BYS, V_BYS_dims, &V_BYS_id);
   check_err(stat,__LINE__,__FILE__);

   V_BYE_dims[0] = Time_dim;
   V_BYE_dims[1] = bdy_width_dim;
   V_BYE_dims[2] = bottom_top_dim;
   V_BYE_dims[3] = west_east_dim;
   stat = nc_def_var(ncid, "V_BYE", NC_FLOAT, RANK_V_BYE, V_BYE_dims, &V_BYE_id);
   check_err(stat,__LINE__,__FILE__);

   V_BTXS_dims[0] = Time_dim;
   V_BTXS_dims[1] = bdy_width_dim;
   V_BTXS_dims[2] = bottom_top_dim;
   V_BTXS_dims[3] = south_north_stag_dim;
   stat = nc_def_var(ncid, "V_BTXS", NC_FLOAT, RANK_V_BTXS, V_BTXS_dims, &V_BTXS_id);
   check_err(stat,__LINE__,__FILE__);

   V_BTXE_dims[0] = Time_dim;
   V_BTXE_dims[1] = bdy_width_dim;
   V_BTXE_dims[2] = bottom_top_dim;
   V_BTXE_dims[3] = south_north_stag_dim;
   stat = nc_def_var(ncid, "V_BTXE", NC_FLOAT, RANK_V_BTXE, V_BTXE_dims, &V_BTXE_id);
   check_err(stat,__LINE__,__FILE__);

   V_BTYS_dims[0] = Time_dim;
   V_BTYS_dims[1] = bdy_width_dim;
   V_BTYS_dims[2] = bottom_top_dim;
   V_BTYS_dims[3] = west_east_dim;
   stat = nc_def_var(ncid, "V_BTYS", NC_FLOAT, RANK_V_BTYS, V_BTYS_dims, &V_BTYS_id);
   check_err(stat,__LINE__,__FILE__);

   V_BTYE_dims[0] = Time_dim;
   V_BTYE_dims[1] = bdy_width_dim;
   V_BTYE_dims[2] = bottom_top_dim;
   V_BTYE_dims[3] = west_east_dim;
   stat = nc_def_var(ncid, "V_BTYE", NC_FLOAT, RANK_V_BTYE, V_BTYE_dims, &V_BTYE_id);
   check_err(stat,__LINE__,__FILE__);

   W_BXS_dims[0] = Time_dim;
   W_BXS_dims[1] = bdy_width_dim;
   W_BXS_dims[2] = bottom_top_stag_dim;
   W_BXS_dims[3] = south_north_dim;
   stat = nc_def_var(ncid, "W_BXS", NC_FLOAT, RANK_W_BXS, W_BXS_dims, &W_BXS_id);
   check_err(stat,__LINE__,__FILE__);

   W_BXE_dims[0] = Time_dim;
   W_BXE_dims[1] = bdy_width_dim;
   W_BXE_dims[2] = bottom_top_stag_dim;
   W_BXE_dims[3] = south_north_dim;
   stat = nc_def_var(ncid, "W_BXE", NC_FLOAT, RANK_W_BXE, W_BXE_dims, &W_BXE_id);
   check_err(stat,__LINE__,__FILE__);

   W_BYS_dims[0] = Time_dim;
   W_BYS_dims[1] = bdy_width_dim;
   W_BYS_dims[2] = bottom_top_stag_dim;
   W_BYS_dims[3] = west_east_dim;
   stat = nc_def_var(ncid, "W_BYS", NC_FLOAT, RANK_W_BYS, W_BYS_dims, &W_BYS_id);
   check_err(stat,__LINE__,__FILE__);

   W_BYE_dims[0] = Time_dim;
   W_BYE_dims[1] = bdy_width_dim;
   W_BYE_dims[2] = bottom_top_stag_dim;
   W_BYE_dims[3] = west_east_dim;
   stat = nc_def_var(ncid, "W_BYE", NC_FLOAT, RANK_W_BYE, W_BYE_dims, &W_BYE_id);
   check_err(stat,__LINE__,__FILE__);

   W_BTXS_dims[0] = Time_dim;
   W_BTXS_dims[1] = bdy_width_dim;
   W_BTXS_dims[2] = bottom_top_stag_dim;
   W_BTXS_dims[3] = south_north_dim;
   stat = nc_def_var(ncid, "W_BTXS", NC_FLOAT, RANK_W_BTXS, W_BTXS_dims, &W_BTXS_id);
   check_err(stat,__LINE__,__FILE__);

   W_BTXE_dims[0] = Time_dim;
   W_BTXE_dims[1] = bdy_width_dim;
   W_BTXE_dims[2] = bottom_top_stag_dim;
   W_BTXE_dims[3] = south_north_dim;
   stat = nc_def_var(ncid, "W_BTXE", NC_FLOAT, RANK_W_BTXE, W_BTXE_dims, &W_BTXE_id);
   check_err(stat,__LINE__,__FILE__);

   W_BTYS_dims[0] = Time_dim;
   W_BTYS_dims[1] = bdy_width_dim;
   W_BTYS_dims[2] = bottom_top_stag_dim;
   W_BTYS_dims[3] = west_east_dim;
   stat = nc_def_var(ncid, "W_BTYS", NC_FLOAT, RANK_W_BTYS, W_BTYS_dims, &W_BTYS_id);
   check_err(stat,__LINE__,__FILE__);

   W_BTYE_dims[0] = Time_dim;
   W_BTYE_dims[1] = bdy_width_dim;
   W_BTYE_dims[2] = bottom_top_stag_dim;
   W_BTYE_dims[3] = west_east_dim;
   stat = nc_def_var(ncid, "W_BTYE", NC_FLOAT, RANK_W_BTYE, W_BTYE_dims, &W_BTYE_id);
   check_err(stat,__LINE__,__FILE__);

   PH_BXS_dims[0] = Time_dim;
   PH_BXS_dims[1] = bdy_width_dim;
   PH_BXS_dims[2] = bottom_top_stag_dim;
   PH_BXS_dims[3] = south_north_dim;
   stat = nc_def_var(ncid, "PH_BXS", NC_FLOAT, RANK_PH_BXS, PH_BXS_dims, &PH_BXS_id);
   check_err(stat,__LINE__,__FILE__);

   PH_BXE_dims[0] = Time_dim;
   PH_BXE_dims[1] = bdy_width_dim;
   PH_BXE_dims[2] = bottom_top_stag_dim;
   PH_BXE_dims[3] = south_north_dim;
   stat = nc_def_var(ncid, "PH_BXE", NC_FLOAT, RANK_PH_BXE, PH_BXE_dims, &PH_BXE_id);
   check_err(stat,__LINE__,__FILE__);

   PH_BYS_dims[0] = Time_dim;
   PH_BYS_dims[1] = bdy_width_dim;
   PH_BYS_dims[2] = bottom_top_stag_dim;
   PH_BYS_dims[3] = west_east_dim;
   stat = nc_def_var(ncid, "PH_BYS", NC_FLOAT, RANK_PH_BYS, PH_BYS_dims, &PH_BYS_id);
   check_err(stat,__LINE__,__FILE__);

   PH_BYE_dims[0] = Time_dim;
   PH_BYE_dims[1] = bdy_width_dim;
   PH_BYE_dims[2] = bottom_top_stag_dim;
   PH_BYE_dims[3] = west_east_dim;
   stat = nc_def_var(ncid, "PH_BYE", NC_FLOAT, RANK_PH_BYE, PH_BYE_dims, &PH_BYE_id);
   check_err(stat,__LINE__,__FILE__);

   PH_BTXS_dims[0] = Time_dim;
   PH_BTXS_dims[1] = bdy_width_dim;
   PH_BTXS_dims[2] = bottom_top_stag_dim;
   PH_BTXS_dims[3] = south_north_dim;
   stat = nc_def_var(ncid, "PH_BTXS", NC_FLOAT, RANK_PH_BTXS, PH_BTXS_dims, &PH_BTXS_id);
   check_err(stat,__LINE__,__FILE__);

   PH_BTXE_dims[0] = Time_dim;
   PH_BTXE_dims[1] = bdy_width_dim;
   PH_BTXE_dims[2] = bottom_top_stag_dim;
   PH_BTXE_dims[3] = south_north_dim;
   stat = nc_def_var(ncid, "PH_BTXE", NC_FLOAT, RANK_PH_BTXE, PH_BTXE_dims, &PH_BTXE_id);
   check_err(stat,__LINE__,__FILE__);

   PH_BTYS_dims[0] = Time_dim;
   PH_BTYS_dims[1] = bdy_width_dim;
   PH_BTYS_dims[2] = bottom_top_stag_dim;
   PH_BTYS_dims[3] = west_east_dim;
   stat = nc_def_var(ncid, "PH_BTYS", NC_FLOAT, RANK_PH_BTYS, PH_BTYS_dims, &PH_BTYS_id);
   check_err(stat,__LINE__,__FILE__);

   PH_BTYE_dims[0] = Time_dim;
   PH_BTYE_dims[1] = bdy_width_dim;
   PH_BTYE_dims[2] = bottom_top_stag_dim;
   PH_BTYE_dims[3] = west_east_dim;
   stat = nc_def_var(ncid, "PH_BTYE", NC_FLOAT, RANK_PH_BTYE, PH_BTYE_dims, &PH_BTYE_id);
   check_err(stat,__LINE__,__FILE__);

   T_BXS_dims[0] = Time_dim;
   T_BXS_dims[1] = bdy_width_dim;
   T_BXS_dims[2] = bottom_top_dim;
   T_BXS_dims[3] = south_north_dim;
   stat = nc_def_var(ncid, "T_BXS", NC_FLOAT, RANK_T_BXS, T_BXS_dims, &T_BXS_id);
   check_err(stat,__LINE__,__FILE__);

   T_BXE_dims[0] = Time_dim;
   T_BXE_dims[1] = bdy_width_dim;
   T_BXE_dims[2] = bottom_top_dim;
   T_BXE_dims[3] = south_north_dim;
   stat = nc_def_var(ncid, "T_BXE", NC_FLOAT, RANK_T_BXE, T_BXE_dims, &T_BXE_id);
   check_err(stat,__LINE__,__FILE__);

   T_BYS_dims[0] = Time_dim;
   T_BYS_dims[1] = bdy_width_dim;
   T_BYS_dims[2] = bottom_top_dim;
   T_BYS_dims[3] = west_east_dim;
   stat = nc_def_var(ncid, "T_BYS", NC_FLOAT, RANK_T_BYS, T_BYS_dims, &T_BYS_id);
   check_err(stat,__LINE__,__FILE__);

   T_BYE_dims[0] = Time_dim;
   T_BYE_dims[1] = bdy_width_dim;
   T_BYE_dims[2] = bottom_top_dim;
   T_BYE_dims[3] = west_east_dim;
   stat = nc_def_var(ncid, "T_BYE", NC_FLOAT, RANK_T_BYE, T_BYE_dims, &T_BYE_id);
   check_err(stat,__LINE__,__FILE__);

   T_BTXS_dims[0] = Time_dim;
   T_BTXS_dims[1] = bdy_width_dim;
   T_BTXS_dims[2] = bottom_top_dim;
   T_BTXS_dims[3] = south_north_dim;
   stat = nc_def_var(ncid, "T_BTXS", NC_FLOAT, RANK_T_BTXS, T_BTXS_dims, &T_BTXS_id);
   check_err(stat,__LINE__,__FILE__);

   T_BTXE_dims[0] = Time_dim;
   T_BTXE_dims[1] = bdy_width_dim;
   T_BTXE_dims[2] = bottom_top_dim;
   T_BTXE_dims[3] = south_north_dim;
   stat = nc_def_var(ncid, "T_BTXE", NC_FLOAT, RANK_T_BTXE, T_BTXE_dims, &T_BTXE_id);
   check_err(stat,__LINE__,__FILE__);

   T_BTYS_dims[0] = Time_dim;
   T_BTYS_dims[1] = bdy_width_dim;
   T_BTYS_dims[2] = bottom_top_dim;
   T_BTYS_dims[3] = west_east_dim;
   stat = nc_def_var(ncid, "T_BTYS", NC_FLOAT, RANK_T_BTYS, T_BTYS_dims, &T_BTYS_id);
   check_err(stat,__LINE__,__FILE__);

   T_BTYE_dims[0] = Time_dim;
   T_BTYE_dims[1] = bdy_width_dim;
   T_BTYE_dims[2] = bottom_top_dim;
   T_BTYE_dims[3] = west_east_dim;
   stat = nc_def_var(ncid, "T_BTYE", NC_FLOAT, RANK_T_BTYE, T_BTYE_dims, &T_BTYE_id);
   check_err(stat,__LINE__,__FILE__);

   MU_BXS_dims[0] = Time_dim;
   MU_BXS_dims[1] = bdy_width_dim;
   MU_BXS_dims[2] = south_north_dim;
   stat = nc_def_var(ncid, "MU_BXS", NC_FLOAT, RANK_MU_BXS, MU_BXS_dims, &MU_BXS_id);
   check_err(stat,__LINE__,__FILE__);

   MU_BXE_dims[0] = Time_dim;
   MU_BXE_dims[1] = bdy_width_dim;
   MU_BXE_dims[2] = south_north_dim;
   stat = nc_def_var(ncid, "MU_BXE", NC_FLOAT, RANK_MU_BXE, MU_BXE_dims, &MU_BXE_id);
   check_err(stat,__LINE__,__FILE__);

   MU_BYS_dims[0] = Time_dim;
   MU_BYS_dims[1] = bdy_width_dim;
   MU_BYS_dims[2] = west_east_dim;
   stat = nc_def_var(ncid, "MU_BYS", NC_FLOAT, RANK_MU_BYS, MU_BYS_dims, &MU_BYS_id);
   check_err(stat,__LINE__,__FILE__);

   MU_BYE_dims[0] = Time_dim;
   MU_BYE_dims[1] = bdy_width_dim;
   MU_BYE_dims[2] = west_east_dim;
   stat = nc_def_var(ncid, "MU_BYE", NC_FLOAT, RANK_MU_BYE, MU_BYE_dims, &MU_BYE_id);
   check_err(stat,__LINE__,__FILE__);

   MU_BTXS_dims[0] = Time_dim;
   MU_BTXS_dims[1] = bdy_width_dim;
   MU_BTXS_dims[2] = south_north_dim;
   stat = nc_def_var(ncid, "MU_BTXS", NC_FLOAT, RANK_MU_BTXS, MU_BTXS_dims, &MU_BTXS_id);
   check_err(stat,__LINE__,__FILE__);

   MU_BTXE_dims[0] = Time_dim;
   MU_BTXE_dims[1] = bdy_width_dim;
   MU_BTXE_dims[2] = south_north_dim;
   stat = nc_def_var(ncid, "MU_BTXE", NC_FLOAT, RANK_MU_BTXE, MU_BTXE_dims, &MU_BTXE_id);
   check_err(stat,__LINE__,__FILE__);

   MU_BTYS_dims[0] = Time_dim;
   MU_BTYS_dims[1] = bdy_width_dim;
   MU_BTYS_dims[2] = west_east_dim;
   stat = nc_def_var(ncid, "MU_BTYS", NC_FLOAT, RANK_MU_BTYS, MU_BTYS_dims, &MU_BTYS_id);
   check_err(stat,__LINE__,__FILE__);

   MU_BTYE_dims[0] = Time_dim;
   MU_BTYE_dims[1] = bdy_width_dim;
   MU_BTYE_dims[2] = west_east_dim;
   stat = nc_def_var(ncid, "MU_BTYE", NC_FLOAT, RANK_MU_BTYE, MU_BTYE_dims, &MU_BTYE_id);
   check_err(stat,__LINE__,__FILE__);

   QVAPOR_BXS_dims[0] = Time_dim;
   QVAPOR_BXS_dims[1] = bdy_width_dim;
   QVAPOR_BXS_dims[2] = bottom_top_dim;
   QVAPOR_BXS_dims[3] = south_north_dim;
   stat = nc_def_var(ncid, "QVAPOR_BXS", NC_FLOAT, RANK_QVAPOR_BXS, QVAPOR_BXS_dims, &QVAPOR_BXS_id);
   check_err(stat,__LINE__,__FILE__);

   QVAPOR_BXE_dims[0] = Time_dim;
   QVAPOR_BXE_dims[1] = bdy_width_dim;
   QVAPOR_BXE_dims[2] = bottom_top_dim;
   QVAPOR_BXE_dims[3] = south_north_dim;
   stat = nc_def_var(ncid, "QVAPOR_BXE", NC_FLOAT, RANK_QVAPOR_BXE, QVAPOR_BXE_dims, &QVAPOR_BXE_id);
   check_err(stat,__LINE__,__FILE__);

   QVAPOR_BYS_dims[0] = Time_dim;
   QVAPOR_BYS_dims[1] = bdy_width_dim;
   QVAPOR_BYS_dims[2] = bottom_top_dim;
   QVAPOR_BYS_dims[3] = west_east_dim;
   stat = nc_def_var(ncid, "QVAPOR_BYS", NC_FLOAT, RANK_QVAPOR_BYS, QVAPOR_BYS_dims, &QVAPOR_BYS_id);
   check_err(stat,__LINE__,__FILE__);

   QVAPOR_BYE_dims[0] = Time_dim;
   QVAPOR_BYE_dims[1] = bdy_width_dim;
   QVAPOR_BYE_dims[2] = bottom_top_dim;
   QVAPOR_BYE_dims[3] = west_east_dim;
   stat = nc_def_var(ncid, "QVAPOR_BYE", NC_FLOAT, RANK_QVAPOR_BYE, QVAPOR_BYE_dims, &QVAPOR_BYE_id);
   check_err(stat,__LINE__,__FILE__);

   QCLOUD_BXS_dims[0] = Time_dim;
   QCLOUD_BXS_dims[1] = bdy_width_dim;
   QCLOUD_BXS_dims[2] = bottom_top_dim;
   QCLOUD_BXS_dims[3] = south_north_dim;
   stat = nc_def_var(ncid, "QCLOUD_BXS", NC_FLOAT, RANK_QCLOUD_BXS, QCLOUD_BXS_dims, &QCLOUD_BXS_id);
   check_err(stat,__LINE__,__FILE__);

   QCLOUD_BXE_dims[0] = Time_dim;
   QCLOUD_BXE_dims[1] = bdy_width_dim;
   QCLOUD_BXE_dims[2] = bottom_top_dim;
   QCLOUD_BXE_dims[3] = south_north_dim;
   stat = nc_def_var(ncid, "QCLOUD_BXE", NC_FLOAT, RANK_QCLOUD_BXE, QCLOUD_BXE_dims, &QCLOUD_BXE_id);
   check_err(stat,__LINE__,__FILE__);

   QCLOUD_BYS_dims[0] = Time_dim;
   QCLOUD_BYS_dims[1] = bdy_width_dim;
   QCLOUD_BYS_dims[2] = bottom_top_dim;
   QCLOUD_BYS_dims[3] = west_east_dim;
   stat = nc_def_var(ncid, "QCLOUD_BYS", NC_FLOAT, RANK_QCLOUD_BYS, QCLOUD_BYS_dims, &QCLOUD_BYS_id);
   check_err(stat,__LINE__,__FILE__);

   QCLOUD_BYE_dims[0] = Time_dim;
   QCLOUD_BYE_dims[1] = bdy_width_dim;
   QCLOUD_BYE_dims[2] = bottom_top_dim;
   QCLOUD_BYE_dims[3] = west_east_dim;
   stat = nc_def_var(ncid, "QCLOUD_BYE", NC_FLOAT, RANK_QCLOUD_BYE, QCLOUD_BYE_dims, &QCLOUD_BYE_id);
   check_err(stat,__LINE__,__FILE__);

   QRAIN_BXS_dims[0] = Time_dim;
   QRAIN_BXS_dims[1] = bdy_width_dim;
   QRAIN_BXS_dims[2] = bottom_top_dim;
   QRAIN_BXS_dims[3] = south_north_dim;
   stat = nc_def_var(ncid, "QRAIN_BXS", NC_FLOAT, RANK_QRAIN_BXS, QRAIN_BXS_dims, &QRAIN_BXS_id);
   check_err(stat,__LINE__,__FILE__);

   QRAIN_BXE_dims[0] = Time_dim;
   QRAIN_BXE_dims[1] = bdy_width_dim;
   QRAIN_BXE_dims[2] = bottom_top_dim;
   QRAIN_BXE_dims[3] = south_north_dim;
   stat = nc_def_var(ncid, "QRAIN_BXE", NC_FLOAT, RANK_QRAIN_BXE, QRAIN_BXE_dims, &QRAIN_BXE_id);
   check_err(stat,__LINE__,__FILE__);

   QRAIN_BYS_dims[0] = Time_dim;
   QRAIN_BYS_dims[1] = bdy_width_dim;
   QRAIN_BYS_dims[2] = bottom_top_dim;
   QRAIN_BYS_dims[3] = west_east_dim;
   stat = nc_def_var(ncid, "QRAIN_BYS", NC_FLOAT, RANK_QRAIN_BYS, QRAIN_BYS_dims, &QRAIN_BYS_id);
   check_err(stat,__LINE__,__FILE__);

   QRAIN_BYE_dims[0] = Time_dim;
   QRAIN_BYE_dims[1] = bdy_width_dim;
   QRAIN_BYE_dims[2] = bottom_top_dim;
   QRAIN_BYE_dims[3] = west_east_dim;
   stat = nc_def_var(ncid, "QRAIN_BYE", NC_FLOAT, RANK_QRAIN_BYE, QRAIN_BYE_dims, &QRAIN_BYE_id);
   check_err(stat,__LINE__,__FILE__);

   QICE_BXS_dims[0] = Time_dim;
   QICE_BXS_dims[1] = bdy_width_dim;
   QICE_BXS_dims[2] = bottom_top_dim;
   QICE_BXS_dims[3] = south_north_dim;
   stat = nc_def_var(ncid, "QICE_BXS", NC_FLOAT, RANK_QICE_BXS, QICE_BXS_dims, &QICE_BXS_id);
   check_err(stat,__LINE__,__FILE__);

   QICE_BXE_dims[0] = Time_dim;
   QICE_BXE_dims[1] = bdy_width_dim;
   QICE_BXE_dims[2] = bottom_top_dim;
   QICE_BXE_dims[3] = south_north_dim;
   stat = nc_def_var(ncid, "QICE_BXE", NC_FLOAT, RANK_QICE_BXE, QICE_BXE_dims, &QICE_BXE_id);
   check_err(stat,__LINE__,__FILE__);

   QICE_BYS_dims[0] = Time_dim;
   QICE_BYS_dims[1] = bdy_width_dim;
   QICE_BYS_dims[2] = bottom_top_dim;
   QICE_BYS_dims[3] = west_east_dim;
   stat = nc_def_var(ncid, "QICE_BYS", NC_FLOAT, RANK_QICE_BYS, QICE_BYS_dims, &QICE_BYS_id);
   check_err(stat,__LINE__,__FILE__);

   QICE_BYE_dims[0] = Time_dim;
   QICE_BYE_dims[1] = bdy_width_dim;
   QICE_BYE_dims[2] = bottom_top_dim;
   QICE_BYE_dims[3] = west_east_dim;
   stat = nc_def_var(ncid, "QICE_BYE", NC_FLOAT, RANK_QICE_BYE, QICE_BYE_dims, &QICE_BYE_id);
   check_err(stat,__LINE__,__FILE__);

   QSNOW_BXS_dims[0] = Time_dim;
   QSNOW_BXS_dims[1] = bdy_width_dim;
   QSNOW_BXS_dims[2] = bottom_top_dim;
   QSNOW_BXS_dims[3] = south_north_dim;
   stat = nc_def_var(ncid, "QSNOW_BXS", NC_FLOAT, RANK_QSNOW_BXS, QSNOW_BXS_dims, &QSNOW_BXS_id);
   check_err(stat,__LINE__,__FILE__);

   QSNOW_BXE_dims[0] = Time_dim;
   QSNOW_BXE_dims[1] = bdy_width_dim;
   QSNOW_BXE_dims[2] = bottom_top_dim;
   QSNOW_BXE_dims[3] = south_north_dim;
   stat = nc_def_var(ncid, "QSNOW_BXE", NC_FLOAT, RANK_QSNOW_BXE, QSNOW_BXE_dims, &QSNOW_BXE_id);
   check_err(stat,__LINE__,__FILE__);

   QSNOW_BYS_dims[0] = Time_dim;
   QSNOW_BYS_dims[1] = bdy_width_dim;
   QSNOW_BYS_dims[2] = bottom_top_dim;
   QSNOW_BYS_dims[3] = west_east_dim;
   stat = nc_def_var(ncid, "QSNOW_BYS", NC_FLOAT, RANK_QSNOW_BYS, QSNOW_BYS_dims, &QSNOW_BYS_id);
   check_err(stat,__LINE__,__FILE__);

   QSNOW_BYE_dims[0] = Time_dim;
   QSNOW_BYE_dims[1] = bdy_width_dim;
   QSNOW_BYE_dims[2] = bottom_top_dim;
   QSNOW_BYE_dims[3] = west_east_dim;
   stat = nc_def_var(ncid, "QSNOW_BYE", NC_FLOAT, RANK_QSNOW_BYE, QSNOW_BYE_dims, &QSNOW_BYE_id);
   check_err(stat,__LINE__,__FILE__);

   QGRAUP_BXS_dims[0] = Time_dim;
   QGRAUP_BXS_dims[1] = bdy_width_dim;
   QGRAUP_BXS_dims[2] = bottom_top_dim;
   QGRAUP_BXS_dims[3] = south_north_dim;
   stat = nc_def_var(ncid, "QGRAUP_BXS", NC_FLOAT, RANK_QGRAUP_BXS, QGRAUP_BXS_dims, &QGRAUP_BXS_id);
   check_err(stat,__LINE__,__FILE__);

   QGRAUP_BXE_dims[0] = Time_dim;
   QGRAUP_BXE_dims[1] = bdy_width_dim;
   QGRAUP_BXE_dims[2] = bottom_top_dim;
   QGRAUP_BXE_dims[3] = south_north_dim;
   stat = nc_def_var(ncid, "QGRAUP_BXE", NC_FLOAT, RANK_QGRAUP_BXE, QGRAUP_BXE_dims, &QGRAUP_BXE_id);
   check_err(stat,__LINE__,__FILE__);

   QGRAUP_BYS_dims[0] = Time_dim;
   QGRAUP_BYS_dims[1] = bdy_width_dim;
   QGRAUP_BYS_dims[2] = bottom_top_dim;
   QGRAUP_BYS_dims[3] = west_east_dim;
   stat = nc_def_var(ncid, "QGRAUP_BYS", NC_FLOAT, RANK_QGRAUP_BYS, QGRAUP_BYS_dims, &QGRAUP_BYS_id);
   check_err(stat,__LINE__,__FILE__);

   QGRAUP_BYE_dims[0] = Time_dim;
   QGRAUP_BYE_dims[1] = bdy_width_dim;
   QGRAUP_BYE_dims[2] = bottom_top_dim;
   QGRAUP_BYE_dims[3] = west_east_dim;
   stat = nc_def_var(ncid, "QGRAUP_BYE", NC_FLOAT, RANK_QGRAUP_BYE, QGRAUP_BYE_dims, &QGRAUP_BYE_id);
   check_err(stat,__LINE__,__FILE__);

   QVAPOR_BTXS_dims[0] = Time_dim;
   QVAPOR_BTXS_dims[1] = bdy_width_dim;
   QVAPOR_BTXS_dims[2] = bottom_top_dim;
   QVAPOR_BTXS_dims[3] = south_north_dim;
   stat = nc_def_var(ncid, "QVAPOR_BTXS", NC_FLOAT, RANK_QVAPOR_BTXS, QVAPOR_BTXS_dims, &QVAPOR_BTXS_id);
   check_err(stat,__LINE__,__FILE__);

   QVAPOR_BTXE_dims[0] = Time_dim;
   QVAPOR_BTXE_dims[1] = bdy_width_dim;
   QVAPOR_BTXE_dims[2] = bottom_top_dim;
   QVAPOR_BTXE_dims[3] = south_north_dim;
   stat = nc_def_var(ncid, "QVAPOR_BTXE", NC_FLOAT, RANK_QVAPOR_BTXE, QVAPOR_BTXE_dims, &QVAPOR_BTXE_id);
   check_err(stat,__LINE__,__FILE__);

   QVAPOR_BTYS_dims[0] = Time_dim;
   QVAPOR_BTYS_dims[1] = bdy_width_dim;
   QVAPOR_BTYS_dims[2] = bottom_top_dim;
   QVAPOR_BTYS_dims[3] = west_east_dim;
   stat = nc_def_var(ncid, "QVAPOR_BTYS", NC_FLOAT, RANK_QVAPOR_BTYS, QVAPOR_BTYS_dims, &QVAPOR_BTYS_id);
   check_err(stat,__LINE__,__FILE__);

   QVAPOR_BTYE_dims[0] = Time_dim;
   QVAPOR_BTYE_dims[1] = bdy_width_dim;
   QVAPOR_BTYE_dims[2] = bottom_top_dim;
   QVAPOR_BTYE_dims[3] = west_east_dim;
   stat = nc_def_var(ncid, "QVAPOR_BTYE", NC_FLOAT, RANK_QVAPOR_BTYE, QVAPOR_BTYE_dims, &QVAPOR_BTYE_id);
   check_err(stat,__LINE__,__FILE__);

   QCLOUD_BTXS_dims[0] = Time_dim;
   QCLOUD_BTXS_dims[1] = bdy_width_dim;
   QCLOUD_BTXS_dims[2] = bottom_top_dim;
   QCLOUD_BTXS_dims[3] = south_north_dim;
   stat = nc_def_var(ncid, "QCLOUD_BTXS", NC_FLOAT, RANK_QCLOUD_BTXS, QCLOUD_BTXS_dims, &QCLOUD_BTXS_id);
   check_err(stat,__LINE__,__FILE__);

   QCLOUD_BTXE_dims[0] = Time_dim;
   QCLOUD_BTXE_dims[1] = bdy_width_dim;
   QCLOUD_BTXE_dims[2] = bottom_top_dim;
   QCLOUD_BTXE_dims[3] = south_north_dim;
   stat = nc_def_var(ncid, "QCLOUD_BTXE", NC_FLOAT, RANK_QCLOUD_BTXE, QCLOUD_BTXE_dims, &QCLOUD_BTXE_id);
   check_err(stat,__LINE__,__FILE__);

   QCLOUD_BTYS_dims[0] = Time_dim;
   QCLOUD_BTYS_dims[1] = bdy_width_dim;
   QCLOUD_BTYS_dims[2] = bottom_top_dim;
   QCLOUD_BTYS_dims[3] = west_east_dim;
   stat = nc_def_var(ncid, "QCLOUD_BTYS", NC_FLOAT, RANK_QCLOUD_BTYS, QCLOUD_BTYS_dims, &QCLOUD_BTYS_id);
   check_err(stat,__LINE__,__FILE__);

   QCLOUD_BTYE_dims[0] = Time_dim;
   QCLOUD_BTYE_dims[1] = bdy_width_dim;
   QCLOUD_BTYE_dims[2] = bottom_top_dim;
   QCLOUD_BTYE_dims[3] = west_east_dim;
   stat = nc_def_var(ncid, "QCLOUD_BTYE", NC_FLOAT, RANK_QCLOUD_BTYE, QCLOUD_BTYE_dims, &QCLOUD_BTYE_id);
   check_err(stat,__LINE__,__FILE__);

   QRAIN_BTXS_dims[0] = Time_dim;
   QRAIN_BTXS_dims[1] = bdy_width_dim;
   QRAIN_BTXS_dims[2] = bottom_top_dim;
   QRAIN_BTXS_dims[3] = south_north_dim;
   stat = nc_def_var(ncid, "QRAIN_BTXS", NC_FLOAT, RANK_QRAIN_BTXS, QRAIN_BTXS_dims, &QRAIN_BTXS_id);
   check_err(stat,__LINE__,__FILE__);

   QRAIN_BTXE_dims[0] = Time_dim;
   QRAIN_BTXE_dims[1] = bdy_width_dim;
   QRAIN_BTXE_dims[2] = bottom_top_dim;
   QRAIN_BTXE_dims[3] = south_north_dim;
   stat = nc_def_var(ncid, "QRAIN_BTXE", NC_FLOAT, RANK_QRAIN_BTXE, QRAIN_BTXE_dims, &QRAIN_BTXE_id);
   check_err(stat,__LINE__,__FILE__);

   QRAIN_BTYS_dims[0] = Time_dim;
   QRAIN_BTYS_dims[1] = bdy_width_dim;
   QRAIN_BTYS_dims[2] = bottom_top_dim;
   QRAIN_BTYS_dims[3] = west_east_dim;
   stat = nc_def_var(ncid, "QRAIN_BTYS", NC_FLOAT, RANK_QRAIN_BTYS, QRAIN_BTYS_dims, &QRAIN_BTYS_id);
   check_err(stat,__LINE__,__FILE__);

   QRAIN_BTYE_dims[0] = Time_dim;
   QRAIN_BTYE_dims[1] = bdy_width_dim;
   QRAIN_BTYE_dims[2] = bottom_top_dim;
   QRAIN_BTYE_dims[3] = west_east_dim;
   stat = nc_def_var(ncid, "QRAIN_BTYE", NC_FLOAT, RANK_QRAIN_BTYE, QRAIN_BTYE_dims, &QRAIN_BTYE_id);
   check_err(stat,__LINE__,__FILE__);

   QICE_BTXS_dims[0] = Time_dim;
   QICE_BTXS_dims[1] = bdy_width_dim;
   QICE_BTXS_dims[2] = bottom_top_dim;
   QICE_BTXS_dims[3] = south_north_dim;
   stat = nc_def_var(ncid, "QICE_BTXS", NC_FLOAT, RANK_QICE_BTXS, QICE_BTXS_dims, &QICE_BTXS_id);
   check_err(stat,__LINE__,__FILE__);

   QICE_BTXE_dims[0] = Time_dim;
   QICE_BTXE_dims[1] = bdy_width_dim;
   QICE_BTXE_dims[2] = bottom_top_dim;
   QICE_BTXE_dims[3] = south_north_dim;
   stat = nc_def_var(ncid, "QICE_BTXE", NC_FLOAT, RANK_QICE_BTXE, QICE_BTXE_dims, &QICE_BTXE_id);
   check_err(stat,__LINE__,__FILE__);

   QICE_BTYS_dims[0] = Time_dim;
   QICE_BTYS_dims[1] = bdy_width_dim;
   QICE_BTYS_dims[2] = bottom_top_dim;
   QICE_BTYS_dims[3] = west_east_dim;
   stat = nc_def_var(ncid, "QICE_BTYS", NC_FLOAT, RANK_QICE_BTYS, QICE_BTYS_dims, &QICE_BTYS_id);
   check_err(stat,__LINE__,__FILE__);

   QICE_BTYE_dims[0] = Time_dim;
   QICE_BTYE_dims[1] = bdy_width_dim;
   QICE_BTYE_dims[2] = bottom_top_dim;
   QICE_BTYE_dims[3] = west_east_dim;
   stat = nc_def_var(ncid, "QICE_BTYE", NC_FLOAT, RANK_QICE_BTYE, QICE_BTYE_dims, &QICE_BTYE_id);
   check_err(stat,__LINE__,__FILE__);

   QSNOW_BTXS_dims[0] = Time_dim;
   QSNOW_BTXS_dims[1] = bdy_width_dim;
   QSNOW_BTXS_dims[2] = bottom_top_dim;
   QSNOW_BTXS_dims[3] = south_north_dim;
   stat = nc_def_var(ncid, "QSNOW_BTXS", NC_FLOAT, RANK_QSNOW_BTXS, QSNOW_BTXS_dims, &QSNOW_BTXS_id);
   check_err(stat,__LINE__,__FILE__);

   QSNOW_BTXE_dims[0] = Time_dim;
   QSNOW_BTXE_dims[1] = bdy_width_dim;
   QSNOW_BTXE_dims[2] = bottom_top_dim;
   QSNOW_BTXE_dims[3] = south_north_dim;
   stat = nc_def_var(ncid, "QSNOW_BTXE", NC_FLOAT, RANK_QSNOW_BTXE, QSNOW_BTXE_dims, &QSNOW_BTXE_id);
   check_err(stat,__LINE__,__FILE__);

   QSNOW_BTYS_dims[0] = Time_dim;
   QSNOW_BTYS_dims[1] = bdy_width_dim;
   QSNOW_BTYS_dims[2] = bottom_top_dim;
   QSNOW_BTYS_dims[3] = west_east_dim;
   stat = nc_def_var(ncid, "QSNOW_BTYS", NC_FLOAT, RANK_QSNOW_BTYS, QSNOW_BTYS_dims, &QSNOW_BTYS_id);
   check_err(stat,__LINE__,__FILE__);

   QSNOW_BTYE_dims[0] = Time_dim;
   QSNOW_BTYE_dims[1] = bdy_width_dim;
   QSNOW_BTYE_dims[2] = bottom_top_dim;
   QSNOW_BTYE_dims[3] = west_east_dim;
   stat = nc_def_var(ncid, "QSNOW_BTYE", NC_FLOAT, RANK_QSNOW_BTYE, QSNOW_BTYE_dims, &QSNOW_BTYE_id);
   check_err(stat,__LINE__,__FILE__);

   QGRAUP_BTXS_dims[0] = Time_dim;
   QGRAUP_BTXS_dims[1] = bdy_width_dim;
   QGRAUP_BTXS_dims[2] = bottom_top_dim;
   QGRAUP_BTXS_dims[3] = south_north_dim;
   stat = nc_def_var(ncid, "QGRAUP_BTXS", NC_FLOAT, RANK_QGRAUP_BTXS, QGRAUP_BTXS_dims, &QGRAUP_BTXS_id);
   check_err(stat,__LINE__,__FILE__);

   QGRAUP_BTXE_dims[0] = Time_dim;
   QGRAUP_BTXE_dims[1] = bdy_width_dim;
   QGRAUP_BTXE_dims[2] = bottom_top_dim;
   QGRAUP_BTXE_dims[3] = south_north_dim;
   stat = nc_def_var(ncid, "QGRAUP_BTXE", NC_FLOAT, RANK_QGRAUP_BTXE, QGRAUP_BTXE_dims, &QGRAUP_BTXE_id);
   check_err(stat,__LINE__,__FILE__);

   QGRAUP_BTYS_dims[0] = Time_dim;
   QGRAUP_BTYS_dims[1] = bdy_width_dim;
   QGRAUP_BTYS_dims[2] = bottom_top_dim;
   QGRAUP_BTYS_dims[3] = west_east_dim;
   stat = nc_def_var(ncid, "QGRAUP_BTYS", NC_FLOAT, RANK_QGRAUP_BTYS, QGRAUP_BTYS_dims, &QGRAUP_BTYS_id);
   check_err(stat,__LINE__,__FILE__);

   QGRAUP_BTYE_dims[0] = Time_dim;
   QGRAUP_BTYE_dims[1] = bdy_width_dim;
   QGRAUP_BTYE_dims[2] = bottom_top_dim;
   QGRAUP_BTYE_dims[3] = west_east_dim;
   stat = nc_def_var(ncid, "QGRAUP_BTYE", NC_FLOAT, RANK_QGRAUP_BTYE, QGRAUP_BTYE_dims, &QGRAUP_BTYE_id);
   check_err(stat,__LINE__,__FILE__);

   QNICE_BXS_dims[0] = Time_dim;
   QNICE_BXS_dims[1] = bdy_width_dim;
   QNICE_BXS_dims[2] = bottom_top_dim;
   QNICE_BXS_dims[3] = south_north_dim;
   stat = nc_def_var(ncid, "QNICE_BXS", NC_FLOAT, RANK_QNICE_BXS, QNICE_BXS_dims, &QNICE_BXS_id);
   check_err(stat,__LINE__,__FILE__);

   QNICE_BXE_dims[0] = Time_dim;
   QNICE_BXE_dims[1] = bdy_width_dim;
   QNICE_BXE_dims[2] = bottom_top_dim;
   QNICE_BXE_dims[3] = south_north_dim;
   stat = nc_def_var(ncid, "QNICE_BXE", NC_FLOAT, RANK_QNICE_BXE, QNICE_BXE_dims, &QNICE_BXE_id);
   check_err(stat,__LINE__,__FILE__);

   QNICE_BYS_dims[0] = Time_dim;
   QNICE_BYS_dims[1] = bdy_width_dim;
   QNICE_BYS_dims[2] = bottom_top_dim;
   QNICE_BYS_dims[3] = west_east_dim;
   stat = nc_def_var(ncid, "QNICE_BYS", NC_FLOAT, RANK_QNICE_BYS, QNICE_BYS_dims, &QNICE_BYS_id);
   check_err(stat,__LINE__,__FILE__);

   QNICE_BYE_dims[0] = Time_dim;
   QNICE_BYE_dims[1] = bdy_width_dim;
   QNICE_BYE_dims[2] = bottom_top_dim;
   QNICE_BYE_dims[3] = west_east_dim;
   stat = nc_def_var(ncid, "QNICE_BYE", NC_FLOAT, RANK_QNICE_BYE, QNICE_BYE_dims, &QNICE_BYE_id);
   check_err(stat,__LINE__,__FILE__);

   QNRAIN_BXS_dims[0] = Time_dim;
   QNRAIN_BXS_dims[1] = bdy_width_dim;
   QNRAIN_BXS_dims[2] = bottom_top_dim;
   QNRAIN_BXS_dims[3] = south_north_dim;
   stat = nc_def_var(ncid, "QNRAIN_BXS", NC_FLOAT, RANK_QNRAIN_BXS, QNRAIN_BXS_dims, &QNRAIN_BXS_id);
   check_err(stat,__LINE__,__FILE__);

   QNRAIN_BXE_dims[0] = Time_dim;
   QNRAIN_BXE_dims[1] = bdy_width_dim;
   QNRAIN_BXE_dims[2] = bottom_top_dim;
   QNRAIN_BXE_dims[3] = south_north_dim;
   stat = nc_def_var(ncid, "QNRAIN_BXE", NC_FLOAT, RANK_QNRAIN_BXE, QNRAIN_BXE_dims, &QNRAIN_BXE_id);
   check_err(stat,__LINE__,__FILE__);

   QNRAIN_BYS_dims[0] = Time_dim;
   QNRAIN_BYS_dims[1] = bdy_width_dim;
   QNRAIN_BYS_dims[2] = bottom_top_dim;
   QNRAIN_BYS_dims[3] = west_east_dim;
   stat = nc_def_var(ncid, "QNRAIN_BYS", NC_FLOAT, RANK_QNRAIN_BYS, QNRAIN_BYS_dims, &QNRAIN_BYS_id);
   check_err(stat,__LINE__,__FILE__);

   QNRAIN_BYE_dims[0] = Time_dim;
   QNRAIN_BYE_dims[1] = bdy_width_dim;
   QNRAIN_BYE_dims[2] = bottom_top_dim;
   QNRAIN_BYE_dims[3] = west_east_dim;
   stat = nc_def_var(ncid, "QNRAIN_BYE", NC_FLOAT, RANK_QNRAIN_BYE, QNRAIN_BYE_dims, &QNRAIN_BYE_id);
   check_err(stat,__LINE__,__FILE__);

   QNICE_BTXS_dims[0] = Time_dim;
   QNICE_BTXS_dims[1] = bdy_width_dim;
   QNICE_BTXS_dims[2] = bottom_top_dim;
   QNICE_BTXS_dims[3] = south_north_dim;
   stat = nc_def_var(ncid, "QNICE_BTXS", NC_FLOAT, RANK_QNICE_BTXS, QNICE_BTXS_dims, &QNICE_BTXS_id);
   check_err(stat,__LINE__,__FILE__);

   QNICE_BTXE_dims[0] = Time_dim;
   QNICE_BTXE_dims[1] = bdy_width_dim;
   QNICE_BTXE_dims[2] = bottom_top_dim;
   QNICE_BTXE_dims[3] = south_north_dim;
   stat = nc_def_var(ncid, "QNICE_BTXE", NC_FLOAT, RANK_QNICE_BTXE, QNICE_BTXE_dims, &QNICE_BTXE_id);
   check_err(stat,__LINE__,__FILE__);

   QNICE_BTYS_dims[0] = Time_dim;
   QNICE_BTYS_dims[1] = bdy_width_dim;
   QNICE_BTYS_dims[2] = bottom_top_dim;
   QNICE_BTYS_dims[3] = west_east_dim;
   stat = nc_def_var(ncid, "QNICE_BTYS", NC_FLOAT, RANK_QNICE_BTYS, QNICE_BTYS_dims, &QNICE_BTYS_id);
   check_err(stat,__LINE__,__FILE__);

   QNICE_BTYE_dims[0] = Time_dim;
   QNICE_BTYE_dims[1] = bdy_width_dim;
   QNICE_BTYE_dims[2] = bottom_top_dim;
   QNICE_BTYE_dims[3] = west_east_dim;
   stat = nc_def_var(ncid, "QNICE_BTYE", NC_FLOAT, RANK_QNICE_BTYE, QNICE_BTYE_dims, &QNICE_BTYE_id);
   check_err(stat,__LINE__,__FILE__);

   QNRAIN_BTXS_dims[0] = Time_dim;
   QNRAIN_BTXS_dims[1] = bdy_width_dim;
   QNRAIN_BTXS_dims[2] = bottom_top_dim;
   QNRAIN_BTXS_dims[3] = south_north_dim;
   stat = nc_def_var(ncid, "QNRAIN_BTXS", NC_FLOAT, RANK_QNRAIN_BTXS, QNRAIN_BTXS_dims, &QNRAIN_BTXS_id);
   check_err(stat,__LINE__,__FILE__);

   QNRAIN_BTXE_dims[0] = Time_dim;
   QNRAIN_BTXE_dims[1] = bdy_width_dim;
   QNRAIN_BTXE_dims[2] = bottom_top_dim;
   QNRAIN_BTXE_dims[3] = south_north_dim;
   stat = nc_def_var(ncid, "QNRAIN_BTXE", NC_FLOAT, RANK_QNRAIN_BTXE, QNRAIN_BTXE_dims, &QNRAIN_BTXE_id);
   check_err(stat,__LINE__,__FILE__);

   QNRAIN_BTYS_dims[0] = Time_dim;
   QNRAIN_BTYS_dims[1] = bdy_width_dim;
   QNRAIN_BTYS_dims[2] = bottom_top_dim;
   QNRAIN_BTYS_dims[3] = west_east_dim;
   stat = nc_def_var(ncid, "QNRAIN_BTYS", NC_FLOAT, RANK_QNRAIN_BTYS, QNRAIN_BTYS_dims, &QNRAIN_BTYS_id);
   check_err(stat,__LINE__,__FILE__);

   QNRAIN_BTYE_dims[0] = Time_dim;
   QNRAIN_BTYE_dims[1] = bdy_width_dim;
   QNRAIN_BTYE_dims[2] = bottom_top_dim;
   QNRAIN_BTYE_dims[3] = west_east_dim;
   stat = nc_def_var(ncid, "QNRAIN_BTYE", NC_FLOAT, RANK_QNRAIN_BTYE, QNRAIN_BTYE_dims, &QNRAIN_BTYE_id);
   check_err(stat,__LINE__,__FILE__);

   HT_SHAD_BXS_dims[0] = Time_dim;
   HT_SHAD_BXS_dims[1] = bdy_width_dim;
   HT_SHAD_BXS_dims[2] = south_north_dim;
   stat = nc_def_var(ncid, "HT_SHAD_BXS", NC_FLOAT, RANK_HT_SHAD_BXS, HT_SHAD_BXS_dims, &HT_SHAD_BXS_id);
   check_err(stat,__LINE__,__FILE__);

   HT_SHAD_BXE_dims[0] = Time_dim;
   HT_SHAD_BXE_dims[1] = bdy_width_dim;
   HT_SHAD_BXE_dims[2] = south_north_dim;
   stat = nc_def_var(ncid, "HT_SHAD_BXE", NC_FLOAT, RANK_HT_SHAD_BXE, HT_SHAD_BXE_dims, &HT_SHAD_BXE_id);
   check_err(stat,__LINE__,__FILE__);

   HT_SHAD_BYS_dims[0] = Time_dim;
   HT_SHAD_BYS_dims[1] = bdy_width_dim;
   HT_SHAD_BYS_dims[2] = west_east_dim;
   stat = nc_def_var(ncid, "HT_SHAD_BYS", NC_FLOAT, RANK_HT_SHAD_BYS, HT_SHAD_BYS_dims, &HT_SHAD_BYS_id);
   check_err(stat,__LINE__,__FILE__);

   HT_SHAD_BYE_dims[0] = Time_dim;
   HT_SHAD_BYE_dims[1] = bdy_width_dim;
   HT_SHAD_BYE_dims[2] = west_east_dim;
   stat = nc_def_var(ncid, "HT_SHAD_BYE", NC_FLOAT, RANK_HT_SHAD_BYE, HT_SHAD_BYE_dims, &HT_SHAD_BYE_id);
   check_err(stat,__LINE__,__FILE__);

   HT_SHAD_BTXS_dims[0] = Time_dim;
   HT_SHAD_BTXS_dims[1] = bdy_width_dim;
   HT_SHAD_BTXS_dims[2] = south_north_dim;
   stat = nc_def_var(ncid, "HT_SHAD_BTXS", NC_FLOAT, RANK_HT_SHAD_BTXS, HT_SHAD_BTXS_dims, &HT_SHAD_BTXS_id);
   check_err(stat,__LINE__,__FILE__);

   HT_SHAD_BTXE_dims[0] = Time_dim;
   HT_SHAD_BTXE_dims[1] = bdy_width_dim;
   HT_SHAD_BTXE_dims[2] = south_north_dim;
   stat = nc_def_var(ncid, "HT_SHAD_BTXE", NC_FLOAT, RANK_HT_SHAD_BTXE, HT_SHAD_BTXE_dims, &HT_SHAD_BTXE_id);
   check_err(stat,__LINE__,__FILE__);

   HT_SHAD_BTYS_dims[0] = Time_dim;
   HT_SHAD_BTYS_dims[1] = bdy_width_dim;
   HT_SHAD_BTYS_dims[2] = west_east_dim;
   stat = nc_def_var(ncid, "HT_SHAD_BTYS", NC_FLOAT, RANK_HT_SHAD_BTYS, HT_SHAD_BTYS_dims, &HT_SHAD_BTYS_id);
   check_err(stat,__LINE__,__FILE__);

   HT_SHAD_BTYE_dims[0] = Time_dim;
   HT_SHAD_BTYE_dims[1] = bdy_width_dim;
   HT_SHAD_BTYE_dims[2] = west_east_dim;
   stat = nc_def_var(ncid, "HT_SHAD_BTYE", NC_FLOAT, RANK_HT_SHAD_BTYE, HT_SHAD_BTYE_dims, &HT_SHAD_BTYE_id);
   check_err(stat,__LINE__,__FILE__);

   PC_BXS_dims[0] = Time_dim;
   PC_BXS_dims[1] = bdy_width_dim;
   PC_BXS_dims[2] = south_north_dim;
   stat = nc_def_var(ncid, "PC_BXS", NC_FLOAT, RANK_PC_BXS, PC_BXS_dims, &PC_BXS_id);
   check_err(stat,__LINE__,__FILE__);

   PC_BXE_dims[0] = Time_dim;
   PC_BXE_dims[1] = bdy_width_dim;
   PC_BXE_dims[2] = south_north_dim;
   stat = nc_def_var(ncid, "PC_BXE", NC_FLOAT, RANK_PC_BXE, PC_BXE_dims, &PC_BXE_id);
   check_err(stat,__LINE__,__FILE__);

   PC_BYS_dims[0] = Time_dim;
   PC_BYS_dims[1] = bdy_width_dim;
   PC_BYS_dims[2] = west_east_dim;
   stat = nc_def_var(ncid, "PC_BYS", NC_FLOAT, RANK_PC_BYS, PC_BYS_dims, &PC_BYS_id);
   check_err(stat,__LINE__,__FILE__);

   PC_BYE_dims[0] = Time_dim;
   PC_BYE_dims[1] = bdy_width_dim;
   PC_BYE_dims[2] = west_east_dim;
   stat = nc_def_var(ncid, "PC_BYE", NC_FLOAT, RANK_PC_BYE, PC_BYE_dims, &PC_BYE_id);
   check_err(stat,__LINE__,__FILE__);

   PC_BTXS_dims[0] = Time_dim;
   PC_BTXS_dims[1] = bdy_width_dim;
   PC_BTXS_dims[2] = south_north_dim;
   stat = nc_def_var(ncid, "PC_BTXS", NC_FLOAT, RANK_PC_BTXS, PC_BTXS_dims, &PC_BTXS_id);
   check_err(stat,__LINE__,__FILE__);

   PC_BTXE_dims[0] = Time_dim;
   PC_BTXE_dims[1] = bdy_width_dim;
   PC_BTXE_dims[2] = south_north_dim;
   stat = nc_def_var(ncid, "PC_BTXE", NC_FLOAT, RANK_PC_BTXE, PC_BTXE_dims, &PC_BTXE_id);
   check_err(stat,__LINE__,__FILE__);

   PC_BTYS_dims[0] = Time_dim;
   PC_BTYS_dims[1] = bdy_width_dim;
   PC_BTYS_dims[2] = west_east_dim;
   stat = nc_def_var(ncid, "PC_BTYS", NC_FLOAT, RANK_PC_BTYS, PC_BTYS_dims, &PC_BTYS_id);
   check_err(stat,__LINE__,__FILE__);

   PC_BTYE_dims[0] = Time_dim;
   PC_BTYE_dims[1] = bdy_width_dim;
   PC_BTYE_dims[2] = west_east_dim;
   stat = nc_def_var(ncid, "PC_BTYE", NC_FLOAT, RANK_PC_BTYE, PC_BTYE_dims, &PC_BTYE_id);
   check_err(stat,__LINE__,__FILE__);

   /* assign global attributes */

   {
      stat = nc_put_att_text(ncid, NC_GLOBAL, "TITLE", 38, " OUTPUT FROM REAL_EM V4.0 PREPROCESSOR");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, NC_GLOBAL, "START_DATE", 19, "2018-01-01_00:00:00");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_WEST_MINUS_EAST_GRID_DIMENSION_att[1] = {74} ;
      stat = nc_put_att_int(ncid, NC_GLOBAL, "WEST-EAST_GRID_DIMENSION", NC_INT, 1, wrfbdy_d01_WEST_MINUS_EAST_GRID_DIMENSION_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_SOUTH_MINUS_NORTH_GRID_DIMENSION_att[1] = {61} ;
      stat = nc_put_att_int(ncid, NC_GLOBAL, "SOUTH-NORTH_GRID_DIMENSION", NC_INT, 1, wrfbdy_d01_SOUTH_MINUS_NORTH_GRID_DIMENSION_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_BOTTOM_MINUS_TOP_GRID_DIMENSION_att[1] = {33} ;
      stat = nc_put_att_int(ncid, NC_GLOBAL, "BOTTOM-TOP_GRID_DIMENSION", NC_INT, 1, wrfbdy_d01_BOTTOM_MINUS_TOP_GRID_DIMENSION_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const float wrfbdy_d01_DX_att[1] = {((float)30000)} ;
      stat = nc_put_att_float(ncid, NC_GLOBAL, "DX", NC_FLOAT, 1, wrfbdy_d01_DX_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const float wrfbdy_d01_DY_att[1] = {((float)30000)} ;
      stat = nc_put_att_float(ncid, NC_GLOBAL, "DY", NC_FLOAT, 1, wrfbdy_d01_DY_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_AERCU_OPT_att[1] = {0} ;
      stat = nc_put_att_int(ncid, NC_GLOBAL, "AERCU_OPT", NC_INT, 1, wrfbdy_d01_AERCU_OPT_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const float wrfbdy_d01_AERCU_FCT_att[1] = {((float)1)} ;
      stat = nc_put_att_float(ncid, NC_GLOBAL, "AERCU_FCT", NC_FLOAT, 1, wrfbdy_d01_AERCU_FCT_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_IDEAL_CASE_att[1] = {0} ;
      stat = nc_put_att_int(ncid, NC_GLOBAL, "IDEAL_CASE", NC_INT, 1, wrfbdy_d01_IDEAL_CASE_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_DIFF_6TH_SLOPEOPT_att[1] = {0} ;
      stat = nc_put_att_int(ncid, NC_GLOBAL, "DIFF_6TH_SLOPEOPT", NC_INT, 1, wrfbdy_d01_DIFF_6TH_SLOPEOPT_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_AUTO_LEVELS_OPT_att[1] = {2} ;
      stat = nc_put_att_int(ncid, NC_GLOBAL, "AUTO_LEVELS_OPT", NC_INT, 1, wrfbdy_d01_AUTO_LEVELS_OPT_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const float wrfbdy_d01_DIFF_6TH_THRESH_att[1] = {((float)0.1)} ;
      stat = nc_put_att_float(ncid, NC_GLOBAL, "DIFF_6TH_THRESH", NC_FLOAT, 1, wrfbdy_d01_DIFF_6TH_THRESH_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const float wrfbdy_d01_DZBOT_att[1] = {((float)50)} ;
      stat = nc_put_att_float(ncid, NC_GLOBAL, "DZBOT", NC_FLOAT, 1, wrfbdy_d01_DZBOT_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const float wrfbdy_d01_DZSTRETCH_S_att[1] = {((float)1.3)} ;
      stat = nc_put_att_float(ncid, NC_GLOBAL, "DZSTRETCH_S", NC_FLOAT, 1, wrfbdy_d01_DZSTRETCH_S_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const float wrfbdy_d01_DZSTRETCH_U_att[1] = {((float)1.1)} ;
      stat = nc_put_att_float(ncid, NC_GLOBAL, "DZSTRETCH_U", NC_FLOAT, 1, wrfbdy_d01_DZSTRETCH_U_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, NC_GLOBAL, "GRIDTYPE", 1, "C");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_DIFF_OPT_att[1] = {1} ;
      stat = nc_put_att_int(ncid, NC_GLOBAL, "DIFF_OPT", NC_INT, 1, wrfbdy_d01_DIFF_OPT_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_KM_OPT_att[1] = {4} ;
      stat = nc_put_att_int(ncid, NC_GLOBAL, "KM_OPT", NC_INT, 1, wrfbdy_d01_KM_OPT_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_DAMP_OPT_att[1] = {3} ;
      stat = nc_put_att_int(ncid, NC_GLOBAL, "DAMP_OPT", NC_INT, 1, wrfbdy_d01_DAMP_OPT_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const float wrfbdy_d01_DAMPCOEF_att[1] = {((float)0.2)} ;
      stat = nc_put_att_float(ncid, NC_GLOBAL, "DAMPCOEF", NC_FLOAT, 1, wrfbdy_d01_DAMPCOEF_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const float wrfbdy_d01_KHDIF_att[1] = {((float)0)} ;
      stat = nc_put_att_float(ncid, NC_GLOBAL, "KHDIF", NC_FLOAT, 1, wrfbdy_d01_KHDIF_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const float wrfbdy_d01_KVDIF_att[1] = {((float)0)} ;
      stat = nc_put_att_float(ncid, NC_GLOBAL, "KVDIF", NC_FLOAT, 1, wrfbdy_d01_KVDIF_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_MP_PHYSICS_att[1] = {8} ;
      stat = nc_put_att_int(ncid, NC_GLOBAL, "MP_PHYSICS", NC_INT, 1, wrfbdy_d01_MP_PHYSICS_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_RA_LW_PHYSICS_att[1] = {4} ;
      stat = nc_put_att_int(ncid, NC_GLOBAL, "RA_LW_PHYSICS", NC_INT, 1, wrfbdy_d01_RA_LW_PHYSICS_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_RA_SW_PHYSICS_att[1] = {4} ;
      stat = nc_put_att_int(ncid, NC_GLOBAL, "RA_SW_PHYSICS", NC_INT, 1, wrfbdy_d01_RA_SW_PHYSICS_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_SF_SFCLAY_PHYSICS_att[1] = {2} ;
      stat = nc_put_att_int(ncid, NC_GLOBAL, "SF_SFCLAY_PHYSICS", NC_INT, 1, wrfbdy_d01_SF_SFCLAY_PHYSICS_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_SF_SURFACE_PHYSICS_att[1] = {2} ;
      stat = nc_put_att_int(ncid, NC_GLOBAL, "SF_SURFACE_PHYSICS", NC_INT, 1, wrfbdy_d01_SF_SURFACE_PHYSICS_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_BL_PBL_PHYSICS_att[1] = {2} ;
      stat = nc_put_att_int(ncid, NC_GLOBAL, "BL_PBL_PHYSICS", NC_INT, 1, wrfbdy_d01_BL_PBL_PHYSICS_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_CU_PHYSICS_att[1] = {6} ;
      stat = nc_put_att_int(ncid, NC_GLOBAL, "CU_PHYSICS", NC_INT, 1, wrfbdy_d01_CU_PHYSICS_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_SF_LAKE_PHYSICS_att[1] = {0} ;
      stat = nc_put_att_int(ncid, NC_GLOBAL, "SF_LAKE_PHYSICS", NC_INT, 1, wrfbdy_d01_SF_LAKE_PHYSICS_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_SURFACE_INPUT_SOURCE_att[1] = {3} ;
      stat = nc_put_att_int(ncid, NC_GLOBAL, "SURFACE_INPUT_SOURCE", NC_INT, 1, wrfbdy_d01_SURFACE_INPUT_SOURCE_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_SST_UPDATE_att[1] = {0} ;
      stat = nc_put_att_int(ncid, NC_GLOBAL, "SST_UPDATE", NC_INT, 1, wrfbdy_d01_SST_UPDATE_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_GRID_FDDA_att[1] = {0} ;
      stat = nc_put_att_int(ncid, NC_GLOBAL, "GRID_FDDA", NC_INT, 1, wrfbdy_d01_GRID_FDDA_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_GFDDA_INTERVAL_M_att[1] = {0} ;
      stat = nc_put_att_int(ncid, NC_GLOBAL, "GFDDA_INTERVAL_M", NC_INT, 1, wrfbdy_d01_GFDDA_INTERVAL_M_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_GFDDA_END_H_att[1] = {0} ;
      stat = nc_put_att_int(ncid, NC_GLOBAL, "GFDDA_END_H", NC_INT, 1, wrfbdy_d01_GFDDA_END_H_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_GRID_SFDDA_att[1] = {0} ;
      stat = nc_put_att_int(ncid, NC_GLOBAL, "GRID_SFDDA", NC_INT, 1, wrfbdy_d01_GRID_SFDDA_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_SGFDDA_INTERVAL_M_att[1] = {0} ;
      stat = nc_put_att_int(ncid, NC_GLOBAL, "SGFDDA_INTERVAL_M", NC_INT, 1, wrfbdy_d01_SGFDDA_INTERVAL_M_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_SGFDDA_END_H_att[1] = {0} ;
      stat = nc_put_att_int(ncid, NC_GLOBAL, "SGFDDA_END_H", NC_INT, 1, wrfbdy_d01_SGFDDA_END_H_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_HYPSOMETRIC_OPT_att[1] = {2} ;
      stat = nc_put_att_int(ncid, NC_GLOBAL, "HYPSOMETRIC_OPT", NC_INT, 1, wrfbdy_d01_HYPSOMETRIC_OPT_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_USE_THETA_M_att[1] = {1} ;
      stat = nc_put_att_int(ncid, NC_GLOBAL, "USE_THETA_M", NC_INT, 1, wrfbdy_d01_USE_THETA_M_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_GWD_OPT_att[1] = {1} ;
      stat = nc_put_att_int(ncid, NC_GLOBAL, "GWD_OPT", NC_INT, 1, wrfbdy_d01_GWD_OPT_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_SF_URBAN_PHYSICS_att[1] = {0} ;
      stat = nc_put_att_int(ncid, NC_GLOBAL, "SF_URBAN_PHYSICS", NC_INT, 1, wrfbdy_d01_SF_URBAN_PHYSICS_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_SF_SURFACE_MOSAIC_att[1] = {0} ;
      stat = nc_put_att_int(ncid, NC_GLOBAL, "SF_SURFACE_MOSAIC", NC_INT, 1, wrfbdy_d01_SF_SURFACE_MOSAIC_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_SF_OCEAN_PHYSICS_att[1] = {0} ;
      stat = nc_put_att_int(ncid, NC_GLOBAL, "SF_OCEAN_PHYSICS", NC_INT, 1, wrfbdy_d01_SF_OCEAN_PHYSICS_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_WEST_MINUS_EAST_PATCH_START_UNSTAG_att[1] = {1} ;
      stat = nc_put_att_int(ncid, NC_GLOBAL, "WEST-EAST_PATCH_START_UNSTAG", NC_INT, 1, wrfbdy_d01_WEST_MINUS_EAST_PATCH_START_UNSTAG_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_WEST_MINUS_EAST_PATCH_END_UNSTAG_att[1] = {73} ;
      stat = nc_put_att_int(ncid, NC_GLOBAL, "WEST-EAST_PATCH_END_UNSTAG", NC_INT, 1, wrfbdy_d01_WEST_MINUS_EAST_PATCH_END_UNSTAG_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_WEST_MINUS_EAST_PATCH_START_STAG_att[1] = {1} ;
      stat = nc_put_att_int(ncid, NC_GLOBAL, "WEST-EAST_PATCH_START_STAG", NC_INT, 1, wrfbdy_d01_WEST_MINUS_EAST_PATCH_START_STAG_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_WEST_MINUS_EAST_PATCH_END_STAG_att[1] = {74} ;
      stat = nc_put_att_int(ncid, NC_GLOBAL, "WEST-EAST_PATCH_END_STAG", NC_INT, 1, wrfbdy_d01_WEST_MINUS_EAST_PATCH_END_STAG_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_SOUTH_MINUS_NORTH_PATCH_START_UNSTAG_att[1] = {1} ;
      stat = nc_put_att_int(ncid, NC_GLOBAL, "SOUTH-NORTH_PATCH_START_UNSTAG", NC_INT, 1, wrfbdy_d01_SOUTH_MINUS_NORTH_PATCH_START_UNSTAG_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_SOUTH_MINUS_NORTH_PATCH_END_UNSTAG_att[1] = {60} ;
      stat = nc_put_att_int(ncid, NC_GLOBAL, "SOUTH-NORTH_PATCH_END_UNSTAG", NC_INT, 1, wrfbdy_d01_SOUTH_MINUS_NORTH_PATCH_END_UNSTAG_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_SOUTH_MINUS_NORTH_PATCH_START_STAG_att[1] = {1} ;
      stat = nc_put_att_int(ncid, NC_GLOBAL, "SOUTH-NORTH_PATCH_START_STAG", NC_INT, 1, wrfbdy_d01_SOUTH_MINUS_NORTH_PATCH_START_STAG_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_SOUTH_MINUS_NORTH_PATCH_END_STAG_att[1] = {61} ;
      stat = nc_put_att_int(ncid, NC_GLOBAL, "SOUTH-NORTH_PATCH_END_STAG", NC_INT, 1, wrfbdy_d01_SOUTH_MINUS_NORTH_PATCH_END_STAG_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_BOTTOM_MINUS_TOP_PATCH_START_UNSTAG_att[1] = {1} ;
      stat = nc_put_att_int(ncid, NC_GLOBAL, "BOTTOM-TOP_PATCH_START_UNSTAG", NC_INT, 1, wrfbdy_d01_BOTTOM_MINUS_TOP_PATCH_START_UNSTAG_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_BOTTOM_MINUS_TOP_PATCH_END_UNSTAG_att[1] = {32} ;
      stat = nc_put_att_int(ncid, NC_GLOBAL, "BOTTOM-TOP_PATCH_END_UNSTAG", NC_INT, 1, wrfbdy_d01_BOTTOM_MINUS_TOP_PATCH_END_UNSTAG_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_BOTTOM_MINUS_TOP_PATCH_START_STAG_att[1] = {1} ;
      stat = nc_put_att_int(ncid, NC_GLOBAL, "BOTTOM-TOP_PATCH_START_STAG", NC_INT, 1, wrfbdy_d01_BOTTOM_MINUS_TOP_PATCH_START_STAG_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_BOTTOM_MINUS_TOP_PATCH_END_STAG_att[1] = {33} ;
      stat = nc_put_att_int(ncid, NC_GLOBAL, "BOTTOM-TOP_PATCH_END_STAG", NC_INT, 1, wrfbdy_d01_BOTTOM_MINUS_TOP_PATCH_END_STAG_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_GRID_ID_att[1] = {1} ;
      stat = nc_put_att_int(ncid, NC_GLOBAL, "GRID_ID", NC_INT, 1, wrfbdy_d01_GRID_ID_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_PARENT_ID_att[1] = {0} ;
      stat = nc_put_att_int(ncid, NC_GLOBAL, "PARENT_ID", NC_INT, 1, wrfbdy_d01_PARENT_ID_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_I_PARENT_START_att[1] = {1} ;
      stat = nc_put_att_int(ncid, NC_GLOBAL, "I_PARENT_START", NC_INT, 1, wrfbdy_d01_I_PARENT_START_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_J_PARENT_START_att[1] = {1} ;
      stat = nc_put_att_int(ncid, NC_GLOBAL, "J_PARENT_START", NC_INT, 1, wrfbdy_d01_J_PARENT_START_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_PARENT_GRID_RATIO_att[1] = {1} ;
      stat = nc_put_att_int(ncid, NC_GLOBAL, "PARENT_GRID_RATIO", NC_INT, 1, wrfbdy_d01_PARENT_GRID_RATIO_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const float wrfbdy_d01_DT_att[1] = {((float)180)} ;
      stat = nc_put_att_float(ncid, NC_GLOBAL, "DT", NC_FLOAT, 1, wrfbdy_d01_DT_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const float wrfbdy_d01_CEN_LAT_att[1] = {((float)34.830021)} ;
      stat = nc_put_att_float(ncid, NC_GLOBAL, "CEN_LAT", NC_FLOAT, 1, wrfbdy_d01_CEN_LAT_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const float wrfbdy_d01_CEN_LON_att[1] = {((float)-81.029999)} ;
      stat = nc_put_att_float(ncid, NC_GLOBAL, "CEN_LON", NC_FLOAT, 1, wrfbdy_d01_CEN_LON_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const float wrfbdy_d01_TRUELAT1_att[1] = {((float)30)} ;
      stat = nc_put_att_float(ncid, NC_GLOBAL, "TRUELAT1", NC_FLOAT, 1, wrfbdy_d01_TRUELAT1_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const float wrfbdy_d01_TRUELAT2_att[1] = {((float)60)} ;
      stat = nc_put_att_float(ncid, NC_GLOBAL, "TRUELAT2", NC_FLOAT, 1, wrfbdy_d01_TRUELAT2_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const float wrfbdy_d01_MOAD_CEN_LAT_att[1] = {((float)34.830021)} ;
      stat = nc_put_att_float(ncid, NC_GLOBAL, "MOAD_CEN_LAT", NC_FLOAT, 1, wrfbdy_d01_MOAD_CEN_LAT_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const float wrfbdy_d01_STAND_LON_att[1] = {((float)-98)} ;
      stat = nc_put_att_float(ncid, NC_GLOBAL, "STAND_LON", NC_FLOAT, 1, wrfbdy_d01_STAND_LON_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const float wrfbdy_d01_POLE_LAT_att[1] = {((float)90)} ;
      stat = nc_put_att_float(ncid, NC_GLOBAL, "POLE_LAT", NC_FLOAT, 1, wrfbdy_d01_POLE_LAT_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const float wrfbdy_d01_POLE_LON_att[1] = {((float)0)} ;
      stat = nc_put_att_float(ncid, NC_GLOBAL, "POLE_LON", NC_FLOAT, 1, wrfbdy_d01_POLE_LON_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_MAP_PROJ_att[1] = {1} ;
      stat = nc_put_att_int(ncid, NC_GLOBAL, "MAP_PROJ", NC_INT, 1, wrfbdy_d01_MAP_PROJ_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, NC_GLOBAL, "MAP_PROJ_CHAR", 17, "Lambert Conformal");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, NC_GLOBAL, "MMINLU", 24, "MODIFIED_IGBP_MODIS_NOAH");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_NUM_LAND_CAT_att[1] = {21} ;
      stat = nc_put_att_int(ncid, NC_GLOBAL, "NUM_LAND_CAT", NC_INT, 1, wrfbdy_d01_NUM_LAND_CAT_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_ISWATER_att[1] = {17} ;
      stat = nc_put_att_int(ncid, NC_GLOBAL, "ISWATER", NC_INT, 1, wrfbdy_d01_ISWATER_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_ISLAKE_att[1] = {21} ;
      stat = nc_put_att_int(ncid, NC_GLOBAL, "ISLAKE", NC_INT, 1, wrfbdy_d01_ISLAKE_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_ISICE_att[1] = {15} ;
      stat = nc_put_att_int(ncid, NC_GLOBAL, "ISICE", NC_INT, 1, wrfbdy_d01_ISICE_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_ISURBAN_att[1] = {13} ;
      stat = nc_put_att_int(ncid, NC_GLOBAL, "ISURBAN", NC_INT, 1, wrfbdy_d01_ISURBAN_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_ISOILWATER_att[1] = {14} ;
      stat = nc_put_att_int(ncid, NC_GLOBAL, "ISOILWATER", NC_INT, 1, wrfbdy_d01_ISOILWATER_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_HYBRID_OPT_att[1] = {2} ;
      stat = nc_put_att_int(ncid, NC_GLOBAL, "HYBRID_OPT", NC_INT, 1, wrfbdy_d01_HYBRID_OPT_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const float wrfbdy_d01_ETAC_att[1] = {((float)0.2)} ;
      stat = nc_put_att_float(ncid, NC_GLOBAL, "ETAC", NC_FLOAT, 1, wrfbdy_d01_ETAC_att);
      check_err(stat,__LINE__,__FILE__);
   }


   /* assign per-variable attributes */

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, U_BXS_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, U_BXS_id, "MemoryOrder", 3, "XSZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, U_BXS_id, "description", 20, "bdy x-wind component");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, U_BXS_id, "units", 5, "m s-1");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, U_BXS_id, "stagger", 1, "X");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, U_BXE_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, U_BXE_id, "MemoryOrder", 3, "XEZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, U_BXE_id, "description", 20, "bdy x-wind component");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, U_BXE_id, "units", 5, "m s-1");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, U_BXE_id, "stagger", 1, "X");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, U_BYS_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, U_BYS_id, "MemoryOrder", 3, "YSZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, U_BYS_id, "description", 20, "bdy x-wind component");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, U_BYS_id, "units", 5, "m s-1");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, U_BYS_id, "stagger", 1, "X");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, U_BYE_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, U_BYE_id, "MemoryOrder", 3, "YEZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, U_BYE_id, "description", 20, "bdy x-wind component");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, U_BYE_id, "units", 5, "m s-1");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, U_BYE_id, "stagger", 1, "X");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, U_BTXS_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, U_BTXS_id, "MemoryOrder", 3, "XSZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, U_BTXS_id, "description", 25, "bdy tend x-wind component");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, U_BTXS_id, "units", 10, "(m s-1)/dt");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, U_BTXS_id, "stagger", 1, "X");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, U_BTXE_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, U_BTXE_id, "MemoryOrder", 3, "XEZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, U_BTXE_id, "description", 25, "bdy tend x-wind component");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, U_BTXE_id, "units", 10, "(m s-1)/dt");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, U_BTXE_id, "stagger", 1, "X");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, U_BTYS_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, U_BTYS_id, "MemoryOrder", 3, "YSZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, U_BTYS_id, "description", 25, "bdy tend x-wind component");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, U_BTYS_id, "units", 10, "(m s-1)/dt");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, U_BTYS_id, "stagger", 1, "X");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, U_BTYE_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, U_BTYE_id, "MemoryOrder", 3, "YEZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, U_BTYE_id, "description", 25, "bdy tend x-wind component");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, U_BTYE_id, "units", 10, "(m s-1)/dt");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, U_BTYE_id, "stagger", 1, "X");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, V_BXS_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, V_BXS_id, "MemoryOrder", 3, "XSZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, V_BXS_id, "description", 20, "bdy y-wind component");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, V_BXS_id, "units", 5, "m s-1");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, V_BXS_id, "stagger", 1, "Y");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, V_BXE_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, V_BXE_id, "MemoryOrder", 3, "XEZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, V_BXE_id, "description", 20, "bdy y-wind component");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, V_BXE_id, "units", 5, "m s-1");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, V_BXE_id, "stagger", 1, "Y");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, V_BYS_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, V_BYS_id, "MemoryOrder", 3, "YSZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, V_BYS_id, "description", 20, "bdy y-wind component");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, V_BYS_id, "units", 5, "m s-1");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, V_BYS_id, "stagger", 1, "Y");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, V_BYE_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, V_BYE_id, "MemoryOrder", 3, "YEZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, V_BYE_id, "description", 20, "bdy y-wind component");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, V_BYE_id, "units", 5, "m s-1");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, V_BYE_id, "stagger", 1, "Y");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, V_BTXS_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, V_BTXS_id, "MemoryOrder", 3, "XSZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, V_BTXS_id, "description", 25, "bdy tend y-wind component");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, V_BTXS_id, "units", 10, "(m s-1)/dt");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, V_BTXS_id, "stagger", 1, "Y");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, V_BTXE_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, V_BTXE_id, "MemoryOrder", 3, "XEZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, V_BTXE_id, "description", 25, "bdy tend y-wind component");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, V_BTXE_id, "units", 10, "(m s-1)/dt");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, V_BTXE_id, "stagger", 1, "Y");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, V_BTYS_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, V_BTYS_id, "MemoryOrder", 3, "YSZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, V_BTYS_id, "description", 25, "bdy tend y-wind component");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, V_BTYS_id, "units", 10, "(m s-1)/dt");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, V_BTYS_id, "stagger", 1, "Y");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, V_BTYE_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, V_BTYE_id, "MemoryOrder", 3, "YEZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, V_BTYE_id, "description", 25, "bdy tend y-wind component");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, V_BTYE_id, "units", 10, "(m s-1)/dt");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, V_BTYE_id, "stagger", 1, "Y");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, W_BXS_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, W_BXS_id, "MemoryOrder", 3, "XSZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, W_BXS_id, "description", 20, "bdy z-wind component");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, W_BXS_id, "units", 5, "m s-1");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, W_BXS_id, "stagger", 1, "Z");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, W_BXE_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, W_BXE_id, "MemoryOrder", 3, "XEZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, W_BXE_id, "description", 20, "bdy z-wind component");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, W_BXE_id, "units", 5, "m s-1");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, W_BXE_id, "stagger", 1, "Z");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, W_BYS_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, W_BYS_id, "MemoryOrder", 3, "YSZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, W_BYS_id, "description", 20, "bdy z-wind component");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, W_BYS_id, "units", 5, "m s-1");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, W_BYS_id, "stagger", 1, "Z");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, W_BYE_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, W_BYE_id, "MemoryOrder", 3, "YEZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, W_BYE_id, "description", 20, "bdy z-wind component");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, W_BYE_id, "units", 5, "m s-1");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, W_BYE_id, "stagger", 1, "Z");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, W_BTXS_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, W_BTXS_id, "MemoryOrder", 3, "XSZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, W_BTXS_id, "description", 25, "bdy tend z-wind component");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, W_BTXS_id, "units", 10, "(m s-1)/dt");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, W_BTXS_id, "stagger", 1, "Z");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, W_BTXE_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, W_BTXE_id, "MemoryOrder", 3, "XEZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, W_BTXE_id, "description", 25, "bdy tend z-wind component");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, W_BTXE_id, "units", 10, "(m s-1)/dt");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, W_BTXE_id, "stagger", 1, "Z");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, W_BTYS_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, W_BTYS_id, "MemoryOrder", 3, "YSZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, W_BTYS_id, "description", 25, "bdy tend z-wind component");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, W_BTYS_id, "units", 10, "(m s-1)/dt");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, W_BTYS_id, "stagger", 1, "Z");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, W_BTYE_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, W_BTYE_id, "MemoryOrder", 3, "YEZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, W_BTYE_id, "description", 25, "bdy tend z-wind component");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, W_BTYE_id, "units", 10, "(m s-1)/dt");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, W_BTYE_id, "stagger", 1, "Z");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, PH_BXS_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, PH_BXS_id, "MemoryOrder", 3, "XSZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, PH_BXS_id, "description", 29, "bdy perturbation geopotential");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, PH_BXS_id, "units", 6, "m2 s-2");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, PH_BXS_id, "stagger", 1, "Z");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, PH_BXE_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, PH_BXE_id, "MemoryOrder", 3, "XEZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, PH_BXE_id, "description", 29, "bdy perturbation geopotential");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, PH_BXE_id, "units", 6, "m2 s-2");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, PH_BXE_id, "stagger", 1, "Z");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, PH_BYS_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, PH_BYS_id, "MemoryOrder", 3, "YSZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, PH_BYS_id, "description", 29, "bdy perturbation geopotential");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, PH_BYS_id, "units", 6, "m2 s-2");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, PH_BYS_id, "stagger", 1, "Z");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, PH_BYE_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, PH_BYE_id, "MemoryOrder", 3, "YEZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, PH_BYE_id, "description", 29, "bdy perturbation geopotential");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, PH_BYE_id, "units", 6, "m2 s-2");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, PH_BYE_id, "stagger", 1, "Z");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, PH_BTXS_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, PH_BTXS_id, "MemoryOrder", 3, "XSZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, PH_BTXS_id, "description", 34, "bdy tend perturbation geopotential");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, PH_BTXS_id, "units", 11, "(m2 s-2)/dt");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, PH_BTXS_id, "stagger", 1, "Z");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, PH_BTXE_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, PH_BTXE_id, "MemoryOrder", 3, "XEZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, PH_BTXE_id, "description", 34, "bdy tend perturbation geopotential");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, PH_BTXE_id, "units", 11, "(m2 s-2)/dt");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, PH_BTXE_id, "stagger", 1, "Z");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, PH_BTYS_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, PH_BTYS_id, "MemoryOrder", 3, "YSZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, PH_BTYS_id, "description", 34, "bdy tend perturbation geopotential");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, PH_BTYS_id, "units", 11, "(m2 s-2)/dt");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, PH_BTYS_id, "stagger", 1, "Z");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, PH_BTYE_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, PH_BTYE_id, "MemoryOrder", 3, "YEZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, PH_BTYE_id, "description", 34, "bdy tend perturbation geopotential");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, PH_BTYE_id, "units", 11, "(m2 s-2)/dt");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, PH_BTYE_id, "stagger", 1, "Z");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, T_BXS_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, T_BXS_id, "MemoryOrder", 3, "XSZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, T_BXS_id, "description", 121, "bdy either 1) pert moist pot temp=(1+Rv/Rd Qv)*(theta)-T0, or 2) pert dry pot temp=theta-T0; based on use_theta_m setting");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, T_BXS_id, "units", 1, "K");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, T_BXS_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, T_BXE_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, T_BXE_id, "MemoryOrder", 3, "XEZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, T_BXE_id, "description", 121, "bdy either 1) pert moist pot temp=(1+Rv/Rd Qv)*(theta)-T0, or 2) pert dry pot temp=theta-T0; based on use_theta_m setting");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, T_BXE_id, "units", 1, "K");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, T_BXE_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, T_BYS_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, T_BYS_id, "MemoryOrder", 3, "YSZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, T_BYS_id, "description", 121, "bdy either 1) pert moist pot temp=(1+Rv/Rd Qv)*(theta)-T0, or 2) pert dry pot temp=theta-T0; based on use_theta_m setting");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, T_BYS_id, "units", 1, "K");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, T_BYS_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, T_BYE_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, T_BYE_id, "MemoryOrder", 3, "YEZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, T_BYE_id, "description", 121, "bdy either 1) pert moist pot temp=(1+Rv/Rd Qv)*(theta)-T0, or 2) pert dry pot temp=theta-T0; based on use_theta_m setting");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, T_BYE_id, "units", 1, "K");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, T_BYE_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, T_BTXS_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, T_BTXS_id, "MemoryOrder", 3, "XSZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, T_BTXS_id, "description", 126, "bdy tend either 1) pert moist pot temp=(1+Rv/Rd Qv)*(theta)-T0, or 2) pert dry pot temp=theta-T0; based on use_theta_m setting");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, T_BTXS_id, "units", 6, "(K)/dt");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, T_BTXS_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, T_BTXE_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, T_BTXE_id, "MemoryOrder", 3, "XEZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, T_BTXE_id, "description", 126, "bdy tend either 1) pert moist pot temp=(1+Rv/Rd Qv)*(theta)-T0, or 2) pert dry pot temp=theta-T0; based on use_theta_m setting");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, T_BTXE_id, "units", 6, "(K)/dt");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, T_BTXE_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, T_BTYS_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, T_BTYS_id, "MemoryOrder", 3, "YSZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, T_BTYS_id, "description", 126, "bdy tend either 1) pert moist pot temp=(1+Rv/Rd Qv)*(theta)-T0, or 2) pert dry pot temp=theta-T0; based on use_theta_m setting");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, T_BTYS_id, "units", 6, "(K)/dt");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, T_BTYS_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, T_BTYE_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, T_BTYE_id, "MemoryOrder", 3, "YEZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, T_BTYE_id, "description", 126, "bdy tend either 1) pert moist pot temp=(1+Rv/Rd Qv)*(theta)-T0, or 2) pert dry pot temp=theta-T0; based on use_theta_m setting");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, T_BTYE_id, "units", 6, "(K)/dt");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, T_BTYE_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, MU_BXS_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, MU_BXS_id, "MemoryOrder", 3, "XS ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, MU_BXS_id, "description", 39, "bdy perturbation dry air mass in column");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, MU_BXS_id, "units", 2, "Pa");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, MU_BXS_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, MU_BXE_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, MU_BXE_id, "MemoryOrder", 3, "XE ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, MU_BXE_id, "description", 39, "bdy perturbation dry air mass in column");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, MU_BXE_id, "units", 2, "Pa");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, MU_BXE_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, MU_BYS_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, MU_BYS_id, "MemoryOrder", 3, "YS ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, MU_BYS_id, "description", 39, "bdy perturbation dry air mass in column");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, MU_BYS_id, "units", 2, "Pa");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, MU_BYS_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, MU_BYE_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, MU_BYE_id, "MemoryOrder", 3, "YE ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, MU_BYE_id, "description", 39, "bdy perturbation dry air mass in column");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, MU_BYE_id, "units", 2, "Pa");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, MU_BYE_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, MU_BTXS_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, MU_BTXS_id, "MemoryOrder", 3, "XS ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, MU_BTXS_id, "description", 44, "bdy tend perturbation dry air mass in column");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, MU_BTXS_id, "units", 7, "(Pa)/dt");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, MU_BTXS_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, MU_BTXE_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, MU_BTXE_id, "MemoryOrder", 3, "XE ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, MU_BTXE_id, "description", 44, "bdy tend perturbation dry air mass in column");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, MU_BTXE_id, "units", 7, "(Pa)/dt");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, MU_BTXE_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, MU_BTYS_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, MU_BTYS_id, "MemoryOrder", 3, "YS ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, MU_BTYS_id, "description", 44, "bdy tend perturbation dry air mass in column");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, MU_BTYS_id, "units", 7, "(Pa)/dt");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, MU_BTYS_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, MU_BTYE_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, MU_BTYE_id, "MemoryOrder", 3, "YE ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, MU_BTYE_id, "description", 44, "bdy tend perturbation dry air mass in column");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, MU_BTYE_id, "units", 7, "(Pa)/dt");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, MU_BTYE_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, QVAPOR_BXS_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QVAPOR_BXS_id, "MemoryOrder", 3, "XSZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QVAPOR_BXS_id, "description", 24, "Water vapor mixing ratio");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QVAPOR_BXS_id, "units", 7, "kg kg-1");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QVAPOR_BXS_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, QVAPOR_BXE_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QVAPOR_BXE_id, "MemoryOrder", 3, "XEZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QVAPOR_BXE_id, "description", 24, "Water vapor mixing ratio");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QVAPOR_BXE_id, "units", 7, "kg kg-1");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QVAPOR_BXE_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, QVAPOR_BYS_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QVAPOR_BYS_id, "MemoryOrder", 3, "YSZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QVAPOR_BYS_id, "description", 24, "Water vapor mixing ratio");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QVAPOR_BYS_id, "units", 7, "kg kg-1");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QVAPOR_BYS_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, QVAPOR_BYE_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QVAPOR_BYE_id, "MemoryOrder", 3, "YEZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QVAPOR_BYE_id, "description", 24, "Water vapor mixing ratio");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QVAPOR_BYE_id, "units", 7, "kg kg-1");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QVAPOR_BYE_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, QCLOUD_BXS_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QCLOUD_BXS_id, "MemoryOrder", 3, "XSZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QCLOUD_BXS_id, "description", 24, "Cloud water mixing ratio");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QCLOUD_BXS_id, "units", 7, "kg kg-1");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QCLOUD_BXS_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, QCLOUD_BXE_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QCLOUD_BXE_id, "MemoryOrder", 3, "XEZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QCLOUD_BXE_id, "description", 24, "Cloud water mixing ratio");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QCLOUD_BXE_id, "units", 7, "kg kg-1");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QCLOUD_BXE_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, QCLOUD_BYS_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QCLOUD_BYS_id, "MemoryOrder", 3, "YSZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QCLOUD_BYS_id, "description", 24, "Cloud water mixing ratio");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QCLOUD_BYS_id, "units", 7, "kg kg-1");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QCLOUD_BYS_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, QCLOUD_BYE_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QCLOUD_BYE_id, "MemoryOrder", 3, "YEZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QCLOUD_BYE_id, "description", 24, "Cloud water mixing ratio");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QCLOUD_BYE_id, "units", 7, "kg kg-1");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QCLOUD_BYE_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, QRAIN_BXS_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QRAIN_BXS_id, "MemoryOrder", 3, "XSZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QRAIN_BXS_id, "description", 23, "Rain water mixing ratio");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QRAIN_BXS_id, "units", 7, "kg kg-1");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QRAIN_BXS_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, QRAIN_BXE_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QRAIN_BXE_id, "MemoryOrder", 3, "XEZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QRAIN_BXE_id, "description", 23, "Rain water mixing ratio");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QRAIN_BXE_id, "units", 7, "kg kg-1");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QRAIN_BXE_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, QRAIN_BYS_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QRAIN_BYS_id, "MemoryOrder", 3, "YSZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QRAIN_BYS_id, "description", 23, "Rain water mixing ratio");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QRAIN_BYS_id, "units", 7, "kg kg-1");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QRAIN_BYS_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, QRAIN_BYE_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QRAIN_BYE_id, "MemoryOrder", 3, "YEZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QRAIN_BYE_id, "description", 23, "Rain water mixing ratio");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QRAIN_BYE_id, "units", 7, "kg kg-1");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QRAIN_BYE_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, QICE_BXS_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QICE_BXS_id, "MemoryOrder", 3, "XSZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QICE_BXS_id, "description", 16, "Ice mixing ratio");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QICE_BXS_id, "units", 7, "kg kg-1");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QICE_BXS_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, QICE_BXE_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QICE_BXE_id, "MemoryOrder", 3, "XEZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QICE_BXE_id, "description", 16, "Ice mixing ratio");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QICE_BXE_id, "units", 7, "kg kg-1");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QICE_BXE_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, QICE_BYS_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QICE_BYS_id, "MemoryOrder", 3, "YSZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QICE_BYS_id, "description", 16, "Ice mixing ratio");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QICE_BYS_id, "units", 7, "kg kg-1");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QICE_BYS_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, QICE_BYE_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QICE_BYE_id, "MemoryOrder", 3, "YEZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QICE_BYE_id, "description", 16, "Ice mixing ratio");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QICE_BYE_id, "units", 7, "kg kg-1");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QICE_BYE_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, QSNOW_BXS_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QSNOW_BXS_id, "MemoryOrder", 3, "XSZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QSNOW_BXS_id, "description", 17, "Snow mixing ratio");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QSNOW_BXS_id, "units", 7, "kg kg-1");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QSNOW_BXS_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, QSNOW_BXE_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QSNOW_BXE_id, "MemoryOrder", 3, "XEZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QSNOW_BXE_id, "description", 17, "Snow mixing ratio");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QSNOW_BXE_id, "units", 7, "kg kg-1");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QSNOW_BXE_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, QSNOW_BYS_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QSNOW_BYS_id, "MemoryOrder", 3, "YSZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QSNOW_BYS_id, "description", 17, "Snow mixing ratio");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QSNOW_BYS_id, "units", 7, "kg kg-1");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QSNOW_BYS_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, QSNOW_BYE_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QSNOW_BYE_id, "MemoryOrder", 3, "YEZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QSNOW_BYE_id, "description", 17, "Snow mixing ratio");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QSNOW_BYE_id, "units", 7, "kg kg-1");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QSNOW_BYE_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, QGRAUP_BXS_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QGRAUP_BXS_id, "MemoryOrder", 3, "XSZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QGRAUP_BXS_id, "description", 20, "Graupel mixing ratio");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QGRAUP_BXS_id, "units", 7, "kg kg-1");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QGRAUP_BXS_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, QGRAUP_BXE_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QGRAUP_BXE_id, "MemoryOrder", 3, "XEZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QGRAUP_BXE_id, "description", 20, "Graupel mixing ratio");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QGRAUP_BXE_id, "units", 7, "kg kg-1");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QGRAUP_BXE_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, QGRAUP_BYS_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QGRAUP_BYS_id, "MemoryOrder", 3, "YSZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QGRAUP_BYS_id, "description", 20, "Graupel mixing ratio");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QGRAUP_BYS_id, "units", 7, "kg kg-1");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QGRAUP_BYS_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, QGRAUP_BYE_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QGRAUP_BYE_id, "MemoryOrder", 3, "YEZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QGRAUP_BYE_id, "description", 20, "Graupel mixing ratio");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QGRAUP_BYE_id, "units", 7, "kg kg-1");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QGRAUP_BYE_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, QVAPOR_BTXS_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QVAPOR_BTXS_id, "MemoryOrder", 3, "XSZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QVAPOR_BTXS_id, "description", 24, "Water vapor mixing ratio");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QVAPOR_BTXS_id, "units", 7, "kg kg-1");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QVAPOR_BTXS_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, QVAPOR_BTXE_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QVAPOR_BTXE_id, "MemoryOrder", 3, "XEZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QVAPOR_BTXE_id, "description", 24, "Water vapor mixing ratio");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QVAPOR_BTXE_id, "units", 7, "kg kg-1");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QVAPOR_BTXE_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, QVAPOR_BTYS_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QVAPOR_BTYS_id, "MemoryOrder", 3, "YSZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QVAPOR_BTYS_id, "description", 24, "Water vapor mixing ratio");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QVAPOR_BTYS_id, "units", 7, "kg kg-1");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QVAPOR_BTYS_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, QVAPOR_BTYE_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QVAPOR_BTYE_id, "MemoryOrder", 3, "YEZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QVAPOR_BTYE_id, "description", 24, "Water vapor mixing ratio");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QVAPOR_BTYE_id, "units", 7, "kg kg-1");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QVAPOR_BTYE_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, QCLOUD_BTXS_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QCLOUD_BTXS_id, "MemoryOrder", 3, "XSZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QCLOUD_BTXS_id, "description", 24, "Cloud water mixing ratio");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QCLOUD_BTXS_id, "units", 7, "kg kg-1");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QCLOUD_BTXS_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, QCLOUD_BTXE_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QCLOUD_BTXE_id, "MemoryOrder", 3, "XEZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QCLOUD_BTXE_id, "description", 24, "Cloud water mixing ratio");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QCLOUD_BTXE_id, "units", 7, "kg kg-1");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QCLOUD_BTXE_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, QCLOUD_BTYS_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QCLOUD_BTYS_id, "MemoryOrder", 3, "YSZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QCLOUD_BTYS_id, "description", 24, "Cloud water mixing ratio");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QCLOUD_BTYS_id, "units", 7, "kg kg-1");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QCLOUD_BTYS_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, QCLOUD_BTYE_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QCLOUD_BTYE_id, "MemoryOrder", 3, "YEZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QCLOUD_BTYE_id, "description", 24, "Cloud water mixing ratio");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QCLOUD_BTYE_id, "units", 7, "kg kg-1");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QCLOUD_BTYE_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, QRAIN_BTXS_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QRAIN_BTXS_id, "MemoryOrder", 3, "XSZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QRAIN_BTXS_id, "description", 23, "Rain water mixing ratio");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QRAIN_BTXS_id, "units", 7, "kg kg-1");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QRAIN_BTXS_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, QRAIN_BTXE_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QRAIN_BTXE_id, "MemoryOrder", 3, "XEZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QRAIN_BTXE_id, "description", 23, "Rain water mixing ratio");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QRAIN_BTXE_id, "units", 7, "kg kg-1");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QRAIN_BTXE_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, QRAIN_BTYS_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QRAIN_BTYS_id, "MemoryOrder", 3, "YSZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QRAIN_BTYS_id, "description", 23, "Rain water mixing ratio");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QRAIN_BTYS_id, "units", 7, "kg kg-1");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QRAIN_BTYS_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, QRAIN_BTYE_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QRAIN_BTYE_id, "MemoryOrder", 3, "YEZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QRAIN_BTYE_id, "description", 23, "Rain water mixing ratio");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QRAIN_BTYE_id, "units", 7, "kg kg-1");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QRAIN_BTYE_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, QICE_BTXS_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QICE_BTXS_id, "MemoryOrder", 3, "XSZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QICE_BTXS_id, "description", 16, "Ice mixing ratio");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QICE_BTXS_id, "units", 7, "kg kg-1");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QICE_BTXS_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, QICE_BTXE_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QICE_BTXE_id, "MemoryOrder", 3, "XEZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QICE_BTXE_id, "description", 16, "Ice mixing ratio");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QICE_BTXE_id, "units", 7, "kg kg-1");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QICE_BTXE_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, QICE_BTYS_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QICE_BTYS_id, "MemoryOrder", 3, "YSZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QICE_BTYS_id, "description", 16, "Ice mixing ratio");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QICE_BTYS_id, "units", 7, "kg kg-1");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QICE_BTYS_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, QICE_BTYE_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QICE_BTYE_id, "MemoryOrder", 3, "YEZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QICE_BTYE_id, "description", 16, "Ice mixing ratio");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QICE_BTYE_id, "units", 7, "kg kg-1");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QICE_BTYE_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, QSNOW_BTXS_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QSNOW_BTXS_id, "MemoryOrder", 3, "XSZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QSNOW_BTXS_id, "description", 17, "Snow mixing ratio");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QSNOW_BTXS_id, "units", 7, "kg kg-1");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QSNOW_BTXS_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, QSNOW_BTXE_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QSNOW_BTXE_id, "MemoryOrder", 3, "XEZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QSNOW_BTXE_id, "description", 17, "Snow mixing ratio");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QSNOW_BTXE_id, "units", 7, "kg kg-1");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QSNOW_BTXE_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, QSNOW_BTYS_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QSNOW_BTYS_id, "MemoryOrder", 3, "YSZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QSNOW_BTYS_id, "description", 17, "Snow mixing ratio");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QSNOW_BTYS_id, "units", 7, "kg kg-1");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QSNOW_BTYS_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, QSNOW_BTYE_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QSNOW_BTYE_id, "MemoryOrder", 3, "YEZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QSNOW_BTYE_id, "description", 17, "Snow mixing ratio");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QSNOW_BTYE_id, "units", 7, "kg kg-1");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QSNOW_BTYE_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, QGRAUP_BTXS_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QGRAUP_BTXS_id, "MemoryOrder", 3, "XSZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QGRAUP_BTXS_id, "description", 20, "Graupel mixing ratio");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QGRAUP_BTXS_id, "units", 7, "kg kg-1");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QGRAUP_BTXS_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, QGRAUP_BTXE_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QGRAUP_BTXE_id, "MemoryOrder", 3, "XEZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QGRAUP_BTXE_id, "description", 20, "Graupel mixing ratio");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QGRAUP_BTXE_id, "units", 7, "kg kg-1");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QGRAUP_BTXE_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, QGRAUP_BTYS_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QGRAUP_BTYS_id, "MemoryOrder", 3, "YSZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QGRAUP_BTYS_id, "description", 20, "Graupel mixing ratio");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QGRAUP_BTYS_id, "units", 7, "kg kg-1");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QGRAUP_BTYS_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, QGRAUP_BTYE_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QGRAUP_BTYE_id, "MemoryOrder", 3, "YEZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QGRAUP_BTYE_id, "description", 20, "Graupel mixing ratio");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QGRAUP_BTYE_id, "units", 7, "kg kg-1");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QGRAUP_BTYE_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, QNICE_BXS_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QNICE_BXS_id, "MemoryOrder", 3, "XSZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QNICE_BXS_id, "description", 24, "Ice Number concentration");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QNICE_BXS_id, "units", 6, "  kg-1");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QNICE_BXS_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, QNICE_BXE_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QNICE_BXE_id, "MemoryOrder", 3, "XEZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QNICE_BXE_id, "description", 24, "Ice Number concentration");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QNICE_BXE_id, "units", 6, "  kg-1");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QNICE_BXE_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, QNICE_BYS_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QNICE_BYS_id, "MemoryOrder", 3, "YSZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QNICE_BYS_id, "description", 24, "Ice Number concentration");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QNICE_BYS_id, "units", 6, "  kg-1");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QNICE_BYS_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, QNICE_BYE_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QNICE_BYE_id, "MemoryOrder", 3, "YEZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QNICE_BYE_id, "description", 24, "Ice Number concentration");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QNICE_BYE_id, "units", 6, "  kg-1");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QNICE_BYE_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, QNRAIN_BXS_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QNRAIN_BXS_id, "MemoryOrder", 3, "XSZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QNRAIN_BXS_id, "description", 25, "Rain Number concentration");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QNRAIN_BXS_id, "units", 8, "  kg(-1)");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QNRAIN_BXS_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, QNRAIN_BXE_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QNRAIN_BXE_id, "MemoryOrder", 3, "XEZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QNRAIN_BXE_id, "description", 25, "Rain Number concentration");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QNRAIN_BXE_id, "units", 8, "  kg(-1)");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QNRAIN_BXE_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, QNRAIN_BYS_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QNRAIN_BYS_id, "MemoryOrder", 3, "YSZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QNRAIN_BYS_id, "description", 25, "Rain Number concentration");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QNRAIN_BYS_id, "units", 8, "  kg(-1)");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QNRAIN_BYS_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, QNRAIN_BYE_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QNRAIN_BYE_id, "MemoryOrder", 3, "YEZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QNRAIN_BYE_id, "description", 25, "Rain Number concentration");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QNRAIN_BYE_id, "units", 8, "  kg(-1)");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QNRAIN_BYE_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, QNICE_BTXS_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QNICE_BTXS_id, "MemoryOrder", 3, "XSZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QNICE_BTXS_id, "description", 24, "Ice Number concentration");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QNICE_BTXS_id, "units", 6, "  kg-1");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QNICE_BTXS_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, QNICE_BTXE_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QNICE_BTXE_id, "MemoryOrder", 3, "XEZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QNICE_BTXE_id, "description", 24, "Ice Number concentration");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QNICE_BTXE_id, "units", 6, "  kg-1");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QNICE_BTXE_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, QNICE_BTYS_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QNICE_BTYS_id, "MemoryOrder", 3, "YSZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QNICE_BTYS_id, "description", 24, "Ice Number concentration");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QNICE_BTYS_id, "units", 6, "  kg-1");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QNICE_BTYS_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, QNICE_BTYE_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QNICE_BTYE_id, "MemoryOrder", 3, "YEZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QNICE_BTYE_id, "description", 24, "Ice Number concentration");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QNICE_BTYE_id, "units", 6, "  kg-1");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QNICE_BTYE_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, QNRAIN_BTXS_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QNRAIN_BTXS_id, "MemoryOrder", 3, "XSZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QNRAIN_BTXS_id, "description", 25, "Rain Number concentration");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QNRAIN_BTXS_id, "units", 8, "  kg(-1)");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QNRAIN_BTXS_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, QNRAIN_BTXE_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QNRAIN_BTXE_id, "MemoryOrder", 3, "XEZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QNRAIN_BTXE_id, "description", 25, "Rain Number concentration");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QNRAIN_BTXE_id, "units", 8, "  kg(-1)");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QNRAIN_BTXE_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, QNRAIN_BTYS_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QNRAIN_BTYS_id, "MemoryOrder", 3, "YSZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QNRAIN_BTYS_id, "description", 25, "Rain Number concentration");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QNRAIN_BTYS_id, "units", 8, "  kg(-1)");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QNRAIN_BTYS_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, QNRAIN_BTYE_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QNRAIN_BTYE_id, "MemoryOrder", 3, "YEZ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QNRAIN_BTYE_id, "description", 25, "Rain Number concentration");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QNRAIN_BTYE_id, "units", 8, "  kg(-1)");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, QNRAIN_BTYE_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, HT_SHAD_BXS_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, HT_SHAD_BXS_id, "MemoryOrder", 3, "XS ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, HT_SHAD_BXS_id, "description", 31, "bdy Height of orographic shadow");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, HT_SHAD_BXS_id, "units", 1, "m");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, HT_SHAD_BXS_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, HT_SHAD_BXE_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, HT_SHAD_BXE_id, "MemoryOrder", 3, "XE ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, HT_SHAD_BXE_id, "description", 31, "bdy Height of orographic shadow");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, HT_SHAD_BXE_id, "units", 1, "m");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, HT_SHAD_BXE_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, HT_SHAD_BYS_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, HT_SHAD_BYS_id, "MemoryOrder", 3, "YS ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, HT_SHAD_BYS_id, "description", 31, "bdy Height of orographic shadow");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, HT_SHAD_BYS_id, "units", 1, "m");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, HT_SHAD_BYS_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, HT_SHAD_BYE_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, HT_SHAD_BYE_id, "MemoryOrder", 3, "YE ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, HT_SHAD_BYE_id, "description", 31, "bdy Height of orographic shadow");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, HT_SHAD_BYE_id, "units", 1, "m");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, HT_SHAD_BYE_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, HT_SHAD_BTXS_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, HT_SHAD_BTXS_id, "MemoryOrder", 3, "XS ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, HT_SHAD_BTXS_id, "description", 36, "bdy tend Height of orographic shadow");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, HT_SHAD_BTXS_id, "units", 6, "(m)/dt");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, HT_SHAD_BTXS_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, HT_SHAD_BTXE_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, HT_SHAD_BTXE_id, "MemoryOrder", 3, "XE ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, HT_SHAD_BTXE_id, "description", 36, "bdy tend Height of orographic shadow");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, HT_SHAD_BTXE_id, "units", 6, "(m)/dt");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, HT_SHAD_BTXE_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, HT_SHAD_BTYS_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, HT_SHAD_BTYS_id, "MemoryOrder", 3, "YS ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, HT_SHAD_BTYS_id, "description", 36, "bdy tend Height of orographic shadow");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, HT_SHAD_BTYS_id, "units", 6, "(m)/dt");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, HT_SHAD_BTYS_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, HT_SHAD_BTYE_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, HT_SHAD_BTYE_id, "MemoryOrder", 3, "YE ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, HT_SHAD_BTYE_id, "description", 36, "bdy tend Height of orographic shadow");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, HT_SHAD_BTYE_id, "units", 6, "(m)/dt");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, HT_SHAD_BTYE_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, PC_BXS_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, PC_BXS_id, "MemoryOrder", 3, "XS ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, PC_BXS_id, "description", 39, "bdy perturbation dry air mass in column");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, PC_BXS_id, "units", 2, "Pa");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, PC_BXS_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, PC_BXE_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, PC_BXE_id, "MemoryOrder", 3, "XE ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, PC_BXE_id, "description", 39, "bdy perturbation dry air mass in column");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, PC_BXE_id, "units", 2, "Pa");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, PC_BXE_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, PC_BYS_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, PC_BYS_id, "MemoryOrder", 3, "YS ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, PC_BYS_id, "description", 39, "bdy perturbation dry air mass in column");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, PC_BYS_id, "units", 2, "Pa");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, PC_BYS_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, PC_BYE_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, PC_BYE_id, "MemoryOrder", 3, "YE ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, PC_BYE_id, "description", 39, "bdy perturbation dry air mass in column");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, PC_BYE_id, "units", 2, "Pa");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, PC_BYE_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, PC_BTXS_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, PC_BTXS_id, "MemoryOrder", 3, "XS ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, PC_BTXS_id, "description", 44, "bdy tend perturbation dry air mass in column");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, PC_BTXS_id, "units", 7, "(Pa)/dt");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, PC_BTXS_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, PC_BTXE_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, PC_BTXE_id, "MemoryOrder", 3, "XE ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, PC_BTXE_id, "description", 44, "bdy tend perturbation dry air mass in column");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, PC_BTXE_id, "units", 7, "(Pa)/dt");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, PC_BTXE_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, PC_BTYS_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, PC_BTYS_id, "MemoryOrder", 3, "YS ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, PC_BTYS_id, "description", 44, "bdy tend perturbation dry air mass in column");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, PC_BTYS_id, "units", 7, "(Pa)/dt");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, PC_BTYS_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      static const int wrfbdy_d01_FieldType_att[1] = {104} ;
      stat = nc_put_att_int(ncid, PC_BTYE_id, "FieldType", NC_INT, 1, wrfbdy_d01_FieldType_att);
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, PC_BTYE_id, "MemoryOrder", 3, "YE ");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, PC_BTYE_id, "description", 44, "bdy tend perturbation dry air mass in column");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, PC_BTYE_id, "units", 7, "(Pa)/dt");
      check_err(stat,__LINE__,__FILE__);
   }

   {
      stat = nc_put_att_text(ncid, PC_BTYE_id, "stagger", 1, "");
      check_err(stat,__LINE__,__FILE__);
   }


   /* leave define mode */
   stat = nc_enddef (ncid);
   check_err(stat,__LINE__,__FILE__);

   /* assign variable data */

   stat = nc_close(ncid);
   check_err(stat,__LINE__,__FILE__);

   return 0;
}

static size_t BASEFLOW_chunksizes[3] = {1, 91, 144} ;
static size_t ECHANGE_chunksizes[3] = {1, 91, 144} ;
static size_t EVLAND_chunksizes[3] = {1, 91, 144} ;
static size_t EVPINTR_chunksizes[3] = {1, 91, 144} ;
static size_t EVPSBLN_chunksizes[3] = {1, 91, 144} ;
static size_t EVPSOIL_chunksizes[3] = {1, 91, 144} ;
static size_t EVPTRNS_chunksizes[3] = {1, 91, 144} ;
static size_t FRSAT_chunksizes[3] = {1, 91, 144} ;
static size_t FRSNO_chunksizes[3] = {1, 91, 144} ;
static size_t FRUNST_chunksizes[3] = {1, 91, 144} ;
static size_t FRWLT_chunksizes[3] = {1, 91, 144} ;
static size_t GHLAND_chunksizes[3] = {1, 91, 144} ;
static size_t GRN_chunksizes[3] = {1, 91, 144} ;
static size_t GWETPROF_chunksizes[3] = {1, 91, 144} ;
static size_t GWETROOT_chunksizes[3] = {1, 91, 144} ;
static size_t GWETTOP_chunksizes[3] = {1, 91, 144} ;
static size_t LAI_chunksizes[3] = {1, 91, 144} ;
static size_t LHLAND_chunksizes[3] = {1, 91, 144} ;
static size_t LWLAND_chunksizes[3] = {1, 91, 144} ;
static size_t PARDFLAND_chunksizes[3] = {1, 91, 144} ;
static size_t PARDRLAND_chunksizes[3] = {1, 91, 144} ;
static size_t PRECSNOLAND_chunksizes[3] = {1, 91, 144} ;
static size_t PRECTOTLAND_chunksizes[3] = {1, 91, 144} ;
static size_t PRMC_chunksizes[3] = {1, 91, 144} ;
static size_t QINFIL_chunksizes[3] = {1, 91, 144} ;
static size_t RUNOFF_chunksizes[3] = {1, 91, 144} ;
static size_t RZMC_chunksizes[3] = {1, 91, 144} ;
static size_t SFMC_chunksizes[3] = {1, 91, 144} ;
static size_t SHLAND_chunksizes[3] = {1, 91, 144} ;
static size_t SMLAND_chunksizes[3] = {1, 91, 144} ;
static size_t SNODP_chunksizes[3] = {1, 91, 144} ;
static size_t SNOMAS_chunksizes[3] = {1, 91, 144} ;
static size_t SPLAND_chunksizes[3] = {1, 91, 144} ;
static size_t SPSNOW_chunksizes[3] = {1, 91, 144} ;
static size_t SPWATR_chunksizes[3] = {1, 91, 144} ;
static size_t SWLAND_chunksizes[3] = {1, 91, 144} ;
static size_t TELAND_chunksizes[3] = {1, 91, 144} ;
static size_t TPSNOW_chunksizes[3] = {1, 91, 144} ;
static size_t TSAT_chunksizes[3] = {1, 91, 144} ;
static size_t TSOIL1_chunksizes[3] = {1, 91, 144} ;
static size_t TSOIL2_chunksizes[3] = {1, 91, 144} ;
static size_t TSOIL3_chunksizes[3] = {1, 91, 144} ;
static size_t TSOIL4_chunksizes[3] = {1, 91, 144} ;
static size_t TSOIL5_chunksizes[3] = {1, 91, 144} ;
static size_t TSOIL6_chunksizes[3] = {1, 91, 144} ;
static size_t TSURF_chunksizes[3] = {1, 91, 144} ;
static size_t TUNST_chunksizes[3] = {1, 91, 144} ;
static size_t TWLAND_chunksizes[3] = {1, 91, 144} ;
static size_t TWLT_chunksizes[3] = {1, 91, 144} ;
static size_t WCHANGE_chunksizes[3] = {1, 91, 144} ;
static size_t time_chunksizes[1] = {1} ;

int
create_merr_file(char *file_name, int cmode) {/* create MERR.nc4.ncdumphs.nc */

    int  stat;  /* return status */
    int  ncid;  /* netCDF id */

    /* group ids */
    int MERR_grp;

    /* dimension ids */
    int time_dim;
    int lat_dim;
    int lon_dim;

    /* dimension lengths */
    size_t time_len = NC_UNLIMITED;
    size_t lat_len = 361;
    size_t lon_len = 576;

    /* variable ids */
    int BASEFLOW_id;
    int ECHANGE_id;
    int EVLAND_id;
    int EVPINTR_id;
    int EVPSBLN_id;
    int EVPSOIL_id;
    int EVPTRNS_id;
    int FRSAT_id;
    int FRSNO_id;
    int FRUNST_id;
    int FRWLT_id;
    int GHLAND_id;
    int GRN_id;
    int GWETPROF_id;
    int GWETROOT_id;
    int GWETTOP_id;
    int LAI_id;
    int LHLAND_id;
    int LWLAND_id;
    int PARDFLAND_id;
    int PARDRLAND_id;
    int PRECSNOLAND_id;
    int PRECTOTLAND_id;
    int PRMC_id;
    int QINFIL_id;
    int RUNOFF_id;
    int RZMC_id;
    int SFMC_id;
    int SHLAND_id;
    int SMLAND_id;
    int SNODP_id;
    int SNOMAS_id;
    int SPLAND_id;
    int SPSNOW_id;
    int SPWATR_id;
    int SWLAND_id;
    int TELAND_id;
    int TPSNOW_id;
    int TSAT_id;
    int TSOIL1_id;
    int TSOIL2_id;
    int TSOIL3_id;
    int TSOIL4_id;
    int TSOIL5_id;
    int TSOIL6_id;
    int TSURF_id;
    int TUNST_id;
    int TWLAND_id;
    int TWLT_id;
    int WCHANGE_id;
    int lat_id;
    int lon_id;
    int time_id;

    /* rank (number of dimensions) for each variable */
#   define RANK_BASEFLOW 3
#   define RANK_ECHANGE 3
#   define RANK_EVLAND 3
#   define RANK_EVPINTR 3
#   define RANK_EVPSBLN 3
#   define RANK_EVPSOIL 3
#   define RANK_EVPTRNS 3
#   define RANK_FRSAT 3
#   define RANK_FRSNO 3
#   define RANK_FRUNST 3
#   define RANK_FRWLT 3
#   define RANK_GHLAND 3
#   define RANK_GRN 3
#   define RANK_GWETPROF 3
#   define RANK_GWETROOT 3
#   define RANK_GWETTOP 3
#   define RANK_LAI 3
#   define RANK_LHLAND 3
#   define RANK_LWLAND 3
#   define RANK_PARDFLAND 3
#   define RANK_PARDRLAND 3
#   define RANK_PRECSNOLAND 3
#   define RANK_PRECTOTLAND 3
#   define RANK_PRMC 3
#   define RANK_QINFIL 3
#   define RANK_RUNOFF 3
#   define RANK_RZMC 3
#   define RANK_SFMC 3
#   define RANK_SHLAND 3
#   define RANK_SMLAND 3
#   define RANK_SNODP 3
#   define RANK_SNOMAS 3
#   define RANK_SPLAND 3
#   define RANK_SPSNOW 3
#   define RANK_SPWATR 3
#   define RANK_SWLAND 3
#   define RANK_TELAND 3
#   define RANK_TPSNOW 3
#   define RANK_TSAT 3
#   define RANK_TSOIL1 3
#   define RANK_TSOIL2 3
#   define RANK_TSOIL3 3
#   define RANK_TSOIL4 3
#   define RANK_TSOIL5 3
#   define RANK_TSOIL6 3
#   define RANK_TSURF 3
#   define RANK_TUNST 3
#   define RANK_TWLAND 3
#   define RANK_TWLT 3
#   define RANK_WCHANGE 3
#   define RANK_lat 1
#   define RANK_lon 1
#   define RANK_time 1

    /* variable shapes */
    int BASEFLOW_dims[RANK_BASEFLOW];
    int ECHANGE_dims[RANK_ECHANGE];
    int EVLAND_dims[RANK_EVLAND];
    int EVPINTR_dims[RANK_EVPINTR];
    int EVPSBLN_dims[RANK_EVPSBLN];
    int EVPSOIL_dims[RANK_EVPSOIL];
    int EVPTRNS_dims[RANK_EVPTRNS];
    int FRSAT_dims[RANK_FRSAT];
    int FRSNO_dims[RANK_FRSNO];
    int FRUNST_dims[RANK_FRUNST];
    int FRWLT_dims[RANK_FRWLT];
    int GHLAND_dims[RANK_GHLAND];
    int GRN_dims[RANK_GRN];
    int GWETPROF_dims[RANK_GWETPROF];
    int GWETROOT_dims[RANK_GWETROOT];
    int GWETTOP_dims[RANK_GWETTOP];
    int LAI_dims[RANK_LAI];
    int LHLAND_dims[RANK_LHLAND];
    int LWLAND_dims[RANK_LWLAND];
    int PARDFLAND_dims[RANK_PARDFLAND];
    int PARDRLAND_dims[RANK_PARDRLAND];
    int PRECSNOLAND_dims[RANK_PRECSNOLAND];
    int PRECTOTLAND_dims[RANK_PRECTOTLAND];
    int PRMC_dims[RANK_PRMC];
    int QINFIL_dims[RANK_QINFIL];
    int RUNOFF_dims[RANK_RUNOFF];
    int RZMC_dims[RANK_RZMC];
    int SFMC_dims[RANK_SFMC];
    int SHLAND_dims[RANK_SHLAND];
    int SMLAND_dims[RANK_SMLAND];
    int SNODP_dims[RANK_SNODP];
    int SNOMAS_dims[RANK_SNOMAS];
    int SPLAND_dims[RANK_SPLAND];
    int SPSNOW_dims[RANK_SPSNOW];
    int SPWATR_dims[RANK_SPWATR];
    int SWLAND_dims[RANK_SWLAND];
    int TELAND_dims[RANK_TELAND];
    int TPSNOW_dims[RANK_TPSNOW];
    int TSAT_dims[RANK_TSAT];
    int TSOIL1_dims[RANK_TSOIL1];
    int TSOIL2_dims[RANK_TSOIL2];
    int TSOIL3_dims[RANK_TSOIL3];
    int TSOIL4_dims[RANK_TSOIL4];
    int TSOIL5_dims[RANK_TSOIL5];
    int TSOIL6_dims[RANK_TSOIL6];
    int TSURF_dims[RANK_TSURF];
    int TUNST_dims[RANK_TUNST];
    int TWLAND_dims[RANK_TWLAND];
    int TWLT_dims[RANK_TWLT];
    int WCHANGE_dims[RANK_WCHANGE];
    int lat_dims[RANK_lat];
    int lon_dims[RANK_lon];
    int time_dims[RANK_time];

    /* enter define mode */
    stat = nc_create(file_name, NC_CLOBBER|cmode, &ncid);
    check_err(stat,__LINE__,__FILE__);
    /* stat = nc_put_att_text(ncid, NC_GLOBAL, "_Format", 1, "netCDF-4"); */
    /* check_err(stat,__LINE__,__FILE__); */
    MERR_grp = ncid;

    /* define dimensions */
    stat = nc_def_dim(MERR_grp, "time", time_len, &time_dim);
    check_err(stat,__LINE__,__FILE__);
    stat = nc_def_dim(MERR_grp, "lat", lat_len, &lat_dim);
    check_err(stat,__LINE__,__FILE__);
    stat = nc_def_dim(MERR_grp, "lon", lon_len, &lon_dim);
    check_err(stat,__LINE__,__FILE__);

    /* define variables */

    BASEFLOW_dims[0] = time_dim;
    BASEFLOW_dims[1] = lat_dim;
    BASEFLOW_dims[2] = lon_dim;
    stat = nc_def_var(MERR_grp, "BASEFLOW", NC_FLOAT, RANK_BASEFLOW, BASEFLOW_dims, &BASEFLOW_id);
    check_err(stat,__LINE__,__FILE__);
    if (cmode & NC_NETCDF4)
    {
       stat = nc_def_var_chunking(MERR_grp, BASEFLOW_id, NC_CHUNKED, BASEFLOW_chunksizes);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_deflate(MERR_grp, BASEFLOW_id, NC_SHUFFLE, 1, 0);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_endian(MERR_grp, BASEFLOW_id, NC_ENDIAN_LITTLE);
       check_err(stat,__LINE__,__FILE__);
    }

    ECHANGE_dims[0] = time_dim;
    ECHANGE_dims[1] = lat_dim;
    ECHANGE_dims[2] = lon_dim;
    stat = nc_def_var(MERR_grp, "ECHANGE", NC_FLOAT, RANK_ECHANGE, ECHANGE_dims, &ECHANGE_id);
    check_err(stat,__LINE__,__FILE__);
    if (cmode & NC_NETCDF4)
    {
       stat = nc_def_var_chunking(MERR_grp, ECHANGE_id, NC_CHUNKED, ECHANGE_chunksizes);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_deflate(MERR_grp, ECHANGE_id, NC_SHUFFLE, 1, 0);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_endian(MERR_grp, ECHANGE_id, NC_ENDIAN_LITTLE);
       check_err(stat,__LINE__,__FILE__);
    }

    EVLAND_dims[0] = time_dim;
    EVLAND_dims[1] = lat_dim;
    EVLAND_dims[2] = lon_dim;
    stat = nc_def_var(MERR_grp, "EVLAND", NC_FLOAT, RANK_EVLAND, EVLAND_dims, &EVLAND_id);
    check_err(stat,__LINE__,__FILE__);
    if (cmode & NC_NETCDF4)
    {
       stat = nc_def_var_chunking(MERR_grp, EVLAND_id, NC_CHUNKED, EVLAND_chunksizes);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_deflate(MERR_grp, EVLAND_id, NC_SHUFFLE, 1, 0);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_endian(MERR_grp, EVLAND_id, NC_ENDIAN_LITTLE);
       check_err(stat,__LINE__,__FILE__);
    }

    EVPINTR_dims[0] = time_dim;
    EVPINTR_dims[1] = lat_dim;
    EVPINTR_dims[2] = lon_dim;
    stat = nc_def_var(MERR_grp, "EVPINTR", NC_FLOAT, RANK_EVPINTR, EVPINTR_dims, &EVPINTR_id);
    check_err(stat,__LINE__,__FILE__);
    if (cmode & NC_NETCDF4)
    {
       stat = nc_def_var_chunking(MERR_grp, EVPINTR_id, NC_CHUNKED, EVPINTR_chunksizes);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_deflate(MERR_grp, EVPINTR_id, NC_SHUFFLE, 1, 0);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_endian(MERR_grp, EVPINTR_id, NC_ENDIAN_LITTLE);
       check_err(stat,__LINE__,__FILE__);
    }

    EVPSBLN_dims[0] = time_dim;
    EVPSBLN_dims[1] = lat_dim;
    EVPSBLN_dims[2] = lon_dim;
    stat = nc_def_var(MERR_grp, "EVPSBLN", NC_FLOAT, RANK_EVPSBLN, EVPSBLN_dims, &EVPSBLN_id);
    check_err(stat,__LINE__,__FILE__);
    if (cmode & NC_NETCDF4)
    {
       stat = nc_def_var_chunking(MERR_grp, EVPSBLN_id, NC_CHUNKED, EVPSBLN_chunksizes);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_deflate(MERR_grp, EVPSBLN_id, NC_SHUFFLE, 1, 0);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_endian(MERR_grp, EVPSBLN_id, NC_ENDIAN_LITTLE);
       check_err(stat,__LINE__,__FILE__);
    }

    EVPSOIL_dims[0] = time_dim;
    EVPSOIL_dims[1] = lat_dim;
    EVPSOIL_dims[2] = lon_dim;
    stat = nc_def_var(MERR_grp, "EVPSOIL", NC_FLOAT, RANK_EVPSOIL, EVPSOIL_dims, &EVPSOIL_id);
    check_err(stat,__LINE__,__FILE__);
    if (cmode & NC_NETCDF4)
    {
       stat = nc_def_var_chunking(MERR_grp, EVPSOIL_id, NC_CHUNKED, EVPSOIL_chunksizes);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_deflate(MERR_grp, EVPSOIL_id, NC_SHUFFLE, 1, 0);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_endian(MERR_grp, EVPSOIL_id, NC_ENDIAN_LITTLE);
       check_err(stat,__LINE__,__FILE__);
    }

    EVPTRNS_dims[0] = time_dim;
    EVPTRNS_dims[1] = lat_dim;
    EVPTRNS_dims[2] = lon_dim;
    stat = nc_def_var(MERR_grp, "EVPTRNS", NC_FLOAT, RANK_EVPTRNS, EVPTRNS_dims, &EVPTRNS_id);
    check_err(stat,__LINE__,__FILE__);
    if (cmode & NC_NETCDF4)
    {
       stat = nc_def_var_chunking(MERR_grp, EVPTRNS_id, NC_CHUNKED, EVPTRNS_chunksizes);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_deflate(MERR_grp, EVPTRNS_id, NC_SHUFFLE, 1, 0);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_endian(MERR_grp, EVPTRNS_id, NC_ENDIAN_LITTLE);
       check_err(stat,__LINE__,__FILE__);
    }

    FRSAT_dims[0] = time_dim;
    FRSAT_dims[1] = lat_dim;
    FRSAT_dims[2] = lon_dim;
    stat = nc_def_var(MERR_grp, "FRSAT", NC_FLOAT, RANK_FRSAT, FRSAT_dims, &FRSAT_id);
    check_err(stat,__LINE__,__FILE__);
    if (cmode & NC_NETCDF4)
    {
       stat = nc_def_var_chunking(MERR_grp, FRSAT_id, NC_CHUNKED, FRSAT_chunksizes);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_deflate(MERR_grp, FRSAT_id, NC_SHUFFLE, 1, 0);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_endian(MERR_grp, FRSAT_id, NC_ENDIAN_LITTLE);
       check_err(stat,__LINE__,__FILE__);
    }

    FRSNO_dims[0] = time_dim;
    FRSNO_dims[1] = lat_dim;
    FRSNO_dims[2] = lon_dim;
    stat = nc_def_var(MERR_grp, "FRSNO", NC_FLOAT, RANK_FRSNO, FRSNO_dims, &FRSNO_id);
    check_err(stat,__LINE__,__FILE__);
    if (cmode & NC_NETCDF4)
    {
       stat = nc_def_var_chunking(MERR_grp, FRSNO_id, NC_CHUNKED, FRSNO_chunksizes);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_deflate(MERR_grp, FRSNO_id, NC_SHUFFLE, 1, 0);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_endian(MERR_grp, FRSNO_id, NC_ENDIAN_LITTLE);
       check_err(stat,__LINE__,__FILE__);
    }

    FRUNST_dims[0] = time_dim;
    FRUNST_dims[1] = lat_dim;
    FRUNST_dims[2] = lon_dim;
    stat = nc_def_var(MERR_grp, "FRUNST", NC_FLOAT, RANK_FRUNST, FRUNST_dims, &FRUNST_id);
    check_err(stat,__LINE__,__FILE__);
    if (cmode & NC_NETCDF4)
    {
       stat = nc_def_var_chunking(MERR_grp, FRUNST_id, NC_CHUNKED, FRUNST_chunksizes);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_deflate(MERR_grp, FRUNST_id, NC_SHUFFLE, 1, 0);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_endian(MERR_grp, FRUNST_id, NC_ENDIAN_LITTLE);
       check_err(stat,__LINE__,__FILE__);
    }

    FRWLT_dims[0] = time_dim;
    FRWLT_dims[1] = lat_dim;
    FRWLT_dims[2] = lon_dim;
    stat = nc_def_var(MERR_grp, "FRWLT", NC_FLOAT, RANK_FRWLT, FRWLT_dims, &FRWLT_id);
    check_err(stat,__LINE__,__FILE__);
    if (cmode & NC_NETCDF4)
    {
       stat = nc_def_var_chunking(MERR_grp, FRWLT_id, NC_CHUNKED, FRWLT_chunksizes);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_deflate(MERR_grp, FRWLT_id, NC_SHUFFLE, 1, 0);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_endian(MERR_grp, FRWLT_id, NC_ENDIAN_LITTLE);
       check_err(stat,__LINE__,__FILE__);
    }

    GHLAND_dims[0] = time_dim;
    GHLAND_dims[1] = lat_dim;
    GHLAND_dims[2] = lon_dim;
    stat = nc_def_var(MERR_grp, "GHLAND", NC_FLOAT, RANK_GHLAND, GHLAND_dims, &GHLAND_id);
    check_err(stat,__LINE__,__FILE__);
    if (cmode & NC_NETCDF4)
    {
       stat = nc_def_var_chunking(MERR_grp, GHLAND_id, NC_CHUNKED, GHLAND_chunksizes);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_deflate(MERR_grp, GHLAND_id, NC_SHUFFLE, 1, 0);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_endian(MERR_grp, GHLAND_id, NC_ENDIAN_LITTLE);
       check_err(stat,__LINE__,__FILE__);
    }

    GRN_dims[0] = time_dim;
    GRN_dims[1] = lat_dim;
    GRN_dims[2] = lon_dim;
    stat = nc_def_var(MERR_grp, "GRN", NC_FLOAT, RANK_GRN, GRN_dims, &GRN_id);
    check_err(stat,__LINE__,__FILE__);
    if (cmode & NC_NETCDF4)
    {
       stat = nc_def_var_chunking(MERR_grp, GRN_id, NC_CHUNKED, GRN_chunksizes);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_deflate(MERR_grp, GRN_id, NC_SHUFFLE, 1, 0);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_endian(MERR_grp, GRN_id, NC_ENDIAN_LITTLE);
       check_err(stat,__LINE__,__FILE__);
    }

    GWETPROF_dims[0] = time_dim;
    GWETPROF_dims[1] = lat_dim;
    GWETPROF_dims[2] = lon_dim;
    stat = nc_def_var(MERR_grp, "GWETPROF", NC_FLOAT, RANK_GWETPROF, GWETPROF_dims, &GWETPROF_id);
    check_err(stat,__LINE__,__FILE__);
    if (cmode & NC_NETCDF4)
    {
       stat = nc_def_var_chunking(MERR_grp, GWETPROF_id, NC_CHUNKED, GWETPROF_chunksizes);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_deflate(MERR_grp, GWETPROF_id, NC_SHUFFLE, 1, 0);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_endian(MERR_grp, GWETPROF_id, NC_ENDIAN_LITTLE);
       check_err(stat,__LINE__,__FILE__);
    }

    GWETROOT_dims[0] = time_dim;
    GWETROOT_dims[1] = lat_dim;
    GWETROOT_dims[2] = lon_dim;
    stat = nc_def_var(MERR_grp, "GWETROOT", NC_FLOAT, RANK_GWETROOT, GWETROOT_dims, &GWETROOT_id);
    check_err(stat,__LINE__,__FILE__);
    if (cmode & NC_NETCDF4)
    {
       stat = nc_def_var_chunking(MERR_grp, GWETROOT_id, NC_CHUNKED, GWETROOT_chunksizes);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_deflate(MERR_grp, GWETROOT_id, NC_SHUFFLE, 1, 0);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_endian(MERR_grp, GWETROOT_id, NC_ENDIAN_LITTLE);
       check_err(stat,__LINE__,__FILE__);
    }

    GWETTOP_dims[0] = time_dim;
    GWETTOP_dims[1] = lat_dim;
    GWETTOP_dims[2] = lon_dim;
    stat = nc_def_var(MERR_grp, "GWETTOP", NC_FLOAT, RANK_GWETTOP, GWETTOP_dims, &GWETTOP_id);
    check_err(stat,__LINE__,__FILE__);
    if (cmode & NC_NETCDF4)
    {
       stat = nc_def_var_chunking(MERR_grp, GWETTOP_id, NC_CHUNKED, GWETTOP_chunksizes);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_deflate(MERR_grp, GWETTOP_id, NC_SHUFFLE, 1, 0);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_endian(MERR_grp, GWETTOP_id, NC_ENDIAN_LITTLE);
       check_err(stat,__LINE__,__FILE__);
    }

    LAI_dims[0] = time_dim;
    LAI_dims[1] = lat_dim;
    LAI_dims[2] = lon_dim;
    stat = nc_def_var(MERR_grp, "LAI", NC_FLOAT, RANK_LAI, LAI_dims, &LAI_id);
    check_err(stat,__LINE__,__FILE__);
    if (cmode & NC_NETCDF4)
    {
       stat = nc_def_var_chunking(MERR_grp, LAI_id, NC_CHUNKED, LAI_chunksizes);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_deflate(MERR_grp, LAI_id, NC_SHUFFLE, 1, 0);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_endian(MERR_grp, LAI_id, NC_ENDIAN_LITTLE);
       check_err(stat,__LINE__,__FILE__);
    }

    LHLAND_dims[0] = time_dim;
    LHLAND_dims[1] = lat_dim;
    LHLAND_dims[2] = lon_dim;
    stat = nc_def_var(MERR_grp, "LHLAND", NC_FLOAT, RANK_LHLAND, LHLAND_dims, &LHLAND_id);
    check_err(stat,__LINE__,__FILE__);
    if (cmode & NC_NETCDF4)
    {
       stat = nc_def_var_chunking(MERR_grp, LHLAND_id, NC_CHUNKED, LHLAND_chunksizes);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_deflate(MERR_grp, LHLAND_id, NC_SHUFFLE, 1, 0);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_endian(MERR_grp, LHLAND_id, NC_ENDIAN_LITTLE);
       check_err(stat,__LINE__,__FILE__);
    }

    LWLAND_dims[0] = time_dim;
    LWLAND_dims[1] = lat_dim;
    LWLAND_dims[2] = lon_dim;
    stat = nc_def_var(MERR_grp, "LWLAND", NC_FLOAT, RANK_LWLAND, LWLAND_dims, &LWLAND_id);
    check_err(stat,__LINE__,__FILE__);
    if (cmode & NC_NETCDF4)
    {
       stat = nc_def_var_chunking(MERR_grp, LWLAND_id, NC_CHUNKED, LWLAND_chunksizes);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_deflate(MERR_grp, LWLAND_id, NC_SHUFFLE, 1, 0);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_endian(MERR_grp, LWLAND_id, NC_ENDIAN_LITTLE);
       check_err(stat,__LINE__,__FILE__);
    }

    PARDFLAND_dims[0] = time_dim;
    PARDFLAND_dims[1] = lat_dim;
    PARDFLAND_dims[2] = lon_dim;
    stat = nc_def_var(MERR_grp, "PARDFLAND", NC_FLOAT, RANK_PARDFLAND, PARDFLAND_dims, &PARDFLAND_id);
    check_err(stat,__LINE__,__FILE__);
    if (cmode & NC_NETCDF4)
    {
       stat = nc_def_var_chunking(MERR_grp, PARDFLAND_id, NC_CHUNKED, PARDFLAND_chunksizes);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_deflate(MERR_grp, PARDFLAND_id, NC_SHUFFLE, 1, 0);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_endian(MERR_grp, PARDFLAND_id, NC_ENDIAN_LITTLE);
       check_err(stat,__LINE__,__FILE__);
    }

    PARDRLAND_dims[0] = time_dim;
    PARDRLAND_dims[1] = lat_dim;
    PARDRLAND_dims[2] = lon_dim;
    stat = nc_def_var(MERR_grp, "PARDRLAND", NC_FLOAT, RANK_PARDRLAND, PARDRLAND_dims, &PARDRLAND_id);
    check_err(stat,__LINE__,__FILE__);
    if (cmode & NC_NETCDF4)
    {
       stat = nc_def_var_chunking(MERR_grp, PARDRLAND_id, NC_CHUNKED, PARDRLAND_chunksizes);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_deflate(MERR_grp, PARDRLAND_id, NC_SHUFFLE, 1, 0);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_endian(MERR_grp, PARDRLAND_id, NC_ENDIAN_LITTLE);
       check_err(stat,__LINE__,__FILE__);
    }

    PRECSNOLAND_dims[0] = time_dim;
    PRECSNOLAND_dims[1] = lat_dim;
    PRECSNOLAND_dims[2] = lon_dim;
    stat = nc_def_var(MERR_grp, "PRECSNOLAND", NC_FLOAT, RANK_PRECSNOLAND, PRECSNOLAND_dims, &PRECSNOLAND_id);
    check_err(stat,__LINE__,__FILE__);
    if (cmode & NC_NETCDF4)
    {
       stat = nc_def_var_chunking(MERR_grp, PRECSNOLAND_id, NC_CHUNKED, PRECSNOLAND_chunksizes);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_deflate(MERR_grp, PRECSNOLAND_id, NC_SHUFFLE, 1, 0);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_endian(MERR_grp, PRECSNOLAND_id, NC_ENDIAN_LITTLE);
       check_err(stat,__LINE__,__FILE__);
    }

    PRECTOTLAND_dims[0] = time_dim;
    PRECTOTLAND_dims[1] = lat_dim;
    PRECTOTLAND_dims[2] = lon_dim;
    stat = nc_def_var(MERR_grp, "PRECTOTLAND", NC_FLOAT, RANK_PRECTOTLAND, PRECTOTLAND_dims, &PRECTOTLAND_id);
    check_err(stat,__LINE__,__FILE__);
    if (cmode & NC_NETCDF4)
    {
       stat = nc_def_var_chunking(MERR_grp, PRECTOTLAND_id, NC_CHUNKED, PRECTOTLAND_chunksizes);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_deflate(MERR_grp, PRECTOTLAND_id, NC_SHUFFLE, 1, 0);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_endian(MERR_grp, PRECTOTLAND_id, NC_ENDIAN_LITTLE);
       check_err(stat,__LINE__,__FILE__);
    }

    PRMC_dims[0] = time_dim;
    PRMC_dims[1] = lat_dim;
    PRMC_dims[2] = lon_dim;
    stat = nc_def_var(MERR_grp, "PRMC", NC_FLOAT, RANK_PRMC, PRMC_dims, &PRMC_id);
    check_err(stat,__LINE__,__FILE__);
    if (cmode & NC_NETCDF4)
    {
       stat = nc_def_var_chunking(MERR_grp, PRMC_id, NC_CHUNKED, PRMC_chunksizes);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_deflate(MERR_grp, PRMC_id, NC_SHUFFLE, 1, 0);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_endian(MERR_grp, PRMC_id, NC_ENDIAN_LITTLE);
       check_err(stat,__LINE__,__FILE__);
    }

    QINFIL_dims[0] = time_dim;
    QINFIL_dims[1] = lat_dim;
    QINFIL_dims[2] = lon_dim;
    stat = nc_def_var(MERR_grp, "QINFIL", NC_FLOAT, RANK_QINFIL, QINFIL_dims, &QINFIL_id);
    check_err(stat,__LINE__,__FILE__);
    if (cmode & NC_NETCDF4)
    {
       stat = nc_def_var_chunking(MERR_grp, QINFIL_id, NC_CHUNKED, QINFIL_chunksizes);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_deflate(MERR_grp, QINFIL_id, NC_SHUFFLE, 1, 0);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_endian(MERR_grp, QINFIL_id, NC_ENDIAN_LITTLE);
       check_err(stat,__LINE__,__FILE__);
    }

    RUNOFF_dims[0] = time_dim;
    RUNOFF_dims[1] = lat_dim;
    RUNOFF_dims[2] = lon_dim;
    stat = nc_def_var(MERR_grp, "RUNOFF", NC_FLOAT, RANK_RUNOFF, RUNOFF_dims, &RUNOFF_id);
    check_err(stat,__LINE__,__FILE__);
    if (cmode & NC_NETCDF4)
    {
       stat = nc_def_var_chunking(MERR_grp, RUNOFF_id, NC_CHUNKED, RUNOFF_chunksizes);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_deflate(MERR_grp, RUNOFF_id, NC_SHUFFLE, 1, 0);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_endian(MERR_grp, RUNOFF_id, NC_ENDIAN_LITTLE);
       check_err(stat,__LINE__,__FILE__);
    }

    RZMC_dims[0] = time_dim;
    RZMC_dims[1] = lat_dim;
    RZMC_dims[2] = lon_dim;
    stat = nc_def_var(MERR_grp, "RZMC", NC_FLOAT, RANK_RZMC, RZMC_dims, &RZMC_id);
    check_err(stat,__LINE__,__FILE__);
    if (cmode & NC_NETCDF4)
    {
       stat = nc_def_var_chunking(MERR_grp, RZMC_id, NC_CHUNKED, RZMC_chunksizes);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_deflate(MERR_grp, RZMC_id, NC_SHUFFLE, 1, 0);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_endian(MERR_grp, RZMC_id, NC_ENDIAN_LITTLE);
       check_err(stat,__LINE__,__FILE__);
    }

    SFMC_dims[0] = time_dim;
    SFMC_dims[1] = lat_dim;
    SFMC_dims[2] = lon_dim;
    stat = nc_def_var(MERR_grp, "SFMC", NC_FLOAT, RANK_SFMC, SFMC_dims, &SFMC_id);
    check_err(stat,__LINE__,__FILE__);
    if (cmode & NC_NETCDF4)
    {
       stat = nc_def_var_chunking(MERR_grp, SFMC_id, NC_CHUNKED, SFMC_chunksizes);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_deflate(MERR_grp, SFMC_id, NC_SHUFFLE, 1, 0);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_endian(MERR_grp, SFMC_id, NC_ENDIAN_LITTLE);
       check_err(stat,__LINE__,__FILE__);
    }

    SHLAND_dims[0] = time_dim;
    SHLAND_dims[1] = lat_dim;
    SHLAND_dims[2] = lon_dim;
    stat = nc_def_var(MERR_grp, "SHLAND", NC_FLOAT, RANK_SHLAND, SHLAND_dims, &SHLAND_id);
    check_err(stat,__LINE__,__FILE__);
    if (cmode & NC_NETCDF4)
    {
       stat = nc_def_var_chunking(MERR_grp, SHLAND_id, NC_CHUNKED, SHLAND_chunksizes);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_deflate(MERR_grp, SHLAND_id, NC_SHUFFLE, 1, 0);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_endian(MERR_grp, SHLAND_id, NC_ENDIAN_LITTLE);
       check_err(stat,__LINE__,__FILE__);
    }

    SMLAND_dims[0] = time_dim;
    SMLAND_dims[1] = lat_dim;
    SMLAND_dims[2] = lon_dim;
    stat = nc_def_var(MERR_grp, "SMLAND", NC_FLOAT, RANK_SMLAND, SMLAND_dims, &SMLAND_id);
    check_err(stat,__LINE__,__FILE__);
    if (cmode & NC_NETCDF4)
    {
       stat = nc_def_var_chunking(MERR_grp, SMLAND_id, NC_CHUNKED, SMLAND_chunksizes);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_deflate(MERR_grp, SMLAND_id, NC_SHUFFLE, 1, 0);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_endian(MERR_grp, SMLAND_id, NC_ENDIAN_LITTLE);
       check_err(stat,__LINE__,__FILE__);
    }

    SNODP_dims[0] = time_dim;
    SNODP_dims[1] = lat_dim;
    SNODP_dims[2] = lon_dim;
    stat = nc_def_var(MERR_grp, "SNODP", NC_FLOAT, RANK_SNODP, SNODP_dims, &SNODP_id);
    check_err(stat,__LINE__,__FILE__);
    if (cmode & NC_NETCDF4)
    {
       stat = nc_def_var_chunking(MERR_grp, SNODP_id, NC_CHUNKED, SNODP_chunksizes);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_deflate(MERR_grp, SNODP_id, NC_SHUFFLE, 1, 0);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_endian(MERR_grp, SNODP_id, NC_ENDIAN_LITTLE);
       check_err(stat,__LINE__,__FILE__);
    }

    SNOMAS_dims[0] = time_dim;
    SNOMAS_dims[1] = lat_dim;
    SNOMAS_dims[2] = lon_dim;
    stat = nc_def_var(MERR_grp, "SNOMAS", NC_FLOAT, RANK_SNOMAS, SNOMAS_dims, &SNOMAS_id);
    check_err(stat,__LINE__,__FILE__);
    if (cmode & NC_NETCDF4)
    {
       stat = nc_def_var_chunking(MERR_grp, SNOMAS_id, NC_CHUNKED, SNOMAS_chunksizes);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_deflate(MERR_grp, SNOMAS_id, NC_SHUFFLE, 1, 0);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_endian(MERR_grp, SNOMAS_id, NC_ENDIAN_LITTLE);
       check_err(stat,__LINE__,__FILE__);
    }

    SPLAND_dims[0] = time_dim;
    SPLAND_dims[1] = lat_dim;
    SPLAND_dims[2] = lon_dim;
    stat = nc_def_var(MERR_grp, "SPLAND", NC_FLOAT, RANK_SPLAND, SPLAND_dims, &SPLAND_id);
    check_err(stat,__LINE__,__FILE__);
    if (cmode & NC_NETCDF4)
    {
       stat = nc_def_var_chunking(MERR_grp, SPLAND_id, NC_CHUNKED, SPLAND_chunksizes);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_deflate(MERR_grp, SPLAND_id, NC_SHUFFLE, 1, 0);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_endian(MERR_grp, SPLAND_id, NC_ENDIAN_LITTLE);
       check_err(stat,__LINE__,__FILE__);
    }

    SPSNOW_dims[0] = time_dim;
    SPSNOW_dims[1] = lat_dim;
    SPSNOW_dims[2] = lon_dim;
    stat = nc_def_var(MERR_grp, "SPSNOW", NC_FLOAT, RANK_SPSNOW, SPSNOW_dims, &SPSNOW_id);
    check_err(stat,__LINE__,__FILE__);
    if (cmode & NC_NETCDF4)
    {
       stat = nc_def_var_chunking(MERR_grp, SPSNOW_id, NC_CHUNKED, SPSNOW_chunksizes);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_deflate(MERR_grp, SPSNOW_id, NC_SHUFFLE, 1, 0);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_endian(MERR_grp, SPSNOW_id, NC_ENDIAN_LITTLE);
       check_err(stat,__LINE__,__FILE__);
    }

    SPWATR_dims[0] = time_dim;
    SPWATR_dims[1] = lat_dim;
    SPWATR_dims[2] = lon_dim;
    stat = nc_def_var(MERR_grp, "SPWATR", NC_FLOAT, RANK_SPWATR, SPWATR_dims, &SPWATR_id);
    check_err(stat,__LINE__,__FILE__);
    if (cmode & NC_NETCDF4)
    {
       stat = nc_def_var_chunking(MERR_grp, SPWATR_id, NC_CHUNKED, SPWATR_chunksizes);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_deflate(MERR_grp, SPWATR_id, NC_SHUFFLE, 1, 0);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_endian(MERR_grp, SPWATR_id, NC_ENDIAN_LITTLE);
       check_err(stat,__LINE__,__FILE__);
    }

    SWLAND_dims[0] = time_dim;
    SWLAND_dims[1] = lat_dim;
    SWLAND_dims[2] = lon_dim;
    stat = nc_def_var(MERR_grp, "SWLAND", NC_FLOAT, RANK_SWLAND, SWLAND_dims, &SWLAND_id);
    check_err(stat,__LINE__,__FILE__);
    if (cmode & NC_NETCDF4)
    {
       stat = nc_def_var_chunking(MERR_grp, SWLAND_id, NC_CHUNKED, SWLAND_chunksizes);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_deflate(MERR_grp, SWLAND_id, NC_SHUFFLE, 1, 0);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_endian(MERR_grp, SWLAND_id, NC_ENDIAN_LITTLE);
       check_err(stat,__LINE__,__FILE__);
    }

    TELAND_dims[0] = time_dim;
    TELAND_dims[1] = lat_dim;
    TELAND_dims[2] = lon_dim;
    stat = nc_def_var(MERR_grp, "TELAND", NC_FLOAT, RANK_TELAND, TELAND_dims, &TELAND_id);
    check_err(stat,__LINE__,__FILE__);
    if (cmode & NC_NETCDF4)
    {
       stat = nc_def_var_chunking(MERR_grp, TELAND_id, NC_CHUNKED, TELAND_chunksizes);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_deflate(MERR_grp, TELAND_id, NC_SHUFFLE, 1, 0);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_endian(MERR_grp, TELAND_id, NC_ENDIAN_LITTLE);
       check_err(stat,__LINE__,__FILE__);
    }

    TPSNOW_dims[0] = time_dim;
    TPSNOW_dims[1] = lat_dim;
    TPSNOW_dims[2] = lon_dim;
    stat = nc_def_var(MERR_grp, "TPSNOW", NC_FLOAT, RANK_TPSNOW, TPSNOW_dims, &TPSNOW_id);
    check_err(stat,__LINE__,__FILE__);
    if (cmode & NC_NETCDF4)
    {
       stat = nc_def_var_chunking(MERR_grp, TPSNOW_id, NC_CHUNKED, TPSNOW_chunksizes);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_deflate(MERR_grp, TPSNOW_id, NC_SHUFFLE, 1, 0);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_endian(MERR_grp, TPSNOW_id, NC_ENDIAN_LITTLE);
       check_err(stat,__LINE__,__FILE__);
    }

    TSAT_dims[0] = time_dim;
    TSAT_dims[1] = lat_dim;
    TSAT_dims[2] = lon_dim;
    stat = nc_def_var(MERR_grp, "TSAT", NC_FLOAT, RANK_TSAT, TSAT_dims, &TSAT_id);
    check_err(stat,__LINE__,__FILE__);
    if (cmode & NC_NETCDF4)
    {
       stat = nc_def_var_chunking(MERR_grp, TSAT_id, NC_CHUNKED, TSAT_chunksizes);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_deflate(MERR_grp, TSAT_id, NC_SHUFFLE, 1, 0);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_endian(MERR_grp, TSAT_id, NC_ENDIAN_LITTLE);
       check_err(stat,__LINE__,__FILE__);
    }

    TSOIL1_dims[0] = time_dim;
    TSOIL1_dims[1] = lat_dim;
    TSOIL1_dims[2] = lon_dim;
    stat = nc_def_var(MERR_grp, "TSOIL1", NC_FLOAT, RANK_TSOIL1, TSOIL1_dims, &TSOIL1_id);
    check_err(stat,__LINE__,__FILE__);
    if (cmode & NC_NETCDF4)
    {
       stat = nc_def_var_chunking(MERR_grp, TSOIL1_id, NC_CHUNKED, TSOIL1_chunksizes);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_deflate(MERR_grp, TSOIL1_id, NC_SHUFFLE, 1, 0);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_endian(MERR_grp, TSOIL1_id, NC_ENDIAN_LITTLE);
       check_err(stat,__LINE__,__FILE__);
    }

    TSOIL2_dims[0] = time_dim;
    TSOIL2_dims[1] = lat_dim;
    TSOIL2_dims[2] = lon_dim;
    stat = nc_def_var(MERR_grp, "TSOIL2", NC_FLOAT, RANK_TSOIL2, TSOIL2_dims, &TSOIL2_id);
    check_err(stat,__LINE__,__FILE__);
    if (cmode & NC_NETCDF4)
    {
       stat = nc_def_var_chunking(MERR_grp, TSOIL2_id, NC_CHUNKED, TSOIL2_chunksizes);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_deflate(MERR_grp, TSOIL2_id, NC_SHUFFLE, 1, 0);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_endian(MERR_grp, TSOIL2_id, NC_ENDIAN_LITTLE);
       check_err(stat,__LINE__,__FILE__);
    }

    TSOIL3_dims[0] = time_dim;
    TSOIL3_dims[1] = lat_dim;
    TSOIL3_dims[2] = lon_dim;
    stat = nc_def_var(MERR_grp, "TSOIL3", NC_FLOAT, RANK_TSOIL3, TSOIL3_dims, &TSOIL3_id);
    check_err(stat,__LINE__,__FILE__);
    if (cmode & NC_NETCDF4)
    {
       stat = nc_def_var_chunking(MERR_grp, TSOIL3_id, NC_CHUNKED, TSOIL3_chunksizes);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_deflate(MERR_grp, TSOIL3_id, NC_SHUFFLE, 1, 0);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_endian(MERR_grp, TSOIL3_id, NC_ENDIAN_LITTLE);
       check_err(stat,__LINE__,__FILE__);
    }

    TSOIL4_dims[0] = time_dim;
    TSOIL4_dims[1] = lat_dim;
    TSOIL4_dims[2] = lon_dim;
    stat = nc_def_var(MERR_grp, "TSOIL4", NC_FLOAT, RANK_TSOIL4, TSOIL4_dims, &TSOIL4_id);
    check_err(stat,__LINE__,__FILE__);
    if (cmode & NC_NETCDF4)
    {
       stat = nc_def_var_chunking(MERR_grp, TSOIL4_id, NC_CHUNKED, TSOIL4_chunksizes);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_deflate(MERR_grp, TSOIL4_id, NC_SHUFFLE, 1, 0);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_endian(MERR_grp, TSOIL4_id, NC_ENDIAN_LITTLE);
       check_err(stat,__LINE__,__FILE__);
    }

    TSOIL5_dims[0] = time_dim;
    TSOIL5_dims[1] = lat_dim;
    TSOIL5_dims[2] = lon_dim;
    stat = nc_def_var(MERR_grp, "TSOIL5", NC_FLOAT, RANK_TSOIL5, TSOIL5_dims, &TSOIL5_id);
    check_err(stat,__LINE__,__FILE__);
    if (cmode & NC_NETCDF4)
    {
       stat = nc_def_var_chunking(MERR_grp, TSOIL5_id, NC_CHUNKED, TSOIL5_chunksizes);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_deflate(MERR_grp, TSOIL5_id, NC_SHUFFLE, 1, 0);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_endian(MERR_grp, TSOIL5_id, NC_ENDIAN_LITTLE);
       check_err(stat,__LINE__,__FILE__);
    }

    TSOIL6_dims[0] = time_dim;
    TSOIL6_dims[1] = lat_dim;
    TSOIL6_dims[2] = lon_dim;
    stat = nc_def_var(MERR_grp, "TSOIL6", NC_FLOAT, RANK_TSOIL6, TSOIL6_dims, &TSOIL6_id);
    check_err(stat,__LINE__,__FILE__);
    if (cmode & NC_NETCDF4)
    {
       stat = nc_def_var_chunking(MERR_grp, TSOIL6_id, NC_CHUNKED, TSOIL6_chunksizes);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_deflate(MERR_grp, TSOIL6_id, NC_SHUFFLE, 1, 0);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_endian(MERR_grp, TSOIL6_id, NC_ENDIAN_LITTLE);
       check_err(stat,__LINE__,__FILE__);
    }

    TSURF_dims[0] = time_dim;
    TSURF_dims[1] = lat_dim;
    TSURF_dims[2] = lon_dim;
    stat = nc_def_var(MERR_grp, "TSURF", NC_FLOAT, RANK_TSURF, TSURF_dims, &TSURF_id);
    check_err(stat,__LINE__,__FILE__);
    if (cmode & NC_NETCDF4)
    {
       stat = nc_def_var_chunking(MERR_grp, TSURF_id, NC_CHUNKED, TSURF_chunksizes);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_deflate(MERR_grp, TSURF_id, NC_SHUFFLE, 1, 0);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_endian(MERR_grp, TSURF_id, NC_ENDIAN_LITTLE);
       check_err(stat,__LINE__,__FILE__);
    }

    TUNST_dims[0] = time_dim;
    TUNST_dims[1] = lat_dim;
    TUNST_dims[2] = lon_dim;
    stat = nc_def_var(MERR_grp, "TUNST", NC_FLOAT, RANK_TUNST, TUNST_dims, &TUNST_id);
    check_err(stat,__LINE__,__FILE__);
    if (cmode & NC_NETCDF4)
    {
       stat = nc_def_var_chunking(MERR_grp, TUNST_id, NC_CHUNKED, TUNST_chunksizes);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_deflate(MERR_grp, TUNST_id, NC_SHUFFLE, 1, 0);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_endian(MERR_grp, TUNST_id, NC_ENDIAN_LITTLE);
       check_err(stat,__LINE__,__FILE__);
    }

    TWLAND_dims[0] = time_dim;
    TWLAND_dims[1] = lat_dim;
    TWLAND_dims[2] = lon_dim;
    stat = nc_def_var(MERR_grp, "TWLAND", NC_FLOAT, RANK_TWLAND, TWLAND_dims, &TWLAND_id);
    check_err(stat,__LINE__,__FILE__);
    if (cmode & NC_NETCDF4)
    {
       stat = nc_def_var_chunking(MERR_grp, TWLAND_id, NC_CHUNKED, TWLAND_chunksizes);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_deflate(MERR_grp, TWLAND_id, NC_SHUFFLE, 1, 0);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_endian(MERR_grp, TWLAND_id, NC_ENDIAN_LITTLE);
       check_err(stat,__LINE__,__FILE__);
    }

    TWLT_dims[0] = time_dim;
    TWLT_dims[1] = lat_dim;
    TWLT_dims[2] = lon_dim;
    stat = nc_def_var(MERR_grp, "TWLT", NC_FLOAT, RANK_TWLT, TWLT_dims, &TWLT_id);
    check_err(stat,__LINE__,__FILE__);
    if (cmode & NC_NETCDF4)
    {
       stat = nc_def_var_chunking(MERR_grp, TWLT_id, NC_CHUNKED, TWLT_chunksizes);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_deflate(MERR_grp, TWLT_id, NC_SHUFFLE, 1, 0);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_endian(MERR_grp, TWLT_id, NC_ENDIAN_LITTLE);
       check_err(stat,__LINE__,__FILE__);
    }

    WCHANGE_dims[0] = time_dim;
    WCHANGE_dims[1] = lat_dim;
    WCHANGE_dims[2] = lon_dim;
    stat = nc_def_var(MERR_grp, "WCHANGE", NC_FLOAT, RANK_WCHANGE, WCHANGE_dims, &WCHANGE_id);
    check_err(stat,__LINE__,__FILE__);
    if (cmode & NC_NETCDF4)
    {
       stat = nc_def_var_chunking(MERR_grp, WCHANGE_id, NC_CHUNKED, WCHANGE_chunksizes);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_deflate(MERR_grp, WCHANGE_id, NC_SHUFFLE, 1, 0);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_endian(MERR_grp, WCHANGE_id, NC_ENDIAN_LITTLE);
       check_err(stat,__LINE__,__FILE__);
    }

    lat_dims[0] = lat_dim;
    stat = nc_def_var(MERR_grp, "lat", NC_DOUBLE, RANK_lat, lat_dims, &lat_id);
    check_err(stat,__LINE__,__FILE__);
    if (cmode & NC_NETCDF4)
    {
       stat = nc_def_var_chunking(MERR_grp, lat_id, NC_CONTIGUOUS, NULL);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_endian(MERR_grp, lat_id, NC_ENDIAN_LITTLE);
       check_err(stat,__LINE__,__FILE__);
    }
    lon_dims[0] = lon_dim;
    stat = nc_def_var(MERR_grp, "lon", NC_DOUBLE, RANK_lon, lon_dims, &lon_id);
    check_err(stat,__LINE__,__FILE__);
    if (cmode & NC_NETCDF4)
    {
       stat = nc_def_var_chunking(MERR_grp, lon_id, NC_CONTIGUOUS, NULL);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_endian(MERR_grp, lon_id, NC_ENDIAN_LITTLE);
       check_err(stat,__LINE__,__FILE__);
    }
    time_dims[0] = time_dim;
    stat = nc_def_var(MERR_grp, "time", NC_INT, RANK_time, time_dims, &time_id);
    check_err(stat,__LINE__,__FILE__);
    if (cmode & NC_NETCDF4)
    {
       stat = nc_def_var_chunking(MERR_grp, time_id, NC_CHUNKED, time_chunksizes);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_deflate(MERR_grp, time_id, NC_SHUFFLE, 1, 0);
       check_err(stat,__LINE__,__FILE__);
       stat = nc_def_var_endian(MERR_grp, time_id, NC_ENDIAN_LITTLE);
       check_err(stat,__LINE__,__FILE__);
    }

    /* assign global attributes */

    {
    stat = nc_put_att_text(MERR_grp, NC_GLOBAL, "History", 120, "Wed Sep 20 09:06:02 2017: ncks -L 0 MERR.nc4 MERR.uncompressed.nc4\nOriginal file generated: Mon Mar 23 09:27:34 2015 GMT");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, NC_GLOBAL, "Comment", 58, "GMAO filename: d5124_m2_jan00.tavg1_2d_lnd_Nx.20100105.nc4");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, NC_GLOBAL, "Filename", 39, "MERRA2_300.tavg1_2d_lnd_Nx.20100105.nc4");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, NC_GLOBAL, "Conventions", 4, "CF-1");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, NC_GLOBAL, "Institution", 44, "NASA Global Modeling and Assimilation Office");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, NC_GLOBAL, "References", 25, "http://gmao.gsfc.nasa.gov");
    check_err(stat,__LINE__,__FILE__);
    }

    /* { */
    /* stat = nc_put_att_text(MERR_grp, NC_GLOBAL, "Format", 14, "NetCDF-4/HDF-5"); */
    /* check_err(stat,__LINE__,__FILE__); */
    /* } */

    {
    stat = nc_put_att_text(MERR_grp, NC_GLOBAL, "SpatialCoverage", 6, "global");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, NC_GLOBAL, "VersionID", 6, "5.12.4");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, NC_GLOBAL, "TemporalRange", 24, "1980-01-01 -> 2016-12-31");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, NC_GLOBAL, "identifier_product_doi_authority", 18, "http://dx.doi.org/");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, NC_GLOBAL, "ShortName", 9, "M2T1NXLND");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, NC_GLOBAL, "RangeBeginningDate", 10, "2010-01-04");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, NC_GLOBAL, "RangeBeginningTime", 15, "23:30:00.000000");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, NC_GLOBAL, "RangeEndingDate", 10, "2010-01-05");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, NC_GLOBAL, "RangeEndingTime", 15, "00:30:00.000000");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, NC_GLOBAL, "GranuleID", 39, "MERRA2_300.tavg1_2d_lnd_Nx.20100105.nc4");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, NC_GLOBAL, "ProductionDateTime", 53, "Original file generated: Mon Mar 23 09:27:34 2015 GMT");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, NC_GLOBAL, "LongName", 100, "MERRA2 tavg1_2d_lnd_Nx: 2d,1-Hourly,Time-Averaged,Single-Level,Assimilation,Land Surface Diagnostics");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, NC_GLOBAL, "Title", 100, "MERRA2 tavg1_2d_lnd_Nx: 2d,1-Hourly,Time-Averaged,Single-Level,Assimilation,Land Surface Diagnostics");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, NC_GLOBAL, "SouthernmostLatitude", 5, "-90.0");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, NC_GLOBAL, "NorthernmostLatitude", 4, "90.0");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, NC_GLOBAL, "WesternmostLongitude", 6, "-180.0");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, NC_GLOBAL, "EasternmostLongitude", 7, "179.375");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, NC_GLOBAL, "LatitudeResolution", 3, "0.5");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, NC_GLOBAL, "LongitudeResolution", 5, "0.625");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, NC_GLOBAL, "DataResolution", 11, "0.5 x 0.625");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, NC_GLOBAL, "identifier_product_doi", 3, "TBD");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, NC_GLOBAL, "Source", 24, "CVS tag: GEOSadas-5_12_4");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, NC_GLOBAL, "Contact", 25, "http://gmao.gsfc.nasa.gov");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, NC_GLOBAL, "NCO", 5, "4.6.7");
    check_err(stat,__LINE__,__FILE__);
    }


    /* assign per-variable attributes */

    {
    stat = nc_put_att_text(MERR_grp, BASEFLOW_id, "long_name", 13, "baseflow_flux");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, BASEFLOW_id, "units", 10, "kg m-2 s-1");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float _FillValue_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, BASEFLOW_id, "_FillValue", NC_FLOAT, 1, _FillValue_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float missing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, BASEFLOW_id, "missing_value", NC_FLOAT, 1, missing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float fmissing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, BASEFLOW_id, "fmissing_value", NC_FLOAT, 1, fmissing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float scale_factor_att[1] = {((float)1)} ;
    stat = nc_put_att_float(MERR_grp, BASEFLOW_id, "scale_factor", NC_FLOAT, 1, scale_factor_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float add_offset_att[1] = {((float)0)} ;
    stat = nc_put_att_float(MERR_grp, BASEFLOW_id, "add_offset", NC_FLOAT, 1, add_offset_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, BASEFLOW_id, "standard_name", 13, "baseflow_flux");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmax_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, BASEFLOW_id, "vmax", NC_FLOAT, 1, vmax_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmin_att[1] = {((float)-9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, BASEFLOW_id, "vmin", NC_FLOAT, 1, vmin_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float valid_range_att[2] = {((float)-9.9999999e+14), ((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, BASEFLOW_id, "valid_range", NC_FLOAT, 2, valid_range_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, ECHANGE_id, "long_name", 35, "rate_of_change_of_total_land_energy");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, ECHANGE_id, "units", 5, "W m-2");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float _FillValue_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, ECHANGE_id, "_FillValue", NC_FLOAT, 1, _FillValue_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float missing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, ECHANGE_id, "missing_value", NC_FLOAT, 1, missing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float fmissing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, ECHANGE_id, "fmissing_value", NC_FLOAT, 1, fmissing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float scale_factor_att[1] = {((float)1)} ;
    stat = nc_put_att_float(MERR_grp, ECHANGE_id, "scale_factor", NC_FLOAT, 1, scale_factor_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float add_offset_att[1] = {((float)0)} ;
    stat = nc_put_att_float(MERR_grp, ECHANGE_id, "add_offset", NC_FLOAT, 1, add_offset_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, ECHANGE_id, "standard_name", 35, "rate_of_change_of_total_land_energy");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmax_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, ECHANGE_id, "vmax", NC_FLOAT, 1, vmax_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmin_att[1] = {((float)-9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, ECHANGE_id, "vmin", NC_FLOAT, 1, vmin_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float valid_range_att[2] = {((float)-9.9999999e+14), ((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, ECHANGE_id, "valid_range", NC_FLOAT, 2, valid_range_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, EVLAND_id, "long_name", 16, "Evaporation_land");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, EVLAND_id, "units", 10, "kg m-2 s-1");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float _FillValue_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, EVLAND_id, "_FillValue", NC_FLOAT, 1, _FillValue_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float missing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, EVLAND_id, "missing_value", NC_FLOAT, 1, missing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float fmissing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, EVLAND_id, "fmissing_value", NC_FLOAT, 1, fmissing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float scale_factor_att[1] = {((float)1)} ;
    stat = nc_put_att_float(MERR_grp, EVLAND_id, "scale_factor", NC_FLOAT, 1, scale_factor_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float add_offset_att[1] = {((float)0)} ;
    stat = nc_put_att_float(MERR_grp, EVLAND_id, "add_offset", NC_FLOAT, 1, add_offset_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, EVLAND_id, "standard_name", 16, "Evaporation_land");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmax_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, EVLAND_id, "vmax", NC_FLOAT, 1, vmax_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmin_att[1] = {((float)-9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, EVLAND_id, "vmin", NC_FLOAT, 1, vmin_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float valid_range_att[2] = {((float)-9.9999999e+14), ((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, EVLAND_id, "valid_range", NC_FLOAT, 2, valid_range_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, EVPINTR_id, "long_name", 29, "interception_loss_energy_flux");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, EVPINTR_id, "units", 5, "W m-2");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float _FillValue_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, EVPINTR_id, "_FillValue", NC_FLOAT, 1, _FillValue_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float missing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, EVPINTR_id, "missing_value", NC_FLOAT, 1, missing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float fmissing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, EVPINTR_id, "fmissing_value", NC_FLOAT, 1, fmissing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float scale_factor_att[1] = {((float)1)} ;
    stat = nc_put_att_float(MERR_grp, EVPINTR_id, "scale_factor", NC_FLOAT, 1, scale_factor_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float add_offset_att[1] = {((float)0)} ;
    stat = nc_put_att_float(MERR_grp, EVPINTR_id, "add_offset", NC_FLOAT, 1, add_offset_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, EVPINTR_id, "standard_name", 29, "interception_loss_energy_flux");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmax_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, EVPINTR_id, "vmax", NC_FLOAT, 1, vmax_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmin_att[1] = {((float)-9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, EVPINTR_id, "vmin", NC_FLOAT, 1, vmin_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float valid_range_att[2] = {((float)-9.9999999e+14), ((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, EVPINTR_id, "valid_range", NC_FLOAT, 2, valid_range_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, EVPSBLN_id, "long_name", 32, "snow_ice_evaporation_energy_flux");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, EVPSBLN_id, "units", 5, "W m-2");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float _FillValue_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, EVPSBLN_id, "_FillValue", NC_FLOAT, 1, _FillValue_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float missing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, EVPSBLN_id, "missing_value", NC_FLOAT, 1, missing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float fmissing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, EVPSBLN_id, "fmissing_value", NC_FLOAT, 1, fmissing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float scale_factor_att[1] = {((float)1)} ;
    stat = nc_put_att_float(MERR_grp, EVPSBLN_id, "scale_factor", NC_FLOAT, 1, scale_factor_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float add_offset_att[1] = {((float)0)} ;
    stat = nc_put_att_float(MERR_grp, EVPSBLN_id, "add_offset", NC_FLOAT, 1, add_offset_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, EVPSBLN_id, "standard_name", 32, "snow_ice_evaporation_energy_flux");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmax_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, EVPSBLN_id, "vmax", NC_FLOAT, 1, vmax_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmin_att[1] = {((float)-9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, EVPSBLN_id, "vmin", NC_FLOAT, 1, vmin_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float valid_range_att[2] = {((float)-9.9999999e+14), ((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, EVPSBLN_id, "valid_range", NC_FLOAT, 2, valid_range_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, EVPSOIL_id, "long_name", 25, "baresoil_evap_energy_flux");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, EVPSOIL_id, "units", 5, "W m-2");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float _FillValue_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, EVPSOIL_id, "_FillValue", NC_FLOAT, 1, _FillValue_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float missing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, EVPSOIL_id, "missing_value", NC_FLOAT, 1, missing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float fmissing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, EVPSOIL_id, "fmissing_value", NC_FLOAT, 1, fmissing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float scale_factor_att[1] = {((float)1)} ;
    stat = nc_put_att_float(MERR_grp, EVPSOIL_id, "scale_factor", NC_FLOAT, 1, scale_factor_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float add_offset_att[1] = {((float)0)} ;
    stat = nc_put_att_float(MERR_grp, EVPSOIL_id, "add_offset", NC_FLOAT, 1, add_offset_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, EVPSOIL_id, "standard_name", 25, "baresoil_evap_energy_flux");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmax_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, EVPSOIL_id, "vmax", NC_FLOAT, 1, vmax_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmin_att[1] = {((float)-9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, EVPSOIL_id, "vmin", NC_FLOAT, 1, vmin_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float valid_range_att[2] = {((float)-9.9999999e+14), ((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, EVPSOIL_id, "valid_range", NC_FLOAT, 2, valid_range_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, EVPTRNS_id, "long_name", 25, "transpiration_energy_flux");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, EVPTRNS_id, "units", 5, "W m-2");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float _FillValue_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, EVPTRNS_id, "_FillValue", NC_FLOAT, 1, _FillValue_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float missing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, EVPTRNS_id, "missing_value", NC_FLOAT, 1, missing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float fmissing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, EVPTRNS_id, "fmissing_value", NC_FLOAT, 1, fmissing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float scale_factor_att[1] = {((float)1)} ;
    stat = nc_put_att_float(MERR_grp, EVPTRNS_id, "scale_factor", NC_FLOAT, 1, scale_factor_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float add_offset_att[1] = {((float)0)} ;
    stat = nc_put_att_float(MERR_grp, EVPTRNS_id, "add_offset", NC_FLOAT, 1, add_offset_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, EVPTRNS_id, "standard_name", 25, "transpiration_energy_flux");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmax_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, EVPTRNS_id, "vmax", NC_FLOAT, 1, vmax_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmin_att[1] = {((float)-9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, EVPTRNS_id, "vmin", NC_FLOAT, 1, vmin_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float valid_range_att[2] = {((float)-9.9999999e+14), ((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, EVPTRNS_id, "valid_range", NC_FLOAT, 2, valid_range_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, FRSAT_id, "long_name", 33, "fractional_area_of_saturated_zone");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, FRSAT_id, "units", 1, "1");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float _FillValue_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, FRSAT_id, "_FillValue", NC_FLOAT, 1, _FillValue_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float missing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, FRSAT_id, "missing_value", NC_FLOAT, 1, missing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float fmissing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, FRSAT_id, "fmissing_value", NC_FLOAT, 1, fmissing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float scale_factor_att[1] = {((float)1)} ;
    stat = nc_put_att_float(MERR_grp, FRSAT_id, "scale_factor", NC_FLOAT, 1, scale_factor_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float add_offset_att[1] = {((float)0)} ;
    stat = nc_put_att_float(MERR_grp, FRSAT_id, "add_offset", NC_FLOAT, 1, add_offset_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, FRSAT_id, "standard_name", 33, "fractional_area_of_saturated_zone");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmax_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, FRSAT_id, "vmax", NC_FLOAT, 1, vmax_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmin_att[1] = {((float)-9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, FRSAT_id, "vmin", NC_FLOAT, 1, vmin_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float valid_range_att[2] = {((float)-9.9999999e+14), ((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, FRSAT_id, "valid_range", NC_FLOAT, 2, valid_range_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, FRSNO_id, "long_name", 33, "fractional_area_of_land_snowcover");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, FRSNO_id, "units", 1, "1");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float _FillValue_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, FRSNO_id, "_FillValue", NC_FLOAT, 1, _FillValue_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float missing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, FRSNO_id, "missing_value", NC_FLOAT, 1, missing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float fmissing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, FRSNO_id, "fmissing_value", NC_FLOAT, 1, fmissing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float scale_factor_att[1] = {((float)1)} ;
    stat = nc_put_att_float(MERR_grp, FRSNO_id, "scale_factor", NC_FLOAT, 1, scale_factor_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float add_offset_att[1] = {((float)0)} ;
    stat = nc_put_att_float(MERR_grp, FRSNO_id, "add_offset", NC_FLOAT, 1, add_offset_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, FRSNO_id, "standard_name", 33, "fractional_area_of_land_snowcover");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmax_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, FRSNO_id, "vmax", NC_FLOAT, 1, vmax_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmin_att[1] = {((float)-9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, FRSNO_id, "vmin", NC_FLOAT, 1, vmin_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float valid_range_att[2] = {((float)-9.9999999e+14), ((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, FRSNO_id, "valid_range", NC_FLOAT, 2, valid_range_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, FRUNST_id, "long_name", 35, "fractional_area_of_unsaturated_zone");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, FRUNST_id, "units", 1, "1");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float _FillValue_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, FRUNST_id, "_FillValue", NC_FLOAT, 1, _FillValue_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float missing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, FRUNST_id, "missing_value", NC_FLOAT, 1, missing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float fmissing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, FRUNST_id, "fmissing_value", NC_FLOAT, 1, fmissing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float scale_factor_att[1] = {((float)1)} ;
    stat = nc_put_att_float(MERR_grp, FRUNST_id, "scale_factor", NC_FLOAT, 1, scale_factor_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float add_offset_att[1] = {((float)0)} ;
    stat = nc_put_att_float(MERR_grp, FRUNST_id, "add_offset", NC_FLOAT, 1, add_offset_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, FRUNST_id, "standard_name", 35, "fractional_area_of_unsaturated_zone");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmax_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, FRUNST_id, "vmax", NC_FLOAT, 1, vmax_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmin_att[1] = {((float)-9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, FRUNST_id, "vmin", NC_FLOAT, 1, vmin_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float valid_range_att[2] = {((float)-9.9999999e+14), ((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, FRUNST_id, "valid_range", NC_FLOAT, 2, valid_range_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, FRWLT_id, "long_name", 31, "fractional_area_of_wilting_zone");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, FRWLT_id, "units", 1, "1");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float _FillValue_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, FRWLT_id, "_FillValue", NC_FLOAT, 1, _FillValue_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float missing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, FRWLT_id, "missing_value", NC_FLOAT, 1, missing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float fmissing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, FRWLT_id, "fmissing_value", NC_FLOAT, 1, fmissing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float scale_factor_att[1] = {((float)1)} ;
    stat = nc_put_att_float(MERR_grp, FRWLT_id, "scale_factor", NC_FLOAT, 1, scale_factor_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float add_offset_att[1] = {((float)0)} ;
    stat = nc_put_att_float(MERR_grp, FRWLT_id, "add_offset", NC_FLOAT, 1, add_offset_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, FRWLT_id, "standard_name", 31, "fractional_area_of_wilting_zone");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmax_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, FRWLT_id, "vmax", NC_FLOAT, 1, vmax_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmin_att[1] = {((float)-9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, FRWLT_id, "vmin", NC_FLOAT, 1, vmin_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float valid_range_att[2] = {((float)-9.9999999e+14), ((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, FRWLT_id, "valid_range", NC_FLOAT, 2, valid_range_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, GHLAND_id, "long_name", 19, "Ground_heating_land");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, GHLAND_id, "units", 5, "W m-2");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float _FillValue_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, GHLAND_id, "_FillValue", NC_FLOAT, 1, _FillValue_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float missing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, GHLAND_id, "missing_value", NC_FLOAT, 1, missing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float fmissing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, GHLAND_id, "fmissing_value", NC_FLOAT, 1, fmissing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float scale_factor_att[1] = {((float)1)} ;
    stat = nc_put_att_float(MERR_grp, GHLAND_id, "scale_factor", NC_FLOAT, 1, scale_factor_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float add_offset_att[1] = {((float)0)} ;
    stat = nc_put_att_float(MERR_grp, GHLAND_id, "add_offset", NC_FLOAT, 1, add_offset_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, GHLAND_id, "standard_name", 19, "Ground_heating_land");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmax_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, GHLAND_id, "vmax", NC_FLOAT, 1, vmax_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmin_att[1] = {((float)-9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, GHLAND_id, "vmin", NC_FLOAT, 1, vmin_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float valid_range_att[2] = {((float)-9.9999999e+14), ((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, GHLAND_id, "valid_range", NC_FLOAT, 2, valid_range_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, GRN_id, "long_name", 17, "greeness_fraction");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, GRN_id, "units", 1, "1");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float _FillValue_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, GRN_id, "_FillValue", NC_FLOAT, 1, _FillValue_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float missing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, GRN_id, "missing_value", NC_FLOAT, 1, missing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float fmissing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, GRN_id, "fmissing_value", NC_FLOAT, 1, fmissing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float scale_factor_att[1] = {((float)1)} ;
    stat = nc_put_att_float(MERR_grp, GRN_id, "scale_factor", NC_FLOAT, 1, scale_factor_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float add_offset_att[1] = {((float)0)} ;
    stat = nc_put_att_float(MERR_grp, GRN_id, "add_offset", NC_FLOAT, 1, add_offset_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, GRN_id, "standard_name", 17, "greeness_fraction");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmax_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, GRN_id, "vmax", NC_FLOAT, 1, vmax_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmin_att[1] = {((float)-9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, GRN_id, "vmin", NC_FLOAT, 1, vmin_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float valid_range_att[2] = {((float)-9.9999999e+14), ((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, GRN_id, "valid_range", NC_FLOAT, 2, valid_range_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, GWETPROF_id, "long_name", 22, "ave_prof_soil_moisture");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, GWETPROF_id, "units", 1, "1");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float _FillValue_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, GWETPROF_id, "_FillValue", NC_FLOAT, 1, _FillValue_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float missing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, GWETPROF_id, "missing_value", NC_FLOAT, 1, missing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float fmissing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, GWETPROF_id, "fmissing_value", NC_FLOAT, 1, fmissing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float scale_factor_att[1] = {((float)1)} ;
    stat = nc_put_att_float(MERR_grp, GWETPROF_id, "scale_factor", NC_FLOAT, 1, scale_factor_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float add_offset_att[1] = {((float)0)} ;
    stat = nc_put_att_float(MERR_grp, GWETPROF_id, "add_offset", NC_FLOAT, 1, add_offset_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, GWETPROF_id, "standard_name", 22, "ave_prof_soil_moisture");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmax_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, GWETPROF_id, "vmax", NC_FLOAT, 1, vmax_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmin_att[1] = {((float)-9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, GWETPROF_id, "vmin", NC_FLOAT, 1, vmin_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float valid_range_att[2] = {((float)-9.9999999e+14), ((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, GWETPROF_id, "valid_range", NC_FLOAT, 2, valid_range_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, GWETROOT_id, "long_name", 22, "root_zone_soil_wetness");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, GWETROOT_id, "units", 1, "1");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float _FillValue_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, GWETROOT_id, "_FillValue", NC_FLOAT, 1, _FillValue_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float missing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, GWETROOT_id, "missing_value", NC_FLOAT, 1, missing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float fmissing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, GWETROOT_id, "fmissing_value", NC_FLOAT, 1, fmissing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float scale_factor_att[1] = {((float)1)} ;
    stat = nc_put_att_float(MERR_grp, GWETROOT_id, "scale_factor", NC_FLOAT, 1, scale_factor_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float add_offset_att[1] = {((float)0)} ;
    stat = nc_put_att_float(MERR_grp, GWETROOT_id, "add_offset", NC_FLOAT, 1, add_offset_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, GWETROOT_id, "standard_name", 22, "root_zone_soil_wetness");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmax_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, GWETROOT_id, "vmax", NC_FLOAT, 1, vmax_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmin_att[1] = {((float)-9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, GWETROOT_id, "vmin", NC_FLOAT, 1, vmin_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float valid_range_att[2] = {((float)-9.9999999e+14), ((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, GWETROOT_id, "valid_range", NC_FLOAT, 2, valid_range_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, GWETTOP_id, "long_name", 20, "surface_soil_wetness");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, GWETTOP_id, "units", 1, "1");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float _FillValue_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, GWETTOP_id, "_FillValue", NC_FLOAT, 1, _FillValue_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float missing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, GWETTOP_id, "missing_value", NC_FLOAT, 1, missing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float fmissing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, GWETTOP_id, "fmissing_value", NC_FLOAT, 1, fmissing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float scale_factor_att[1] = {((float)1)} ;
    stat = nc_put_att_float(MERR_grp, GWETTOP_id, "scale_factor", NC_FLOAT, 1, scale_factor_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float add_offset_att[1] = {((float)0)} ;
    stat = nc_put_att_float(MERR_grp, GWETTOP_id, "add_offset", NC_FLOAT, 1, add_offset_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, GWETTOP_id, "standard_name", 20, "surface_soil_wetness");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmax_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, GWETTOP_id, "vmax", NC_FLOAT, 1, vmax_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmin_att[1] = {((float)-9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, GWETTOP_id, "vmin", NC_FLOAT, 1, vmin_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float valid_range_att[2] = {((float)-9.9999999e+14), ((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, GWETTOP_id, "valid_range", NC_FLOAT, 2, valid_range_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, LAI_id, "long_name", 15, "leaf_area_index");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, LAI_id, "units", 1, "1");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float _FillValue_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, LAI_id, "_FillValue", NC_FLOAT, 1, _FillValue_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float missing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, LAI_id, "missing_value", NC_FLOAT, 1, missing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float fmissing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, LAI_id, "fmissing_value", NC_FLOAT, 1, fmissing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float scale_factor_att[1] = {((float)1)} ;
    stat = nc_put_att_float(MERR_grp, LAI_id, "scale_factor", NC_FLOAT, 1, scale_factor_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float add_offset_att[1] = {((float)0)} ;
    stat = nc_put_att_float(MERR_grp, LAI_id, "add_offset", NC_FLOAT, 1, add_offset_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, LAI_id, "standard_name", 15, "leaf_area_index");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmax_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, LAI_id, "vmax", NC_FLOAT, 1, vmax_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmin_att[1] = {((float)-9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, LAI_id, "vmin", NC_FLOAT, 1, vmin_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float valid_range_att[2] = {((float)-9.9999999e+14), ((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, LAI_id, "valid_range", NC_FLOAT, 2, valid_range_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, LHLAND_id, "long_name", 21, "Latent_heat_flux_land");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, LHLAND_id, "units", 5, "W m-2");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float _FillValue_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, LHLAND_id, "_FillValue", NC_FLOAT, 1, _FillValue_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float missing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, LHLAND_id, "missing_value", NC_FLOAT, 1, missing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float fmissing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, LHLAND_id, "fmissing_value", NC_FLOAT, 1, fmissing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float scale_factor_att[1] = {((float)1)} ;
    stat = nc_put_att_float(MERR_grp, LHLAND_id, "scale_factor", NC_FLOAT, 1, scale_factor_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float add_offset_att[1] = {((float)0)} ;
    stat = nc_put_att_float(MERR_grp, LHLAND_id, "add_offset", NC_FLOAT, 1, add_offset_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, LHLAND_id, "standard_name", 21, "Latent_heat_flux_land");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmax_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, LHLAND_id, "vmax", NC_FLOAT, 1, vmax_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmin_att[1] = {((float)-9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, LHLAND_id, "vmin", NC_FLOAT, 1, vmin_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float valid_range_att[2] = {((float)-9.9999999e+14), ((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, LHLAND_id, "valid_range", NC_FLOAT, 2, valid_range_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, LWLAND_id, "long_name", 17, "Net_longwave_land");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, LWLAND_id, "units", 5, "W m-2");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float _FillValue_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, LWLAND_id, "_FillValue", NC_FLOAT, 1, _FillValue_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float missing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, LWLAND_id, "missing_value", NC_FLOAT, 1, missing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float fmissing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, LWLAND_id, "fmissing_value", NC_FLOAT, 1, fmissing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float scale_factor_att[1] = {((float)1)} ;
    stat = nc_put_att_float(MERR_grp, LWLAND_id, "scale_factor", NC_FLOAT, 1, scale_factor_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float add_offset_att[1] = {((float)0)} ;
    stat = nc_put_att_float(MERR_grp, LWLAND_id, "add_offset", NC_FLOAT, 1, add_offset_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, LWLAND_id, "standard_name", 17, "Net_longwave_land");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmax_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, LWLAND_id, "vmax", NC_FLOAT, 1, vmax_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmin_att[1] = {((float)-9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, LWLAND_id, "vmin", NC_FLOAT, 1, vmin_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float valid_range_att[2] = {((float)-9.9999999e+14), ((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, LWLAND_id, "valid_range", NC_FLOAT, 2, valid_range_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, PARDFLAND_id, "long_name", 36, "surface_downwelling_par_diffuse_flux");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, PARDFLAND_id, "units", 5, "W m-2");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float _FillValue_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, PARDFLAND_id, "_FillValue", NC_FLOAT, 1, _FillValue_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float missing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, PARDFLAND_id, "missing_value", NC_FLOAT, 1, missing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float fmissing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, PARDFLAND_id, "fmissing_value", NC_FLOAT, 1, fmissing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float scale_factor_att[1] = {((float)1)} ;
    stat = nc_put_att_float(MERR_grp, PARDFLAND_id, "scale_factor", NC_FLOAT, 1, scale_factor_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float add_offset_att[1] = {((float)0)} ;
    stat = nc_put_att_float(MERR_grp, PARDFLAND_id, "add_offset", NC_FLOAT, 1, add_offset_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, PARDFLAND_id, "standard_name", 36, "surface_downwelling_par_diffuse_flux");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmax_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, PARDFLAND_id, "vmax", NC_FLOAT, 1, vmax_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmin_att[1] = {((float)-9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, PARDFLAND_id, "vmin", NC_FLOAT, 1, vmin_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float valid_range_att[2] = {((float)-9.9999999e+14), ((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, PARDFLAND_id, "valid_range", NC_FLOAT, 2, valid_range_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, PARDRLAND_id, "long_name", 33, "surface_downwelling_par_beam_flux");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, PARDRLAND_id, "units", 5, "W m-2");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float _FillValue_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, PARDRLAND_id, "_FillValue", NC_FLOAT, 1, _FillValue_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float missing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, PARDRLAND_id, "missing_value", NC_FLOAT, 1, missing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float fmissing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, PARDRLAND_id, "fmissing_value", NC_FLOAT, 1, fmissing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float scale_factor_att[1] = {((float)1)} ;
    stat = nc_put_att_float(MERR_grp, PARDRLAND_id, "scale_factor", NC_FLOAT, 1, scale_factor_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float add_offset_att[1] = {((float)0)} ;
    stat = nc_put_att_float(MERR_grp, PARDRLAND_id, "add_offset", NC_FLOAT, 1, add_offset_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, PARDRLAND_id, "standard_name", 33, "surface_downwelling_par_beam_flux");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmax_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, PARDRLAND_id, "vmax", NC_FLOAT, 1, vmax_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmin_att[1] = {((float)-9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, PARDRLAND_id, "vmin", NC_FLOAT, 1, vmin_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float valid_range_att[2] = {((float)-9.9999999e+14), ((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, PARDRLAND_id, "valid_range", NC_FLOAT, 2, valid_range_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, PRECSNOLAND_id, "long_name", 13, "snowfall_land");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, PRECSNOLAND_id, "units", 10, "kg m-2 s-1");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float _FillValue_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, PRECSNOLAND_id, "_FillValue", NC_FLOAT, 1, _FillValue_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float missing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, PRECSNOLAND_id, "missing_value", NC_FLOAT, 1, missing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float fmissing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, PRECSNOLAND_id, "fmissing_value", NC_FLOAT, 1, fmissing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float scale_factor_att[1] = {((float)1)} ;
    stat = nc_put_att_float(MERR_grp, PRECSNOLAND_id, "scale_factor", NC_FLOAT, 1, scale_factor_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float add_offset_att[1] = {((float)0)} ;
    stat = nc_put_att_float(MERR_grp, PRECSNOLAND_id, "add_offset", NC_FLOAT, 1, add_offset_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, PRECSNOLAND_id, "standard_name", 13, "snowfall_land");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmax_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, PRECSNOLAND_id, "vmax", NC_FLOAT, 1, vmax_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmin_att[1] = {((float)-9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, PRECSNOLAND_id, "vmin", NC_FLOAT, 1, vmin_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float valid_range_att[2] = {((float)-9.9999999e+14), ((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, PRECSNOLAND_id, "valid_range", NC_FLOAT, 2, valid_range_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, PRECTOTLAND_id, "long_name", 24, "Total_precipitation_land");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, PRECTOTLAND_id, "units", 10, "kg m-2 s-1");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float _FillValue_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, PRECTOTLAND_id, "_FillValue", NC_FLOAT, 1, _FillValue_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float missing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, PRECTOTLAND_id, "missing_value", NC_FLOAT, 1, missing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float fmissing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, PRECTOTLAND_id, "fmissing_value", NC_FLOAT, 1, fmissing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float scale_factor_att[1] = {((float)1)} ;
    stat = nc_put_att_float(MERR_grp, PRECTOTLAND_id, "scale_factor", NC_FLOAT, 1, scale_factor_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float add_offset_att[1] = {((float)0)} ;
    stat = nc_put_att_float(MERR_grp, PRECTOTLAND_id, "add_offset", NC_FLOAT, 1, add_offset_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, PRECTOTLAND_id, "standard_name", 24, "Total_precipitation_land");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmax_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, PRECTOTLAND_id, "vmax", NC_FLOAT, 1, vmax_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmin_att[1] = {((float)-9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, PRECTOTLAND_id, "vmin", NC_FLOAT, 1, vmin_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float valid_range_att[2] = {((float)-9.9999999e+14), ((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, PRECTOTLAND_id, "valid_range", NC_FLOAT, 2, valid_range_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, PRMC_id, "long_name", 13, "water_profile");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, PRMC_id, "units", 7, "m-3 m-3");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float _FillValue_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, PRMC_id, "_FillValue", NC_FLOAT, 1, _FillValue_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float missing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, PRMC_id, "missing_value", NC_FLOAT, 1, missing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float fmissing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, PRMC_id, "fmissing_value", NC_FLOAT, 1, fmissing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float scale_factor_att[1] = {((float)1)} ;
    stat = nc_put_att_float(MERR_grp, PRMC_id, "scale_factor", NC_FLOAT, 1, scale_factor_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float add_offset_att[1] = {((float)0)} ;
    stat = nc_put_att_float(MERR_grp, PRMC_id, "add_offset", NC_FLOAT, 1, add_offset_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, PRMC_id, "standard_name", 13, "water_profile");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmax_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, PRMC_id, "vmax", NC_FLOAT, 1, vmax_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmin_att[1] = {((float)-9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, PRMC_id, "vmin", NC_FLOAT, 1, vmin_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float valid_range_att[2] = {((float)-9.9999999e+14), ((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, PRMC_id, "valid_range", NC_FLOAT, 2, valid_range_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, QINFIL_id, "long_name", 28, "Soil_water_infiltration_rate");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, QINFIL_id, "units", 10, "kg m-2 s-1");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float _FillValue_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, QINFIL_id, "_FillValue", NC_FLOAT, 1, _FillValue_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float missing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, QINFIL_id, "missing_value", NC_FLOAT, 1, missing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float fmissing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, QINFIL_id, "fmissing_value", NC_FLOAT, 1, fmissing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float scale_factor_att[1] = {((float)1)} ;
    stat = nc_put_att_float(MERR_grp, QINFIL_id, "scale_factor", NC_FLOAT, 1, scale_factor_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float add_offset_att[1] = {((float)0)} ;
    stat = nc_put_att_float(MERR_grp, QINFIL_id, "add_offset", NC_FLOAT, 1, add_offset_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, QINFIL_id, "standard_name", 28, "Soil_water_infiltration_rate");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmax_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, QINFIL_id, "vmax", NC_FLOAT, 1, vmax_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmin_att[1] = {((float)-9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, QINFIL_id, "vmin", NC_FLOAT, 1, vmin_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float valid_range_att[2] = {((float)-9.9999999e+14), ((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, QINFIL_id, "valid_range", NC_FLOAT, 2, valid_range_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, RUNOFF_id, "long_name", 37, "overland_runoff_including_throughflow");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, RUNOFF_id, "units", 10, "kg m-2 s-1");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float _FillValue_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, RUNOFF_id, "_FillValue", NC_FLOAT, 1, _FillValue_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float missing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, RUNOFF_id, "missing_value", NC_FLOAT, 1, missing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float fmissing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, RUNOFF_id, "fmissing_value", NC_FLOAT, 1, fmissing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float scale_factor_att[1] = {((float)1)} ;
    stat = nc_put_att_float(MERR_grp, RUNOFF_id, "scale_factor", NC_FLOAT, 1, scale_factor_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float add_offset_att[1] = {((float)0)} ;
    stat = nc_put_att_float(MERR_grp, RUNOFF_id, "add_offset", NC_FLOAT, 1, add_offset_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, RUNOFF_id, "standard_name", 37, "overland_runoff_including_throughflow");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmax_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, RUNOFF_id, "vmax", NC_FLOAT, 1, vmax_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmin_att[1] = {((float)-9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, RUNOFF_id, "vmin", NC_FLOAT, 1, vmin_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float valid_range_att[2] = {((float)-9.9999999e+14), ((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, RUNOFF_id, "valid_range", NC_FLOAT, 2, valid_range_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, RZMC_id, "long_name", 15, "water_root_zone");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, RZMC_id, "units", 7, "m-3 m-3");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float _FillValue_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, RZMC_id, "_FillValue", NC_FLOAT, 1, _FillValue_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float missing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, RZMC_id, "missing_value", NC_FLOAT, 1, missing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float fmissing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, RZMC_id, "fmissing_value", NC_FLOAT, 1, fmissing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float scale_factor_att[1] = {((float)1)} ;
    stat = nc_put_att_float(MERR_grp, RZMC_id, "scale_factor", NC_FLOAT, 1, scale_factor_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float add_offset_att[1] = {((float)0)} ;
    stat = nc_put_att_float(MERR_grp, RZMC_id, "add_offset", NC_FLOAT, 1, add_offset_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, RZMC_id, "standard_name", 15, "water_root_zone");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmax_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, RZMC_id, "vmax", NC_FLOAT, 1, vmax_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmin_att[1] = {((float)-9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, RZMC_id, "vmin", NC_FLOAT, 1, vmin_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float valid_range_att[2] = {((float)-9.9999999e+14), ((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, RZMC_id, "valid_range", NC_FLOAT, 2, valid_range_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, SFMC_id, "long_name", 19, "water_surface_layer");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, SFMC_id, "units", 7, "m-3 m-3");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float _FillValue_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, SFMC_id, "_FillValue", NC_FLOAT, 1, _FillValue_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float missing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, SFMC_id, "missing_value", NC_FLOAT, 1, missing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float fmissing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, SFMC_id, "fmissing_value", NC_FLOAT, 1, fmissing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float scale_factor_att[1] = {((float)1)} ;
    stat = nc_put_att_float(MERR_grp, SFMC_id, "scale_factor", NC_FLOAT, 1, scale_factor_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float add_offset_att[1] = {((float)0)} ;
    stat = nc_put_att_float(MERR_grp, SFMC_id, "add_offset", NC_FLOAT, 1, add_offset_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, SFMC_id, "standard_name", 19, "water_surface_layer");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmax_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, SFMC_id, "vmax", NC_FLOAT, 1, vmax_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmin_att[1] = {((float)-9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, SFMC_id, "vmin", NC_FLOAT, 1, vmin_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float valid_range_att[2] = {((float)-9.9999999e+14), ((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, SFMC_id, "valid_range", NC_FLOAT, 2, valid_range_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, SHLAND_id, "long_name", 23, "Sensible_heat_flux_land");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, SHLAND_id, "units", 5, "W m-2");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float _FillValue_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, SHLAND_id, "_FillValue", NC_FLOAT, 1, _FillValue_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float missing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, SHLAND_id, "missing_value", NC_FLOAT, 1, missing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float fmissing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, SHLAND_id, "fmissing_value", NC_FLOAT, 1, fmissing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float scale_factor_att[1] = {((float)1)} ;
    stat = nc_put_att_float(MERR_grp, SHLAND_id, "scale_factor", NC_FLOAT, 1, scale_factor_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float add_offset_att[1] = {((float)0)} ;
    stat = nc_put_att_float(MERR_grp, SHLAND_id, "add_offset", NC_FLOAT, 1, add_offset_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, SHLAND_id, "standard_name", 23, "Sensible_heat_flux_land");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmax_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, SHLAND_id, "vmax", NC_FLOAT, 1, vmax_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmin_att[1] = {((float)-9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, SHLAND_id, "vmin", NC_FLOAT, 1, vmin_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float valid_range_att[2] = {((float)-9.9999999e+14), ((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, SHLAND_id, "valid_range", NC_FLOAT, 2, valid_range_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, SMLAND_id, "long_name", 18, "Snowmelt_flux_land");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, SMLAND_id, "units", 10, "kg m-2 s-1");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float _FillValue_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, SMLAND_id, "_FillValue", NC_FLOAT, 1, _FillValue_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float missing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, SMLAND_id, "missing_value", NC_FLOAT, 1, missing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float fmissing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, SMLAND_id, "fmissing_value", NC_FLOAT, 1, fmissing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float scale_factor_att[1] = {((float)1)} ;
    stat = nc_put_att_float(MERR_grp, SMLAND_id, "scale_factor", NC_FLOAT, 1, scale_factor_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float add_offset_att[1] = {((float)0)} ;
    stat = nc_put_att_float(MERR_grp, SMLAND_id, "add_offset", NC_FLOAT, 1, add_offset_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, SMLAND_id, "standard_name", 18, "Snowmelt_flux_land");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmax_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, SMLAND_id, "vmax", NC_FLOAT, 1, vmax_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmin_att[1] = {((float)-9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, SMLAND_id, "vmin", NC_FLOAT, 1, vmin_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float valid_range_att[2] = {((float)-9.9999999e+14), ((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, SMLAND_id, "valid_range", NC_FLOAT, 2, valid_range_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, SNODP_id, "long_name", 10, "snow_depth");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, SNODP_id, "units", 1, "m");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float _FillValue_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, SNODP_id, "_FillValue", NC_FLOAT, 1, _FillValue_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float missing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, SNODP_id, "missing_value", NC_FLOAT, 1, missing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float fmissing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, SNODP_id, "fmissing_value", NC_FLOAT, 1, fmissing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float scale_factor_att[1] = {((float)1)} ;
    stat = nc_put_att_float(MERR_grp, SNODP_id, "scale_factor", NC_FLOAT, 1, scale_factor_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float add_offset_att[1] = {((float)0)} ;
    stat = nc_put_att_float(MERR_grp, SNODP_id, "add_offset", NC_FLOAT, 1, add_offset_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, SNODP_id, "standard_name", 10, "snow_depth");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmax_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, SNODP_id, "vmax", NC_FLOAT, 1, vmax_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmin_att[1] = {((float)-9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, SNODP_id, "vmin", NC_FLOAT, 1, vmin_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float valid_range_att[2] = {((float)-9.9999999e+14), ((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, SNODP_id, "valid_range", NC_FLOAT, 2, valid_range_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, SNOMAS_id, "long_name", 23, "Total_snow_storage_land");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, SNOMAS_id, "units", 6, "kg m-2");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float _FillValue_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, SNOMAS_id, "_FillValue", NC_FLOAT, 1, _FillValue_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float missing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, SNOMAS_id, "missing_value", NC_FLOAT, 1, missing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float fmissing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, SNOMAS_id, "fmissing_value", NC_FLOAT, 1, fmissing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float scale_factor_att[1] = {((float)1)} ;
    stat = nc_put_att_float(MERR_grp, SNOMAS_id, "scale_factor", NC_FLOAT, 1, scale_factor_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float add_offset_att[1] = {((float)0)} ;
    stat = nc_put_att_float(MERR_grp, SNOMAS_id, "add_offset", NC_FLOAT, 1, add_offset_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, SNOMAS_id, "standard_name", 23, "Total_snow_storage_land");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmax_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, SNOMAS_id, "vmax", NC_FLOAT, 1, vmax_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmin_att[1] = {((float)-9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, SNOMAS_id, "vmin", NC_FLOAT, 1, vmin_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float valid_range_att[2] = {((float)-9.9999999e+14), ((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, SNOMAS_id, "valid_range", NC_FLOAT, 2, valid_range_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, SPLAND_id, "long_name", 35, "rate_of_spurious_land_energy_source");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, SPLAND_id, "units", 5, "W m-2");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float _FillValue_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, SPLAND_id, "_FillValue", NC_FLOAT, 1, _FillValue_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float missing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, SPLAND_id, "missing_value", NC_FLOAT, 1, missing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float fmissing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, SPLAND_id, "fmissing_value", NC_FLOAT, 1, fmissing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float scale_factor_att[1] = {((float)1)} ;
    stat = nc_put_att_float(MERR_grp, SPLAND_id, "scale_factor", NC_FLOAT, 1, scale_factor_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float add_offset_att[1] = {((float)0)} ;
    stat = nc_put_att_float(MERR_grp, SPLAND_id, "add_offset", NC_FLOAT, 1, add_offset_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, SPLAND_id, "standard_name", 35, "rate_of_spurious_land_energy_source");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmax_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, SPLAND_id, "vmax", NC_FLOAT, 1, vmax_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmin_att[1] = {((float)-9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, SPLAND_id, "vmin", NC_FLOAT, 1, vmin_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float valid_range_att[2] = {((float)-9.9999999e+14), ((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, SPLAND_id, "valid_range", NC_FLOAT, 2, valid_range_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, SPSNOW_id, "long_name", 28, "rate_of_spurious_snow_energy");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, SPSNOW_id, "units", 5, "W m-2");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float _FillValue_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, SPSNOW_id, "_FillValue", NC_FLOAT, 1, _FillValue_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float missing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, SPSNOW_id, "missing_value", NC_FLOAT, 1, missing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float fmissing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, SPSNOW_id, "fmissing_value", NC_FLOAT, 1, fmissing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float scale_factor_att[1] = {((float)1)} ;
    stat = nc_put_att_float(MERR_grp, SPSNOW_id, "scale_factor", NC_FLOAT, 1, scale_factor_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float add_offset_att[1] = {((float)0)} ;
    stat = nc_put_att_float(MERR_grp, SPSNOW_id, "add_offset", NC_FLOAT, 1, add_offset_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, SPSNOW_id, "standard_name", 28, "rate_of_spurious_snow_energy");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmax_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, SPSNOW_id, "vmax", NC_FLOAT, 1, vmax_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmin_att[1] = {((float)-9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, SPSNOW_id, "vmin", NC_FLOAT, 1, vmin_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float valid_range_att[2] = {((float)-9.9999999e+14), ((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, SPSNOW_id, "valid_range", NC_FLOAT, 2, valid_range_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, SPWATR_id, "long_name", 34, "rate_of_spurious_land_water_source");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, SPWATR_id, "units", 10, "kg m-2 s-1");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float _FillValue_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, SPWATR_id, "_FillValue", NC_FLOAT, 1, _FillValue_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float missing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, SPWATR_id, "missing_value", NC_FLOAT, 1, missing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float fmissing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, SPWATR_id, "fmissing_value", NC_FLOAT, 1, fmissing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float scale_factor_att[1] = {((float)1)} ;
    stat = nc_put_att_float(MERR_grp, SPWATR_id, "scale_factor", NC_FLOAT, 1, scale_factor_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float add_offset_att[1] = {((float)0)} ;
    stat = nc_put_att_float(MERR_grp, SPWATR_id, "add_offset", NC_FLOAT, 1, add_offset_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, SPWATR_id, "standard_name", 34, "rate_of_spurious_land_water_source");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmax_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, SPWATR_id, "vmax", NC_FLOAT, 1, vmax_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmin_att[1] = {((float)-9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, SPWATR_id, "vmin", NC_FLOAT, 1, vmin_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float valid_range_att[2] = {((float)-9.9999999e+14), ((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, SPWATR_id, "valid_range", NC_FLOAT, 2, valid_range_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, SWLAND_id, "long_name", 18, "Net_shortwave_land");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, SWLAND_id, "units", 5, "W m-2");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float _FillValue_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, SWLAND_id, "_FillValue", NC_FLOAT, 1, _FillValue_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float missing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, SWLAND_id, "missing_value", NC_FLOAT, 1, missing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float fmissing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, SWLAND_id, "fmissing_value", NC_FLOAT, 1, fmissing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float scale_factor_att[1] = {((float)1)} ;
    stat = nc_put_att_float(MERR_grp, SWLAND_id, "scale_factor", NC_FLOAT, 1, scale_factor_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float add_offset_att[1] = {((float)0)} ;
    stat = nc_put_att_float(MERR_grp, SWLAND_id, "add_offset", NC_FLOAT, 1, add_offset_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, SWLAND_id, "standard_name", 18, "Net_shortwave_land");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmax_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, SWLAND_id, "vmax", NC_FLOAT, 1, vmax_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmin_att[1] = {((float)-9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, SWLAND_id, "vmin", NC_FLOAT, 1, vmin_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float valid_range_att[2] = {((float)-9.9999999e+14), ((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, SWLAND_id, "valid_range", NC_FLOAT, 2, valid_range_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, TELAND_id, "long_name", 25, "Total_energy_storage_land");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, TELAND_id, "units", 5, "J m-2");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float _FillValue_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TELAND_id, "_FillValue", NC_FLOAT, 1, _FillValue_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float missing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TELAND_id, "missing_value", NC_FLOAT, 1, missing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float fmissing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TELAND_id, "fmissing_value", NC_FLOAT, 1, fmissing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float scale_factor_att[1] = {((float)1)} ;
    stat = nc_put_att_float(MERR_grp, TELAND_id, "scale_factor", NC_FLOAT, 1, scale_factor_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float add_offset_att[1] = {((float)0)} ;
    stat = nc_put_att_float(MERR_grp, TELAND_id, "add_offset", NC_FLOAT, 1, add_offset_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, TELAND_id, "standard_name", 25, "Total_energy_storage_land");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmax_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TELAND_id, "vmax", NC_FLOAT, 1, vmax_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmin_att[1] = {((float)-9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TELAND_id, "vmin", NC_FLOAT, 1, vmin_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float valid_range_att[2] = {((float)-9.9999999e+14), ((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TELAND_id, "valid_range", NC_FLOAT, 2, valid_range_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, TPSNOW_id, "long_name", 27, "surface_temperature_of_snow");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, TPSNOW_id, "units", 1, "K");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float _FillValue_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TPSNOW_id, "_FillValue", NC_FLOAT, 1, _FillValue_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float missing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TPSNOW_id, "missing_value", NC_FLOAT, 1, missing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float fmissing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TPSNOW_id, "fmissing_value", NC_FLOAT, 1, fmissing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float scale_factor_att[1] = {((float)1)} ;
    stat = nc_put_att_float(MERR_grp, TPSNOW_id, "scale_factor", NC_FLOAT, 1, scale_factor_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float add_offset_att[1] = {((float)0)} ;
    stat = nc_put_att_float(MERR_grp, TPSNOW_id, "add_offset", NC_FLOAT, 1, add_offset_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, TPSNOW_id, "standard_name", 27, "surface_temperature_of_snow");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmax_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TPSNOW_id, "vmax", NC_FLOAT, 1, vmax_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmin_att[1] = {((float)-9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TPSNOW_id, "vmin", NC_FLOAT, 1, vmin_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float valid_range_att[2] = {((float)-9.9999999e+14), ((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TPSNOW_id, "valid_range", NC_FLOAT, 2, valid_range_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, TSAT_id, "long_name", 37, "surface_temperature_of_saturated_zone");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, TSAT_id, "units", 1, "K");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float _FillValue_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TSAT_id, "_FillValue", NC_FLOAT, 1, _FillValue_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float missing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TSAT_id, "missing_value", NC_FLOAT, 1, missing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float fmissing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TSAT_id, "fmissing_value", NC_FLOAT, 1, fmissing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float scale_factor_att[1] = {((float)1)} ;
    stat = nc_put_att_float(MERR_grp, TSAT_id, "scale_factor", NC_FLOAT, 1, scale_factor_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float add_offset_att[1] = {((float)0)} ;
    stat = nc_put_att_float(MERR_grp, TSAT_id, "add_offset", NC_FLOAT, 1, add_offset_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, TSAT_id, "standard_name", 37, "surface_temperature_of_saturated_zone");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmax_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TSAT_id, "vmax", NC_FLOAT, 1, vmax_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmin_att[1] = {((float)-9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TSAT_id, "vmin", NC_FLOAT, 1, vmin_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float valid_range_att[2] = {((float)-9.9999999e+14), ((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TSAT_id, "valid_range", NC_FLOAT, 2, valid_range_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, TSOIL1_id, "long_name", 25, "soil_temperatures_layer_1");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, TSOIL1_id, "units", 1, "K");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float _FillValue_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TSOIL1_id, "_FillValue", NC_FLOAT, 1, _FillValue_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float missing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TSOIL1_id, "missing_value", NC_FLOAT, 1, missing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float fmissing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TSOIL1_id, "fmissing_value", NC_FLOAT, 1, fmissing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float scale_factor_att[1] = {((float)1)} ;
    stat = nc_put_att_float(MERR_grp, TSOIL1_id, "scale_factor", NC_FLOAT, 1, scale_factor_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float add_offset_att[1] = {((float)0)} ;
    stat = nc_put_att_float(MERR_grp, TSOIL1_id, "add_offset", NC_FLOAT, 1, add_offset_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, TSOIL1_id, "standard_name", 25, "soil_temperatures_layer_1");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmax_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TSOIL1_id, "vmax", NC_FLOAT, 1, vmax_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmin_att[1] = {((float)-9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TSOIL1_id, "vmin", NC_FLOAT, 1, vmin_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float valid_range_att[2] = {((float)-9.9999999e+14), ((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TSOIL1_id, "valid_range", NC_FLOAT, 2, valid_range_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, TSOIL2_id, "long_name", 25, "soil_temperatures_layer_2");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, TSOIL2_id, "units", 1, "K");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float _FillValue_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TSOIL2_id, "_FillValue", NC_FLOAT, 1, _FillValue_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float missing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TSOIL2_id, "missing_value", NC_FLOAT, 1, missing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float fmissing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TSOIL2_id, "fmissing_value", NC_FLOAT, 1, fmissing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float scale_factor_att[1] = {((float)1)} ;
    stat = nc_put_att_float(MERR_grp, TSOIL2_id, "scale_factor", NC_FLOAT, 1, scale_factor_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float add_offset_att[1] = {((float)0)} ;
    stat = nc_put_att_float(MERR_grp, TSOIL2_id, "add_offset", NC_FLOAT, 1, add_offset_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, TSOIL2_id, "standard_name", 25, "soil_temperatures_layer_2");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmax_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TSOIL2_id, "vmax", NC_FLOAT, 1, vmax_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmin_att[1] = {((float)-9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TSOIL2_id, "vmin", NC_FLOAT, 1, vmin_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float valid_range_att[2] = {((float)-9.9999999e+14), ((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TSOIL2_id, "valid_range", NC_FLOAT, 2, valid_range_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, TSOIL3_id, "long_name", 25, "soil_temperatures_layer_3");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, TSOIL3_id, "units", 1, "K");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float _FillValue_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TSOIL3_id, "_FillValue", NC_FLOAT, 1, _FillValue_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float missing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TSOIL3_id, "missing_value", NC_FLOAT, 1, missing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float fmissing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TSOIL3_id, "fmissing_value", NC_FLOAT, 1, fmissing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float scale_factor_att[1] = {((float)1)} ;
    stat = nc_put_att_float(MERR_grp, TSOIL3_id, "scale_factor", NC_FLOAT, 1, scale_factor_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float add_offset_att[1] = {((float)0)} ;
    stat = nc_put_att_float(MERR_grp, TSOIL3_id, "add_offset", NC_FLOAT, 1, add_offset_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, TSOIL3_id, "standard_name", 25, "soil_temperatures_layer_3");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmax_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TSOIL3_id, "vmax", NC_FLOAT, 1, vmax_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmin_att[1] = {((float)-9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TSOIL3_id, "vmin", NC_FLOAT, 1, vmin_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float valid_range_att[2] = {((float)-9.9999999e+14), ((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TSOIL3_id, "valid_range", NC_FLOAT, 2, valid_range_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, TSOIL4_id, "long_name", 25, "soil_temperatures_layer_4");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, TSOIL4_id, "units", 1, "K");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float _FillValue_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TSOIL4_id, "_FillValue", NC_FLOAT, 1, _FillValue_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float missing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TSOIL4_id, "missing_value", NC_FLOAT, 1, missing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float fmissing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TSOIL4_id, "fmissing_value", NC_FLOAT, 1, fmissing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float scale_factor_att[1] = {((float)1)} ;
    stat = nc_put_att_float(MERR_grp, TSOIL4_id, "scale_factor", NC_FLOAT, 1, scale_factor_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float add_offset_att[1] = {((float)0)} ;
    stat = nc_put_att_float(MERR_grp, TSOIL4_id, "add_offset", NC_FLOAT, 1, add_offset_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, TSOIL4_id, "standard_name", 25, "soil_temperatures_layer_4");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmax_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TSOIL4_id, "vmax", NC_FLOAT, 1, vmax_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmin_att[1] = {((float)-9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TSOIL4_id, "vmin", NC_FLOAT, 1, vmin_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float valid_range_att[2] = {((float)-9.9999999e+14), ((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TSOIL4_id, "valid_range", NC_FLOAT, 2, valid_range_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, TSOIL5_id, "long_name", 25, "soil_temperatures_layer_5");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, TSOIL5_id, "units", 1, "K");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float _FillValue_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TSOIL5_id, "_FillValue", NC_FLOAT, 1, _FillValue_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float missing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TSOIL5_id, "missing_value", NC_FLOAT, 1, missing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float fmissing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TSOIL5_id, "fmissing_value", NC_FLOAT, 1, fmissing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float scale_factor_att[1] = {((float)1)} ;
    stat = nc_put_att_float(MERR_grp, TSOIL5_id, "scale_factor", NC_FLOAT, 1, scale_factor_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float add_offset_att[1] = {((float)0)} ;
    stat = nc_put_att_float(MERR_grp, TSOIL5_id, "add_offset", NC_FLOAT, 1, add_offset_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, TSOIL5_id, "standard_name", 25, "soil_temperatures_layer_5");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmax_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TSOIL5_id, "vmax", NC_FLOAT, 1, vmax_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmin_att[1] = {((float)-9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TSOIL5_id, "vmin", NC_FLOAT, 1, vmin_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float valid_range_att[2] = {((float)-9.9999999e+14), ((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TSOIL5_id, "valid_range", NC_FLOAT, 2, valid_range_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, TSOIL6_id, "long_name", 25, "soil_temperatures_layer_6");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, TSOIL6_id, "units", 1, "K");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float _FillValue_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TSOIL6_id, "_FillValue", NC_FLOAT, 1, _FillValue_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float missing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TSOIL6_id, "missing_value", NC_FLOAT, 1, missing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float fmissing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TSOIL6_id, "fmissing_value", NC_FLOAT, 1, fmissing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float scale_factor_att[1] = {((float)1)} ;
    stat = nc_put_att_float(MERR_grp, TSOIL6_id, "scale_factor", NC_FLOAT, 1, scale_factor_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float add_offset_att[1] = {((float)0)} ;
    stat = nc_put_att_float(MERR_grp, TSOIL6_id, "add_offset", NC_FLOAT, 1, add_offset_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, TSOIL6_id, "standard_name", 25, "soil_temperatures_layer_6");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmax_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TSOIL6_id, "vmax", NC_FLOAT, 1, vmax_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmin_att[1] = {((float)-9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TSOIL6_id, "vmin", NC_FLOAT, 1, vmin_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float valid_range_att[2] = {((float)-9.9999999e+14), ((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TSOIL6_id, "valid_range", NC_FLOAT, 2, valid_range_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, TSURF_id, "long_name", 37, "surface_temperature_of_land_incl_snow");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, TSURF_id, "units", 1, "K");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float _FillValue_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TSURF_id, "_FillValue", NC_FLOAT, 1, _FillValue_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float missing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TSURF_id, "missing_value", NC_FLOAT, 1, missing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float fmissing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TSURF_id, "fmissing_value", NC_FLOAT, 1, fmissing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float scale_factor_att[1] = {((float)1)} ;
    stat = nc_put_att_float(MERR_grp, TSURF_id, "scale_factor", NC_FLOAT, 1, scale_factor_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float add_offset_att[1] = {((float)0)} ;
    stat = nc_put_att_float(MERR_grp, TSURF_id, "add_offset", NC_FLOAT, 1, add_offset_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, TSURF_id, "standard_name", 37, "surface_temperature_of_land_incl_snow");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmax_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TSURF_id, "vmax", NC_FLOAT, 1, vmax_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmin_att[1] = {((float)-9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TSURF_id, "vmin", NC_FLOAT, 1, vmin_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float valid_range_att[2] = {((float)-9.9999999e+14), ((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TSURF_id, "valid_range", NC_FLOAT, 2, valid_range_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, TUNST_id, "long_name", 39, "surface_temperature_of_unsaturated_zone");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, TUNST_id, "units", 1, "K");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float _FillValue_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TUNST_id, "_FillValue", NC_FLOAT, 1, _FillValue_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float missing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TUNST_id, "missing_value", NC_FLOAT, 1, missing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float fmissing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TUNST_id, "fmissing_value", NC_FLOAT, 1, fmissing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float scale_factor_att[1] = {((float)1)} ;
    stat = nc_put_att_float(MERR_grp, TUNST_id, "scale_factor", NC_FLOAT, 1, scale_factor_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float add_offset_att[1] = {((float)0)} ;
    stat = nc_put_att_float(MERR_grp, TUNST_id, "add_offset", NC_FLOAT, 1, add_offset_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, TUNST_id, "standard_name", 39, "surface_temperature_of_unsaturated_zone");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmax_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TUNST_id, "vmax", NC_FLOAT, 1, vmax_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmin_att[1] = {((float)-9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TUNST_id, "vmin", NC_FLOAT, 1, vmin_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float valid_range_att[2] = {((float)-9.9999999e+14), ((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TUNST_id, "valid_range", NC_FLOAT, 2, valid_range_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, TWLAND_id, "long_name", 24, "Avail_water_storage_land");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, TWLAND_id, "units", 6, "kg m-2");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float _FillValue_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TWLAND_id, "_FillValue", NC_FLOAT, 1, _FillValue_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float missing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TWLAND_id, "missing_value", NC_FLOAT, 1, missing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float fmissing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TWLAND_id, "fmissing_value", NC_FLOAT, 1, fmissing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float scale_factor_att[1] = {((float)1)} ;
    stat = nc_put_att_float(MERR_grp, TWLAND_id, "scale_factor", NC_FLOAT, 1, scale_factor_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float add_offset_att[1] = {((float)0)} ;
    stat = nc_put_att_float(MERR_grp, TWLAND_id, "add_offset", NC_FLOAT, 1, add_offset_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, TWLAND_id, "standard_name", 24, "Avail_water_storage_land");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmax_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TWLAND_id, "vmax", NC_FLOAT, 1, vmax_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmin_att[1] = {((float)-9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TWLAND_id, "vmin", NC_FLOAT, 1, vmin_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float valid_range_att[2] = {((float)-9.9999999e+14), ((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TWLAND_id, "valid_range", NC_FLOAT, 2, valid_range_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, TWLT_id, "long_name", 34, "surface_temperature_of_wilted_zone");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, TWLT_id, "units", 1, "K");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float _FillValue_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TWLT_id, "_FillValue", NC_FLOAT, 1, _FillValue_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float missing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TWLT_id, "missing_value", NC_FLOAT, 1, missing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float fmissing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TWLT_id, "fmissing_value", NC_FLOAT, 1, fmissing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float scale_factor_att[1] = {((float)1)} ;
    stat = nc_put_att_float(MERR_grp, TWLT_id, "scale_factor", NC_FLOAT, 1, scale_factor_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float add_offset_att[1] = {((float)0)} ;
    stat = nc_put_att_float(MERR_grp, TWLT_id, "add_offset", NC_FLOAT, 1, add_offset_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, TWLT_id, "standard_name", 34, "surface_temperature_of_wilted_zone");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmax_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TWLT_id, "vmax", NC_FLOAT, 1, vmax_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmin_att[1] = {((float)-9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TWLT_id, "vmin", NC_FLOAT, 1, vmin_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float valid_range_att[2] = {((float)-9.9999999e+14), ((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, TWLT_id, "valid_range", NC_FLOAT, 2, valid_range_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, WCHANGE_id, "long_name", 34, "rate_of_change_of_total_land_water");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, WCHANGE_id, "units", 10, "kg m-2 s-1");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float _FillValue_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, WCHANGE_id, "_FillValue", NC_FLOAT, 1, _FillValue_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float missing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, WCHANGE_id, "missing_value", NC_FLOAT, 1, missing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float fmissing_value_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, WCHANGE_id, "fmissing_value", NC_FLOAT, 1, fmissing_value_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float scale_factor_att[1] = {((float)1)} ;
    stat = nc_put_att_float(MERR_grp, WCHANGE_id, "scale_factor", NC_FLOAT, 1, scale_factor_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float add_offset_att[1] = {((float)0)} ;
    stat = nc_put_att_float(MERR_grp, WCHANGE_id, "add_offset", NC_FLOAT, 1, add_offset_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, WCHANGE_id, "standard_name", 34, "rate_of_change_of_total_land_water");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmax_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, WCHANGE_id, "vmax", NC_FLOAT, 1, vmax_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmin_att[1] = {((float)-9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, WCHANGE_id, "vmin", NC_FLOAT, 1, vmin_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float valid_range_att[2] = {((float)-9.9999999e+14), ((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, WCHANGE_id, "valid_range", NC_FLOAT, 2, valid_range_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, lat_id, "long_name", 8, "latitude");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, lat_id, "units", 13, "degrees_north");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmax_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, lat_id, "vmax", NC_FLOAT, 1, vmax_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmin_att[1] = {((float)-9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, lat_id, "vmin", NC_FLOAT, 1, vmin_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float valid_range_att[2] = {((float)-9.9999999e+14), ((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, lat_id, "valid_range", NC_FLOAT, 2, valid_range_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, lon_id, "long_name", 9, "longitude");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, lon_id, "units", 12, "degrees_east");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmax_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, lon_id, "vmax", NC_FLOAT, 1, vmax_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmin_att[1] = {((float)-9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, lon_id, "vmin", NC_FLOAT, 1, vmin_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float valid_range_att[2] = {((float)-9.9999999e+14), ((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, lon_id, "valid_range", NC_FLOAT, 2, valid_range_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, time_id, "long_name", 4, "time");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    stat = nc_put_att_text(MERR_grp, time_id, "units", 33, "minutes since 2010-01-05 00:30:00");
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const int time_increment_att[1] = {10000} ;
    stat = nc_put_att_int(MERR_grp, time_id, "time_increment", NC_INT, 1, time_increment_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const int begin_date_att[1] = {20100105} ;
    stat = nc_put_att_int(MERR_grp, time_id, "begin_date", NC_INT, 1, begin_date_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const int begin_time_att[1] = {3000} ;
    stat = nc_put_att_int(MERR_grp, time_id, "begin_time", NC_INT, 1, begin_time_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmax_att[1] = {((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, time_id, "vmax", NC_FLOAT, 1, vmax_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float vmin_att[1] = {((float)-9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, time_id, "vmin", NC_FLOAT, 1, vmin_att);
    check_err(stat,__LINE__,__FILE__);
    }

    {
    static const float valid_range_att[2] = {((float)-9.9999999e+14), ((float)9.9999999e+14)} ;
    stat = nc_put_att_float(MERR_grp, time_id, "valid_range", NC_FLOAT, 2, valid_range_att);
    check_err(stat,__LINE__,__FILE__);
    }


    /* leave define mode */
    stat = nc_enddef (MERR_grp);
    check_err(stat,__LINE__,__FILE__);

    /* assign variable data */

    stat = nc_close(MERR_grp);
    check_err(stat,__LINE__,__FILE__);
    return 0;
}

int
run_test(int test_no, char *file_name, int cmode, int *open_time, int *create_time, int *close_time)
{
   int ncid;
   struct timeval start_time, end_time, diff_time;

   if (gettimeofday(&start_time, NULL)) ERR_RET;
   if (test_no)
   {
      if (create_wrfbdy_file(file_name, cmode)) ERR;
   }
   else
   {
      if (create_merr_file(file_name, cmode)) ERR;
   }

   if (gettimeofday(&end_time, NULL)) ERR_RET;
   if (nc4_timeval_subtract(&diff_time, &end_time, &start_time)) ERR_RET;
   *create_time += (int)diff_time.tv_sec * MILLION + (int)diff_time.tv_usec;

   if (gettimeofday(&start_time, NULL)) ERR_RET;
   if (nc_open(file_name, 0, &ncid)) ERR;
   if (gettimeofday(&end_time, NULL)) ERR_RET;
   if (nc4_timeval_subtract(&diff_time, &end_time, &start_time)) ERR_RET;
   *open_time += (int)diff_time.tv_sec * MILLION + (int)diff_time.tv_usec;

   if (gettimeofday(&start_time, NULL)) ERR_RET;
   if (nc_close(ncid)) ERR;
   if (gettimeofday(&end_time, NULL)) ERR_RET;
   if (nc4_timeval_subtract(&diff_time, &end_time, &start_time)) ERR_RET;
   *close_time += (int)diff_time.tv_sec * MILLION + (int)diff_time.tv_usec;

   return 0;
}

#define MAX_I 100
#define NUM_CMODES 2

int
main(int argc, char **argv)
{
   int cmode[NUM_CMODES] = {0, NC_NETCDF4};
   int c, i;

   printf("Testing with wrfbdy file...\n");
   {
      printf("cmode \t\tcreate time (us) \topen time (us) \tclose time (us)\n");
      for (c = 0; c < NUM_CMODES; c++)
      {
         {
            int open_time = 0, create_time = 0, close_time = 0;

            for (i = 0; i < MAX_I; i++)
            {
               char file_name[NC_MAX_NAME + 1];

               nc_set_log_level(4);
               sprintf(file_name, "%s_%d.nc", FILE_NAME, i);
               run_test(0, file_name, cmode[c], &open_time, &create_time, &close_time);
               remove(file_name);
            }
            printf("%s \t %d \t\t\t %d \t\t %d\n", cmode[c] ? "netcdf-4" : "classic",
                   create_time/MAX_I, open_time/MAX_I, close_time/MAX_I);
         }
      }
   }
   SUMMARIZE_ERR;
   printf("Testing with merr file...\n");
   {
      printf("cmode \t\tcreate time (us) \topen time (us) \tclose time (us)\n");
      for (c = 0; c < NUM_CMODES; c++)
      {
         {
            int open_time = 0, create_time = 0, close_time = 0;

            for (i = 0; i < MAX_I; i++)
            {
               char file_name[NC_MAX_NAME + 1];

               nc_set_log_level(4);
               sprintf(file_name, "%s_%d.nc", FILE_NAME, i);
               run_test(1, file_name, cmode[c], &open_time, &create_time, &close_time);
               remove(file_name);
            }
            printf("%s \t %d \t\t\t %d \t\t %d\n", cmode[c] ? "netcdf-4" : "classic",
                   create_time/MAX_I, open_time/MAX_I, close_time/MAX_I);
         }
      }
   }
   SUMMARIZE_ERR;
   FINAL_RESULTS;
}
