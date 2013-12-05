#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "tcpconnection.h"
#include "httpresponsewriter.h"

struct HTTPResponseWriter {
  struct TcpConnection *tcp_connection;

  HTTPResponseWriterNotifyCallback headers_sent_callback;
  HTTPResponseWriterNotifyCallback body_sent_callback;
  void *data;
};

struct HTTPResponseWriter*
http_response_writer_new(struct TcpConnection            *tcp_connection,
                         HTTPResponseWriterNotifyCallback headers_sent_callback,
                         HTTPResponseWriterNotifyCallback body_sent_callback,
                         void                            *data)
{
  struct HTTPResponseWriter *response_writer = NULL;

  assert(tcp_connection != NULL);

  if ((response_writer = calloc(sizeof(struct HTTPResponseWriter), 1)) == NULL) {
    perror("calloc");
    return NULL;
  }

  response_writer->tcp_connection = tcp_connection;

  response_writer->headers_sent_callback = headers_sent_callback;
  response_writer->body_sent_callback = body_sent_callback;
  response_writer->data = data;

  return response_writer;
}

void
http_response_writer_destroy(struct HTTPResponseWriter *response_writer)
{
  assert(response_writer != NULL);

  free(response_writer);
}

void http_response_writer_notify_headers_sent(struct HTTPResponseWriter *response_writer)
{
  assert(response_writer != NULL);

  if (response_writer->headers_sent_callback)
    response_writer->headers_sent_callback(response_writer, response_writer->data);
}

void http_response_writer_notify_body_sent(struct HTTPResponseWriter *response_writer)
{
  if (response_writer->body_sent_callback)
    response_writer->body_sent_callback(response_writer, response_writer->data);
}

ssize_t
http_response_writer_write_data(struct HTTPResponseWriter *response_writer,
                                const void                *data,
                                size_t                     length)
{
  assert(response_writer != NULL);

  return tcp_connection_write_data(response_writer->tcp_connection, data, length);
}

ssize_t
http_response_writer_sendfile(struct HTTPResponseWriter *response_writer,
                              const char                *path)
{
  assert(response_writer != NULL);

  return tcp_connection_sendfile(response_writer->tcp_connection, path);
}
