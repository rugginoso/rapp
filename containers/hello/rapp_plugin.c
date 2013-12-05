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
    char *content_length = alloca(1024 + 1);
    size_t len = strlen(handle->message);

    snprintf(content_length, 1024, "Content-Length: %d\r\n", len);

    http_response_writer_write_data(response_writer, "HTTP/1.1 200 OK\r\n", 17);
    http_response_writer_write_data(response_writer, "Content-Type: text/plain; charset=utf-8\r\n", 41);
    http_response_writer_write_data(response_writer, content_length, strlen(content_length));

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

