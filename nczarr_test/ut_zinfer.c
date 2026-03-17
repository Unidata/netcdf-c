#include "ut_includes.h"
#include "zformat.h"

int mockunimplemented() { return 1; }

int mockV2exists(NCZMAP *map, const char *key) {
  return !(strcmp(key, "/.zmetadata") == 0 || strcmp(key, "/.zgroup") == 0);
}

int mockV3exists(NCZMAP *map, const char *key) {
  return !(strcmp(key, "/zarr.json") == 0);
}

NCZMAP *mockmap(int version) {
  NCZMAP *m = calloc(1, sizeof(NCZMAP));
  memset(m, 0, sizeof(NCZMAP));
  m->format = NCZM_UNDEF, m->url = strdup("mockmap");
  m->mode = NC_NOWRITE;

  m->api = calloc(1, sizeof(NCZMAP_API));
  memset(m->api, 0, sizeof(NCZMAP_API));
  m->api->version = 0;
  m->api->close = (int (*)(NCZMAP *, int))mockunimplemented;

  m->api->exists =
      (version == 2)
          ? (mockV2exists)
          : ((version == 3) ? mockV3exists
                            : (int (*)(NCZMAP * map, const char *key))
                                  mockunimplemented);
  m->api->len = (int (*)(NCZMAP * map, const char *key, size64_t *sizep))
      mockunimplemented;
  m->api->read = (int (*)(NCZMAP * map, const char *key, size64_t start,
                          size64_t count, void *content)) mockunimplemented;
  m->api->write = (int (*)(NCZMAP *, const char *, size64_t,
                           const void *))mockunimplemented;
  m->api->search =
      (int (*)(NCZMAP *, const char *, struct NClist *))mockunimplemented;

  return m;
}

void freemockmap(NCZMAP *map) {
  free(map->api);
  free(map);
}

// Tests

int test_NCZ_infer_zarr_format() {
  // Requires zfile->map to be set
  NC_FILE_INFO_T file = {0};
  memset(&file, 0, sizeof(file));
  NCZ_FILE_INFO_T zinfo = {0};
  memset(&zinfo, 0, sizeof(zinfo));
  file.format_file_info = &zinfo;

  fprintf(stderr, "Testing NCZ_infer_zarr_format\n");
  int ret = NC_NOERR;

  for (int v = -1; v <= 1; v++) {
    zinfo.map = mockmap(v);
    ret = NCZ_infer_zarr_format(&file);
    freemockmap(zinfo.map);
    if (ret != NC_ENOTZARR) {
      fprintf(stderr,
              "Failed! Expected return code NC_ENOTZARR (%d), got (%d)\n",
              NC_ENOTZARR, ret);
      return 1;
    }
    if (zinfo.zarr.zarr_version != 0) {
      fprintf(stderr,
              "Failed! Expected zarr version to be left unset (0) but it set "
              "to %d\n",
              zinfo.zarr.zarr_version);
      return 2;
    }
  }

  for (int v = 2; v <= 3; v++) {
    zinfo.map = mockmap(v);
    ret = NCZ_infer_zarr_format(&file);
    freemockmap(zinfo.map);
    if (ret != NC_NOERR) {
      fprintf(stderr, "Failed! Unexpected error (%d)\n", ret);
      return 3;
    }
    if (zinfo.zarr.zarr_version != v) {
      fprintf(stderr, "Failed! Expected version 2 detection (%d)\n",
              zinfo.zarr.zarr_version);
      return 4;
    }
  }

  return 0;
}

int main() {
  int stat = 0;

  if ((stat = test_NCZ_infer_zarr_format())) {
    fprintf(stderr, "Unexpected default selection of metadata handler\n");
    return 1;
  }
}
