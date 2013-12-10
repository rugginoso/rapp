#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <config.h>

#include "tcpserver.h"
#include "tcpconnection.h"
#include "eloop.h"

#define BACKLOG 1024
#define STRLEN(s) (sizeof(s)/sizeof(s[0]))
#define PORT_S_LEN STRLEN("65535")


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

  if ((server = calloc(1, sizeof(struct TcpServer))) == NULL) {
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
  struct addrinfo *addrinfos, hints = {0, };
  char *port_s = NULL;
  int addrinfo_ret = 0;
  ELoopWatchFdCallback callbacks[ELOOP_CALLBACK_MAX];
  int on = 1;

  assert(server != NULL);
  assert(host != NULL);

  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  port_s = alloca(PORT_S_LEN);
  snprintf(port_s, PORT_S_LEN, "%d", port);

  if ((addrinfo_ret = getaddrinfo(host, port_s, &hints, &addrinfos)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(addrinfo_ret));
    return -1;
  }

  if ((server->listen_fd = socket(addrinfos->ai_family, addrinfos->ai_socktype, 0)) < 0) {
    perror("socket");
    freeaddrinfo(addrinfos);
    return -1;
  }

  if (setsockopt(server->listen_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int)) < 0) {
    perror("setsockopt");
    freeaddrinfo(addrinfos);
    return -1;
  }

  #ifdef SO_REUSEPORT_FOUND
  if (setsockopt(server->listen_fd, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(int)) < 0) {
    perror("setsockopt");
    freeaddrinfo(addrinfos);
    return -1;
  }
  #endif

  if (bind(server->listen_fd, addrinfos->ai_addr, addrinfos->ai_addrlen) < 0) {
    perror("bind");
    freeaddrinfo(addrinfos);
    return -1;
  }
  freeaddrinfo(addrinfos);

  if (listen(server->listen_fd, BACKLOG) < 0) {
    perror("listen");
    return -1;
  }

  callbacks[ELOOP_CALLBACK_READ] = on_incoming_connection;
  callbacks[ELOOP_CALLBACK_WRITE] = NULL;
  callbacks[ELOOP_CALLBACK_CLOSE] = NULL;

  return event_loop_add_fd_watch(server->eloop, server->listen_fd, callbacks, server);
}

