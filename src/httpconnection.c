#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <http_parser.h>

#include "tcpconnection.h"
#include "httpconnection.h"

#define RECV_BUFSIZE 80 * 1024
#define MAX_HEADERS 1024 // FIXME: choose an appropriate size


struct MemoryRange {
  size_t offset;
  size_t length;
};

struct HeaderMemoryRange {
  struct MemoryRange key;
  struct MemoryRange value;
};

struct HTTPConnection {
  struct TcpConnection *tcp_connection;

  http_parser *parser;
  http_parser_settings parser_settings;
  unsigned current_header;

  HTTPConnectionFinishCallback finish_callback;
  void *data;

  struct MemoryRange url_range;
  struct HeaderMemoryRange headers_ranges[MAX_HEADERS];

  char *buffer;
  size_t buffer_length;
};

static int
on_url(http_parser *parser, const char *at, size_t length)
{
  struct HTTPConnection *http_connection = NULL;

  http_connection = (struct HTTPConnection *)parser->data;

  http_connection->url_range.offset = at - http_connection->buffer;
  http_connection->url_range.length = length;

  return 0;
}

static int
on_header_field(http_parser *parser, const char *at, size_t length)
{
  struct HTTPConnection *http_connection = NULL;

  http_connection = (struct HTTPConnection *)parser->data;

  http_connection->headers_ranges[http_connection->current_header].key.offset = at - http_connection->buffer;
  http_connection->headers_ranges[http_connection->current_header].key.length = length;

  return 0;
}

static int
on_header_value(http_parser *parser, const char *at, size_t length)
{
  struct HTTPConnection *http_connection = NULL;

  http_connection = (struct HTTPConnection *)parser->data;

  http_connection->headers_ranges[http_connection->current_header].value.offset = at - http_connection->buffer;
  http_connection->headers_ranges[http_connection->current_header].value.length = length;

  http_connection->current_header++;

  return 0;
}

static int
on_headers_complete(http_parser *parser)
{
  struct HTTPConnection *http_connection = NULL;
  char *buf = NULL;
  int i = 0;

  http_connection = (struct HTTPConnection *)parser->data;

  buf = alloca(http_connection->url_range.length + 1);
  memcpy(buf, &(http_connection->buffer[http_connection->url_range.offset]), http_connection->url_range.length);
  buf[http_connection->url_range.length] = 0;

  printf("REQUEST: %s\n", buf);
  printf("HEADERS:\n");

  while (i < http_connection->current_header) {
    buf = alloca(http_connection->headers_ranges[i].key.length+1);
    memcpy(buf, &(http_connection->buffer[http_connection->headers_ranges[i].key.offset]), http_connection->headers_ranges[i].key.length);
    buf[http_connection->headers_ranges[i].key.length] = 0;
    printf("KEY: %s\t", buf);

    buf = alloca(http_connection->headers_ranges[i].value.length+1);
    memcpy(buf, &(http_connection->buffer[http_connection->headers_ranges[i].value.offset]), http_connection->headers_ranges[i].value.length);
    buf[http_connection->headers_ranges[i].value.length] = 0;
    printf("VALUE: %s\n", buf);

    i++;
  }

  printf("\n\n");

  return 0;
}

static int
on_message_complete(http_parser *parser)
{
  struct HTTPConnection *http_connection = NULL;
  http_connection = (struct HTTPConnection *)parser->data;

  tcp_connection_close(http_connection->tcp_connection);
  http_connection->finish_callback(http_connection, http_connection->data);

  return 0;
}

static void
on_read(struct TcpConnection *tcp_connection, const void *data)
{
  char buffer[RECV_BUFSIZE];
  ssize_t got = -1;
  ssize_t parsed = -1;
  struct HTTPConnection *http_connection = NULL;

  assert(data != NULL);

  http_connection = (struct HTTPConnection *)data;

  if ((got = tcp_connection_read_data(tcp_connection, buffer, RECV_BUFSIZE)) < 0) {
    perror("read");
    http_connection->finish_callback(http_connection, http_connection->data);
    return;
  }

  if ((http_connection->buffer = realloc(http_connection->buffer, http_connection->buffer_length + got)) == NULL) {
    perror("realloc");
    http_connection->finish_callback(http_connection, http_connection->data);
  }
  memcpy(&(http_connection->buffer[http_connection->buffer_length]), buffer, got);

  parsed = http_parser_execute(http_connection->parser,
                               &(http_connection->parser_settings),
                               &(http_connection->buffer[http_connection->buffer_length]),
                               got);

  http_connection->buffer_length += got;

  if (parsed != got)
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

  if ((http_connection->parser = calloc(sizeof(http_parser), 1)) == NULL) {
    perror("calloc");
    free(http_connection);
    return NULL;
  }

  http_parser_init(http_connection->parser, HTTP_REQUEST);
  http_connection->parser->data = http_connection;

  http_connection->parser_settings.on_url = on_url;
  http_connection->parser_settings.on_header_field = on_header_field;
  http_connection->parser_settings.on_header_value = on_header_value;
  http_connection->parser_settings.on_headers_complete = on_headers_complete;
  http_connection->parser_settings.on_message_complete = on_message_complete;

  http_connection->tcp_connection = tcp_connection;
  tcp_connection_set_callbacks(http_connection->tcp_connection, on_read, NULL, on_close, http_connection);

  return http_connection;
}

void
http_connection_destroy(struct HTTPConnection *http_connection)
{
  struct HTTPHeader *hp = NULL;

  assert(http_connection != NULL);

  if (http_connection->parser)
    free(http_connection->parser);

  if (http_connection->buffer)
    free(http_connection->buffer);

  if (http_connection->tcp_connection)
    tcp_connection_destroy(http_connection->tcp_connection);

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
