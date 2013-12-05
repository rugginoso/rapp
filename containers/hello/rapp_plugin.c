#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "httpresponsewriter.h"
#include "httprequest.h"
#include "container.h"

struct HelloContainer {
  struct Container parent;
  struct HelloContainerPrivate *private;
};


struct HelloContainerPrivate {
  char *message;
};

static void
hello_container_serve(struct Container          *container,
                      struct HTTPRequest        *request,
                      struct HTTPResponseWriter *response_writer)
{
  struct HelloContainer *hello_container = (struct HelloContainer *)container;
  char *content_length = alloca(1024 + 1);

  snprintf(content_length, 1024, "Content-Length: %d\r\n", strlen(hello_container->private->message));

  http_response_writer_write_data(response_writer, "HTTP/1.1 200 OK\r\n", 17);
  http_response_writer_write_data(response_writer, "Content-Type: text/plain; charset=utf-8\r\n", 41);
  http_response_writer_write_data(response_writer, content_length, strlen(content_length));

  http_response_writer_notify_headers_sent(response_writer);

  http_response_writer_write_data(response_writer, hello_container->private->message, strlen(hello_container->private->message));
  http_response_writer_notify_body_sent(response_writer);
}

static void
hello_container_destroy(struct Container *container)
{
  struct HelloContainer *hello_container = (struct HelloContainer *)container;

  if (hello_container->private != NULL)
    free(hello_container->private);

  free(hello_container);
}

int
container_get_abi_version()
{
  return ABI_VERSION;
}

struct Container *
container_new(void)
{
  struct HelloContainer *container;

  if ((container = calloc(sizeof(struct HelloContainer), 1)) == NULL) {
    perror("calloc");
    return NULL;
  }

  if ((container->private = calloc(sizeof(struct HelloContainerPrivate), 1)) == NULL) {
    perror("calloc");
    return NULL;
  }

  container->private->message = "Hello world!";

  container->parent.serve = hello_container_serve;
  container->parent.destroy = hello_container_destroy;

  return (struct Container *)container;
}
