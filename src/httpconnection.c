/*
 * httpconnection.c - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013-2014 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "httprequest.h"
#include "httpresponse.h"
#include "httpconnection.h"
#include "httprouter.h"
#include "logger.h"
#include "memory.h"
#include "tcpconnection.h"
#include "rapp/rapp_version.h"


#define BUFSIZE 80 * 1024

struct HTTPConnection {
  struct TcpConnection *tcp_connection;

  HTTPConnectionFinishCallback finish_callback;
  void *data;

  struct HTTPRequest *request;
  struct HTTPResponse *response;

  struct HTTPRouter *router;
  struct Logger *logger;
};

static void
on_read(struct TcpConnection *tcp_connection,
        const void           *data)
{
  char buffer[BUFSIZE];
  ssize_t got = -1;
  struct HTTPConnection *http_connection = NULL;

  assert(data != NULL);

  http_connection = (struct HTTPConnection *)data;

  if ((got = tcp_connection_read_data(tcp_connection, buffer, BUFSIZE)) < 0) {
    LOGGER_PERROR(http_connection->logger, "read");
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
on_write(struct TcpConnection *tcp_connection,
         const void           *data)
{
  char buffer[BUFSIZE];
  ssize_t got = -1;
  struct HTTPConnection *http_connection = NULL;

  assert(data != NULL);

  http_connection = (struct HTTPConnection *)data;

  if ((got = http_response_read_data(http_connection->response, buffer, BUFSIZE)) > 0) {
    tcp_connection_write_data(tcp_connection, buffer, got);
  }
  else {
    http_connection->finish_callback(http_connection, http_connection->data);
  }
}

static void
on_close(struct TcpConnection *tcp_connection,
         const void           *data)
{
  struct HTTPConnection *http_connection = NULL;

  assert(data != NULL);

  http_connection = (struct HTTPConnection *)data;

  http_connection->finish_callback(http_connection, http_connection->data);
}

static void
on_parse_finish(struct HTTPRequest *request,
                void               *data)
{
  struct HTTPConnection *http_connection = NULL;

  assert(data != NULL);

  http_connection = (struct HTTPConnection *)data;

  tcp_connection_set_callbacks(http_connection->tcp_connection, NULL, on_write, on_close, http_connection);

  http_router_serve(http_connection->router, request, http_connection->response);
}

struct HTTPConnection *
http_connection_new(struct Logger        *logger,
                    struct TcpConnection *tcp_connection,
                    struct HTTPRouter    *router)
{
  struct HTTPConnection *http_connection = NULL;

  assert(tcp_connection != NULL);

  if ((http_connection = memory_create(sizeof(struct HTTPConnection))) == NULL) {
    LOGGER_PERROR(logger, "memory_create");
    return NULL;
  }

  if ((http_connection->request = http_request_new(logger)) == NULL) {
    memory_destroy(http_connection);
    return NULL;
  }
  http_request_set_parse_finish_callback(http_connection->request, on_parse_finish, http_connection);

  if ((http_connection->response = http_response_new(logger, rapp_get_banner())) == NULL) {
    memory_destroy(http_connection->request);
    memory_destroy(http_connection);
    return NULL;
  }

  http_connection->router = router;

  http_connection->tcp_connection = tcp_connection;
  tcp_connection_set_callbacks(http_connection->tcp_connection, on_read, NULL, on_close, http_connection);

  http_connection->logger = logger;

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

  if (http_connection->response != NULL)
    http_response_destroy(http_connection->response);

  memory_destroy(http_connection);
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

/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */

