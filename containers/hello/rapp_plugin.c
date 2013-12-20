/*
 * rapp_plugin.c - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */

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

    http_response_printf(response, "HTTP/1.1 200 OK" HTTP_EOL);
    http_response_printf(response, "Content-Type: text/plain; charset=utf-8" HTTP_EOL);
    http_response_printf(response, "Content-Length: %d" HTTP_EOL, len);

    http_response_notify_headers_sent(response);

    http_response_write_data(response, handle->message, len);
    http_response_notify_body_sent(response);
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
rapp_create(void *cookie, int argc, char **argv, int *err)
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

