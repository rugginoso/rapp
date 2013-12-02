#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <http_parser.h>

#include "tcpconnection.h"
#include "httpconnection.h"

#define RECV_BUFSIZE 80 * 1024


struct HTTPHeader {
  char *key;
  char *value;

  struct HTTPHeader *next;
};

struct HTTPConnection {
  struct TcpConnection *tcp_connection;
  http_parser *parser;
  http_parser_settings parser_settings;

  HTTPConnectionFinishCallback finish_callback;
  void *data;

  char *url;
  struct HTTPHeader *headers_list;
};

static int
on_url(http_parser *parser, const char *at, size_t length)
{
  struct HTTPConnection *http_connection = NULL;

  http_connection = (struct HTTPConnection *)parser->data;

  if ((http_connection->url = calloc(sizeof(char), length + 1)) == NULL) {
    perror("calloc");
    return -1;
  }

  memcpy(http_connection->url, at, length);
  http_connection->url[length] = 0;

  return 0;
}

static int
on_header_field(http_parser *parser, const char *at, size_t length)
{
  struct HTTPConnection *http_connection = NULL;
  struct HTTPHeader *header = NULL;

  if ((header = calloc(sizeof(struct HTTPHeader), 1)) == NULL) {
    perror("calloc");
    return -1;
  }

  if ((header->key = calloc(sizeof(char), length + 1)) == NULL) {
    perror("calloc");
    return -1;
  }
  memcpy(header->key, at, length);
  header->key[length] = 0;

  http_connection = (struct HTTPConnection *)parser->data;

  header->next = http_connection->headers_list;
  http_connection->headers_list = header;

  return 0;
}

static int
on_header_value(http_parser *parser, const char *at, size_t length)
{
  struct HTTPConnection *http_connection = NULL;
  struct HTTPHeader *header = NULL;

  http_connection = (struct HTTPConnection *)parser->data;
  header = http_connection->headers_list;

  if ((header->value = calloc(sizeof(char), length + 1)) == NULL) {
    perror("calloc");
    return -1;
  }
  memcpy(header->value, at, length);
  header->value[length] = 0;

  return 0;
}

static int
on_headers_complete(http_parser *parser)
{
  struct HTTPConnection *http_connection = NULL;
  struct HTTPHeader *hp = NULL;

  http_connection = (struct HTTPConnection *)parser->data;

  printf("REQUEST: %s\n", http_connection->url);
  printf("HEADERS:\n");

  for (hp = http_connection->headers_list; hp != NULL; hp = hp->next) {
    printf("KEY: %s\tVALUE: %s\n", hp->key, hp->value);
  }

  printf("\n\n");

  return 0;
}

static int
on_message_complete(http_parser *parser)
{
  struct HTTPConnection *http_connection = NULL;
  http_connection = (struct HTTPConnection *)parser->data;

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

  memset(buffer, 0, RECV_BUFSIZE);

  got = tcp_connection_read_data(tcp_connection, buffer, RECV_BUFSIZE);

  parsed = http_parser_execute(http_connection->parser, &(http_connection->parser_settings), buffer, got);

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

  if (http_connection->url)
    free(http_connection->url);

  while (http_connection->headers_list != NULL) {
    hp = http_connection->headers_list->next;

    free(http_connection->headers_list->key);
    free(http_connection->headers_list->value);
    free(http_connection->headers_list);

    http_connection->headers_list = hp;
  }

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
