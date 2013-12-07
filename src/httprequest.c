#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <http_parser.h>

#include "httprequest.h"


struct HTTPRequest {
  http_parser parser;
  http_parser_settings parser_settings;
  unsigned current_header;

  struct MemoryRange url_range;
  struct HeaderMemoryRange headers_ranges[HTTP_REQUEST_MAX_HEADERS];
  struct MemoryRange body_range;

  char *buffer;
  size_t buffer_length;

  HTTPRequestParseFinishCallback parse_finish_callback;
  void *data;
};

static int
on_url(http_parser *parser, const char *at, size_t length)
{
  struct HTTPRequest *request = NULL;

  request = (struct HTTPRequest *)parser->data;

  request->url_range.offset = at - request->buffer;
  request->url_range.length = length;

  return 0;
}

static int
on_header_field(http_parser *parser, const char *at, size_t length)
{
  struct HTTPRequest *request = NULL;

  request = (struct HTTPRequest *)parser->data;

  if (request->current_header >= HTTP_REQUEST_MAX_HEADERS)
    return -1;

  request->headers_ranges[request->current_header].key.offset = at - request->buffer;
  request->headers_ranges[request->current_header].key.length = length;

  return 0;
}

static int
on_header_value(http_parser *parser, const char *at, size_t length)
{
  struct HTTPRequest *request = NULL;

  request = (struct HTTPRequest *)parser->data;

  request->headers_ranges[request->current_header].value.offset = at - request->buffer;
  request->headers_ranges[request->current_header].value.length = length;

  request->current_header++;

  return 0;
}

static int
on_body(http_parser *parser, const char *at, size_t length)
{
  struct HTTPRequest *request = NULL;

  request = (struct HTTPRequest *)parser->data;

  request->body_range.offset = at - request->buffer;
  request->body_range.length = length;

  return 0;
}

static int
on_message_complete(http_parser *parser)
{
  struct HTTPRequest *request = NULL;

  request = (struct HTTPRequest *)parser->data;

  request->parse_finish_callback(request, request->data);

  return 0;
}

struct HTTPRequest *
http_request_new(void)
{
  struct HTTPRequest *request = NULL;

  if ((request = calloc(sizeof(struct HTTPRequest), 1)) == NULL) {
    perror("calloc");
    return NULL;
  }

  http_parser_init(&(request->parser), HTTP_REQUEST);
  request->parser.data = request;

  request->parser_settings.on_url = on_url;
  request->parser_settings.on_header_field = on_header_field;
  request->parser_settings.on_header_value = on_header_value;
  request->parser_settings.on_body = on_body;
  request->parser_settings.on_message_complete = on_message_complete;

  return request;
}

void
http_request_destroy(struct HTTPRequest *request)
{
  assert(request != NULL);

  if (request->buffer && request->buffer_length > 0)
    free(request->buffer);

  free(request);
}

void
http_request_set_parse_finish_callback(struct HTTPRequest            *request,
                                       HTTPRequestParseFinishCallback callback,
                                       void                          *data)
{
  assert(request != NULL);

  request->parse_finish_callback = callback;
  request->data = data;
}

int
http_request_append_data(struct HTTPRequest *request,
                         void               *data,
                         size_t              length)
{
  assert(request != NULL);
  assert(data != NULL);
  assert(length > 0);

  ssize_t parsed = -1;

  if ((request->buffer = realloc(request->buffer, request->buffer_length + length)) == NULL) {
    perror("realloc");
    return -1;
  }
  memcpy(&(request->buffer[request->buffer_length]), data, length);

  parsed = http_parser_execute(&(request->parser),
                               &(request->parser_settings),
                               &(request->buffer[request->buffer_length]),
                               length);

  request->buffer_length += length;

  if (parsed != length)
    return -1;

  return 0;
}

const char *
http_request_get_buffer(struct HTTPRequest *request)
{
  assert(request != NULL);

  return request->buffer;
}

void
http_request_get_url_range(struct HTTPRequest *request,
                           struct MemoryRange *range)
{
  assert(request != 0);
  assert(range != 0);

  *range = request->url_range;
}

int
http_request_get_header_value_range(struct HTTPRequest *request,
                                    const char         *header_name,
                                    struct MemoryRange *range)
{
  int i = 0;
  char *current_header_name = NULL;
  size_t current_header_name_length = 0;

  assert(request != NULL);
  assert(header_name != NULL);
  assert(range != NULL);

  while (i < request->current_header) {
    current_header_name_length = request->headers_ranges[i].key.length;

    current_header_name = alloca(current_header_name_length + 1);

    memcpy(current_header_name, &(request->buffer[request->headers_ranges[i].key.offset]), current_header_name_length);
    current_header_name[current_header_name_length] = 0;

    if (strcasecmp(header_name, current_header_name) == 0) {
      *range = request->headers_ranges[i].value;
      return 0;
    }
  }

  return -1;
}

void
http_request_get_headers_ranges(struct HTTPRequest *request,
                                struct HeaderMemoryRange **ranges,
                                unsigned *n_ranges)
{
  assert(request != NULL);
  assert(ranges != NULL);
  assert(n_ranges != NULL);

  *ranges = request->headers_ranges;
  *n_ranges = request->current_header;
}

void
http_request_get_body_range(struct HTTPRequest *request,
                            struct MemoryRange *range)
{
  assert(request != NULL);
  assert(range != NULL);

  *range = request->body_range;
}
