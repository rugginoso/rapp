#ifndef HTTPRESPONSEWRITER_H
#define HTTPRESPONSEWRITER_H

#include "rapp/rapp_httpresponse.h"

struct TcpConnection;

typedef void (*HTTPResponseWriterNotifyCallback)(struct HTTPResponseWriter *response_writer, void *data);


struct HTTPResponseWriter* http_response_writer_new(struct TcpConnection *tcp_connection, HTTPResponseWriterNotifyCallback headers_sent_callback, HTTPResponseWriterNotifyCallback body_sent_callback, void *data);
void http_response_writer_destroy(struct HTTPResponseWriter *response_writer);

#endif /* HTTTPRESPONSEWRITER_H */
