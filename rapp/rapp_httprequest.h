#ifndef RAPP_HTTPREQUEST_H
#define RAPP_HTTPREQUEST_H

#define HTTP_REQUEST_MAX_HEADERS 1024 // FIXME: choose an appropriate size

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

struct HTTPRequest;

const char *http_request_get_buffer(struct HTTPRequest *request);

enum http_method http_request_get_method(struct HTTPRequest *request);

void http_request_get_url_range(struct HTTPRequest *request, struct MemoryRange *range);

int http_request_get_header_value_range(struct HTTPRequest *request, const char *header_name, struct MemoryRange *range);
void http_request_get_headers_ranges(struct HTTPRequest *request, struct HeaderMemoryRange **ranges, unsigned *n_ranges);

void http_request_get_body_range(struct HTTPRequest *request, struct MemoryRange *range);

#endif /* RAPP_HTTPREQUEST_H */
