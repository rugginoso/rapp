/*
 * rapp_httprequest.h - is part of RApp.
 * RApp is a modular web application container made for linux and for speed.
 * (C) 2013 the RApp devs. Licensed under GPLv2 with additional rights.
 *     see LICENSE for all the details.
 */

#ifndef RAPP_HTTPREQUEST_H
#define RAPP_HTTPREQUEST_H

/*
 * Same limit as apache:
 * http://httpd.apache.org/docs/current/mod/core.html#limitrequestfields
 */
#define HTTP_REQUEST_MAX_HEADERS 100

#define EXTRACT_MEMORY_RANGE(dest, buffer, range)       \
do {                                                    \
  dest = alloca(range.length + 1);                      \
  memcpy(dest, &(buffer[range.offset]), range.length);  \
  dest[range.length] = 0;                               \
} while(0)

struct MemoryRange {
  size_t offset;
  size_t length;
};

struct HeaderMemoryRange {
  struct MemoryRange key;
  struct MemoryRange value;
};

enum HTTPMethod {
  HTTP_METHOD_DELETE,
  HTTP_METHOD_GET,
  HTTP_METHOD_HEAD,
  HTTP_METHOD_POST,
  HTTP_METHOD_PUT,
  HTTP_METHOD_CONNECT,
  HTTP_METHOD_OPTIONS,
  HTTP_METHOD_TRACE,
  HTTP_METHOD_COPY,
  HTTP_METHOD_LOCK,
  HTTP_METHOD_MKCOL,
  HTTP_METHOD_MOVE,
  HTTP_METHOD_PROPFIND,
  HTTP_METHOD_PROPPATCH,
  HTTP_METHOD_SEARCH,
  HTTP_METHOD_UNLOCK,
  HTTP_METHOD_REPORT,
  HTTP_METHOD_MKACTIVITY,
  HTTP_METHOD_CHECKOUT,
  HTTP_METHOD_MERGE,
  HTTP_METHOD_MSEARCH,
  HTTP_METHOD_NOTIFY,
  HTTP_METHOD_SUBSCRIBE,
  HTTP_METHOD_UNSUBSCRIBE,
  HTTP_METHOD_PATCH,
  HTTP_METHOD_PURGE,
  HTTP_METHOD_MAX
};

struct HTTPRequest;

const char *http_request_get_buffer(struct HTTPRequest *request);

enum HTTPMethod http_request_get_method(struct HTTPRequest *request);

void http_request_get_url_range(struct HTTPRequest *request, struct MemoryRange *range);

int http_request_get_header_value_range(struct HTTPRequest *request, const char *header_name, struct MemoryRange *range);
void http_request_get_headers_ranges(struct HTTPRequest *request, struct HeaderMemoryRange **ranges, unsigned *n_ranges);

void http_request_get_body_range(struct HTTPRequest *request, struct MemoryRange *range);

#endif /* RAPP_HTTPREQUEST_H */
/*
 * vim: expandtab shiftwidth=2 tabstop=2:
 */

