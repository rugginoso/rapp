/*
 * tcpserver.h - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */

#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <inttypes.h>

struct ELoop;
struct TcpConnection;
struct TcpServer;

typedef void (*TcpServerAcceptCallback)(struct TcpConnection *connection, const void *data);

struct TcpServer *tcp_server_new(struct ELoop *eloop);
void tcp_server_destroy(struct TcpServer *server);

void tcp_server_set_accept_callback(struct TcpServer *server, TcpServerAcceptCallback callback, const void *data);

int tcp_server_start_listen(struct TcpServer *server, const char *host, uint16_t port);

#endif /* TCPSERVER_H */

/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */

