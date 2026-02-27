#include "ut_includes.h"
#include "zmetadata.h"

void showHandlers(const NCZ_Metadata *h1, const NCZ_Metadata *h2) {
#define BOOLSTR(x) (x) ? "true" : "false"
  fprintf(stderr,
          "Handler:\n"
          "\tFormat: %d vs %d (%s)\n"
          "\tVersion: %d vs %d (%s)\n"
          "\tFlags: %llu vs %llu (%s)\n"
          "\tJson: %p vs %p (%s)\n"
          "\tlist_nodes: %p vs %p (%s)\n"
          "\tlist_groups: %p vs %p (%s)\n"
          "\tlist_variables: %p vs %p (%s)\n"
          "\tupdate_json_content: %p vs %p (%s)\n"
          "\tfetch_json_content: %p vs %p (%s)\n"
          "\tvalidate_consolidated: %p vs %p (%s)\n",
          h1->zarr_format, h2->zarr_format,
          BOOLSTR(h1->zarr_format == h2->zarr_format), h1->dispatch_version,
          h2->dispatch_version,
          BOOLSTR(h1->dispatch_version == h2->dispatch_version), h1->flags,
          h2->flags, BOOLSTR(h1->flags == h2->flags), h1->jcsl, h2->jcsl,
          BOOLSTR(h1->jcsl == h2->jcsl), h1->list_nodes, h2->list_nodes,
          BOOLSTR(h1->list_nodes == h2->list_nodes), h1->list_groups,
          h2->list_groups, BOOLSTR(h1->list_groups == h2->list_groups),
          h1->list_variables, h2->list_variables,
          BOOLSTR(h1->list_variables == h2->list_variables),
          h1->fetch_json_content, h2->fetch_json_content,
          BOOLSTR(h1->fetch_json_content == h2->fetch_json_content),
          h1->update_json_content, h2->update_json_content,
          BOOLSTR(h1->update_json_content == h2->update_json_content),
          h1->validate_consolidated, h2->validate_consolidated,
          BOOLSTR(h1->validate_consolidated == h2->validate_consolidated));
#undef BOOLSTR
}

int cmpHandlers(const NCZ_Metadata *h1, const NCZ_Metadata *h2) {
  return h1->zarr_format == h2->zarr_format &&
         h1->dispatch_version == h2->dispatch_version &&
         h1->flags == h2->flags && // h*->jcsl aren't compared!
         h1->list_nodes == h2->list_nodes &&
         h1->list_groups == h2->list_groups &&
         h1->list_variables == h2->list_variables &&
         h1->fetch_json_content == h2->fetch_json_content &&
         h1->update_json_content == h2->update_json_content &&
         h1->validate_consolidated == h2->validate_consolidated;
}

#define MOCK_ZMETADATA                                                         \
  "{\"metadata\":{\".zgroup\":{\"zarr_format\":2}},\"zarr_consolidated_"       \
  "format\":1}"

int mockedunimplemented() {
  return 1;
}

int mockedlen(NCZMAP *map, const char *key, size64_t *sizep) {
  if (*sizep) {
    if (strcmp(key, "/.zmetadata") == 0) {
      *sizep = sizeof(MOCK_ZMETADATA);
    } else {
      *sizep = 0;
    }
  }
  return 0;
}
int mockedread(NCZMAP *map, const char *key, size64_t start, size64_t count,
               void *content) {
  if (content) {
    if (strcmp(key, "/.zmetadata") == 0) {
      memcpy(content, MOCK_ZMETADATA + start, count - start);
    }
  }
  return 0;
}

NCZMAP *mockmap() {
  NCZMAP *m = calloc(1, sizeof(NCZMAP));
  memset(m, 0, sizeof(NCZMAP));
  m->format = NCZM_UNDEF, m->url = strdup("mockedmap");
  m->mode = NC_NOWRITE;

  m->api = calloc(1, sizeof(NCZMAP_API));
  memset(m->api, 0, sizeof(NCZMAP_API));
  m->api->version = 0;
  m->api->close = mockedunimplemented;
  m->api->exists = mockedunimplemented;
  m->api->len = mockedlen;
  m->api->read = mockedread;
  m->api->write = mockedunimplemented;
  m->api->search = mockedunimplemented;

  return m;
}

void freemockmap(NCZMAP *map) {
  free(map->api);
  free(map);
}

// Tests

int test_default_handler_selection() {
  fprintf(stderr, "Running %-45s ... ", __func__);
  NCZ_FILE_INFO_T zinfo = {0};
  memset(&zinfo, 0, sizeof(NCZ_FILE_INFO_T));
  NCZMD_set_metadata_handler(&zinfo);
  int result = !cmpHandlers(&zinfo.metadata, NCZ_metadata_handler2);
  fprintf(stderr, "(%s)\n", result ? "FAILED" : "OK");
  if (result)
    showHandlers(&zinfo.metadata, NCZ_metadata_handler2);
  return result;
}

int test_consolidated_handler_selection_env() {
  fprintf(stderr, "Running %-45s ... ", __func__);
  NCZ_FILE_INFO_T zinfo = {0};
  memset(&zinfo, 0, sizeof(NCZ_FILE_INFO_T));
  char *consolidated_env = getenv(NCZARR_CONSOLIDATED_ENV);
  putenv(NCZARR_CONSOLIDATED_ENV "=True");
  zinfo.map = mockmap();
  NCZMD_set_metadata_handler(&zinfo);
  freemockmap(zinfo.map);
  if (consolidated_env == NULL) {
    unsetenv(NCZARR_CONSOLIDATED_ENV);
  } else {
    char buf[256] = {0};
    memcpy(&buf, NCZARR_CONSOLIDATED_ENV "=", sizeof(NCZARR_CONSOLIDATED_ENV));
    memcpy(&buf + sizeof(NCZARR_CONSOLIDATED_ENV "="), consolidated_env,
           strlen(consolidated_env));
    putenv(buf);
  }
  int result = !cmpHandlers(&zinfo.metadata, NCZ_csl_metadata_handler2);
  fprintf(stderr, "(%s)\n", result ? "FAILED" : "OK");
  if (result)
    showHandlers(&zinfo.metadata, NCZ_csl_metadata_handler2);
  return result;
}

int test_consolidated_handler_selection_controls() {
  fprintf(stderr, "Running %-45s ... ", __func__);
  NCZ_FILE_INFO_T zinfo = {0};
  memset(&zinfo, 0, sizeof(NCZ_FILE_INFO_T));
  zinfo.map = mockmap();
  zinfo.controls.flags = FLAG_CONSOLIDATED;
  NCZMD_set_metadata_handler(&zinfo);
  freemockmap(zinfo.map);
  int result = !cmpHandlers(&zinfo.metadata, NCZ_csl_metadata_handler2);
  fprintf(stderr, "(%s)\n", result ? "FAILED" : "OK");
  if (result)
    showHandlers(&zinfo.metadata, NCZ_csl_metadata_handler2);
  return result;
}

int test_consolidated_handler_selection_fallback() {
  fprintf(stderr, "Running %-45s ... ", __func__);
  NCZ_FILE_INFO_T zinfo = {0};
  memset(&zinfo, 0, sizeof(NCZ_FILE_INFO_T));
  zinfo.map = mockmap();
  zinfo.map->api->read = mockedunimplemented;
  zinfo.controls.flags = FLAG_CONSOLIDATED;
  NCZMD_set_metadata_handler(&zinfo);
  freemockmap(zinfo.map);
  int result = !cmpHandlers(&zinfo.metadata, NCZ_metadata_handler2);
  fprintf(stderr, "(%s)\n", result ? "FAILED" : "OK");
  if (result)
    showHandlers(&zinfo.metadata, NCZ_metadata_handler2);
  return result;
}

int main() {
  int stat = 0;

  if ((stat = test_default_handler_selection())) {
    fprintf(stderr, "Unexpected default selection of metadata handler\n");
    return 1;
  }

  if ((stat = test_consolidated_handler_selection_env())) {
    fprintf(stderr, "Unexpected selection of metadata handler, should have "
                    "been consolidated, via env\n");
    return 2;
  }

  if ((stat = test_consolidated_handler_selection_controls())) {
    fprintf(stderr, "Unexpected selection of metadata handler, should have "
                    "been consolidated, via controls\n");
    return 3;
  }

  if ((stat = test_consolidated_handler_selection_fallback())) {
    fprintf(stderr, "Unexpected selection of metadata handler, should have "
                    "been default, not consolidated!\n");
    return 4;
  }

}
