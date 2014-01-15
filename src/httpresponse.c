/*
 * httpresponse.c - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013-2014 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <limits.h>
#include <pthread.h>
#include <assert.h>

#include <sys/uio.h>

#include "httpresponse.h"
#include "tcpconnection.h"
#include "logger.h"
#include "memory.h"

/* code + space + message + NULL */
#define STATUSLINE_LEN 36

/* HTTP/1.1 */
#define PROTOCOL_LEN 8

/* %a, %d %b %Y %H:%M:%S %z */
#define DATETIME_LEN 32

struct HTTPResponse {
  struct iovec chunks[IOV_MAX]; /* FIXME: adjust value or use a circular buffer */
  int current_chunk;
  int wrote_chunks;

  int completed;

  const char *server_name;
  int is_last;

  pthread_mutex_t mutex;

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

  if ((response = memory_create(sizeof(struct HTTPResponse))) == NULL) {
    LOGGER_PERROR(logger, "memory_create");
    return NULL;
  }

  if (pthread_mutex_init(&(response->mutex), NULL) != 0) {
    LOGGER_PERROR(logger, "pthread_mutex_init");
    memory_destroy(response);
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

  pthread_mutex_destroy(&(response->mutex));

  memory_destroy(response);
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

  if (memory_asprintf(&header, "%s: %s" HTTP_EOL, key, value) < 0)
    return -1;

  ret = http_response_append_data(response, header, strlen(header));
  memory_destroy(header);

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
    logger_trace(response->logger, LOG_ERROR, "httpresponse", "time: %s", strerror(errno));
    return -1;
  }

  if (strftime(datetime, DATETIME_LEN, "%a, %d %b %Y %H:%M:%S %z", gmtime(&now)) == 0) {
    logger_trace(response->logger, LOG_ERROR, "httpresponse", "strftime: error formatting datetime");
    return -1;
  }

  if ((ret = http_response_write_header(response, "Date", datetime)) < 0)
    return -1;
  total_length += ret;

  if (response->is_last != 0) {
    if ((ret = http_response_write_header(response, "Connection", "Close")) < 0)
      return -1;
    total_length += ret;
  }

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

  pthread_mutex_lock(&(response->mutex));

  if ((response->chunks[response->current_chunk].iov_base = memory_create(length)) == NULL) {
    LOGGER_PERROR(response->logger, "memory_create");
    return -1;
  }
  memcpy(response->chunks[response->current_chunk].iov_base, data, length);
  response->chunks[response->current_chunk].iov_len = length;
  response->current_chunk++;

  pthread_mutex_unlock(&(response->mutex));

  return length;
}

void
http_response_set_last(struct HTTPResponse *response,
                       int                  last)
{
  assert(response != NULL);

  response->is_last = last;
}

int
http_response_is_last(struct HTTPResponse *response)
{
  assert(response != NULL);

  return response->is_last;
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

  if (memory_asprintf(&body, error_body, message, message) < 0) {
    LOGGER_PERROR(response->logger, "memory_asprintf: body");
    return -1;
  }

  if (memory_asprintf(&len_s, "%lu", strlen(body)) < 0) {
    LOGGER_PERROR(response->logger, "memory_asprintf: len_s");
    memory_destroy(body);
    return -1;
  }

  if ((ret = http_response_write_status_line_by_code(response, code)) < 0) {
    memory_destroy(body);
    memory_destroy(len_s);
    return -1;
  }
  total_length += ret;

  if ((ret = http_response_write_header(response, "Content-Type", "text/html")) < 0) {
    memory_destroy(body);
    memory_destroy(len_s);
    return -1;
  }
  total_length += ret;

  if ((ret = http_response_write_header(response, "Content-Length", len_s)) < 0) {
    memory_destroy(body);
    memory_destroy(len_s);
    return -1;
  }
  memory_destroy(len_s);
  total_length += ret;

  if ((ret = http_response_end_headers(response)) < 0) {
    memory_destroy(body);
    return -1;
  }
  total_length += ret;

  if ((ret = http_response_append_data(response, body, strlen(body))) < 0) {
    memory_destroy(body);
    return -1;
  }
  total_length += ret;

  memory_destroy(body);

  return total_length;
}

void
http_response_end_body(struct HTTPResponse *response)
{
  assert(response != NULL);

  response->completed = 1;
}

int
http_response_is_complete(struct HTTPResponse *response)
{
  assert(response != NULL);

  return response->completed;
}

int
http_response_send(struct HTTPResponse  *response,
                   struct TcpConnection *connection)
{
  int chunk = 0;
  int chunks_to_send = 0;
  ssize_t sent = -1;
  ssize_t remain = -1;
  ssize_t offset = -1;

  assert(response != NULL);
  assert(connection != NULL);

  pthread_mutex_lock(&(response->mutex));

  chunks_to_send = response->current_chunk - response->wrote_chunks;

  if (chunks_to_send == 0 && response->completed == 0) {
    pthread_mutex_unlock(&(response->mutex));
    return -1;
  }

  if ((sent = tcp_connection_writev(connection, &(response->chunks[response->wrote_chunks]), chunks_to_send)) < 0) {
    pthread_mutex_unlock(&(response->mutex));
    return errno == EAGAIN;
  }

  for (chunk = response->wrote_chunks; chunk <= response->current_chunk; chunk++) {
    sent -= response->chunks[chunk].iov_len;
    if (sent < 0) {
        remain = sent * -1;
        offset = response->chunks[chunk].iov_len - remain;

        response->chunks[chunk].iov_len = remain;
        memmove(response->chunks[chunk].iov_base, &(((char *)response->chunks[chunk].iov_base)[offset]), remain);
        response->wrote_chunks = chunk;
        break;
    }
    else if (sent == 0) {
      memory_destroy(response->chunks[chunk].iov_base);
      response->wrote_chunks = chunk + 1;
      break;
    }
    else {
      memory_destroy(response->chunks[chunk].iov_base);
    }
  }

  pthread_mutex_unlock(&(response->mutex));

  return 0;
}

/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */
