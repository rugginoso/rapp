/*
 * httpresponse.c - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "httpresponse.h"


struct HTTPResponse {
  char *buffer;
  size_t buffer_length;
};

struct HTTPResponse*
http_response_new(void)
{
  struct HTTPResponse *response = NULL;

  if ((response = calloc(1, sizeof(struct HTTPResponse))) == NULL) {
    perror("calloc");
    return NULL;
  }

  return response;
}

void
http_response_destroy(struct HTTPResponse *response)
{
  assert(response != NULL);

  if (response->buffer != NULL && response->buffer_length != 0)
    free(response->buffer);

  free(response);
}

ssize_t
http_response_write_header(struct HTTPResponse *response,
                           const char           *key,
                           const char           *value)
{
  char *header = NULL;
  ssize_t ret = 0;

  assert(response != NULL);
  assert(key != NULL);
  assert(value != NULL);

  if (asprintf(&header, "%s: %s" HTTP_EOL, key, value) < 0)
    return -1;

  ret = http_response_append_data(response, header, strlen(header));
  free(header);

  return ret;
}

void
http_response_end_headers(struct HTTPResponse *response)
{
  http_response_append_data(response, HTTP_EOL, strlen(HTTP_EOL));
}

ssize_t
http_response_append_data(struct HTTPResponse *response,
                          const void          *data,
                          size_t               length)
{
  assert(response != NULL);
  assert(data != NULL);
  assert(length > 0);

  if ((response->buffer = realloc(response->buffer, response->buffer_length + length)) == NULL) {
    perror("realloc");
    return -1;
  }

  memcpy(&(response->buffer[response->buffer_length]), data, length);

  response->buffer_length += length;

  return length;
}

ssize_t
http_response_read_data(struct HTTPResponse *response,
                        void                *data,
                        size_t               length)
{
  ssize_t avaiable_length = 0;

  assert(response != NULL);
  assert(data != NULL);
  assert(length > 0);

  avaiable_length = response->buffer_length < length ? response->buffer_length : length;

  memcpy(data, response->buffer, avaiable_length);

  response->buffer_length -= avaiable_length;

  if (response->buffer_length == 0) {
    free(response->buffer);
    response->buffer = NULL;
  }
  else {
    memmove(response->buffer, &(response->buffer[avaiable_length]), response->buffer_length);
    if ((response->buffer = realloc(response->buffer, response->buffer_length)) == NULL) {
      perror("realloc");
      return -1;
    }
  }

  return avaiable_length;
}

/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */
