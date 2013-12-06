#ifndef RAPP_HTTPRESPONSE_H
#define RAPP_HTTPRESPONSE_H

#define HTTP_EOL "\r\n"

struct HTTPResponseWriter;

void http_response_writer_notify_headers_sent(struct HTTPResponseWriter *response_writer);
void http_response_writer_notify_body_sent(struct HTTPResponseWriter *response_writer);

ssize_t http_response_writer_printf(struct HTTPResponseWriter *response_writer, const char *fmt, ...);
ssize_t http_response_writer_write_data(struct HTTPResponseWriter *response_writer, const void *data, size_t length);
ssize_t http_response_writer_sendfile(struct HTTPResponseWriter *response_writer, const char *path);

#endif /* RAPP_HTTTPRESPONSE_H */
