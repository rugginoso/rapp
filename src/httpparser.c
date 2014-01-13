/*
 * httpparser.c - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>


#include <http_parser.h>

#include "memory.h"
#include "logger.h"
#include "httprequest.h"
#include "httpparser.h"

#define MAX_REQUESTS 100

struct HTTPParser {
  http_parser http_parser;
  http_parser_settings http_parser_settings;
  unsigned current_header;

  struct HTTPRequest *request;
  uint8_t current_request;

  char *buffer;
  size_t buffer_length;

  HTTPParserNewRequestCallback new_request_callback;
  void *data;

  struct Logger *logger;
};

static int
on_message_begin(http_parser *p)
{
  struct HTTPParser *parser = (struct HTTPParser *)p->data;

  if ((parser->request = http_request_new(parser->logger)) == NULL)
    return -1;

  return 0;
}

static int
on_url(http_parser *p,
       const char  *at,
       size_t       length)
{
  struct HTTPParser *parser = NULL;
  struct http_parser_url url;
  int i = 0;

  parser = (struct HTTPParser *)p->data;

  if (http_parser_parse_url(at, length, 0, &url) != 0)
    return -1;

  http_request_set_url_range(parser->request, at - parser->buffer, length);

  while (i < HTTP_URL_FIELD_MAX) {
    if (url.field_set & (1 << i))
      http_request_set_url_field_range(parser->request, i, (at - parser->buffer) + url.field_data[i].off, url.field_data[i].len);
    i++;
  }

  return 0;
}

static int
on_header_field(http_parser *p,
                const char  *at,
                size_t       length)
{
  struct HTTPParser *parser = NULL;

  parser = (struct HTTPParser *)p->data;

  if (parser->current_header >= HTTP_REQUEST_MAX_HEADERS)
    return -1;

  http_request_set_header_key_range(parser->request, parser->current_header, at - parser->buffer, length);

  return 0;
}

static int
on_header_value(http_parser *p,
                const char  *at,
                size_t       length)
{
  struct HTTPParser *parser = NULL;

  parser = (struct HTTPParser *)p->data;

  http_request_set_header_value_range(parser->request, parser->current_header, at - parser->buffer, length);

  parser->current_header++;

  return 0;
}

static int
on_headers_complete(http_parser *p)
{
  struct HTTPParser *parser = NULL;

  parser = (struct HTTPParser *)p->data;

  http_request_set_method(parser->request, p->method);
  http_request_set_last(parser->request, http_should_keep_alive(p) == 0);

  if (http_request_set_headers_buffer(parser->request, parser->buffer, p->nread) < 0)
    return -1;

  parser->buffer_length -= (p->nread);

  if (parser->buffer_length == 0) {
    free(parser->buffer);
    parser->buffer = NULL;
  }
  else {
    memmove(parser->buffer, &(parser->buffer[p->nread]), parser->buffer_length);
    if ((parser->buffer = realloc(parser->buffer, parser->buffer_length)) == NULL) {
      LOGGER_PERROR(parser->logger, "realloc");
      return -1;
    }
  }

  if (((int64_t)p->content_length) > 0 && http_request_set_body_length(parser->request, p->content_length) < 0)
    return -1;

  return 0;
}

static int
on_body(http_parser *p,
        const char  *at,
        size_t       length)
{
  struct HTTPParser *parser = NULL;

  parser = (struct HTTPParser *)p->data;

  http_request_append_body_data(parser->request, at, length);

  return 0;
}

static int
on_message_complete(http_parser *p)
{
  struct HTTPParser *parser = NULL;

  parser = (struct HTTPParser *)p->data;

  parser->current_request++;
  parser->current_header = 0;

  if (parser->current_request == MAX_REQUESTS)
    http_request_set_last(parser->request, 1);

  if (parser->new_request_callback != NULL)
    parser->new_request_callback(parser, parser->data);

  parser->request = NULL;

  return 0;
}


struct HTTPParser *
http_parser_new(struct Logger *logger)
{
  struct HTTPParser *parser = NULL;

  assert(logger != NULL);

  if ((parser = memory_create(sizeof(struct HTTPParser))) == NULL) {
    LOGGER_PERROR(logger, "memory_create");
    return NULL;
  }

  http_parser_init(&(parser->http_parser), HTTP_REQUEST);
  parser->http_parser.data = parser;

  parser->http_parser_settings.on_message_begin = on_message_begin;
  parser->http_parser_settings.on_url = on_url;
  parser->http_parser_settings.on_header_field = on_header_field;
  parser->http_parser_settings.on_header_value = on_header_value;
  parser->http_parser_settings.on_headers_complete = on_headers_complete;
  parser->http_parser_settings.on_body = on_body;
  parser->http_parser_settings.on_message_complete = on_message_complete;

  parser->logger = logger;

  return parser;
}

void
http_parser_destroy(struct HTTPParser *parser)
{
  assert(parser != NULL);

  if (parser->buffer != NULL)
    memory_destroy(parser->buffer);

  memory_destroy(parser);
}

void
http_parser_set_new_request_callback(struct HTTPParser                  *parser,
                                            HTTPParserNewRequestCallback callback,
                                            void                        *data)
{
  assert(parser != NULL);
  assert(callback != NULL);

  parser->new_request_callback = callback;
  parser->data = data;
}

int
http_parser_append_data(struct HTTPParser *parser,
                        void              *data,
                        size_t             length)
{
  size_t offset = 0;

  assert(parser != NULL);
  assert(data != NULL);
  assert(length > 0);

  ssize_t parsed = -1;

  if ((parser->buffer = memory_resize(parser->buffer, parser->buffer_length + length)) == NULL) {
    LOGGER_PERROR(parser->logger, "realloc");
    return -1;
  }
  memcpy(&(parser->buffer[parser->buffer_length]), data, length);

  offset = parser->buffer_length;
  parser->buffer_length += length;

  parsed = http_parser_execute(&(parser->http_parser),
                               &(parser->http_parser_settings),
                               &(parser->buffer[offset]),
                               length);

  if (parsed != length) {
    logger_trace(parser->logger, LOG_ERROR, "httprequestqueue", "parser error: %s: %s",
                                                                 http_errno_name(parser->http_parser.http_errno),
                                                                 http_errno_description(parser->http_parser.http_errno));
    return -1;
  }

  return 0;
}

struct HTTPRequest *
http_parser_get_next_request(struct HTTPParser *parser)
{
  assert(parser != NULL);

  return parser->request;
}

/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */

