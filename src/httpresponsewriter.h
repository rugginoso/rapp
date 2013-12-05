#ifndef HTTPRESPONSEWRITER_H
#define HTTPRESPONSEWRITER_H

struct TcpConnection;
struct HTTPResponseWriter;

typedef void (*HTTPResponseWriterNotifyCallback)(struct HTTPResponseWriter *response_writer, void *data);


struct HTTPResponseWriter* http_response_writer_new(struct TcpConnection *tcp_connection, HTTPResponseWriterNotifyCallback headers_sent_callback, HTTPResponseWriterNotifyCallback body_sent_callback, void *data);
void http_response_writer_destroy(struct HTTPResponseWriter *response_writer);

void http_response_writer_notify_headers_sent(struct HTTPResponseWriter *response_writer);
void http_response_writer_notify_body_sent(struct HTTPResponseWriter *response_writer);

ssize_t http_response_writer_write_data(struct HTTPResponseWriter *response_writer, const void *data, size_t length);
ssize_t http_response_writer_sendfile(struct HTTPResponseWriter *response_writer, const char *path);

#endif /* HTTTPRESPONSEWRITER_H */
