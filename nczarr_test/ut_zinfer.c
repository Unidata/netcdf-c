#include "ut_includes.h"
#include "zformat.h"

#define MOCK_ZMETADATA                                                         \
  "{\"metadata\":{\".zgroup\":{\"zarr_format\":2}},\"zarr_consolidated_"       \
  "format\":1}"

#define MOCK_V2_GROUP_PURE "{\".zgroup\":{\"zarr_format\":2}}"
#define MOCK_V2_ATTRS_PURE "{\"dummy\": 1 }"

#define MOCK_V2_GROUP_NCZARR                                                   \
  "{\".zgroup\":{\"zarr_format\":2}, \"_nczarr_superblock\": {\"version\": "   \
  "\"2.0.0\"}}"
#define MOCK_V2_ATTRS_NCZARR                                                   \
  "{\"_nczarr_group\" : {\"dimensions\": [{name: time, size: 8, unlimited: "   \
  "1}], \"arrays\": [\"v1\", \"v2\"], \"groups\": [\"g1\", \"g2\"]}}"

static char *KV2pure[][2] = {
    {"/.zmetadata", MOCK_ZMETADATA},
    {"/.zgroup", MOCK_V2_GROUP_PURE},
    {"/.zattrs", MOCK_V2_ATTRS_PURE},
    {NULL, NULL},
};

static char *KV2nczarr[][2] = {
    {"/.zmetadata", MOCK_ZMETADATA},
    {"/.zgroup", MOCK_V2_GROUP_NCZARR},
    {"/.zattrs", MOCK_V2_ATTRS_NCZARR},
    {NULL, NULL},
};

static char *KV3[][2] = {
    {"/zarr.json", ""},
    {NULL, NULL},
};

static char *KVempty[][2] = {
    {NULL, NULL},
};

static char *(*KV_ptr)[2] = KV2pure;

int mockunimplemented() { return 1; }

int mocklen(NCZMAP *map, const char *key, size64_t *sizep) {
  for (int i = 0; KV_ptr[i][0] != NULL; i++) {
    if (strcmp(key, KV_ptr[i][0]) == 0) {
      *sizep = strlen(KV_ptr[i][1]);
      return NC_NOERR;
    }
  }
  return NC_ENOOBJECT;
}

int mockexists(NCZMAP *map, const char *key) {
  size64_t size;
  return mocklen(map, key, &size);
}

int mockread(NCZMAP *map, const char *key, size64_t start, size64_t count,
             void *content) {
  for (int i = 0; KV_ptr[i][0] != NULL; i++) {
    if (strcmp(key, KV_ptr[i][0]) == 0) {
      memcpy(content, KV_ptr[i][1] + start, count - start);
      return NC_NOERR;
    }
  }
  return NC_ENOOBJECT;
}

NCZMAP *mockmap() {
  NCZMAP *m = calloc(1, sizeof(NCZMAP));
  memset(m, 0, sizeof(NCZMAP));
  m->format = NCZM_UNDEF, m->url = strdup("mockmap");
  m->mode = NC_NOWRITE;

  m->api = calloc(1, sizeof(NCZMAP_API));
  memset(m->api, 0, sizeof(NCZMAP_API));
  m->api->version = 0;
  m->api->close = (int (*)(NCZMAP *, int))mockunimplemented;
  m->api->exists = mockexists;
  m->api->len = mocklen;
  m->api->read = mockread;
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

  KV_ptr = KVempty;
  // Expected failure
  for (int v = -1; v <= 1; v++) {
    zinfo.map = mockmap();
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

  KV_ptr = KV2pure;
  // Expected success
  for (int v = 2; v <= 3; v++) {
    zinfo.map = mockmap();
    ret = NCZ_infer_zarr_format(&file);
    freemockmap(zinfo.map);
    if (ret != NC_NOERR) {
      fprintf(stderr, "Failed! Unexpected error (%d) when testing version %d\n",
              ret, v);
      return 3;
    }
    if (zinfo.zarr.zarr_version != v) {
      fprintf(stderr, "Failed! Expected version 2 detection (%d)\n",
              zinfo.zarr.zarr_version);
      return 4;
    }
    KV_ptr = KV3;
  }

  return 0;
}

int test_NCZ_infer_nczarr_format() {
  // Requires zfile->map
  NC_FILE_INFO_T file = {0};
  memset(&file, 0, sizeof(file));
  NCZ_FILE_INFO_T zinfo = {0};
  memset(&zinfo, 0, sizeof(zinfo));
  NC_GRP_INFO_T root = {0};
  memset(&root, 0, sizeof(root));
  NCZ_GRP_INFO_T zroot = {0};
  memset(&zroot, 0, sizeof(zroot));

  file.format_file_info = &zinfo;
  root.format_grp_info = &zroot;
  file.root_grp = &root;

  fprintf(stderr, "Testing NCZ_infer_nczarr_format\n");
  int ret = NC_NOERR;

  assert(zinfo.zarr.zarr_version == 0);
  ret = NCZ_infer_nczarr_format(&file);
  if (ret != NC_ENOTZARR) {
    fprintf(stderr, "Failed! Expected return code NC_ENOTZARR (%d), got (%d)\n",
            NC_ENOTZARR, ret);
    return 1;
  }

  zinfo.zarr.zarr_version = 2;

  KV_ptr = KV2pure;
  // Alternative to NCZMD_set_metadata_handler();
  zinfo.map = mockmap();
  zinfo.metadata = *NCZ_metadata_handler2;

  ret = NCZ_infer_nczarr_format(&file);
  if (ret != NC_NOERR) {
    fprintf(stderr, "Failed! Unexpected error (%d)\n", ret);
    return 2;
  }

  if (0 != zinfo.zarr.nczarr_version.major ||
      0 != zinfo.zarr.nczarr_version.minor ||
      0 != zinfo.zarr.nczarr_version.release) {
    fprintf(stderr, "Failed! Wrong nczarr version detectedr (%lu.%lu.%lu)\n",
            zinfo.zarr.nczarr_version.major, zinfo.zarr.nczarr_version.minor,
            zinfo.zarr.nczarr_version.release);
    return 3;
  }

  KV_ptr = KV2nczarr;
  ret = NCZ_infer_nczarr_format(&file);
  if (2 != zinfo.zarr.nczarr_version.major ||
      0 != zinfo.zarr.nczarr_version.minor ||
      0 != zinfo.zarr.nczarr_version.release) {
    fprintf(stderr, "Failed! Wrong nczarr version detectedr (%lu.%lu.%lu)\n",
            zinfo.zarr.nczarr_version.major, zinfo.zarr.nczarr_version.minor,
            zinfo.zarr.nczarr_version.release);
    return 4;
  }

  // TODO:
  // zinfo.zarr.zarr_version = 3;
  // KV_ptr = KV3;
  // zinfo.map = mockmap();
  // zinfo.metadata = *NCZ_metadata_handler3;
  // ret = NCZ_infer_nczarr_format(&file);

  return 0;
}

int main() {
  int stat = 0;

  if ((stat = test_NCZ_infer_zarr_format())) {
    fprintf(stderr, "Unexpected default selection of metadata handler\n");
    return 1;
  }

  if ((stat = test_NCZ_infer_nczarr_format())) {
    fprintf(stderr, "Unexpected default selection of metadata handler\n");
    return 2;
  }
}
