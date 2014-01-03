/*
 * httprequest.c - is part of RApp.
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
#include "logger.h"
#include "memory.h"


struct HTTPRequest {
  enum HTTPMethod method;

  struct MemoryRange url_range;
  struct MemoryRange url_field_ranges[HTTP_URL_FIELD_MAX];
  struct HeaderMemoryRange headers_ranges[HTTP_REQUEST_MAX_HEADERS];
  unsigned current_header;

  char *headers_buffer;

  char *body;
  size_t body_length;

  int is_last;

  struct Logger *logger;
};

struct HTTPRequest *
http_request_new(struct Logger *logger)
{
  struct HTTPRequest *request = NULL;

  if ((request = memory_create(sizeof(struct HTTPRequest))) == NULL) {
    LOGGER_PERROR(logger, "memory_create");
    return NULL;
  }

  request->logger = logger;

  return request;
}

void
http_request_destroy(struct HTTPRequest *request)
{
  assert(request != NULL);

  if (request->headers_buffer != NULL)
    memory_destroy(request->headers_buffer);

  if (request->body != NULL)
    memory_destroy(request->body);

  memory_destroy(request);
}

int
http_request_set_headers_buffer(struct HTTPRequest *request,
                                const char         *buffer,
                                size_t              length)
{
  assert(request != NULL);
  assert(buffer != NULL);
  assert(length > 0);

  if ((request->headers_buffer = memory_create(length)) == NULL) {
    LOGGER_PERROR(request->logger, "memory_create");
    return -1;
  }
  memcpy(request->headers_buffer, buffer, length);

  return 0;
}

const char *
http_request_get_headers_buffer(struct HTTPRequest *request)
{
  assert(request != NULL);

  return request->headers_buffer;
}

void
http_request_set_method(struct HTTPRequest *request,
                        enum HTTPMethod     method)
{
  assert(request != NULL);

  request->method = method;
}

enum HTTPMethod
http_request_get_method(struct HTTPRequest *request)
{
  assert(request != NULL);

  return request->method;
}

void
http_request_set_url_range(struct HTTPRequest *request,
                           size_t              offset,
                           size_t              length)
{
  assert(request != NULL);

  request->url_range.offset = offset;
  request->url_range.length = length;
}

void
http_request_get_url_range(struct HTTPRequest *request,
                           struct MemoryRange *range)
{
  assert(request != NULL);
  assert(range != NULL);

  *range = request->url_range;
}

void
http_request_set_url_field_range(struct HTTPRequest *request,
                                 enum HTTPURLField   field,
                                 size_t              offset,
                                 size_t              length)
{
  assert(request != NULL);

  request->url_field_ranges[field].offset = offset;
  request->url_field_ranges[field].length = length;
}

int
http_request_get_url_field_range(struct HTTPRequest *request,
                                 enum HTTPURLField   field,
                                 struct MemoryRange *range)
{
  assert(request != NULL);
  assert(range != NULL);

  if (request->url_field_ranges[field].length == 0)
    return -1;

  *range = request->url_field_ranges[field];

  return 0;
}

void
http_request_set_header_key_range(struct HTTPRequest *request,
                                  unsigned            header_index,
                                  size_t              offset,
                                  size_t              length)
{
  assert(request != NULL);

  request->headers_ranges[header_index].key.offset = offset;
  request->headers_ranges[header_index].key.length = length;
}

void
http_request_set_header_value_range(struct HTTPRequest *request,
                                    unsigned            header_index,
                                    size_t              offset,
                                    size_t              length)
{
  assert(request != NULL);

  request->headers_ranges[header_index].value.offset = offset;
  request->headers_ranges[header_index].value.length = length;

  request->current_header++;
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

    memcpy(current_header_name, &(request->headers_buffer[request->headers_ranges[i].key.offset]), current_header_name_length);
    current_header_name[current_header_name_length] = 0;

    if (strcasecmp(header_name, current_header_name) == 0) {
      *range = request->headers_ranges[i].value;
      return 0;
    }

    i++;
  }

  return -1;
}

void
http_request_get_headers_ranges(struct HTTPRequest        *request,
                                struct HeaderMemoryRange **ranges,
                                unsigned                  *n_ranges)
{
  assert(request != NULL);
  assert(ranges != NULL);
  assert(n_ranges != NULL);

  *ranges = request->headers_ranges;
  *n_ranges = request->current_header;
}

const char *
http_request_get_body(struct HTTPRequest *request)
{
  assert(request != NULL);

  return request->body;
}

int
http_request_set_body_length(struct HTTPRequest *request,
                             size_t              length)
{
  assert(request != NULL);
  assert(length > 0);

  if ((request->body = memory_create(length)) == NULL) {
    LOGGER_PERROR(request->logger, "memory_create");
    return -1;
  }

  return 0;
}

void
http_request_append_body_data(struct HTTPRequest *request,
                              const char         *data,
                              size_t              length)
{
    assert(request != NULL);
    assert(data != NULL);
    assert(length > 0);

    memcpy(&(request->body[request->body_length]), data, length);
    request->body_length += length;
}

void
http_request_set_last(struct HTTPRequest *request,
                      int                 last)
{
  assert(request != NULL);

  request->is_last = last;
}

int
http_request_is_last(struct HTTPRequest *request)
{
  assert(request != NULL);

  return request->is_last;
}

struct HTTPRequest *
http_request_new_fake_url(struct Logger *logger,
                          const char    *url)
{
  struct HTTPRequest *request = NULL;
  char *request_url = NULL;
  size_t request_len = 0;

  assert(url);

  request_len = strlen(url);
  if ((request_url = memory_strdup(url)) == NULL) {
    LOGGER_PERROR(logger, "memory_strdup");
    return NULL;
  }

  if ((request = http_request_new(logger)) == NULL) {
    memory_destroy(request_url);
    return NULL;
  }

  request->headers_buffer = request_url;
  request->url_range.offset = 0;
  request->url_range.length = request_len;
  return request;
}

/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */

