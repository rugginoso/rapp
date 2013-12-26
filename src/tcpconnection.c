/*
 * tcpconnection.c - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

#include <sys/socket.h>
#include <sys/sendfile.h>

#include "logger.h"
#include "tcpconnection.h"
#include "eloop.h"

struct TcpConnection
{
  int fd;
  struct ELoop *eloop;
  struct Logger *logger;
  TcpConnectionReadCallback read_callback;
  TcpConnectionWriteCallback write_callback;
  TcpConnectionCloseCallback close_callback;
  const void *data;
};

struct TcpConnection *
tcp_connection_with_fd(int            fd,
                       struct Logger *logger,
                       struct ELoop  *eloop)
{
  struct TcpConnection *connection = NULL;

  assert(fd >= 0);
  assert(logger != NULL);
  assert(eloop != NULL);

  if ((connection = calloc(1, sizeof(struct TcpConnection))) == NULL) {
    LOGGER_PERROR(logger, "calloc");
    return NULL;
  }

  connection->fd = fd;
  connection->logger = logger;
  connection->eloop = eloop;

  return connection;
}

void
tcp_connection_destroy(struct TcpConnection *connection)
{
  assert(connection != NULL);

  if (connection->fd != -1)
    tcp_connection_close(connection);
  free(connection);
}

void
tcp_connection_close(struct TcpConnection *connection)
{
  assert(connection != NULL);

  if (connection->fd >= 0) {
    event_loop_remove_fd_watch(connection->eloop, connection->fd, ELOOP_CALLBACK_READ);
    event_loop_remove_fd_watch(connection->eloop, connection->fd, ELOOP_CALLBACK_WRITE);
    event_loop_remove_fd_watch(connection->eloop, connection->fd, ELOOP_CALLBACK_CLOSE);
    close(connection->fd);
    connection->fd = -1;
  }
}

static int
on_ready_read(int         fd,
              const void *data)
{
  struct TcpConnection *connection = NULL;

  assert(data != NULL);

  connection = (struct TcpConnection *)data;

  if (connection->read_callback)
    connection->read_callback(connection, connection->data);

  return 0;
}

static int
on_ready_write(int         fd,
               const void *data)
{
  struct TcpConnection *connection = NULL;

  assert(data != NULL);

  connection = (struct TcpConnection *)data;

  if (connection->write_callback)
    connection->write_callback(connection, connection->data);

  return 0;
}

static int
on_close(int         fd,
         const void *data)
{
  struct TcpConnection *connection = NULL;

  assert(data != NULL);

  connection = (struct TcpConnection *)data;

  if (connection->close_callback)
    connection->close_callback(connection, connection->data);

  return 0;
}

int
tcp_connection_set_callbacks(struct TcpConnection      *connection,
                             TcpConnectionReadCallback  read_callback,
                             TcpConnectionWriteCallback write_callback,
                             TcpConnectionCloseCallback close_callback,
                             const void                *data)
{
  assert(connection != NULL);

  if (read_callback != NULL) {
    if (event_loop_add_fd_watch(connection->eloop, connection->fd, ELOOP_CALLBACK_READ, on_ready_read, connection) < 0)
      return -1;
    connection->read_callback = read_callback;
  }

  if (write_callback != NULL) {
    if (event_loop_add_fd_watch(connection->eloop, connection->fd, ELOOP_CALLBACK_WRITE, on_ready_write, connection) < 0)
      return -1;
    connection->write_callback = write_callback;
  }

  if (close_callback != NULL) {
    if (event_loop_add_fd_watch(connection->eloop, connection->fd, ELOOP_CALLBACK_CLOSE, on_close, connection))
      return -1;
    connection->close_callback = close_callback;
  }

  connection->data = data;

  return 0;
}

ssize_t
tcp_connection_read_data(struct TcpConnection *connection,
                         void                 *data,
                         size_t                length)
{
  assert(connection != NULL);

  return recv(connection->fd, data, length, 0);
}

ssize_t
tcp_connection_write_data(struct TcpConnection *connection,
                          const void           *data,
                          size_t                length)
{
  assert(connection != NULL);

  return send(connection->fd, data, length, MSG_NOSIGNAL);
}

ssize_t
tcp_connection_sendfile(struct TcpConnection *connection,
                        int                   file_fd,
                        size_t                length)
{
  assert(connection != NULL);
  assert(file_fd >= 0);

  return sendfile(connection->fd, file_fd, NULL, length);
}
/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */

