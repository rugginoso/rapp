/*
 * httprequestqueue.c - is part of RApp.
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
#include "httprequestqueue.h"

#define MAX_REQUESTS 100

struct HTTPRequestQueue {
  http_parser parser;
  http_parser_settings parser_settings;
  unsigned current_header;

  struct HTTPRequest *requests[MAX_REQUESTS];
  size_t incoming_index;
  size_t outgoing_index;

  char *buffer;
  size_t buffer_length;

  HTTPRequestQueueNewRequestCallback new_request_callback;
  void *data;

  struct Logger *logger;
};

static int
on_message_begin(http_parser *parser)
{
  struct HTTPRequestQueue *queue = (struct HTTPRequestQueue *)parser->data;

  if ((queue->requests[queue->incoming_index] = http_request_new(queue->logger)) == NULL)
    return -1;

  return 0;
}

static int
on_url(http_parser *parser,
       const char  *at,
       size_t       length)
{
  struct HTTPRequestQueue *queue = NULL;
  struct HTTPRequest *request = NULL;
  struct http_parser_url url;
  int i = 0;

  queue = (struct HTTPRequestQueue *)parser->data;
  request = queue->requests[queue->incoming_index];

  if (http_parser_parse_url(at, length, 0, &url) != 0)
    return -1;

  http_request_set_url_range(request, at - queue->buffer, length);

  while (i < HTTP_URL_FIELD_MAX) {
    if (url.field_set & (1 << i))
      http_request_set_url_field_range(request, i, (at - queue->buffer) + url.field_data[i].off, url.field_data[i].len);
    i++;
  }

  return 0;
}

static int
on_header_field(http_parser *parser,
                const char  *at,
                size_t       length)
{
  struct HTTPRequestQueue *queue = NULL;
  struct HTTPRequest *request = NULL;

  queue = (struct HTTPRequestQueue *)parser->data;
  request = queue->requests[queue->incoming_index];

  if (queue->current_header >= HTTP_REQUEST_MAX_HEADERS)
    return -1;

  http_request_set_header_key_range(request, queue->current_header, at - queue->buffer, length);

  return 0;
}

static int
on_header_value(http_parser *parser,
                const char  *at,
                size_t       length)
{
  struct HTTPRequestQueue *queue = NULL;
  struct HTTPRequest *request = NULL;

  queue = (struct HTTPRequestQueue *)parser->data;
  request = queue->requests[queue->incoming_index];

  http_request_set_header_value_range(request, queue->current_header, at - queue->buffer, length);

  queue->current_header++;

  return 0;
}

static int
on_headers_complete(http_parser *parser)
{
  struct HTTPRequestQueue *queue = NULL;
  struct HTTPRequest *request = NULL;

  queue = (struct HTTPRequestQueue *)parser->data;
  request = queue->requests[queue->incoming_index];

  http_request_set_method(request, parser->method);
  http_request_set_last(request, http_should_keep_alive(parser) == 0);

  if (http_request_set_headers_buffer(request, queue->buffer, parser->nread) < 0)
    return -1;

  queue->buffer_length -= (parser->nread);

  if (queue->buffer_length == 0) {
    free(queue->buffer);
    queue->buffer = NULL;
  }
  else {
    memmove(queue->buffer, &(queue->buffer[parser->nread]), queue->buffer_length);
    if ((queue->buffer = realloc(queue->buffer, queue->buffer_length)) == NULL) {
      LOGGER_PERROR(queue->logger, "realloc");
      return -1;
    }
  }

  if (((int64_t)parser->content_length) > 0 && http_request_set_body_length(request, parser->content_length) < 0)
    return -1;

  return 0;
}

static int
on_body(http_parser *parser,
        const char  *at,
        size_t       length)
{
  struct HTTPRequestQueue *queue = NULL;
  struct HTTPRequest *request = NULL;

  queue = (struct HTTPRequestQueue *)parser->data;
  request = queue->requests[queue->incoming_index];

  http_request_append_body_data(request, at, length);

  return 0;
}

static int
on_message_complete(http_parser *parser)
{
  struct HTTPRequestQueue *queue = NULL;

  queue = (struct HTTPRequestQueue *)parser->data;

  queue->incoming_index++;
  queue->current_header = 0;

  if (queue->incoming_index == MAX_REQUESTS)
    http_request_set_last(queue->requests[queue->incoming_index - 1], 1);

  if (queue->new_request_callback != NULL)
    queue->new_request_callback(queue, queue->data);

  return 0;
}


struct HTTPRequestQueue *
http_request_queue_new(struct Logger *logger)
{
  struct HTTPRequestQueue *queue = NULL;

  assert(logger != NULL);

  if ((queue = memory_create(sizeof(struct HTTPRequestQueue))) == NULL) {
    LOGGER_PERROR(logger, "memory_create");
    return NULL;
  }

  http_parser_init(&(queue->parser), HTTP_REQUEST);
  queue->parser.data = queue;

  queue->parser_settings.on_message_begin = on_message_begin;
  queue->parser_settings.on_url = on_url;
  queue->parser_settings.on_header_field = on_header_field;
  queue->parser_settings.on_header_value = on_header_value;
  queue->parser_settings.on_headers_complete = on_headers_complete;
  queue->parser_settings.on_body = on_body;
  queue->parser_settings.on_message_complete = on_message_complete;

  queue->logger = logger;

  return queue;
}

void
http_request_queue_destroy(struct HTTPRequestQueue *queue)
{
  assert(queue != NULL);

  if (queue->buffer != NULL)
    memory_destroy(queue->buffer);

  memory_destroy(queue);
}

void
http_request_queue_set_new_request_callback(struct HTTPRequestQueue           *queue,
                                            HTTPRequestQueueNewRequestCallback callback,
                                            void                              *data)
{
  assert(queue != NULL);
  assert(callback != NULL);

  queue->new_request_callback = callback;
  queue->data = data;
}

int
http_request_queue_append_data(struct HTTPRequestQueue *queue,
                               void                    *data,
                               size_t                   length)
{
  size_t offset = 0;

  assert(queue != NULL);
  assert(data != NULL);
  assert(length > 0);

  ssize_t parsed = -1;

  if ((queue->buffer = memory_resize(queue->buffer, queue->buffer_length + length)) == NULL) {
    LOGGER_PERROR(queue->logger, "realloc");
    return -1;
  }
  memcpy(&(queue->buffer[queue->buffer_length]), data, length);

  offset = queue->buffer_length;
  queue->buffer_length += length;

  parsed = http_parser_execute(&(queue->parser),
                               &(queue->parser_settings),
                               &(queue->buffer[offset]),
                               length);

  if (parsed != length) {
    logger_trace(queue->logger, LOG_ERROR, "httprequestqueue", "parser error: %s: %s",
                                                               http_errno_name(queue->parser.http_errno),
                                                               http_errno_description(queue->parser.http_errno));
    return -1;
  }

  return 0;
}

struct HTTPRequest *
http_request_queue_get_next_request(struct HTTPRequestQueue *queue)
{
  assert(queue != NULL);

  return queue->requests[queue->outgoing_index++];
}

/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */

