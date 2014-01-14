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

#include <sys/queue.h>

#include "logger.h"
#include "tcpconnection.h"
#include "httpparser.h"
#include "httprequest.h"
#include "httpresponse.h"
#include "httpconnection.h"
#include "httprouter.h"
#include "logger.h"
#include "memory.h"
#include "tcpconnection.h"
#include "rapp/rapp_version.h"


#define BUFSIZE 80 * 1024

struct HTTPQueueEntry {
  struct HTTPRequest *request;
  struct HTTPResponse *response;

  TAILQ_ENTRY(HTTPQueueEntry) entries;
};

TAILQ_HEAD(HTTPQueue, HTTPQueueEntry);

struct HTTPConnection {
  struct TcpConnection *tcp_connection;

  HTTPConnectionFinishCallback finish_callback;
  void *data;

  struct HTTPParser *parser;
  struct HTTPQueue *queue;

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
    if (errno != EAGAIN) {
      LOGGER_PERROR(http_connection->logger, "read");
      http_connection->finish_callback(http_connection, http_connection->data);
    }
    return;
  }

  if (got == 0) {
    return;
  }

  if (http_parser_append_data(http_connection->parser, buffer, got) < 0) {
    logger_trace(http_connection->logger, LOG_ERROR, "httpconnection", "Error appending data to queue");
    http_connection->finish_callback(http_connection, http_connection->data);
  }
}

static void
on_write(struct TcpConnection *tcp_connection,
         const void           *data)
{
  struct HTTPConnection *http_connection = NULL;
  struct HTTPQueueEntry *entry = NULL;

  assert(data != NULL);

  http_connection = (struct HTTPConnection *)data;

  if (TAILQ_EMPTY(http_connection->queue) != 0)
    return;

  entry = TAILQ_LAST(http_connection->queue, HTTPQueue);

  if (http_response_send(entry->response, tcp_connection) == 0) {
    if (http_response_is_complete(entry->response) != 0) {
      if (http_response_is_last(entry->response) != 0) {
        tcp_connection_close(tcp_connection);
        http_connection->finish_callback(http_connection, http_connection->data);
      }
      TAILQ_REMOVE(http_connection->queue, entry, entries);
      http_request_destroy(entry->request);
      http_response_destroy(entry->response);
      memory_destroy(entry);
    }
  }
}

static void
on_close(struct TcpConnection *tcp_connection,
         const void           *data)
{
  struct HTTPConnection *http_connection = NULL;

  assert(data != NULL);

  http_connection = (struct HTTPConnection *)data;

  tcp_connection_close(tcp_connection);
  http_connection->finish_callback(http_connection, http_connection->data);
}

static void
on_new_request(struct HTTPParser *parser,
               void              *data)
{
  struct HTTPConnection *http_connection = NULL;
  struct HTTPRequest *request = NULL;
  struct HTTPQueueEntry *entry = NULL;

  assert(data != NULL);

  http_connection = (struct HTTPConnection *)data;

  request = http_parser_get_next_request(parser);

  if (request != NULL) {
    if ((entry = memory_create(sizeof(struct HTTPQueueEntry))) == NULL) {
      LOGGER_PERROR(http_connection->logger, "memory_create");
      http_request_destroy(request);
      return;
    }

    if ((entry->response = http_response_new(http_connection->logger, "RApp")) == NULL) {
      memory_destroy(entry);
      http_request_destroy(request);
      return;
    }

    entry->request = request;
    http_response_set_last(entry->response, http_request_is_last(request));

    TAILQ_INSERT_HEAD(http_connection->queue, entry, entries);

    http_router_serve(http_connection->router, request, entry->response);
  }
  else {
    http_request_destroy(request);
    logger_trace(http_connection->logger, LOG_ERROR, "httpconnection", "NULL request");
    http_connection->finish_callback(http_connection, http_connection->data);
  }
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

  if ((http_connection->parser = http_parser_new(logger)) == NULL) {
    memory_destroy(http_connection);
    return NULL;
  }
  http_parser_set_new_request_callback(http_connection->parser, on_new_request, http_connection);

  if ((http_connection->queue = memory_create(sizeof(struct HTTPQueue))) == NULL) {
    memory_destroy(http_connection->parser);
    memory_destroy(http_connection);
    return NULL;
  }
  TAILQ_INIT(http_connection->queue);

  http_connection->router = router;

  http_connection->tcp_connection = tcp_connection;
  tcp_connection_set_callbacks(http_connection->tcp_connection, on_read, on_write, on_close, http_connection);

  http_connection->logger = logger;

  return http_connection;
}

void
http_connection_destroy(struct HTTPConnection *http_connection)
{
  assert(http_connection != NULL);

  if (http_connection->tcp_connection != NULL)
    tcp_connection_destroy(http_connection->tcp_connection);

  if (http_connection->parser != NULL)
    http_parser_destroy(http_connection->parser);

  if (http_connection->queue != NULL)
    memory_destroy(http_connection->queue);

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

