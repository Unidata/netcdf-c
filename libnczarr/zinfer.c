#include "zincludes.h"

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
