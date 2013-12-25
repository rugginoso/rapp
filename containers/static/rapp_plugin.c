/*
 * rapp_plugin.c - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>

#include <rapp/rapp.h>

#include "path.h"
#include "mimetypes.h"
#include "errors.h"

struct RappContainer {
  char *document_root;
};

int
rapp_get_abi_version()
{
  return ABI_VERSION;
}

int
rapp_serve(struct RappContainer *handle,
           struct HTTPRequest   *http_request,
           struct HTTPResponse  *http_response)
{
  enum HTTPMethod method;
  char path[MAXPATHLEN + 1];
  struct stat file_stat;
  char len_s[30];

  int fd = -1;

  if (handle == NULL)
    return -1;

  method = http_request_get_method(http_request);

  if (method != HTTP_METHOD_HEAD && method != HTTP_METHOD_GET) {
    if (http_response_write_error_by_code(http_response, 405) < 0)
      return -1;
  }

  if (get_path(path, http_request, handle->document_root) < 0)
    return -1;

  if (stat(path, &file_stat) < 0) {
    if (http_response_write_error_by_code(http_response, errno_to_http_code()) < 0)
      return -1;
    return 0;
  }

  if (S_ISREG(file_stat.st_mode) == 0) {
    http_response_write_error_by_code(http_response, 403);
    return 0;
  }

  /*
   * FIXME: cache
   */

  http_response_write_status_line_by_code(http_response, 200);

  snprintf(len_s, 30, "%d", file_stat.st_size);
  http_response_write_header(http_response, "Content-Length", len_s);

  http_response_write_header(http_response, "Content-Type", mimetype(path));
  http_response_end_headers(http_response);

  if (method == HTTP_METHOD_HEAD)
    return 0;

  if (http_response_write_file(http_response, path, file_stat.st_size) < 0) {
    if (http_response_write_error_by_code(http_response, errno_to_http_code()) < 0)
      return -1;
    return 0;
  }

  return 0;
}

int
rapp_destroy(struct RappContainer *handle)
{
  free(handle->document_root);
  free(handle);

  return 0;
}

struct RappContainer *
rapp_create(void  *cookie,
            int    argc,
            char **argv,
            int   *err)
{
  struct RappContainer *handle = calloc(1, sizeof(struct RappContainer));
  size_t document_root_len = 0;
  if (handle) {
    handle->document_root = strdup(argv[2]);
    document_root_len = strlen(handle->document_root);
    if (handle->document_root[document_root_len - 1] == '/')
      handle->document_root[document_root_len - 1] = 0;
    *err = 0;
  } else {
    *err = -1;
  }
  return handle;
}

/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */

