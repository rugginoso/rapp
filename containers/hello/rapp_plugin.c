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
rapp_serve(struct RappContainer      *handle,
           struct HTTPRequest        *http_request,
           struct HTTPResponseWriter *response_writer)
{
  int err = -1;
  if (handle) {
    size_t len = strlen(handle->message);

    http_response_writer_begin(response_writer, "HTTP/1.1 200 OK" HTTP_EOL);
    http_response_writer_printf(response_writer, "Content-Type: text/plain; charset=utf-8" HTTP_EOL);
    http_response_writer_printf(response_writer, "Content-Length: %d" HTTP_EOL, len);

    http_response_writer_notify_headers_sent(response_writer);

    http_response_writer_write_data(response_writer, handle->message, len);
    http_response_writer_notify_body_sent(response_writer);
    err = 0;
  }
  return err;
}

int
rapp_destroy(struct RappContainer *handle)
{
  free(handle);
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

