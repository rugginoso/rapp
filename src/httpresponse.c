#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sendfile.h>

#include "tcpconnection.h"
#include "httpresponse.h"

struct HTTPResponse {
  struct TcpConnection *tcp_connection;

  HTTPResponseNotifyCallback headers_sent_callback;
  HTTPResponseNotifyCallback body_sent_callback;
  void *data;
};

struct HTTPResponse*
http_response_new(struct TcpConnection      *tcp_connection,
                  HTTPResponseNotifyCallback headers_sent_callback,
                  HTTPResponseNotifyCallback body_sent_callback,
                  void                      *data)
{
  struct HTTPResponse *response = NULL;

  assert(tcp_connection != NULL);

  if ((response = calloc(1, sizeof(struct HTTPResponse))) == NULL) {
    perror("calloc");
    return NULL;
  }

  response->tcp_connection = tcp_connection;

  response->headers_sent_callback = headers_sent_callback;
  response->body_sent_callback = body_sent_callback;
  response->data = data;

  return response;
}

void
http_response_destroy(struct HTTPResponse *response)
{
  assert(response != NULL);

  free(response);
}

void http_response_notify_headers_sent(struct HTTPResponse *response)
{
  assert(response != NULL);

  if (response->headers_sent_callback)
    response->headers_sent_callback(response, response->data);
}

void http_response_notify_body_sent(struct HTTPResponse *response)
{
  if (response->body_sent_callback)
    response->body_sent_callback(response, response->data);
}

ssize_t
http_response_write_data(struct HTTPResponse *response,
                         const void          *data,
                         size_t               length)
{
  assert(response != NULL);

  return tcp_connection_write_data(response->tcp_connection, data, length);
}

ssize_t
http_response_sendfile(struct HTTPResponse *response,
                       const char          *path)
{
  int file_fd = -1;
  struct stat file_stat = {0,};
  ssize_t ret = 0;

  assert(response != NULL);
  assert(path != NULL);

  if (lstat(path, &file_stat) < 0) {
    /*
     * TODO: respond with 404
     */
    perror("lstat");
    return -1;
  }

  if ((file_fd = open(path, O_RDONLY)) < 0) {
    /*
     * TODO: respond with 403
     */
    perror("open");
    return -1;
  }

  ret = tcp_connection_sendfile(response->tcp_connection, file_fd, file_stat.st_size);

  close(file_fd);

  return ret;
}

// FIXME arbitrary
#define BUFFER_SIZE 1024

ssize_t
http_response_printf(struct HTTPResponse *response,
                     const char *fmt, ...)
{
  ssize_t ret = 0;
  char buffer[BUFFER_SIZE];

  assert(response != NULL);
  assert(fmt != NULL);

  va_list args;

  va_start(args, fmt);
  ret = vsnprintf(buffer, sizeof(buffer), fmt, args);
  va_end(args);

  if (ret > 0) {
    ret = http_response_write_data(response, buffer, ret);
  }
  return ret;
}

