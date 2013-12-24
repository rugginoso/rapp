/*
 * tcpserver.c - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <config.h>

#include "logger.h"
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
  struct Logger *logger;
};


struct TcpServer *
tcp_server_new(struct Logger *logger,
               struct ELoop  *eloop)
{
  struct TcpServer *server = NULL;

  assert(eloop != NULL);

  if ((server = calloc(1, sizeof(struct TcpServer))) == NULL) {
    LOGGER_PERROR(logger, "calloc");
    return NULL;
  }

  server->listen_fd = -1;
  server->eloop = eloop;
  server->logger = logger;

  return server;
}

void
tcp_server_destroy(struct TcpServer *server)
{
  assert(server != NULL);

  if (server->listen_fd >= 0) {
    event_loop_remove_fd_watch(server->eloop, server->listen_fd, ELOOP_CALLBACK_READ);
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
on_incoming_connection(int         server_fd,
                       const void *data)
{
  struct TcpServer *server = NULL;
  struct TcpConnection *connection = NULL;
  int client_fd = -1;

  assert(data != NULL);

  server = (struct TcpServer *)data;

  if ((client_fd = accept(server_fd, NULL, NULL)) < 0) {
    LOGGER_PERROR(server->logger, "accept");
    return -1;
  }

  if ((connection = tcp_connection_with_fd(client_fd, server->logger, server->eloop)) == NULL) {
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
  char port_s[PORT_S_LEN] = { '\0' };
  int addrinfo_ret = 0;
  int on = 1;

  assert(server != NULL);
  assert(host != NULL);

  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  snprintf(port_s, sizeof(port_s), "%d", port);

  if ((addrinfo_ret = getaddrinfo(host, port_s, &hints, &addrinfos)) != 0) {
    LOGGER_PERROR(server->logger, "getaddrinfo");
    return -1;
  }

  if ((server->listen_fd = socket(addrinfos->ai_family, addrinfos->ai_socktype, 0)) < 0) {
    LOGGER_PERROR(server->logger, "socket");
    freeaddrinfo(addrinfos);
    return -1;
  }

  if (setsockopt(server->listen_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int)) < 0) {
    LOGGER_PERROR(server->logger, "setsockopt: reuseaddr");
    freeaddrinfo(addrinfos);
    return -1;
  }

  #ifdef SO_REUSEPORT_FOUND
  if (setsockopt(server->listen_fd, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(int)) < 0) {
    LOGGER_PERROR(server->logger, "setsockopt: reuseport");
    freeaddrinfo(addrinfos);
    return -1;
  }
  #endif

  if (bind(server->listen_fd, addrinfos->ai_addr, addrinfos->ai_addrlen) < 0) {
    LOGGER_PERROR(server->logger, "bind");
    freeaddrinfo(addrinfos);
    return -1;
  }
  freeaddrinfo(addrinfos);

  if (listen(server->listen_fd, BACKLOG) < 0) {
    LOGGER_PERROR(server->logger, "listen");
    return -1;
  }

  return event_loop_add_fd_watch(server->eloop, server->listen_fd, ELOOP_CALLBACK_READ, on_incoming_connection, server);
}

/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */

