#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H

#include "rapp/rapp_httpresponse.h"

struct TcpConnection;

typedef void (*HTTPResponseNotifyCallback)(struct HTTPResponse *response, void *data);


struct HTTPResponse* http_response_new(struct TcpConnection *tcp_connection, HTTPResponseNotifyCallback headers_sent_callback, HTTPResponseNotifyCallback body_sent_callback, void *data);
void http_response_destroy(struct HTTPResponse *response);

#endif /* HTTTPRESPONSE_H */
