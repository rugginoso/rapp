/*
 * rapp_plugin.c - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <rapp/rapp.h>


struct RappContainer {
  char *message;
};

int
rapp_get_abi_version()
{
  return ABI_VERSION;
}


int
rapp_serve(struct RappContainer *handle,
           struct HTTPRequest   *http_request,
           struct HTTPResponse  *response)
{
  int err = -1;
  if (handle) {
    size_t len = strlen(handle->message);
    char *len_s = NULL;

    http_response_write_status_line_by_code(response, 200);
    http_response_write_header(response, "Content-Type", "text/plain; charset=utf-8");

    asprintf(&len_s, "%lu", len);
    http_response_write_header(response, "Content-Length", len_s);
    free(len_s);

    http_response_end_headers(response);

    http_response_append_data(response, handle->message, len);

    err = 0;
  }
  return err;
}

int
rapp_destroy(struct RappContainer *handle)
{
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
  if (handle) {
    handle->message = "Hello world!";
    *err = 0;
  } else {
    *err = -1;
  }
  return handle;
}

/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */

