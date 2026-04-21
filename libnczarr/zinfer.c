#include "zincludes.h"

int NCZ_infer_zarr_format(NC_FILE_INFO_T *file) {
  int stat = NC_ENOTZARR;
  NCZ_FILE_INFO_T *zfile = (NCZ_FILE_INFO_T *)file->format_file_info;

  struct ZarrObjects {
    const char *name;
    int format;
  } zarrobjects[] = {
      {"/zarr.json", 3}, {"/.zgroup", 2}, {"/.zarray", 2},
      {"/.zattrs", 2},   {NULL, -1},
  };
  struct ZarrObjects *zo = NULL;

  /* check for the existence of **any** of the keys, infer format based on it*/
  for (zo = zarrobjects; zo->name; zo++) {
    if (NC_NOERR == nczmap_exists(zfile->map, zo->name)) {
      zfile->zarr.zarr_version = zo->format;
      stat = NC_NOERR;
      break;
    }
  }

  return stat;
}

int NCZ_infer_nczarr_format(NC_FILE_INFO_T *file) {
  int stat = NC_ENOTZARR;
  const NCjson *jsuperblock = NULL, *jnczarrversion = NULL;
  NCZ_FILE_INFO_T *zfile = (NCZ_FILE_INFO_T *)file->format_file_info;
  struct ZARROBJ *zobjs =
      &(((NCZ_GRP_INFO_T *)file->root_grp->format_grp_info)->zgroup);

  int zarrformat = zfile->zarr.zarr_version;

  zfile->zarr.nczarr_version.major = 0;
  zfile->zarr.nczarr_version.minor = 0;
  zfile->zarr.nczarr_version.release = 0;

  if (zarrformat == 2) {
    /* Fetch /.zattrs and /.zgroup contents */
    if ((stat = NCZMD_fetch_json_group(zfile, "/", &zobjs->obj)) ||
        (stat = NCZMD_fetch_json_attrs(zfile, "/", &zobjs->atts))) {
      stat = NC_ENCZARR;
      goto done;
    }

    /* Look for superblock; first in .zattrs and then in .zgroup */
    if ((NCJ_OK == NCJdictget(zobjs->atts, NCZ_V2_SUPERBLOCK, &jsuperblock) &&
         jsuperblock != NULL) ||
        (NCJ_OK == NCJdictget(zobjs->obj, NCZ_V2_SUPERBLOCK, &jsuperblock) &&
         jsuperblock != NULL && (1 == (zobjs->nczv1 = 1)))) {

      if (1 == zobjs->nczv1) { // Key stored in .zgroup!!
        zfile->controls.flags |= FLAG_NCZARR_KEY;
        file->no_write = 1;
      }

      if (NCJsort(jsuperblock) == NCJ_DICT &&
          NCJ_OK == NCJdictget(jsuperblock, "version", &jnczarrversion) &&
          jnczarrversion != NULL && NCJsort(jnczarrversion) == NCJ_STRING) {

        if (sscanf(NCJstring(jnczarrversion), "%lu.%lu.%lu",
                   &zfile->zarr.nczarr_version.major,
                   &zfile->zarr.nczarr_version.minor,
                   &zfile->zarr.nczarr_version.release) != 3) {
          nclog(NCLOGERR, "Issue detecting NCZARR version from %s",
                NCJstring(jnczarrversion));
          stat = NC_ENCZARR;
        }
      }
    } else {
      zfile->controls.flags |= FLAG_PUREZARR;
      stat = NC_NOERR;
    }
  }
done:
  return THROW(stat);
}


int NCZ_get_map(NC_FILE_INFO_T *file, NCURI *uri, mode_t mode,
                size64_t constraints, void *params, NCZMAP **mapp) {
  int stat = NC_NOERR;
  NCZ_FILE_INFO_T *zfile = NULL;
  NCZM_IMPL impl = NCZM_UNDEF;
  char *path = NULL;

  zfile = (NCZ_FILE_INFO_T *)file->format_file_info;
  assert(zfile != NULL);

  impl = zfile->controls
             .mapimpl; // TODO: properly infer map implementation from uri

  if ((path = ncuribuild(uri, NULL, NULL, NCURIALL)) == NULL) {
    stat = NC_ENCZARR;
    goto done;
  }

  if (zfile->creating) {
    stat =
        nczmap_create(impl, path, (int)mode, zfile->controls.flags, NULL, mapp);
  } else {
    stat =
        nczmap_open(impl, path, (int)mode, zfile->controls.flags, NULL, mapp);
  }

done:
  nullfree(path);
  return stat;
}
