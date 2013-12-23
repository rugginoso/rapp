/*
 * httpresponse.c - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <assert.h>

#include "logger.h"
#include "httpresponse.h"

/* code + space + message + NULL */
#define STATUSLINE_LEN 36

/* HTTP/1.1 */
#define PROTOCOL_LEN 8

/* %a, %d %b %Y %H:%M:%S %z */
#define DATETIME_LEN 32

struct HTTPResponse {
  char *buffer;
  size_t buffer_length;

  const char *server_name;

  struct Logger *logger;
};

struct HTTPStatus {
  unsigned code;
  const char *message;
};

/* http://www.iana.org/assignments/http-status-codes/http-status-codes.xhtml */
static const struct HTTPStatus http_statuses[] = {
  {100, "Continue"},
  {101, "Switching Protocols"},
  {102, "Processing"},
  {200, "OK"},
  {201, "Created"},
  {202, "Accepted"},
  {203, "Non-Authoritative Information"},
  {204, "No Content"},
  {205, "Reset Content"},
  {206, "Partial Content"},
  {207, "Multi-Status"},
  {208, "Already Reported"},
  {226, "IM Used"},
  {300, "Multiple Choices"},
  {301, "Moved Permanently"},
  {302, "Found"},
  {303, "See Other"},
  {304, "Not Modified"},
  {305, "Use Proxy"},
  {306, "Reserved"},
  {307, "Temporary Redirect"},
  {308, "Permanent Redirect"},
  {400, "Bad Request"},
  {401, "Unauthorized"},
  {402, "Payment Required"},
  {403, "Forbidden"},
  {404, "Not Found"},
  {405, "Method Not Allowed"},
  {406, "Not Acceptable"},
  {407, "Proxy Authentication Required"},
  {408, "Request Timeout"},
  {409, "Conflict"},
  {410, "Gone"},
  {411, "Length Required"},
  {412, "Precondition Failed"},
  {413, "Request Entity Too Large"},
  {414, "Request-URI Too Long"},
  {415, "Unsupported Media Type"},
  {416, "Requested Range Not Satisfiable"},
  {417, "Expectation Failed"},
  {422, "Unprocessable Entity"},
  {423, "Locked"},
  {424, "Failed Dependency"},
  {426, "Upgrade Required"},
  {428, "Precondition Required"},
  {429, "Too Many Requests"},
  {431, "Request Header Fields Too Large"},
  {500, "Internal Server Error"},
  {501, "Not Implemented"},
  {502, "Bad Gateway"},
  {503, "Service Unavailable"},
  {504, "Gateway Timeout"},
  {505, "HTTP Version Not Supported"},
  {506, "Variant Also Negotiates"},
  {507, "Insufficient Storage"},
  {508, "Loop Detected"},
  {510, "Not Extended"},
  {511, "Network Authentication Required"},
  {0, NULL}
};

static const char *error_body = \
"<!DOCTYPE html>"       \
"<html>"                \
  "<head>"              \
    "<title>%s</title>" \
  "</head>"             \
  "<body>"              \
    "<h1>%s</h1>"       \
  "</body>"             \
"</html>";


struct HTTPResponse*
http_response_new(struct Logger *logger, const char *server_name)
{
  struct HTTPResponse *response = NULL;

  assert(logger != NULL);
  assert(server_name != NULL);

  if ((response = calloc(1, sizeof(struct HTTPResponse))) == NULL) {
    logger_trace(logger, LOG_ERROR, "httpresponse", "calloc: %s", strerror(errno));
    return NULL;
  }

  response->logger = logger;
  response->server_name = server_name;

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

static const char *
status_message_by_code(unsigned code)
{
  const struct HTTPStatus *sp = NULL;

  for (sp = http_statuses; sp->code != 0; sp++) {
    if (sp->code == code)
      return sp->message;
  }

  return NULL;
}

ssize_t http_response_write_status_line_by_code(struct HTTPResponse *response,
                                                unsigned             code)
{
  const char *message = NULL;
  char *status_line = alloca(STATUSLINE_LEN);

  if ((message = status_message_by_code(code)) == NULL)
    return -1;

  snprintf(status_line, STATUSLINE_LEN, "%d %s", code, message);

  return http_response_write_status_line(response, status_line);
}

ssize_t http_response_write_status_line(struct HTTPResponse *response,
                                        const char          *status_line)
{
  /* HTTP/1.1 + space + status_line + HTTP_EOL + NULL */
  size_t status_len = PROTOCOL_LEN + 1 + strlen(status_line) + strlen(HTTP_EOL) + 1;
  char *status = alloca(status_len);

  snprintf(status, status_len, "HTTP/1.1 %s" HTTP_EOL, status_line);

  return http_response_append_data(response, status, status_len - 1);
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

ssize_t
http_response_end_headers(struct HTTPResponse *response)
{
  char *datetime = alloca(DATETIME_LEN);
  time_t now = 0;
  ssize_t total_length = 0;
  ssize_t ret = 0;

  assert(response != NULL);

  if ((ret = http_response_write_header(response, "Server", response->server_name)) < 0)
    return -1;
  total_length += ret;

  if (time(&now) < 0) {
    logger_trace(response->logger, LOG_ERROR, "httpresponse", "time: %s", error(errno));
    return -1;
  }

  if (strftime(datetime, DATETIME_LEN, "%a, %d %b %Y %H:%M:%S %z", gmtime(&now)) == 0) {
    logger_trace(response->logger, LOG_ERROR, "httpresponse", "strftime: error formatting datetime");
    return -1;
  }

  if ((ret = http_response_write_header(response, "Date", datetime)) < 0)
    return -1;
  total_length += ret;

  if ((ret = http_response_append_data(response, HTTP_EOL, strlen(HTTP_EOL))) < 0)
    return -1;
  total_length += ret;

  return total_length;
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
    logger_trace(response->logger, LOG_ERROR, "httpresponse", "realloc: %s", strerror(errno));
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
      logger_trace(response->logger, LOG_ERROR, "httpresponse", "realloc: %s", strerror(errno));
      return -1;
    }
  }

  return avaiable_length;
}

ssize_t
http_response_write_error_by_code(struct HTTPResponse *response,
                                  unsigned             code)
{
  const char *message = NULL;
  char *body = NULL;
  char *len_s = NULL;
  ssize_t total_length = 0;
  ssize_t ret;

  assert(response != NULL);

  if ((message = status_message_by_code(code)) == NULL)
    return -1;

  if (asprintf(&body, error_body, message, message) < 0) {
    logger_trace(response->logger, LOG_ERROR, "httpresponse", "asprintf: %s", strerror(errno));
    return -1;
  }

  if (asprintf(&len_s, "%d", strlen(body)) < 0) {
    logger_trace(response->logger, LOG_ERROR, "httpresponse", "asprintf: %s", strerror(errno));
    free(body);
    return -1;
  }

  if ((ret = http_response_write_status_line_by_code(response, code)) < 0) {
    free(body);
    free(len_s);
    return -1;
  }
  total_length += ret;

  if ((ret = http_response_write_header(response, "Content-Type", "text/html")) < 0) {
    free(body);
    free(len_s);
    return -1;
  }
  total_length += ret;

  if ((ret = http_response_write_header(response, "Content-Length", len_s)) < 0) {
    free(body);
    free(len_s);
    return -1;
  }
  free(len_s);
  total_length += ret;

  if ((ret = http_response_end_headers(response)) < 0) {
    free(body);
    return -1;
  }
  total_length += ret;

  if (ret = http_response_append_data(response, body, strlen(body)) < 0) {
    free(body);
    return -1;
  }
  total_length += ret;

  free(body);

  return total_length;
}

/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */
