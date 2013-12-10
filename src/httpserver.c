#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "eloop.h"
#include "tcpserver.h"
#include "httpconnection.h"
#include "httpserver.h"


struct HTTPServer {
  struct TcpServer *tcp_server;
  struct ELoop *eloop;
};


static void
on_request_finish(struct HTTPConnection *http_connection, void *data)
{
  struct HTTPServer *http_server = NULL;

  assert(data != NULL);

  http_server = (struct HTTPServer *)data;

  event_loop_schedule_free(http_server->eloop, (CollectorFreeFunc)http_connection_destroy, http_connection);
}

static void
on_accept(struct TcpConnection *tcp_connection, const void *data)
{
  struct HTTPServer *http_server = NULL;
  struct HTTPConnection *http_connection = NULL;

  assert(data != NULL);

  http_server = (struct HTTPServer *)data;

  if ((http_connection = http_connection_new(tcp_connection)) == NULL) {
    return;
  }

  http_connection_set_finish_callback(http_connection, on_request_finish, http_server);
}

struct HTTPServer *
http_server_new(struct ELoop *eloop)
{
  struct HTTPServer *http_server = NULL;

  if ((http_server = calloc(1, sizeof(struct HTTPServer))) == NULL) {
    perror("calloc");
    return NULL;
  }

  if ((http_server->tcp_server = tcp_server_new(eloop)) == NULL) {
    free(http_server);
    return NULL;
  }

  tcp_server_set_accept_callback(http_server->tcp_server, on_accept, http_server);

  http_server->eloop = eloop;

  return http_server;
}

void
http_server_destroy(struct HTTPServer *http_server)
{
  assert(http_server != NULL);

  if (http_server->tcp_server != NULL)
    tcp_server_destroy(http_server->tcp_server);

  free(http_server);
}

int http_server_start(struct HTTPServer *http_server,
                      const char       *host,
                      uint16_t          port)
{
  assert(http_server != NULL);
  assert(host != NULL);

  return tcp_server_start_listen(http_server->tcp_server, host, port);
}

