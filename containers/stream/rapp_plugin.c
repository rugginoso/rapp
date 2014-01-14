/*
 * rapp_plugin.c - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013-2014 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <rapp/rapp.h>


struct RappContainer {
  char *message;
  unsigned count;
  unsigned sleep;
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
    unsigned count = handle->count;

    http_response_write_status_line_by_code(response, 200);
    http_response_write_header(response, "Content-Type", "text/plain; charset=utf-8");
    http_response_write_header(response, "Transfer-Encoding", "Chunked");
    http_response_write_header(response, "x-content-type-options", "nosniff");

    http_response_end_headers(response);

    asprintf(&len_s, "%x\r\n", len);

    while (count > 0) {
      http_response_append_data(response, len_s, strlen(len_s));

      http_response_append_data(response, handle->message, len);
      http_response_append_data(response, HTTP_EOL, strlen(HTTP_EOL));

      count--;
      sleep(handle->sleep);
    }

    http_response_append_data(response, "0\r\n\r\n", 5);

    free(len_s);

    http_response_end_body(response);

    err = 0;
  }
  return err;
}

int
rapp_destroy(struct RappContainer *handle)
{
  free(handle->message);
  free(handle);
  return 0;
}

int
rapp_init(struct RappContainer *handle,
          struct RappConfig    *config)
{
  char *message = NULL;

  if (rapp_config_get_string(config, "stream", "message", &message) < 0)
    return -1;

  asprintf(&(handle->message), "%s\n", message);
  free(message);

  if (rapp_config_get_int(config, "stream", "count", (long *)&(handle->count)) < 0)
    return -1;

  if (rapp_config_get_int(config, "stream", "sleep", (long *)&(handle->sleep)) < 0)
    return -1;

  return 0;
}

struct RappContainer *
rapp_create(void              *cookie,
            struct RappConfig *config,
            int               *err)
{
  struct RappContainer *handle = calloc(1, sizeof(struct RappContainer));
  if (handle) {
    rapp_config_opt_add(config, "stream", "message", PARAM_STRING, "Stream message", "MESSAGE");
    rapp_config_opt_set_default_string(config, "stream", "message", "Hello world!");

    rapp_config_opt_add(config, "stream", "count", PARAM_INT, "Message count", "COUNT");
    rapp_config_opt_set_default_int(config, "stream", "count", 10);

    rapp_config_opt_add(config, "stream", "sleep", PARAM_INT, "Sleep time between messages", "SLEEP");
    rapp_config_opt_set_default_int(config, "stream", "sleep", 1);

    *err = 0;
  } else {
    *err = -1;
  }
  return handle;
}

/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */

