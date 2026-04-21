#ifndef ZFORMAT_H
#define ZFORMAT_H

int NCZ_infer_zarr_format(NC_FILE_INFO_T *file);
int NCZ_infer_nczarr_format(NC_FILE_INFO_T *file);
int NCZ_get_map(NC_FILE_INFO_T *file, NCURI *uri, mode_t mode,
                size64_t constraints, void *params, NCZMAP **mapp);
#endif
