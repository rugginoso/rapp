#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "tcpconnection.h"
#include "httprequest.h"
#include "httpresponsewriter.h"
#include "httpconnection.h"
#include "httprouter.h"


#define RECV_BUFSIZE 80 * 1024

struct HTTPConnection {
  struct TcpConnection *tcp_connection;

  HTTPConnectionFinishCallback finish_callback;
  void *data;

  struct HTTPRequest *request;
  struct HTTPRouter *router;
};

static void
on_headers_sent(struct HTTPResponseWriter *response_writer, void *data)
{
  struct HTTPConnection *http_connection = NULL;

  assert(data != NULL);

  http_connection = (struct HTTPConnection *)data;

  http_response_writer_write_data(response_writer, "Server: RApp dev\r\n", 18);
  http_response_writer_write_data(response_writer, "\r\n", 2);
}

static void
on_body_sent(struct HTTPResponseWriter *response_writer, void *data)
{
  struct HTTPConnection *http_connection = NULL;

  assert(data != NULL);

  http_connection = (struct HTTPConnection *)data;

  http_response_writer_write_data(response_writer, "\r\n", 2);
  http_connection->finish_callback(http_connection, http_connection->data);
}

static void
on_parse_finish(struct HTTPRequest *request, void *data)
{
  struct HTTPConnection *http_connection = NULL;
  struct HTTPResponseWriter *response_writer = NULL;

  assert(data != NULL);

  http_connection = (struct HTTPConnection *)data;

  if ((response_writer = http_response_writer_new(http_connection->tcp_connection, on_headers_sent, on_body_sent, http_connection)) == NULL) {
    http_connection->finish_callback(http_connection, http_connection->data);
    return;
  }

  http_router_serve(http_connection->router, request, response_writer);

  http_response_writer_destroy(response_writer);
}

static void
on_read(struct TcpConnection *tcp_connection, const void *data)
{
  char buffer[RECV_BUFSIZE];
  ssize_t got = -1;
  struct HTTPConnection *http_connection = NULL;

  assert(data != NULL);

  http_connection = (struct HTTPConnection *)data;

  if ((got = tcp_connection_read_data(tcp_connection, buffer, RECV_BUFSIZE)) < 0) {
    perror("read");
    http_connection->finish_callback(http_connection, http_connection->data);
    return;
  }

  if (got == 0) {
    return;
  }

  if (http_request_append_data(http_connection->request, buffer, got) < 0)
    http_connection->finish_callback(http_connection, http_connection->data);
}

static void
on_close(struct TcpConnection *tcp_connection, const void *data)
{
  struct HTTPConnection *http_connection = NULL;

  assert(data != NULL);

  http_connection = (struct HTTPConnection *)data;

  http_connection->finish_callback(http_connection, http_connection->data);
}

struct HTTPConnection *
http_connection_new(struct TcpConnection *tcp_connection, struct HTTPRouter *router)
{
  struct HTTPConnection *http_connection = NULL;

  assert(tcp_connection != NULL);

  if ((http_connection = calloc(1, sizeof(struct HTTPConnection))) == NULL) {
    perror("calloc");
    return NULL;
  }

  if ((http_connection->request = http_request_new()) == NULL) {
    free(http_connection);
    return NULL;
  }
  http_request_set_parse_finish_callback(http_connection->request, on_parse_finish, http_connection);

  http_connection->router = router;
  http_connection->tcp_connection = tcp_connection;
  tcp_connection_set_callbacks(http_connection->tcp_connection, on_read, NULL, on_close, http_connection);

  return http_connection;
}

void
http_connection_destroy(struct HTTPConnection *http_connection)
{
  assert(http_connection != NULL);

  if (http_connection->tcp_connection != NULL)
    tcp_connection_destroy(http_connection->tcp_connection);

  if (http_connection->request != NULL)
    http_request_destroy(http_connection->request);

  free(http_connection);
}

void
http_connection_set_finish_callback(struct HTTPConnection        *http_connection,
                                    HTTPConnectionFinishCallback  finish_callback,
                                    void                         *data)
{
  assert(http_connection != NULL);
  assert(finish_callback != NULL);

  http_connection->finish_callback = finish_callback;
  http_connection->data = data;
}

