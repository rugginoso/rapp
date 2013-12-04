#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "tcpconnection.h"
#include "httprequest.h"
#include "httpconnection.h"

#define RECV_BUFSIZE 80 * 1024

struct HTTPConnection {
  struct TcpConnection *tcp_connection;

  HTTPConnectionFinishCallback finish_callback;
  void *data;

  struct HTTPRequest *request;
};

static void
on_parse_finish(struct HTTPRequest *request, void *data)
{
  struct HTTPConnection *http_connection = NULL;

  assert(data != NULL);

  http_connection = (struct HTTPConnection *)data;

  printf("Parsing finished\n");

  http_connection->finish_callback(http_connection, http_connection->data);
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
http_connection_new(struct TcpConnection *tcp_connection)
{
  struct HTTPConnection *http_connection = NULL;

  assert(tcp_connection != NULL);

  if ((http_connection = calloc(sizeof(struct HTTPConnection), 1)) == NULL) {
    perror("calloc");
    return NULL;
  }

  if ((http_connection->request = http_request_new()) == NULL) {
    free(http_connection);
    return NULL;
  }
  http_request_set_parse_finish_callback(http_connection->request, on_parse_finish, http_connection);

  http_connection->tcp_connection = tcp_connection;
  tcp_connection_set_callbacks(http_connection->tcp_connection, on_read, NULL, on_close, http_connection);

  return http_connection;
}

void
http_connection_destroy(struct HTTPConnection *http_connection)
{
  assert(http_connection != NULL);

  if (http_connection->tcp_connection)
    tcp_connection_destroy(http_connection->tcp_connection);

  if (http_connection->request)
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

