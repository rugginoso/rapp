#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#include "tcpserver.h"
#include "tcpconnection.h"
#include "eloop.h"

#define BACKLOG 1024


struct TcpServer {
  struct ELoop *eloop;
  int listen_fd;
  TcpServerAcceptCallback accept_callback;
  const void *data;
};


struct TcpServer *
tcp_server_new(struct ELoop *eloop)
{
  struct TcpServer *server = NULL;

  assert(eloop != NULL);

  if ((server = calloc(sizeof(struct TcpServer), 1)) == NULL) {
    perror("calloc");
    return NULL;
  }

  server->listen_fd = -1;
  server->eloop = eloop;

  return server;
}

void
tcp_server_destroy(struct TcpServer *server)
{
  assert(server != NULL);

  if (server->listen_fd >= 0) {
    event_loop_remove_fd_watch(server->eloop, server->listen_fd);
    close(server->listen_fd);
  }

  free(server);
}

void
tcp_server_set_accept_callback(struct TcpServer      *server,
                              TcpServerAcceptCallback callback,
                              const void             *data)
{
  assert(server != NULL);
  assert(callback != NULL);

  server->accept_callback = callback;
  server->data = data;
}

static int
on_incoming_connection(int server_fd, const void *data)
{
  struct TcpServer *server = NULL;
  struct TcpConnection *connection = NULL;
  int client_fd = -1;

  assert(data != NULL);

  server = (struct TcpServer *)data;

  if ((client_fd = accept(server_fd, NULL, NULL)) < 0) {
    perror("accept");
    return -1;
  }

  if ((connection = tcp_connection_with_fd(client_fd, server->eloop)) == NULL) {
    close(client_fd);
    return -1;
  }

  if (server->accept_callback)
    server->accept_callback(connection, server->data);

  return 0;
}

int
tcp_server_start_listen(struct TcpServer *server,
                        const char       *host,
                        uint16_t          port)
{
  /*
   * TODO support inet6
   */
  struct sockaddr_in addr;
  ELoopWatchFdCallback callbacks[ELOOP_CALLBACK_MAX];
  int on = 1;

  assert(server != NULL);
  assert(host != NULL);

  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);

  if (inet_pton(AF_INET, host, &(addr.sin_addr)) != 1) {
    perror("inet_pton");
    return -1;
  }

  if ((server->listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket");
    return -1;
  }

  if (setsockopt(server->listen_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int)) < 0) {
    perror("setsockopt");
    return -1;
  }

  if (setsockopt(server->listen_fd, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(int)) < 0) {
    perror("setsockopt");
    return -1;
  }

  if (bind(server->listen_fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) < 0) {
    perror("bind");
    return -1;
  }

  if (listen(server->listen_fd, BACKLOG) < 0) {
    perror("listen");
    return -1;
  }

  callbacks[ELOOP_CALLBACK_READ] = on_incoming_connection;
  callbacks[ELOOP_CALLBACK_WRITE] = NULL;
  callbacks[ELOOP_CALLBACK_CLOSE] = NULL;

  return event_loop_add_fd_watch(server->eloop, server->listen_fd, callbacks, server);
}

