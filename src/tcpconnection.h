/*
 * tcpconnection.h - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013-2014 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */

#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H

struct Logger;
struct ELoop;
struct TcpConnection;

typedef void (*TcpConnectionReadCallback)(struct TcpConnection *connection, const void *data);
typedef void (*TcpConnectionWriteCallback)(struct TcpConnection *connection, const void *data);
typedef void (*TcpConnectionCloseCallback)(struct TcpConnection *connection, const void *data);

struct TcpConnection *tcp_connection_with_fd(int fd, struct Logger *logger, struct ELoop *eloop);
void tcp_connection_destroy(struct TcpConnection *connection);

void tcp_connection_close(struct TcpConnection *connection);

int tcp_connection_set_callbacks(struct TcpConnection *connection, TcpConnectionReadCallback read_callback, TcpConnectionWriteCallback write_callback, TcpConnectionCloseCallback close_callback, const void *data);

ssize_t tcp_connection_read_data(struct TcpConnection *connection, void *data, size_t length);
ssize_t tcp_connection_write_data(struct TcpConnection *connection, const void *data, size_t length);

ssize_t tcp_connection_sendfile(struct TcpConnection *connection, int file_fd, size_t length);

#endif /* TCPCONNECTION_H */

/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */

